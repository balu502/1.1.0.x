#include "protocol.h"
#include "utility.h"
#include "algorithm.h"
#include "define_PCR.h"

#include "qwindowdefs_win.h"
#include "windows.h"


typedef void (__stdcall *TranslateDTR)(char *input, char** output);
typedef void (__stdcall *Init) (void* appl_handle, char *dir_forms, char *ver, int Lang, void *sr, void *si, void *r, void *sp, void *ss, void *sq);

//-----------------------------------------------------------------------------
//--- Parsing_ProgramAmplification
//-----------------------------------------------------------------------------
void Parsing_ProgramAmplification(rt_Protocol *prot)
{
    int i,j,k,m;
    QString text;
    QStringList fields;
    int count_PCR = 0;
    int count_MC = 0;
    QVector<int> vec_state;
    QVector<QPointF> vec_level;
    QPointF point;
    double time,value;
    int count_LC;
    QString time_levels = "";

    prot->dT_mc = 0.;
    prot->T_initial = 0.;

    prot->PrAmpl_value.clear();
    prot->PrAmpl_time.clear();
    prot->PrAmpl_color.clear();
    prot->PrAmpl_name = "";
    prot->PrAmpl_countLevelsCycles.clear();
    prot->PrAmpl_timesInBlocks.clear();
    prot->PrAmpl_minimumLevel = 100;

    enum {null,pcr,mc,hold}state;
    int state_block;

    QVector<string> prog = QVector<string>::fromStdVector(prot->program);

    state = null;
    state_block = 0;
    for(i=0; i<prog.size(); i++)
    {
        text = QString::fromStdString(prog.at(i));        
        fields = text.trimmed().split(QRegExp("\\s+"));
        if(fields.at(0) == "XLEV")
        {
            k = fields.at(6).toInt();
            switch(k)
            {
            case 1: state = pcr;    break;
            case 2: state = mc;
                    if(prot->T_initial == 0.)
                    {
                        prot->T_initial = fields.at(1).toDouble()/100.;
                        prot->dT_mc = fields.at(3).toDouble()/100.;
                    }
                    break;
            default:
                    state = null;   break;
            }

            state_block |= state;
            vec_state.push_back((int)state);

            value = (double)(fields.at(1).toInt())/100.;
            time = fields.at(2).toInt();
            point.setX(value);
            point.setY(time);
            vec_level.push_back(point);

            if(k == 1 || k == 2)
            {
                if(time < prot->PrAmpl_minimumLevel) prot->PrAmpl_minimumLevel = time;
            }

            if(!time_levels.isEmpty()) time_levels += " ";
            time_levels += QString::number((int)point.y());
        }
        if(fields.at(0) == "XCYC")
        {
            k = fields.at(1).toInt();            
            switch(state_block)
            {
            case 1:     count_PCR += k;   break;        // pcr
            case 2:     count_MC  += k;   break;        // mc
            default:                      break;
            }            

            for(j=0; j<k; j++)
            {
                for(m=0; m<vec_level.size(); m++)
                {
                    value = vec_level.at(m).x();
                    if(state == mc) value = prot->dT_mc*j + value;
                    prot->PrAmpl_value.push_back(value);
                    prot->PrAmpl_time.push_back(vec_level.at(m).y());
                    prot->PrAmpl_color.push_back(vec_state.at(m));
                }
            }

            count_LC = (short)k | ((short)vec_level.size()) << 16;
            prot->PrAmpl_countLevelsCycles.push_back(count_LC);
            prot->PrAmpl_timesInBlocks.push_back(time_levels.toStdString());
            time_levels = "";

            vec_level.clear();
            vec_state.clear();
            state = null;
            state_block = 0;
        }
        if(fields.at(0) == "XHLD")
        {
            if(vec_level.size() == 1)
            {
                prot->PrAmpl_value.push_back(vec_level.at(0).x());
                prot->PrAmpl_time.push_back(100);
                prot->PrAmpl_color.push_back(3);
                prot->PrAmpl_countLevelsCycles.push_back(0x10001);
                prot->PrAmpl_timesInBlocks.push_back("100");
            }
            vec_level.clear();
            vec_state.clear();
        }
        if(fields.at(0) == "XSAV" && fields.size() > 1)
        {
            text = fields.at(1);
            prot->PrAmpl_name = text.trimmed().toStdString();
        }
    }
    prot->count_PCR = count_PCR;
    prot->count_MC = count_MC;

    //qDebug() << "count_PCR_MC = " << count_PCR << count_MC;
}

//-----------------------------------------------------------------------------
//--- MakeElement
//-----------------------------------------------------------------------------
QDomElement MakeElement(QDomDocument &doc, QString name, QString text)
{
    QDomElement domElement = doc.createElement(name);
    if (!text.isEmpty())
    {
        QDomText domText = doc.createTextNode(text);
        domElement.appendChild(domText);
    }
    return(domElement);
}

//-----------------------------------------------------------------------------
//--- MakeElementResults
//-----------------------------------------------------------------------------
QDomElement MakeElementResults(QDomDocument &doc, QString name, QVector<string> *vec)
{
    int i;
    QString     text;
    QStringList list;

    QDomElement domElement = doc.createElement(name);

    for(i=0; i<vec->size(); i++)
    {
        text = QString::fromStdString(vec->at(i));
        if(text.isEmpty()) continue;
        list = text.split("=");

        QDomElement item = doc.createElement("item");
        if(list.count() < 2)
        {
            item.appendChild(MakeElement(doc,"value",text));
        }
        else
        {
            item.appendChild(MakeElement(doc,"name",list.at(0)));
            item.appendChild(MakeElement(doc,"value",list.at(1)));
        }
        domElement.appendChild(item);
    }

    return(domElement);
}

//-----------------------------------------------------------------------------
//--- SaveXML_Test
//-----------------------------------------------------------------------------
QDomElement SaveXML_Test(QDomDocument &doc, rt_Test *ptest)
{
    int i,j,k;
    int active_ch;
    bool ok;
    int r,g,b,key;
    QColor color;

    QString text;
    QDomElement res;
    QDomElement DTReport;
    QDomDocument doc_temp;

    rt_Preference   *preference;
    rt_TubeTest     *ptube;
    rt_ChannelTest  *pchannel;

    res = doc.createElement("item");

    //.........................................................................
    //qDebug() << "Save test: " << QString::fromStdString(ptest->header.Name_Test);

    if(ptest->preference_Test.size())
    {
       for(i=0; i<ptest->preference_Test.size(); i++)
       {
            preference = ptest->preference_Test.at(i);
            if(QString::fromStdString(preference->name) == "xml_node")
            {
                //qDebug() << "find xml_node: ";
                text = QString::fromStdString(preference->value);
                if(!text.isEmpty())
                {
                    doc_temp.setContent(text);
                    res = doc_temp.firstChild().toElement();
                    return(res);
                }
            }
       }
    }
    //.........................................................................

    //--- Header ---
    //res.appendChild(MakeElement(doc,"IDTest",QString::number(ptest->header.ID_Test)));
    res.appendChild(MakeElement(doc,"IDTest",QString::fromStdString(ptest->header.ID_Test)));
    res.appendChild(MakeElement(doc,"nameTest",QString::fromStdString(ptest->header.Name_Test)));
    res.appendChild(MakeElement(doc,"method",QString::number(ptest->header.Type_analysis)));
    res.appendChild(MakeElement(doc,"volumeTube",QString::number(ptest->header.Volume_Tube)));
    res.appendChild(MakeElement(doc,"activeChannels",QString::number(ptest->header.Active_channel,16)));
    res.appendChild(MakeElement(doc,"comment",QString::fromStdString(ptest->header.comment)));
    res.appendChild(MakeElement(doc,"Catalog",QString::fromStdString(ptest->header.Catalog)));
    //--- Programm ---
    text = "";
    QVector<string> prog = QVector<string>::fromStdVector(ptest->header.program);
    for(i=0; i<prog.size(); i++)
    {
        if(!text.isEmpty()) text += "\r\n";
        text += QString::fromStdString(prog.at(i));
    }
    res.appendChild(MakeElement(doc, "amProgramm" , text));

    //--- Property of Test ---
    QDomElement property = MakeElement(doc,"properties","");
    for(i=0; i<ptest->preference_Test.size(); i++)
    {        
        preference = ptest->preference_Test.at(i);
        if(QString::fromStdString(preference->name).trimmed() == "FormID")
        {
            DTReport = MakeElement(doc,"DTReport","");
            DTReport.appendChild(MakeElement(doc,"FormID",QString::fromStdString(preference->value)));
            res.appendChild(DTReport);
            continue;
        }
        if(QString::fromStdString(preference->name).trimmed() == "ReportSettings")
        {
            QDomElement ReportSettings = MakeElement(doc,"ReportSettings","");
            ReportSettings.appendChild(MakeElement(doc,"value",QString::fromStdString(preference->value)));
            DTReport.appendChild(ReportSettings);
            continue;
        }

        QDomElement item = MakeElement(doc,"item","");
        item.appendChild(MakeElement(doc,"name",QString::fromStdString(preference->name)));
        item.appendChild(MakeElement(doc,"value",QString::fromStdString(preference->value)));
        item.appendChild(MakeElement(doc,"unit",QString::fromStdString(preference->unit)));
        property.appendChild(item);

        //qDebug() << "Properties Name: " <<  QString::fromStdString(preference->name);
    }
    res.appendChild(property);

    //--- Channels ---
    QDomElement channels = MakeElement(doc,"channels","");
    active_ch = ptest->header.Active_channel;
    for(i=0; i<COUNT_CH; i++)
    {
        if(active_ch & (1<<i*4))
        {
            QDomElement item_TestCh = MakeElement(doc,"item","");
            item_TestCh.appendChild(MakeElement(doc,"IDChannel", QString("%1").arg(i)));
            item_TestCh.appendChild(MakeElement(doc,"number", QString("%1").arg(i)));
            item_TestCh.appendChild(MakeElement(doc,"ffor", QString("%1").arg(i)));
            channels.appendChild(item_TestCh);
        }
    }
    res.appendChild(channels);

    //--- Template ---
    QDomElement templates = MakeElement(doc,"templates","");
    for(i=0; i<ptest->tubes.size(); i++)
    {
        QDomElement item = MakeElement(doc,"item","");
        ptube = ptest->tubes.at(i);
        item.appendChild(MakeElement(doc,"IDTube", QString::number(i)));
        item.appendChild(MakeElement(doc,"name",QString::fromStdString(ptube->name)));
        // color
        color = ptube->color;
        color.getRgb(&r,&g,&b);
        key = QColor(b,g,r).rgb() & 0xffffff;
        item.appendChild(MakeElement(doc,"color", QString::number(key,16)));
        //

        QDomElement property = MakeElement(doc,"properties","");        
        for(j=0; j<ptube->preference_TubeTest.size(); j++)
        {
            preference = ptube->preference_TubeTest.at(j);            
            QDomElement item_tp = MakeElement(doc,"item","");
            item_tp.appendChild(MakeElement(doc,"name",QString::fromStdString(preference->name)));
            item_tp.appendChild(MakeElement(doc,"value",QString::fromStdString(preference->value)));
            item_tp.appendChild(MakeElement(doc,"unit",QString::fromStdString(preference->unit)));
            property.appendChild(item_tp);
        }

        item.appendChild(property);

        QDomElement channels = MakeElement(doc,"channels","");
        for(j=0; j<ptube->channels.size(); j++)
        {
            QDomElement item_ch = MakeElement(doc,"item","");
            pchannel = ptube->channels.at(j);
            item_ch.appendChild(MakeElement(doc,"IDChannel", QString::number(pchannel->ID_Channel)));
            item_ch.appendChild(MakeElement(doc,"name",QString::fromStdString(pchannel->name)));

            QDomElement property = MakeElement(doc,"properties","");
            for(k=0; k<pchannel->preference_ChannelTest.size(); k++)
            {
                preference = pchannel->preference_ChannelTest.at(k);
                QDomElement item_chp = MakeElement(doc,"item","");
                item_chp.appendChild(MakeElement(doc,"name",QString::fromStdString(preference->name)));
                item_chp.appendChild(MakeElement(doc,"value",QString::fromStdString(preference->value)));
                item_chp.appendChild(MakeElement(doc,"unit",QString::fromStdString(preference->unit)));
                property.appendChild(item_chp);
            }

            item_ch.appendChild(property);

            channels.appendChild(item_ch);
        }
        item.appendChild(channels);

        templates.appendChild(item);
    }

    res.appendChild(templates);    


    return(res);
}

//-----------------------------------------------------------------------------
//--- SaveXML_SourceSet
//-----------------------------------------------------------------------------
QDomElement SaveXML_SourceSet(QDomDocument &doc, rt_GroupSamples *pgroup, rt_Protocol *prot)
{
    int i;
    QString text, str_value, str;
    QStringList list, list_ch;

    rt_Preference   *preference;
    rt_Sample       *psample;

    QDomElement res;
    res = doc.createElement("item");

    text = QString::fromStdString(pgroup->ID_Group);
    if(text.isEmpty()) {Sleep(5); text = GetRandomString(10);}
    res.appendChild(MakeElement(doc, "IDGroup", text));
    res.appendChild(MakeElement(doc, "name", QString::fromStdString(pgroup->Unique_NameGroup)));

    QDomElement property = MakeElement(doc,"properties","");
    for(i=0; i<pgroup->preference_Group.size(); i++)
    {

        preference = pgroup->preference_Group.at(i);

        str_value = QString::fromStdString(preference->value);
        if(QString::fromStdString(preference->name) == "additional" && str_value.contains("||"))
        {
            list = str_value.split("#13#10");
            QDomElement item = MakeElement(doc,"item","");
            property.appendChild(item);
            item.appendChild(MakeElement(doc,"name", "Additional"));
            QDomElement item_value = MakeElement(doc,"value","");
            item.appendChild(item_value);

            foreach(str, list)
            {
                list_ch = str.split("||");
                if(list_ch.size() < 5) continue;
                QDomElement item_ch = MakeElement(doc,"item","");
                item_value.appendChild(item_ch);

                item_ch.appendChild(MakeElement(doc,"name", list_ch.at(0)));
                item_ch.appendChild(MakeElement(doc,"value", list_ch.at(3)));
                item_ch.appendChild(MakeElement(doc,"unit", str));
            }
            continue;
        }

        QDomElement item = MakeElement(doc,"item","");
        item.appendChild(MakeElement(doc,"name",QString::fromStdString(preference->name)));
        item.appendChild(MakeElement(doc,"value",QString::fromStdString(preference->value)));
        item.appendChild(MakeElement(doc,"unit",QString::fromStdString(preference->unit)));
        property.appendChild(item);
    }
    res.appendChild(property);

    QDomElement samples = doc.createElement("samples");
    for(i=0; i<pgroup->samples.size(); i++)
    {
        psample = pgroup->samples.at(i);
        samples.appendChild(SaveXML_Sample(doc, psample, prot));
    }
    res.appendChild(samples);

    return(res);
}


//-----------------------------------------------------------------------------
//--- SaveXML_Sample
//-----------------------------------------------------------------------------
QDomElement SaveXML_Sample(QDomDocument &doc, rt_Sample *psample, rt_Protocol *p)
{
    int i,j,k;
    rt_Tube         *ptube;
    rt_Channel      *pchannel;
    rt_Preference   *preference;
    int r,g,b,key;
    QColor color;
    int row,col;
    int x,y,pos;

    rt_GroupSamples *group;
    rt_Sample       *sample;

    //... pos_protocol ...
    int pos_protocol = 1;
    bool sts = false;
    foreach(group, p->Plate.groups)
    {
        foreach(sample, group->samples)
        {
            if(sample == psample)
            {
                sts = true;
                break;
            }
            pos_protocol += sample->tubes.size();
        }
        if(sts) break;
    }
    //....................

    int count_tubes = p->count_Tubes;
    p->Plate.PlateSize(count_tubes, row, col);

    QDomElement res = doc.createElement("item");

    res.appendChild(MakeElement(doc, "IDSample", QString::fromStdString(psample->ID_Sample)));
    res.appendChild(MakeElement(doc, "IDTest", QString::fromStdString(psample->p_Test->header.ID_Test)));
    res.appendChild(MakeElement(doc, "name", QString::fromStdString(psample->Unique_NameSample)));

    QDomElement property = MakeElement(doc,"properties","");
    for(i=0; i<psample->preference_Sample.size(); i++)
    {
        QDomElement item = MakeElement(doc,"item","");
        preference = psample->preference_Sample.at(i);
        item.appendChild(MakeElement(doc,"name",QString::fromStdString(preference->name)));
        item.appendChild(MakeElement(doc,"value",QString::fromStdString(preference->value)));
        item.appendChild(MakeElement(doc,"unit",QString::fromStdString(preference->unit)));
        property.appendChild(item);
    }
    res.appendChild(property);

    QDomElement tubes = doc.createElement("tubes");
    for(i=0; i<psample->tubes.size(); i++)
    {
        QDomElement item = doc.createElement("item");
        ptube = psample->tubes.at(i);

        item.appendChild(MakeElement(doc,"IDTube",QString::number(ptube->ID_Tube)));
        item.appendChild(MakeElement(doc,"name",QString::fromStdString(ptube->Unique_NameTube)));
        item.appendChild(MakeElement(doc,"pos",QString::number(ptube->pos)));

        pos = ptube->pos;
        x = div(pos,col).rem; // + 1;
        y = div(pos,col).quot; // + 1;
        item.appendChild(MakeElement(doc,"x",QString::number(x)));
        item.appendChild(MakeElement(doc,"y",QString::number(y)));

        item.appendChild(MakeElement(doc,"pos_protocol",QString::number(pos_protocol)));
        pos_protocol++;

        color = ptube->color;
        //color = p->color_tube.at(pos);
        color.getRgb(&r,&g,&b);
        key = QColor(b,g,r).rgb() & 0xffffff;
        item.appendChild(MakeElement(doc,"color",QString::number(key,16)));

        QDomElement channels = doc.createElement("channels");
        for(j=0; j<ptube->channels.size(); j++)
        {
            pchannel = ptube->channels.at(j);
            QDomElement item_ch = doc.createElement("item");
            item_ch.appendChild(MakeElement(doc,"IDChannel",QString::number(pchannel->ID_Channel)));

            QDomElement property_channel = MakeElement(doc,"properties","");
            for(k=0; k<pchannel->preference_Channel.size(); k++)
            {
                QDomElement item_pro = MakeElement(doc,"item","");
                preference = pchannel->preference_Channel.at(k);
                item_pro.appendChild(MakeElement(doc,"name",QString::fromStdString(preference->name)));
                item_pro.appendChild(MakeElement(doc,"value",QString::fromStdString(preference->value)));
                item_pro.appendChild(MakeElement(doc,"unit",QString::fromStdString(preference->unit)));
                property_channel.appendChild(item_pro);
            }
            item_ch.appendChild(property_channel);

            channels.appendChild(item_ch);
        }
        item.appendChild(channels);

        tubes.appendChild(item);
    }

    res.appendChild(tubes);

    return(res);
}

//-----------------------------------------------------------------------------
//--- SaveXML_Device
//-----------------------------------------------------------------------------
QDomElement SaveXML_Device(QDomDocument &doc, rt_Protocol *p)
{
    int i;

    QDomElement res = doc.createElement("device");

    res.appendChild(MakeElement(doc, "SerialName", QString::fromStdString(p->SerialName)));
    res.appendChild(MakeElement(doc, "uC_Versions", QString::fromStdString(p->uC_Versions)));
    res.appendChild(MakeElement(doc, "Type_ThermoBlock", QString::fromStdString(p->Type_ThermoBlock)));
    res.appendChild(MakeElement(doc, "FluorDevice_MASK", QString::number(p->FluorDevice_MASK,16)));
    res.appendChild(MakeElement(doc, "Optical_MASK", QString::fromStdString(p->DeviceParameters)));

    //... Coeff_Spectral ...
    QDomElement coeff_Spectral = doc.createElement("Coeff_Spectral");
    res.appendChild(coeff_Spectral);
    for(i=0; i<p->COEFF_Spectral.size(); i++)
    {
        coeff_Spectral.appendChild(MakeElement(doc,"item",QString::fromStdString(p->COEFF_Spectral.at(i))));
    }
    //... Coeff_UnequalCh ...
    QDomElement coeff_UnequalCh = doc.createElement("Coeff_UnequalCh");
    res.appendChild(coeff_UnequalCh);
    for(i=0; i<p->COEFF_UnequalCh.size(); i++)
    {
        coeff_UnequalCh.appendChild(MakeElement(doc,"item",QString::fromStdString(p->COEFF_UnequalCh.at(i))));
    }
    //... Coeff_Optic ...
    QDomElement coeff_Optic = doc.createElement("Coeff_Optic");
    res.appendChild(coeff_Optic);
    for(i=0; i<p->COEFF_Optic.size(); i++)
    {
        coeff_Optic.appendChild(MakeElement(doc,"item",QString::fromStdString(p->COEFF_Optic.at(i))));
    }


    return(res);
}

//-----------------------------------------------------------------------------
//--- SaveXML_Measurements
//-----------------------------------------------------------------------------
QDomElement SaveXML_Measurements(QDomDocument &doc, rt_Protocol *p)
{
    int i,j,k;
    QString fluor_name[COUNT_CH] = FLUOR_NAME;
    rt_Measurement  *measure;
    QString text;

    int col,row;
    p->Plate.PlateSize(p->count_Tubes, row, col);

    QDomElement res = doc.createElement("measurements");

    for(i=0; i<p->meas.size(); i++)
    {
        QDomElement item = doc.createElement("item");

        measure = p->meas.at(i);

        item.appendChild(MakeElement(doc,"channel",fluor_name[measure->optical_channel] + "_" +
                         QString::number(measure->num_exp)));
        item.appendChild(MakeElement(doc,"fn",QString::number(measure->fn)));
        item.appendChild(MakeElement(doc,"type_meas",QString::number(measure->type_meas)));
        item.appendChild(MakeElement(doc,"block_number",QString::number(measure->block_number)));
        item.appendChild(MakeElement(doc,"cycle_rep",QString::number(measure->cycle_rep)));
        item.appendChild(MakeElement(doc,"optical_channel",QString::number(measure->optical_channel)));
        item.appendChild(MakeElement(doc,"exp_number",QString::number(measure->num_exp)));
        item.appendChild(MakeElement(doc,"blk_exp",QString::number(measure->blk_exp)));
        item.appendChild(MakeElement(doc,"ovf_exp","0"));
        item.appendChild(MakeElement(doc,"expVal",QString::number((double)measure->exp_value,'f',0)));

        text = "\r\n";

        for(j=0; j<measure->measurements.size(); j++)
        {
            k = div(j,col).rem;
            if(!k && j) text += "\r\n";
            text += QString::number((ushort)measure->measurements.at(j)) + " ";
        }
        item.appendChild(MakeElement(doc,"data",text));

        QDomAttr fluor_attr = doc.createAttribute("channel");
        fluor_attr.setValue(fluor_name[measure->optical_channel] + "_" +
                            QString::number(measure->num_exp));

        QDomAttr fn_attr = doc.createAttribute("fn");
        fn_attr.setValue(QString::number(measure->fn));

        res.appendChild(item);
    }

    return(res);
}

//-----------------------------------------------------------------------------
//--- Save as XML
//-----------------------------------------------------------------------------
bool SaveAsXML(void *pobj, callback_process *Callback, rt_Protocol *prot, QString fn, QString mess, bool only_config)
{
    int i,j,k;
    QDomDocument doc;
    QString text;
    QString str;
    QStringList list;
    int vol;
    bool ok;
    int col,row;
    QString fluor_name[COUNT_CH] = FLUOR_NAME;
    bool res = false;


    QFileInfo fi(fn);

    rt_Test         *ptest;
    rt_GroupSamples *pgroup;
    rt_Measurement  *measure;
    rt_Preference   *preference;    

    if(Callback) Callback(pobj,0, mess); //"Save as XML...");

    QDomNode xmlNode = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\"");
    doc.insertBefore(xmlNode, doc.firstChild());

    QDomElement  root = doc.createElement("protocol");
    doc.appendChild(root);

    //--- Header ---
    if(Callback) Callback(pobj,10, mess);
    root.appendChild(MakeElement(doc, "name", QString::fromStdString(prot->name)));
    root.appendChild(MakeElement(doc, "barcode", QString::fromStdString(prot->barcode)));
    root.appendChild(MakeElement(doc, "regNumber", QString::fromStdString(prot->regNumber)));
    root.appendChild(MakeElement(doc, "nameOperator", QString::fromStdString(prot->owned_by)));
    root.appendChild(MakeElement(doc, "created", QString::fromStdString(prot->time_created)));
    //root.appendChild(MakeElement(doc, "status", "analysis"));
    switch(prot->state)
    {
    default:
    case mNotExist:     root.appendChild(MakeElement(doc, "status", "notexist"));       break;
    case mWait:         root.appendChild(MakeElement(doc, "status", "wait"));           break;
    case mDrop:         root.appendChild(MakeElement(doc, "status", "drop"));           break;
    case mRun:          root.appendChild(MakeElement(doc, "status", "run"));            break;
    case mData:         root.appendChild(MakeElement(doc, "status", "measurements"));   break;
    case mAnalysis:     root.appendChild(MakeElement(doc, "status", "analysis"));       break;
    case mReAnalysis:   root.appendChild(MakeElement(doc, "status", "reanalysis"));     break;
    }

    root.appendChild(MakeElement(doc, "activeChannels", QString::number(prot->active_Channels,16)));

    prot->Plate.PlateSize(prot->count_Tubes, row, col);
    root.appendChild(MakeElement(doc, "xsize", QString::number(col)));
    root.appendChild(MakeElement(doc, "ysize", QString::number(row)));

    root.appendChild(MakeElement(doc, "volumeTube", QString::number(prot->volume)));

    //--- Programm ---
    text = "";
    QVector<string> prog = QVector<string>::fromStdVector(prot->program);
    for(i=0; i<prog.size(); i++)
    {
        if(prot->volume && i==0)
        {
            str = QString::fromStdString(prog.at(i));
            list = str.split(" ");
            if(list.size() > 2 && str.startsWith("XPRG "))
            {
                vol = QString(list.at(2)).toInt(&ok);
                if(vol != prot->volume)
                {
                    list.replace(2, QString::number(prot->volume));
                    str = list.join(" ");
                    text = str;
                    continue;
                }
            }
        }
        if(!text.isEmpty()) text += "\r\n";
        text += QString::fromStdString(prog.at(i));
    }
    root.appendChild(MakeElement(doc, "amProgramm" , text));

    //--- Property ---
    QDomElement property = doc.createElement("properties");
    root.appendChild(property);
    //qDebug() << "properties: " << prot->preference_Pro.size() << prot;
    for(i=0; i<prot->preference_Pro.size(); i++)
    {
        preference = prot->preference_Pro.at(i);
        //qDebug() << "properties: " << QString::fromStdString(preference->name) << QString::fromStdString(preference->value);
        QDomElement item_property = doc.createElement("item");
        item_property.appendChild(MakeElement(doc,"name",QString::fromStdString(preference->name)));
        item_property.appendChild(MakeElement(doc,"value",QString::fromStdString(preference->value)));
        item_property.appendChild(MakeElement(doc,"unit",QString::fromStdString(preference->unit)));
        property.appendChild(item_property);
    }

    //--- Dropping ---
    QDomElement dropping = doc.createElement("dropping");
    root.appendChild(dropping);
    Adding_DropInformation(doc, dropping, prot);
    /*
    QDomElement specSet = doc.createElement("specSet");
    QDomElement reagentSet = doc.createElement("reagentSet");
    dropping.appendChild(specSet);
    dropping.appendChild(reagentSet);
    */

    //--- Tests ---
    if(Callback) Callback(pobj,20,mess);
    QDomElement tests = doc.createElement("tests");
    root.appendChild(tests);

    for(i=0; i<prot->tests.size(); i++)
    {        
        ptest = prot->tests.at(i);
        tests.appendChild(SaveXML_Test(doc,ptest));
    }

    //--- sourceSets ---
    if(Callback) Callback(pobj,30,mess);
    QDomElement  SampleSets = doc.createElement("sourceSet");
    root.appendChild(SampleSets);

    for(i=0; i<prot->Plate.groups.size(); i++)
    {
        pgroup = prot->Plate.groups.at(i);

        if(fi.suffix() == "trt")
        {
            foreach(preference, pgroup->preference_Group)
            {
                delete preference;
            }
            pgroup->preference_Group.clear();
        }

        SampleSets.appendChild(SaveXML_SourceSet(doc, pgroup, prot));
    }

    //--- device parameters ---
    //... device ...
    if(!only_config)
    {
        QDomElement device = SaveXML_Device(doc, prot);
        root.appendChild(device);
    }


    //--- measurements ---
    if(!only_config)
    {
        if(Callback) Callback(pobj,40,mess);
        QDomElement  measurements = SaveXML_Measurements(doc, prot);
        root.appendChild(measurements);
    }


    QFile file(fn);
    if(file.open(QIODevice::WriteOnly))
    {
        QTextStream(&file) << doc.toString();
        file.close();
        res = true;

        if(!only_config)
        {
            res = SaveResultsXML(pobj, Callback, prot, fn, mess);
        }
    }


    if(Callback) Callback(pobj,0,"");

    return(res);
}

//-----------------------------------------------------------------------------
//--- Adding_DropInformation
//-----------------------------------------------------------------------------
void Adding_DropInformation(QDomDocument &doc, QDomElement &element, rt_Protocol *p)
{
    int i,j;
    int num;
    int x,y;
    int s_xsize = 12;
    int s_ysize = 4;
    int r_xsize = 12;
    int r_ysize = 4;
    rt_GroupSamples *pgroup;
    rt_Test         *ptest;
    rt_TubeTest     *ptube;
    QStringList list;
    QString name;
    QString text;
    QVector<QString> *vec;

    QDomElement specSet = doc.createElement("specSet");
    QDomElement reagentSet = doc.createElement("reagentSet");
    QDomElement tableTESTs = doc.createElement("tableTESTs");
    element.appendChild(specSet);
    element.appendChild(reagentSet);
    element.appendChild(tableTESTs);

    //... specSet ...
    specSet.appendChild(MakeElement(doc, "xsize",  QString::number(s_xsize)));
    specSet.appendChild(MakeElement(doc, "ysize",  QString::number(s_ysize)));    
    QDomElement s_tubes = doc.createElement("tubes");
    specSet.appendChild(s_tubes);

    list.clear();
    num = 0;
    for(i=0; i<p->Plate.groups.size(); i++)
    {
        if(i >= s_xsize*s_ysize) break;

        pgroup = p->Plate.groups.at(i);
        name = QString::fromStdString(pgroup->Unique_NameGroup);
        //if(list.contains(name)) continue;
        //else list.append(name);
        list.append(name);

        x = div(num,s_xsize).rem;
        y = div(num,s_xsize).quot;

        QDomElement item = MakeElement(doc,"item","");
        item.appendChild(MakeElement(doc, "IDTube", QString::number(num)));
        item.appendChild(MakeElement(doc, "IDSample", QString::fromStdString(pgroup->ID_Group)));
        item.appendChild(MakeElement(doc, "name", QString::fromStdString(pgroup->Unique_NameGroup)));
        item.appendChild(MakeElement(doc, "x", QString::number(x)));
        item.appendChild(MakeElement(doc, "y", QString::number(y)));

        s_tubes.appendChild(item);
        num++;
    }

    //... reagentSet ...
    reagentSet.appendChild(MakeElement(doc, "xsize",  QString::number(r_xsize)));
    reagentSet.appendChild(MakeElement(doc, "ysize",  QString::number(r_ysize)));
    QDomElement r_tubes = doc.createElement("tubes");
    reagentSet.appendChild(r_tubes);

    num = 0;
    for(i=0; i<p->tests.size(); i++)
    {
        ptest = p->tests.at(i);
        for(j=0; j<ptest->tubes.size(); j++)
        {
            ptube = ptest->tubes.at(j);
            if(num >= r_xsize*r_ysize) break;

            x = div(num,r_xsize).rem;
            y = div(num,r_xsize).quot;

            QDomElement item = MakeElement(doc,"item","");
            item.appendChild(MakeElement(doc, "IDTube", QString::number(num)));
            item.appendChild(MakeElement(doc, "IDTest", QString::fromStdString(ptest->header.ID_Test)));
            item.appendChild(MakeElement(doc, "IDTemplate", QString::number(ptube->ID_Tube)));
            item.appendChild(MakeElement(doc, "name", QString::fromStdString(ptest->header.Name_Test)));
            item.appendChild(MakeElement(doc, "x", QString::number(x)));
            item.appendChild(MakeElement(doc, "y", QString::number(y)));

            r_tubes.appendChild(item);

            num++;
        }
    }

    //... tableTESTs ...
    foreach(text, p->Map_Reserve.keys())
    {
        if(text == "DropperTableTests")
        {
            vec = p->Map_Reserve.value(text);

            foreach(text, *vec)
            {
                list.clear();
                list = text.split(";");
                if(list.size() < 3) continue;

                QDomElement item = doc.createElement("item");
                item.setAttribute("num", list.at(0));
                item.appendChild(MakeElement(doc, "ID_Test", list.at(1)));
                item.appendChild(MakeElement(doc, "Name_Test", list.at(2)));
                tableTESTs.appendChild(item);
            }

            break;
        }
    }

}

//-----------------------------------------------------------------------------
//--- SaveXML_AnalysisVersion
//-----------------------------------------------------------------------------
QDomElement SaveXML_AnalysisVersion(QDomDocument &doc, rt_Protocol *prot)
{
    QDomElement res = doc.createElement("Analysis_Version");

    res.appendChild(MakeElement(doc, "version" , QCoreApplication::applicationVersion()));

    QString text, str = "";
    QVector<QString> *vec;
    if(prot->Map_Reserve.keys().contains("DTR_Version"))
    {
        vec = prot->Map_Reserve.value("DTR_Version");
        foreach(text, *vec)
        {
            if(!str.isEmpty()) str.append("; ");
            str.append(text);
        }
        res.appendChild(MakeElement(doc, "dtr_version", str));
    }

    qDebug() << "version: " << QCoreApplication::applicationVersion() << str;

    return(res);
}

//-----------------------------------------------------------------------------
//--- SaveXML_AnalysisCurves
//-----------------------------------------------------------------------------
QDomElement SaveXML_AnalysisCurves(QDomDocument &doc, rt_Protocol *prot)
{
    int i,j,k,m,n;
    QString text;
    QVector<double> Y;
    vector<double> *y;
    int active_ch = prot->active_Channels;
    int count = prot->count_PCR;
    int count_mc = prot->count_MC;
    int count_tubes = prot->count_Tubes;

    QColor color;
    int r,g,b;

    QDomElement raw_data;    
    QDomElement analysis_data;

    QDomElement res = doc.createElement("Analysis_Curves");

    //--- colors ---
    QDomElement colors = doc.createElement("colors");
    for(i=0; i<count_tubes; i++)
    {
        color = prot->color_tube.at(i);
        color.getRgb(&r,&g,&b);
        m = QColor(b,g,r).rgb() & 0xffffff;
        QDomElement item = doc.createElement("item");
        item.appendChild(MakeElement(doc,"IDTube",QString::number(i)));
        item.appendChild(MakeElement(doc,"value",QString::number(m,16)));
        colors.appendChild(item);
    }

    //--- Melting_Curve ---
    QDomElement mc = MakeElement(doc,"MC","");

        mc.appendChild(MakeElement(doc, "count" , QString::number(count_mc)));

        if(count_mc)
        {
            mc.appendChild(colors);

            for(n=0; n<2; n++)          // cycle for raw_data and analysis_data(dF/dT)
            {

            //--- MC Raw Data & dF/dT ---
            switch(n)
            {
            case 0:     raw_data = MakeElement(doc,"RawData",""); break;
            case 1:     analysis_data = MakeElement(doc,"AnalysisData",""); break;
            }

            QDomElement channels = doc.createElement("channels");
            k = 0;
            for(i=0; i<5; i++)
            {
                if((active_ch & (0x0f << 4*i)) == 0) continue;

                QDomElement item_ch = doc.createElement("item");
                item_ch.appendChild(MakeElement(doc,"IDChannel",QString::number(i)));

                QDomElement tubes = doc.createElement("tubes");
                for(j=0; j<prot->count_Tubes; j++)
                {
                    QDomElement item_tube = doc.createElement("item");
                    item_tube.appendChild(MakeElement(doc,"IDTube",QString::number(j)));

                    //--- data ---
                    switch(n)
                    {
                    case 0: y = &prot->MC_RawData;  break;
                    case 1: y = &prot->MC_dF_dT;  break;
                    }

                    Y = QVector<double>::fromStdVector(*y).mid(j*count+count_tubes*count_mc*k, count_mc);
                    text = "";
                    for(m=0; m<Y.size(); m++)
                    {
                        text += QString::number(Y.at(m),'f',0) + " ";
                    }
                    //------------

                    item_tube.appendChild(MakeElement(doc,"data",text));
                    tubes.appendChild(item_tube);
                }
                item_ch.appendChild(tubes);
                channels.appendChild(item_ch);
                k++;
            }
            switch(n)
            {
            case 0:     raw_data.appendChild(channels);
                        mc.appendChild(raw_data);
                        break;
            case 1:     analysis_data.appendChild(channels);
                        mc.appendChild(analysis_data);
                        break;
            }
            //--------------------
            }
        }
        res.appendChild(mc);

    //--- PCR ---
    QDomElement pcr = MakeElement(doc,"PCR","");

        pcr.appendChild(MakeElement(doc, "count" , QString::number(count)));        

        if(count)
        {
            pcr.appendChild(colors);

            for(n=0; n<2; n++)          // cycle for raw_data and base_data
            {

            //--- PCR Raw Data & Base Data ---
            switch(n)
            {
            case 0:     raw_data = MakeElement(doc,"RawData",""); break;
            case 1:     analysis_data = MakeElement(doc,"AnalysisData",""); break;
            }

            QDomElement channels = doc.createElement("channels");
            k = 0;
            for(i=0; i<5; i++)
            {
                if((active_ch & (0x0f << 4*i)) == 0) continue;

                QDomElement item_ch = doc.createElement("item");
                item_ch.appendChild(MakeElement(doc,"IDChannel",QString::number(i)));

                QDomElement tubes = doc.createElement("tubes");
                for(j=0; j<prot->count_Tubes; j++)
                {
                    QDomElement item_tube = doc.createElement("item");
                    item_tube.appendChild(MakeElement(doc,"IDTube",QString::number(j)));

                    //--- data ---
                    switch(n)
                    {
                    case 0: y = &prot->PCR_RawData;  break;
                    case 1: y = &prot->PCR_Bace;  break;
                    }

                    Y = QVector<double>::fromStdVector(*y).mid(j*count+count_tubes*count*k, count);
                    text = "";
                    for(m=0; m<Y.size(); m++)
                    {
                        text += QString::number(Y.at(m),'f',0) + " ";
                    }
                    //------------

                    item_tube.appendChild(MakeElement(doc,"data",text));
                    tubes.appendChild(item_tube);
                }
                item_ch.appendChild(tubes);
                channels.appendChild(item_ch);
                k++;
            }
            switch(n)
            {
            case 0:     raw_data.appendChild(channels);
                        pcr.appendChild(raw_data);
                        break;
            case 1:     analysis_data.appendChild(channels);
                        pcr.appendChild(analysis_data);
                        break;
            }
            //--------------------
            }
        }

        res.appendChild(pcr);

    return(res);
}

//-----------------------------------------------------------------------------
//--- SaveResultsXML
//-----------------------------------------------------------------------------
bool SaveResultsXML(void *pobj, callback_process *Callback, rt_Protocol *prot, QString fn, QString mess,
                                                                               QString fn_out)
{
    int i,j,k,m;
    QDomDocument doc;
    QString text;
    int count;
    bool res = false;

    QDomElement     root;
    QDomElement     sourceSet;
    QDomElement     groups;
    QDomElement     samples;
    QDomNode        sample;
    QDomElement     tubes;
    QDomElement     channels;

    QDomElement     result;
    QDomElement     item;

    rt_GroupSamples *pgroup;
    rt_Sample       *psample;
    rt_Tube         *ptube;
    rt_Channel      *pchannel;

    QVector<string> Y;

    if(Callback) Callback(pobj,0, mess); //"Save Results XML...");

    QFile file(fn);
    if(file.exists() && file.open(QIODevice::ReadOnly))
    {
        if(doc.setContent(&file))
        {
            file.close();

            root = doc.documentElement();

            //--- Change Status ---
            item = root.firstChildElement("status");
            switch(prot->state)
            {
            default:
            case mAnalysis:     result = MakeElement(doc,"status","analysis");
                                break;

            case mReAnalysis:   result = MakeElement(doc,"status","reanalysis");
                                break;
            }
            if(item.isNull())
            {
                root.insertAfter(result, root.firstChildElement("regNumber"));
            }
            else    root.replaceChild(result,item);

            //--- SourceSet ---
            sourceSet = root.firstChildElement("sourceSet");
            count = sourceSet.childNodes().size();
            for(i=0; i<count; i++)
            {                
                pgroup = prot->Plate.groups.at(i);
                samples = sourceSet.childNodes().at(i).firstChildElement("samples");
                if(samples.isNull()) continue;
                for(j=0; j<samples.childNodes().size(); j++)
                {
                    sample = samples.childNodes().at(j);
                    psample = pgroup->samples.at(j);
                    Y = QVector<string>::fromStdVector(psample->result_Sample);

                    result = MakeElementResults(doc,"sample_results",&Y);                    

                    item = sample.firstChildElement("sample_results");
                    if(item.isNull()) sample.appendChild(result);
                    else sample.replaceChild(result,item);

                    //sample.toElement().attribute("pp","balu");

                    tubes = samples.childNodes().at(j).firstChildElement("tubes");
                    if(tubes.isNull()) continue;
                    for(k=0; k<tubes.childNodes().size(); k++)
                    {
                        ptube = psample->tubes.at(k);
                        Y = QVector<string>::fromStdVector(ptube->result_Tube);
                        result = MakeElementResults(doc,"tube_results",&Y);

                        item = tubes.childNodes().at(k).firstChildElement("tube_results");
                        if(item.isNull()) tubes.childNodes().at(k).appendChild(result);
                        else tubes.childNodes().at(k).replaceChild(result,item);

                        channels = tubes.childNodes().at(k).firstChildElement("channels");
                        if(channels.isNull()) continue;
                        for(m=0; m<channels.childNodes().size(); m++)
                        {
                            pchannel = ptube->channels.at(m);
                            Y = QVector<string>::fromStdVector(pchannel->result_Channel);
                            result = MakeElementResults(doc,"channel_results",&Y);                            

                            item = channels.childNodes().at(m).firstChildElement("channel_results");
                            if(item.isNull()) channels.childNodes().at(m).appendChild(result);
                            else channels.childNodes().at(m).replaceChild(result,item);
                        }
                    }
                }

                if(Callback) Callback(pobj,(i*100)/count, mess);
                QCoreApplication::processEvents();
            }

            //--- Analysis Curves ---
            item = root.firstChildElement("Analysis_Curves");
            result = SaveXML_AnalysisCurves(doc, prot);

            if(item.isNull()) root.appendChild(result);
            else root.replaceChild(result,item);

            //--- Analysis Data ---
            item = root.firstChildElement("Analysis_Data");
            result = doc.createElement("Analysis_Data");

            if(item.isNull()) root.appendChild(result);
            else root.replaceChild(result,item);            

            //--- Analysis Version ---
            item = root.firstChildElement("Analysis_Version");
            result = SaveXML_AnalysisVersion(doc, prot);            

            if(item.isNull()) root.appendChild(result);
            else root.replaceChild(result,item);


            if(Callback) Callback(pobj,100,mess);
            QCoreApplication::processEvents();

            //---
            if(fn_out.isEmpty()) fn_out = fn;
            QFile file_out(fn_out);
            if(file_out.open(QIODevice::WriteOnly))
            {
                QTextStream(&file_out) << doc.toString();
                file_out.close();
                res = true;
            }
        }
        else file.close();
    }

    if(Callback) Callback(pobj,0,"");

    return(res);
}

//-----------------------------------------------------------------------------
//--- Read XML protocol
//-----------------------------------------------------------------------------
int Read_XML(void *pobj, callback_process *Callback, rt_Protocol *prot, QString fn, QString mess, bool only_config)
{
    int i,j,k;
    int res = 0;
    QString text, str;
    QString dir, version;
    QStringList list, list_pr;
    int value;
    bool ok;
    QColor color;
    int key;
    HINSTANCE ext_dll_handle;
    char *p_ch;
    int ID_LANG;

    int row=0,col=0;

    QFile file(fn);
    QDomDocument    doc;
    QDomElement     root;
    QDomNode        child;
    QDomNode        node;
    QDomNode        item;

    QDomDocument    doc_tests;

    rt_Test         *test;
    rt_GroupSamples *group;
    rt_Sample       *sample;
    rt_Tube         *tube;
    rt_Measurement  *measure;
    rt_Preference   *preference;

    QRegExp rx("[^A-Za-z0-9_-]");

    if(file.exists() && file.open(QIODevice::ReadOnly))
    {
        if(!doc.setContent(&file))
        {
            file.close();
            return(-1);
        }
        file.close();

        root = doc.documentElement();

        //---
        col = root.firstChildElement("xsize").text().toInt();
        row = root.firstChildElement("ysize").text().toInt();
        prot->count_Tubes = col*row;

        prot->color_tube.clear();
        prot->enable_tube.clear();
        color = QColor(Qt::gray);
        key = color.rgb();
        for(i=0; i<prot->count_Tubes; i++)
        {
            prot->color_tube.push_back(key);
            prot->enable_tube.push_back(0);
        }
        //---

        int count_nodes = root.childNodes().size();
        for(i=0; i<root.childNodes().size(); i++)
        {
            if(Callback) Callback(pobj,i*100/count_nodes,mess);
            qApp->processEvents();

            child = root.childNodes().at(i);

            //--- Header ------------------------------------------------------
            if(child.nodeName() == "name") {prot->name = child.toElement().text().toStdString(); continue;}
            if(child.nodeName() == "barcode") {prot->barcode = child.toElement().text().toStdString(); continue;}
            if(child.nodeName() == "regNumber") {prot->regNumber = child.toElement().text().toStdString(); continue;}
            if(child.nodeName() == "nameOperator") {prot->owned_by = child.toElement().text().toStdString(); continue;}
            if(child.nodeName() == "created") {prot->time_created = child.toElement().text().toStdString(); continue;}
            if(child.nodeName() == "status")
            {
                text = child.toElement().text().trimmed();
                prot->state = mNotExist;
                if(text == "wait")              prot->state = mWait;
                if(text == "drop")              prot->state = mDrop;
                if(text == "run")               prot->state = mRun;
                if(text == "measurements")      prot->state = mData;
                if(text == "analysis")          prot->state = mAnalysis;
                if(text == "reanalysis")        prot->state = mReAnalysis;
                continue;
            }
            if(child.nodeName() == "activeChannels") {prot->active_Channels = child.toElement().text().toInt(&ok,16); continue;}
            if(child.nodeName() == "volumeTube")
            {
                prot->volume = child.toElement().text().toInt(&ok);
                if(!ok) prot->volume = 0;
                continue;
            }
            //if(child.nodeName() == "xsize") {col = child.toElement().text().toInt(&ok); continue;}
            //if(child.nodeName() == "ysize") {row = child.toElement().text().toInt(&ok); continue;}

            //--- Program of Amplification ------------------------------------
            if(child.nodeName() == "amProgramm")
            {                
                text = child.toElement().text();
                list = text.split("\r\n");
                for(j=0; j<list.size(); j++)
                {
                    str = QString(list.at(j)).trimmed();
                    //text = str;
                    if(str.isEmpty()) continue;
                    if(str.contains("XSAV"))
                    {
                        list_pr = str.split(" ");
                        if(list_pr.size() > 1)
                        {
                            str = list_pr.at(1);
                            str = str.replace(rx,"_");
                            if(str.length() > 14) str.resize(14);
                            str = QString("XSAV %1").arg(str);
                        }
                    }
                    //qDebug() << "pro: " << str << text;
                    prot->program.push_back(str.toStdString());
                }
                Parsing_ProgramAmplification(prot);

                if(prot->volume == 0)
                {
                    foreach(text,list)
                    {
                        if(text.startsWith("XPRG "))
                        {
                            list_pr = text.split(" ");
                            if(list_pr.size() > 2)
                            {
                                value = QString(list_pr.at(2)).toInt(&ok);
                                if(!ok) value = 25;
                                else
                                {
                                    if(value > 200 || value < 5) value = 25;
                                }
                                prot->volume = value;
                            }

                            break;
                        }
                    }
                }
                else    // prot->volume != 0
                {
                    if(list.size())
                    {
                        text = list.at(0);
                        if(text.startsWith("XPRG "))
                        {
                            list_pr = text.split(" ");
                            if(list_pr.size() > 2)
                            {
                                value = QString(list_pr.at(2)).toInt(&ok);
                                if(value != prot->volume)
                                {
                                    list_pr.replace(2, QString::number(prot->volume));
                                    text = list_pr.join(" ");
                                    prot->program.at(0) = text.toStdString();

                                    //qDebug() << "Volume: " << text;
                                }
                            }
                        }
                    }
                }

                continue;
            }


            //--- Properties of Protocol --------------------------------------
            if(child.nodeName() == "properties")
            {
                for(j=0; j<child.childNodes().size(); j++)
                {
                     node = child.childNodes().at(j);                     
                     preference = new rt_Preference;
                     prot->preference_Pro.push_back(preference);
                     LoadXML_Preference(node, preference);
                }
                continue;
            }

            //--- Information about TESTs: ------------------------------------
            if(child.nodeName() == "tests")
            {
               /*
               doc_tests.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"utf-8\"");
               doc_tests.appendChild(doc_tests.importNode(child, true));
               qDebug() << "Tests: " << doc_tests.toString();
               text = doc_tests.toString();
               ext_dll_handle = ::LoadLibraryW(L"DTReport2.dll");
               qDebug() << "ext_dll_handle: " << ext_dll_handle;
               if(ext_dll_handle)
               {
                    Init initialize = (Init)(::GetProcAddress(ext_dll_handle,"Initialize"));
                    TranslateDTR translate = (TranslateDTR)(::GetProcAddress(ext_dll_handle,"TranslateService"));
                    qDebug() << "TranslateDTR: " << translate;
                    if(translate && initialize)
                    {
                        // 1. Init
                        dir = qApp->applicationDirPath() + "/forms";
                        version = "9.0";
                        ID_LANG = 0x0409;
                        initialize(NULL, dir.toLatin1().data(), version.toLatin1().data(), ID_LANG, NULL,NULL,NULL,NULL,NULL,NULL);
                        qDebug() << "init - ok";

                        // 2. Translate
                        translate(text.toUtf8().data(), &p_ch);
                        qDebug() << "translate - ok";
                    }
               }

               doc_tests.clear();
               text.clear();
               text.append(p_ch);
               qDebug() << "Tests_out: " << text;

               if(ext_dll_handle) ::FreeLibrary(ext_dll_handle);

               if(doc_tests.setContent(text))
               {
                    child = doc_tests.documentElement();
                    qDebug() << "new CHILD:";
               }*/

               for(j=0; j<child.childNodes().size(); j++)
               {
                    node = child.childNodes().at(j);
                    test = new rt_Test();
                    prot->tests.push_back(test);
                    LoadXML_Test(node, test);
               }
               doc_tests.clear();

               continue;
            }


            //--- sourceSet ---------------------------------------------------
            if(child.nodeName() == "sourceSet")
            {
                for(j=0; j<child.childNodes().size(); j++)
                {
                     node = child.childNodes().at(j);
                     group = new rt_GroupSamples();
                     prot->Plate.groups.push_back(group);
                     LoadXML_Group(node, group, prot);


                     //QApplication::processEvents();
                }
                continue;
            }

            //--- device parameters -------------------------------------------
            if(child.nodeName() == "device" && !only_config)
            {
                for(j=0; j<child.childNodes().size(); j++)
                {
                    item = child.childNodes().at(j);

                    if(item.nodeName() == "SerialName") {prot->SerialName = item.toElement().text().toStdString(); continue;}
                    if(item.nodeName() == "uC_Versions") {prot->uC_Versions = item.toElement().text().toStdString(); continue;}
                    if(item.nodeName() == "Type_ThermoBlock") {prot->Type_ThermoBlock = item.toElement().text().toStdString(); continue;}
                    if(child.nodeName() == "FluorDevice_MASK") {prot->FluorDevice_MASK = child.toElement().text().toInt(&ok,16); continue;}
                    if(item.nodeName() == "Optical_MASK") {prot->DeviceParameters = item.toElement().text().toStdString(); continue;}

                    //... Coeff_Spectral ...
                    if(item.nodeName() == "Coeff_Spectral")
                    {
                        prot->COEFF_Spectral.clear();

                        for(k=0; k<item.childNodes().size(); k++)
                        {
                            node = item.childNodes().at(k);
                            text = node.toElement().text();
                            prot->COEFF_Spectral.push_back(text.toStdString());
                        }
                        continue;
                    }

                    //... Coeff_UnequalCh ...
                    if(item.nodeName() == "Coeff_UnequalCh")
                    {
                        prot->COEFF_UnequalCh.clear();

                        for(k=0; k<item.childNodes().size(); k++)
                        {
                            node = item.childNodes().at(k);
                            text = node.toElement().text();
                            prot->COEFF_UnequalCh.push_back(text.toStdString());
                        }
                        continue;
                    }

                    //... Coeff_Optic ...
                    if(item.nodeName() == "Coeff_Optic")
                    {
                        prot->COEFF_Optic.clear();

                        for(k=0; k<item.childNodes().size(); k++)
                        {
                            node = item.childNodes().at(k);
                            text = node.toElement().text();
                            prot->COEFF_Optic.push_back(text.toStdString());
                        }
                        continue;
                    }

                }
                continue;
            }

            //--- measurements ------------------------------------------------
            if(child.nodeName() == "measurements" && !only_config)
            {
                for(j=0; j<child.childNodes().size(); j++)
                {
                    //if(div(j,2).rem) continue;
                    node = child.childNodes().at(j);
                    measure = new rt_Measurement();
                    prot->meas.push_back(measure);
                    LoadXML_Measure(node,measure);
                }                            
                continue;
            }

        }

        // ... check validity of the xml_protocol ...
        //if(root.firstChildElement("measurements").isNull() && !only_config) res = -1;

        // ...

        // ... Check validity of the Protocol Hash
        prot->validity_hash = false;
        item = root.firstChildElement("Analysis_HASH");
        if(!item.isNull())
        {
            child = item.firstChildElement("hash");
            if(!child.isNull())
            {
                QString hash_protocol = child.toElement().text();

                root.removeChild(item);

                QCryptographicHash Hash(QCryptographicHash::Sha256);
                Hash.addData(doc.toString().toUtf8());
                QByteArray hash(Hash.result());             // QCryptographicHash::Sha256
                std::reverse(hash.begin(), hash.end());     // reverse
                QByteArray hash_compress(qCompress(hash));  // Compress

                if(hash_protocol == hash_compress.toHex()) prot->validity_hash = true;

                //qDebug() << "hash: " << hash_protocol;
                //qDebug() << "HASH: " << hash_compress.toHex() << prot->validity_hash;
            }
        }

        // ...


        return(res);
    }
    else return(-1);


}

//-----------------------------------------------------------------------------
//--- Load XML Measure
//-----------------------------------------------------------------------------
void LoadXML_Measure(QDomNode &node, rt_Measurement *measure)
{
    int i,j;
    bool ok;
    QString text;
    QStringList list;

    QDomNode        child;
    QDomNode        item;

    for(i=0; i<node.childNodes().size(); i++)
    {
        child = node.childNodes().at(i);

        if(child.nodeName() == "fn") {measure->fn = child.toElement().text().toInt(&ok); continue;}
        if(child.nodeName() == "type_meas") {measure->type_meas = child.toElement().text().toInt(&ok); continue;}
        if(child.nodeName() == "block_number") {measure->block_number = child.toElement().text().toInt(&ok); continue;}
        if(child.nodeName() == "cycle_rep") {measure->cycle_rep = child.toElement().text().toInt(&ok); continue;}
        if(child.nodeName() == "optical_channel") {measure->optical_channel = child.toElement().text().toInt(&ok); continue;}
        if(child.nodeName() == "exp_number") {measure->num_exp = child.toElement().text().toInt(&ok); continue;}
        if(child.nodeName() == "blk_exp") {measure->blk_exp = child.toElement().text().toInt(&ok); continue;}
        if(child.nodeName() == "ovf_exp") {continue;}
        if(child.nodeName() == "expVal") {measure->exp_value = child.toElement().text().toInt(&ok); continue;}
        if(child.nodeName() == "data")
        {
            text = child.toElement().text().trimmed();
            list = text.split(" ");
            for(j=0; j<list.size(); j++)
            {
                measure->measurements.push_back(list.at(j).toInt(&ok));
            }
        }
    }

}

//-----------------------------------------------------------------------------
//--- Load XML Group
//-----------------------------------------------------------------------------
void LoadXML_Group(QDomNode &node, rt_GroupSamples *group, rt_Protocol *prot)
{
    int i,j,k;
    bool ok;
    QString text;

    QString str, str_name, str_value, str_all = "";
    QStringList list_ch;

    rt_Sample       *sample;
    rt_Preference   *preference;

    QDomNode        child;
    QDomNode        item;
    QDomNode        item_preference;
    QDomNode        item_ch;
    QDomNode        item_add;

    for(i=0; i<node.childNodes().size(); i++)
    {
        child = node.childNodes().at(i);

        if(child.nodeName() == "IDGroup")
        {
            text = child.toElement().text().trimmed();
            if(text.isEmpty()) {Sleep(5); text = GetRandomString(10);}
            group->ID_Group = text.toStdString();
            continue;
        }
        if(child.nodeName() == "name") {group->Unique_NameGroup = child.toElement().text().toStdString(); continue;}
        if(child.nodeName() == "properties")
        {
            for(j=0; j<child.childNodes().size(); j++)
            {
                item = child.childNodes().at(j);

                //... additional properties ...






                item_preference = item.firstChildElement("name");
                if(!item_preference.isNull() && item_preference.toElement().text().trimmed() == "Additional")
                {
                    item_preference = item.firstChildElement("value");
                    if(item_preference.isNull()) continue;

                    for(k=0; k<item_preference.childNodes().size(); k++)
                    {
                        item_ch = item_preference.childNodes().at(k);
                        item_add = item_ch.firstChildElement("name");
                        str_name = item_add.toElement().text().trimmed();
                        item_add = item_ch.firstChildElement("value");
                        str_value = item_add.toElement().text().trimmed();
                        item_add = item_ch.firstChildElement("unit");
                        str = item_add.toElement().text().trimmed();

                        list_ch = str.split("||");
                        if(list_ch.size() >= 5 && str_name == list_ch.at(0))
                        {
                            list_ch.replace(3, str_value);
                            str = list_ch.join("||");
                            if(!str_all.isEmpty()) str_all += "#13#10";
                            str_all += str;
                        }
                    }
                    if(!str_all.isEmpty())
                    {

                        preference = new rt_Preference();
                        group->preference_Group.push_back(preference);












                        preference->name = "additional";
                        preference->value = str_all.toStdString();
                        preference->unit = "";
                    }

                    continue;
                }
                //...

                preference = new rt_Preference();
                group->preference_Group.push_back(preference);
                for(k=0; k<item.childNodes().size(); k++)
                {
                    item_preference = item.childNodes().at(k);
                    if(item_preference.nodeName() == "name") {preference->name = item_preference.toElement().text().toStdString(); continue;}
                    if(item_preference.nodeName() == "value") {preference->value = item_preference.toElement().text().toStdString(); continue;}
                    if(item_preference.nodeName() == "unit") {preference->unit = item_preference.toElement().text().toStdString(); continue;}
                }
            }

            continue;
        }

        if(child.nodeName() == "samples")
        {
            for(j=0; j<child.childNodes().size(); j++)
            {
                item = child.childNodes().at(j);
                sample = new rt_Sample();
                group->samples.push_back(sample);
                LoadXML_Sample(item,sample,prot);
            }
            continue;
        }
    }
}


//-----------------------------------------------------------------------------
//--- Load XML Sample
//-----------------------------------------------------------------------------
void LoadXML_Sample(QDomNode &node, rt_Sample *sample, rt_Protocol *prot)
{
    int i,j,k;
    bool ok;
    QString id_test;

    rt_Tube         *tube;
    rt_Test         *test;
    rt_Preference   *preference;

    QDomNode        child;
    QDomNode        item;
    QDomNode        item_preference;

    for(i=0; i<node.childNodes().size(); i++)
    {
        child = node.childNodes().at(i);

        if(child.nodeName() == "IDSample") {sample->ID_Sample = child.toElement().text().toStdString(); continue;}
        if(child.nodeName() == "IDTest") {sample->ID_Test = child.toElement().text().toStdString(); continue;}
        if(child.nodeName() == "name") {sample->Unique_NameSample = child.toElement().text().toStdString(); continue;}
        if(child.nodeName() == "properties")
        {
            for(j=0; j<child.childNodes().size(); j++)
            {
                item = child.childNodes().at(j);
                preference = new rt_Preference();
                sample->preference_Sample.push_back(preference);
                for(k=0; k<item.childNodes().size(); k++)
                {
                    item_preference = item.childNodes().at(k);
                    if(item_preference.nodeName() == "name") {preference->name = item_preference.toElement().text().toStdString(); continue;}
                    if(item_preference.nodeName() == "value") {preference->value = item_preference.toElement().text().toStdString(); continue;}
                    if(item_preference.nodeName() == "unit") {preference->unit = item_preference.toElement().text().toStdString(); continue;}
                }
            }
        }
        if(child.nodeName() == "tubes")
        {
            for(j=0; j<child.childNodes().size(); j++)
            {
                item = child.childNodes().at(j);
                tube = new rt_Tube();
                sample->tubes.push_back(tube);                
                LoadXML_Tube(item,tube,prot);
            }
            continue;
        }
    }

    id_test = QString::fromStdString(sample->ID_Test);
    for(i=0; i<prot->tests.size(); i++)
    {
        test = prot->tests.at(i);
        if(id_test == QString::fromStdString(test->header.ID_Test))
        {
            sample->p_Test = test;
            break;
        }
    }
}

//-----------------------------------------------------------------------------
//--- Load XML Tube
//-----------------------------------------------------------------------------
void LoadXML_Tube(QDomNode &node, rt_Tube *tube, rt_Protocol *prot)
{
    int i,j;
    bool ok;
    int r,g,b,key;
    QColor color;



    rt_Channel      *channel;

    QDomNode        child;
    QDomNode        item;

    int pos = node.firstChildElement("pos").toElement().text().toInt();
    //if(pos < 0) pos = 0;
    color = node.firstChildElement("color").toElement().text().toInt(&ok,16);
    color.getRgb(&r,&g,&b);
    key = QColor(b,g,r).rgb() & 0xffffff;
    tube->pos = pos;
    if(pos >= 0) prot->enable_tube.at(pos) = 1;
    if(pos >= 0) prot->color_tube.at(pos) = key;
    tube->color = key;

    for(i=0; i<node.childNodes().size(); i++)
    {
        child = node.childNodes().at(i);

        if(child.nodeName() == "IDTube") {tube->ID_Tube = child.toElement().text().toInt(&ok); continue;}
        if(child.nodeName() == "name") {tube->Unique_NameTube = child.toElement().text().toStdString(); continue;}
        //if(child.nodeName() == "pos") {tube->pos = child.toElement().text().toInt(&ok); continue;}
        //if(child.nodeName() == "color") {tube->color = child.toElement().text().toInt(&ok,16); continue;}
        if(child.nodeName() == "channels")
        {
            for(j=0; j<child.childNodes().size(); j++)
            {
                item = child.childNodes().at(j);
                channel = new rt_Channel();
                tube->channels.push_back(channel);
                LoadXML_Channel(item,channel,prot);
            }
            continue;
        }
     }
}

//-----------------------------------------------------------------------------
//--- Load XML Channel
//-----------------------------------------------------------------------------
void LoadXML_Channel(QDomNode &node, rt_Channel *channel, rt_Protocol *prot)
{
    int i,j;
    bool ok;

    QDomNode        child;
    QDomNode        item;
    rt_Preference   *preference;

    for(i=0; i<node.childNodes().size(); i++)
    {
        child = node.childNodes().at(i);

        if(child.nodeName() == "IDChannel") {channel->ID_Channel = child.toElement().text().toInt(&ok); continue;}
        if(child.nodeName() == "properties")
        {
            for(j=0; j<child.childNodes().size(); j++)
            {
                item = child.childNodes().at(j);
                preference = new rt_Preference;
                channel->preference_Channel.push_back(preference);
                LoadXML_Preference(item, preference);
            }
            continue;
        }
    }
}

//-----------------------------------------------------------------------------
//--- Load XML Preference
//-----------------------------------------------------------------------------
void LoadXML_Preference(QDomNode &node, rt_Preference *preference)
{
    int i;
    QDomNode child;

    for(i=0; i<node.childNodes().size(); i++)
    {
        child = node.childNodes().at(i);
        if(child.nodeName() == "name") {preference->name = child.toElement().text().toStdString(); continue;}
        if(child.nodeName() == "value") {preference->value = child.toElement().text().toStdString(); continue;}
        if(child.nodeName() == "unit") {preference->unit = child.toElement().text().toStdString(); continue;}
    }
}

//-----------------------------------------------------------------------------
//--- Load XML Test
//-----------------------------------------------------------------------------
void LoadXML_Test(QDomNode &node, rt_Test *ptest)
{
    int i,j,k,m,n;
    bool ok;
    QString text,str;
    QStringList list;
    QByteArray ba;
    quint16 crc16;
    QColor color;
    int r,g,b,key;
    int id = 0x20000;

    QString Name,Value;
    bool new_property;

    QDomNode        child;
    QDomNode        item;
    QDomNode        tube_node;
    QDomNode        ch_item;
    QDomNode        ch_node;
    QDomNode        properties;
    QDomNode        properties_item;
    QDomElement     color_element;
    QDomElement     item_value;
    QDomElement     item_analysis;
    QDomElement     item_version;
    QDomElement     item_param;

    bool find_param;
    QStringList list_param;
    list_param << "ServiceVersion" << "ControlTubesCount" << "PosControlCount" << "NegControlCount";

    rt_TubeTest     *tube;
    rt_ChannelTest  *channel;
    rt_Preference   *preference_test;
    rt_Preference   *preference;

    QString text_xml;
    QTextStream stream(&text_xml);

    QRegExp rx("[^A-Za-z0-9_-]");
    QStringList list_pr;

    if(node.isElement())        // save test as text(QString) in preference: name = xml_node  value = text
    {
        //text_xml = node.toElement().toText();
        node.save(stream, QDomNode::CDATASectionNode);

        for(i=0; i<ptest->preference_Test.size(); i++)  // Delete previous preference with name == "xml_node"
        {
            preference_test = ptest->preference_Test.at(i);
            if(preference_test->name == "xml_node")
            {                
                delete preference_test;
                ptest->preference_Test.erase(ptest->preference_Test.begin() + i);
                break;
            }            
        }        
        preference_test = new rt_Preference;
        preference_test->name = "xml_node";
        preference_test->value = text_xml.toStdString();
        ptest->preference_Test.push_back(preference_test);
    }

    for(i=0; i<node.childNodes().size(); i++)
    {
        qApp->processEvents();

        child = node.childNodes().at(i);

        if(child.nodeName() == "IDTest") {ptest->header.ID_Test = child.toElement().text().toStdString(); continue;}
        if(child.nodeName() == "nameTest") {ptest->header.Name_Test = child.toElement().text().toStdString(); continue;}
        if(child.nodeName() == "method") {ptest->header.Type_analysis = child.toElement().text().toInt(&ok); continue;}
        if(child.nodeName() == "activeChannels") {ptest->header.Active_channel = child.toElement().text().toInt(&ok,16); continue;}
        if(child.nodeName() == "comment") {ptest->header.comment = child.toElement().text().toStdString(); continue;}
        if(child.nodeName() == "volumeTube") {ptest->header.Volume_Tube = child.toElement().text().toInt(&ok); continue;}
        if(child.nodeName() == "Catalog") {ptest->header.Catalog = child.toElement().text().toStdString(); continue;}
        if(child.nodeName() == "amProgramm")
        {
            ptest->header.program.clear();
            text = child.toElement().text();
            list = text.split("\r\n");
            for(j=0; j<list.size(); j++)
            {
                text = QString(list.at(j)).trimmed();
                if(text.isEmpty()) continue;
                if(text.contains("XSAV"))
                {
                    list_pr = text.split(" ");
                    if(list_pr.size() > 1)
                    {
                        text = list_pr.at(1);
                        //text = text.remove(rx);
                        text = text.replace(rx,"_");
                        if(text.length() > 14) text.resize(14);
                        text = QString("XSAV %1").arg(text);
                    }
                }
                ptest->header.program.push_back(text.toStdString());
            }
            continue;
        }
        if(child.nodeName() == "properties")
        {
            for(j=0; j<child.childNodes().size(); j++)
            {
                properties_item = child.childNodes().at(j);
                Name = properties_item.firstChildElement("name").toElement().text();
                Value = properties_item.firstChildElement("value").toElement().text();
                if(Name == "xml_node") continue;
                new_property = false;
                for(k=0; k<ptest->preference_Test.size(); k++)
                {
                    preference_test = ptest->preference_Test.at(k);
                    if(preference_test->name == Name.toStdString())
                    {
                        preference_test->value = Value.toStdString();
                        new_property = true;
                        break;
                    }
                }
                if(new_property) continue;

                preference_test = new rt_Preference;
                ptest->preference_Test.push_back(preference_test);

                preference_test->name = Name.toStdString();
                preference_test->value = Value.toStdString();
            }
            continue;
        }
        if(child.nodeName() == "templates")
        {
            // clear all tubes in templates of the test
            for(j=0; j<ptest->tubes.size(); j++) delete ptest->tubes.at(j);
            ptest->tubes.clear();
            // ...

            for(j=0; j<child.childNodes().size(); j++)
            {
                tube = new rt_TubeTest();
                ptest->tubes.push_back(tube);
                item = child.childNodes().at(j);

                for(k=0; k<item.childNodes().size(); k++)
                {
                    tube_node = item.childNodes().at(k);
                    if(tube_node.nodeName() == "IDTube") {tube->ID_Tube = tube_node.toElement().text().toInt(&ok); continue;}                                        
                    if(tube_node.nodeName() == "color")
                    {
                        text = tube_node.toElement().text();
                        text.remove("$");
                        color = text.toInt(&ok,16);
                        color.getRgb(&r,&g,&b);
                        tube->color = QColor(b,g,r).rgb() & 0xffffff;
                        continue;
                    }

                    if(tube_node.nodeName() == "channels")
                    {
                        for(m=0; m<tube_node.childNodes().size(); m++)
                        {
                            channel = new rt_ChannelTest();
                            tube->channels.push_back(channel);
                            ch_node = tube_node.childNodes().at(m);
                            channel->ID_Channel = ch_node.firstChildElement("IDChannel").toElement().text().toInt(&ok);
                            channel->name = ch_node.firstChildElement("name").toElement().text().toStdString();

                            properties = ch_node.firstChildElement("properties");
                            if(!properties.isNull())
                            {
                                for(n=0; n<properties.childNodes().size(); n++)
                                {
                                    preference = new rt_Preference();
                                    channel->preference_ChannelTest.push_back(preference);
                                    properties_item = properties.childNodes().at(n);
                                    preference->name = properties_item.firstChildElement("name").toElement().text().toStdString();
                                    preference->value = properties_item.firstChildElement("value").toElement().text().toStdString();
                                }
                            }
                        }
                    }
                }
            }            
            continue;
        }
        if(child.nodeName() == "DTReport")
        {
            for(j=0; j<child.childNodes().size(); j++)
            {
                item = child.childNodes().at(j);
                if(item.nodeName() == "FormID")
                {
                    text = " \r\nDTReport = " + item.toElement().text();
                    ptest->header.comment += text.toStdString();

                    text = item.toElement().text();
                    ba.clear();
                    ba.append(text.toLatin1());
                    crc16 = qChecksum(ba.data(),ba.size());
                    ptest->header.Type_analysis = id + crc16;
                    continue;
                }

                //
                if(item.nodeName() == "ReportSettings")
                {
                    item_value = item.firstChildElement("value");
                    if(!item_value.isNull())
                    {
                        item_analysis = item_value.firstChildElement("TAnalisys");
                        if(!item_analysis.isNull())
                        {
                            foreach(text, list_param)
                            {
                                item_param = item_analysis.firstChildElement(text);
                                if(!item_param.isNull())
                                {
                                    str = item_param.text().trimmed();

                                    //if(text == "ServiceVersion" && str.isEmpty()) continue;
                                    if(text == "ServiceVersion")
                                    {
                                        if(str == "1.0") str = "";
                                        ptest->header.version = str.toStdString();
                                        continue;
                                    }

                                    if(text == "ControlTubesCount" && str == "0") continue;
                                    if(text == "PosControlCount" && str == "0") continue;
                                    if(text == "NegControlCount" && str == "0") continue;

                                    //qDebug() << QString::fromStdString(ptest->header.Name_Test) << text << str;

                                    find_param = false;
                                    foreach(preference_test, ptest->preference_Test)
                                    {
                                        if(preference_test->name == text.toStdString())
                                        {
                                            preference_test->value = str.toStdString();
                                            find_param = true;
                                            break;
                                        }
                                    }
                                    if(!find_param)
                                    {
                                        preference_test = new rt_Preference;
                                        ptest->preference_Test.push_back(preference_test);

                                        preference_test->name = text.toStdString();
                                        preference_test->value = str.toStdString();
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

    }
}

//-----------------------------------------------------------------------------
//--- Read OLD(*.r96) protocol
//-----------------------------------------------------------------------------
int Read_r96(void *pobj, callback_process *Callback, rt_Protocol *prot, QString fn, QString mess)
{
    int res = 0;
    QString line, text;
    QStringList fields;
    bool ok, sts_ch;
    int id_sample = 0, id_tube, active_ch, ch;
    int new_sample = 0, col;
    int value;
    int i,j,k,l,m,n;
    QColor color;
    int r,g,b;
    int id;
    QString id_str;
    int num_ch;
    QString fluor_name[COUNT_CH] = FLUOR_NAME;


    rt_Test 		*p_Test;
    rt_GroupSamples	*p_Group;
    rt_Sample		*p_Sample;
    rt_Tube 		*p_Tube;
    rt_Channel 		*p_Channel;
    rt_Measurement  *p_Measurement;
    rt_TemplateTest *p_TemplateTest;
    rt_TubeTest     *p_TubeTest;
    rt_ChannelTest  *p_ChannelTest;
    rt_Preference   *p_PreferenceTest;

    QMap<int, QString> map;                 // temporary container
    QMap<int, QString>::iterator it_map;    // ...
    int key;                                // ...
    QHash<QString, rt_GroupSamples*> hash;  // ...
    QVector<int> active_tests;


    QFile file(fn);
    QFileInfo file_Info(file);

    if(file.open(QFile::ReadOnly))
    {
        if(Callback) Callback(pobj,0,mess);
        QTextStream stream(&file);
        stream.setCodec("Windows-1251");
        line = stream.readLine();

        while(!stream.atEnd())
        {
            //--- Header ------------------------------------------------------
            text = file_Info.completeBaseName() + "." + file_Info.suffix();
            prot->name = text.toStdString();
            if(line.indexOf("$Data:$") >= 0)        prot->time_created = line.mid(line.indexOf(" ")+1).toStdString();
            if(line.indexOf("$BarCode:$") >= 0)     prot->barcode = line.mid(line.indexOf(" ")+1).toStdString();
            if(line.indexOf("$Number protocol:$") >= 0) prot->regNumber = line.mid(line.indexOf(":$")+3).toStdString();
            if(line.indexOf("$Operator:$") >= 0)    prot->owned_by = line.mid(line.indexOf(" ")+1).toStdString();

            //... state ...
            prot->state = mData;

            //--- Program of Amplification ------------------------------------
            if(line.indexOf("$Program of amplification:$") >= 0)
            {
                prot->program.clear();
                line = stream.readLine();
                while(line.indexOf("$") < 0)
                {
                    if(line.trimmed().length() > 0) prot->program.push_back(line.trimmed().toStdString());
                    line = stream.readLine();
                }
                Parsing_ProgramAmplification(prot);
                continue;
            }

            //--- Information about TESTs: ------------------------------------
            if(line.indexOf("$Information about TESTs:$") >= 0)
            {
                    if(Callback) Callback(pobj,10,mess);
                    line = stream.readLine();
                    while(line.indexOf("$") < 0 || line.indexOf("$") > 1)
                    {
                        if(line.trimmed() != "")
                        {
                            fields = line.trimmed().split(QRegExp("\\s+"));

                            p_Test = new rt_Test();
                            prot->tests.push_back(p_Test);

                            p_Test->header.ID_Test = fields.at(0).toStdString();
                            p_Test->header.Name_Test = fields.at(1).toStdString();
                            p_Test->header.Type_analysis = fields.at(2).toInt(&ok, 16);
                            p_Test->header.Active_channel = fields.at(5).toInt(&ok, 16);
                        }
                        line = stream.readLine();
                    }

                    // Attention! If tests are absent, we must create single simple test...
                    if(prot->tests.size() == 0)
                    {
                        p_Test = new rt_Test();
                        p_Test->header.ID_Test = "0";
                        p_Test->header.Name_Test = "NULL";
                        p_Test->header.Type_analysis = -1; //11111;
                        p_Test->header.Active_channel = 0xfffff;
                        prot->tests.push_back(p_Test);
                    }
                    //...
                    p_Test = prot->tests.at(0);
                    //qDebug() << "tests: " << prot->tests.size() << QString::fromStdString(p_Test->header.Name_Test);

                    continue;
            }


            //--- Information about Biocenose Parameters: ---------------------
            if(line.indexOf("$Biocenose Parameters:$") >= 0)
            {
                if(Callback) Callback(pobj,11,mess);
                line = stream.readLine();
                while(line.indexOf("$") < 0)
                {
                    if(line.trimmed() != "")
                    {
                        fields = line.trimmed().split(QRegExp("\\s+"));
                        id_str = fields.at(0).trimmed();
                        id = fields.at(0).toInt();
                        for(i=0; i<prot->tests.size(); i++)
                        {
                            p_Test = prot->tests.at(i);
                            if(id_str == QString::fromStdString(p_Test->header.ID_Test))
                            {
                                Load_BiocenoseChannel(p_Test, line);
                                break;
                            }
                        }
                    }
                    line = stream.readLine();
                }
                continue;
            }

            //--- Information about HLA_Package Names: ---------------------
            if(line.indexOf("$HLA_Package Names:$") >= 0)
            {
                if(Callback) Callback(pobj,12,mess);
                line = stream.readLine();
                while(line.indexOf("$") < 0)
                {
                    if(line.trimmed() != "")
                    {
                        fields = line.trimmed().split(QRegExp("\\t+"));
                        id_str = fields.at(0).trimmed();
                        id = fields.at(0).toInt();
                        for(i=0; i<prot->tests.size(); i++)
                        {
                            p_Test = prot->tests.at(i);
                            if(id_str == QString::fromStdString(p_Test->header.ID_Test))
                            {
                                Load_HLAChannel(p_Test, line);
                                break;
                            }
                        }
                    }
                    line = stream.readLine();
                }
                continue;
            }

            //--- Information about HPV_Package Names: ---------------------
            if(line.indexOf("$HPV_Package Names:$") >= 0)
            {
                if(Callback) Callback(pobj,13,mess);
                line = stream.readLine();
                while(line.indexOf("$") < 0)
                {
                    if(line.trimmed() != "")
                    {
                        fields = line.trimmed().split(QRegExp("\\t+"));
                        id_str = fields.at(0).trimmed();
                        id = fields.at(0).toInt();
                        for(i=0; i<prot->tests.size(); i++)
                        {
                            p_Test = prot->tests.at(i);
                            if(id_str == QString::fromStdString(p_Test->header.ID_Test))
                            {
                                Load_HLAChannel(p_Test, line);
                                break;
                            }
                        }
                    }
                    line = stream.readLine();
                }
                continue;
            }
            //--- Information about Qualitate_Package Names: ---------------------
            if(line.indexOf("$Qualitate_Package Names:$") >= 0)
            {
                if(Callback) Callback(pobj,14,mess);
                line = stream.readLine();
                while(line.indexOf("$") < 0)
                {
                    if(line.trimmed() != "")
                    {
                        fields = line.trimmed().split(QRegExp("\\t+"));
                        id_str = fields.at(0).trimmed();
                        id = fields.at(0).toInt();
                        for(i=0; i<prot->tests.size(); i++)
                        {
                            p_Test = prot->tests.at(i);
                            if(id_str == QString::fromStdString(p_Test->header.ID_Test))
                            {
                                Load_HLAChannel(p_Test, line);
                                break;
                            }
                        }
                    }
                    line = stream.readLine();
                }
                continue;
            }

            //--- Information about Gender_Package Names: ---------------------
            if(line.indexOf("$Gender_Package Names:$") >= 0)
            {
                if(Callback) Callback(pobj,15,mess);
                line = stream.readLine();
                while(line.indexOf("$") < 0)
                {
                    if(line.trimmed() != "")
                    {
                        fields = line.trimmed().split(QRegExp("\\t+"));
                        id_str = fields.at(0).trimmed();
                        id = fields.at(0).toInt();
                        for(i=0; i<prot->tests.size(); i++)
                        {
                            p_Test = prot->tests.at(i);
                            if(id_str == QString::fromStdString(p_Test->header.ID_Test))
                            {
                                Load_HLAChannel(p_Test, line);
                                break;
                            }
                        }
                    }
                    line = stream.readLine();
                }
                continue;
            }

            //--- Information about RHD_Package Names: ---------------------
            if(line.indexOf("$RHD_Package Names:$") >= 0)
            {
                if(Callback) Callback(pobj,16,mess);
                line = stream.readLine();
                while(line.indexOf("$") < 0)
                {
                    if(line.trimmed() != "")
                    {
                        fields = line.trimmed().split(QRegExp("\\t+"));
                        id_str = fields.at(0).trimmed();
                        id = fields.at(0).toInt();
                        for(i=0; i<prot->tests.size(); i++)
                        {
                            p_Test = prot->tests.at(i);
                            if(id_str == QString::fromStdString(p_Test->header.ID_Test))
                            {
                                Load_HLAChannel(p_Test, line);
                                break;
                            }
                        }
                    }
                    line = stream.readLine();
                }
                continue;
            }

            //--- Information about AZF_Package Names: ---------------------
            if(line.indexOf("$AZF_Package Names:$") >= 0)
            {
                if(Callback) Callback(pobj,17,mess);
                line = stream.readLine();
                while(line.indexOf("$") < 0)
                {
                    if(line.trimmed() != "")
                    {
                        fields = line.trimmed().split(QRegExp("\\t+"));
                        id_str = fields.at(0).trimmed();
                        id = fields.at(0).toInt();
                        for(i=0; i<prot->tests.size(); i++)
                        {
                            p_Test = prot->tests.at(i);
                            if(id_str == QString::fromStdString(p_Test->header.ID_Test))
                            {
                                Load_HLAChannel(p_Test, line);
                                break;
                            }
                        }
                    }
                    line = stream.readLine();
                }
                continue;
            }

            //--- Information about Cp_ALL_Package Names: ---------------------
            if(line.indexOf("$Cp_ALL_Package Names:$") >= 0)
            {
                if(Callback) Callback(pobj,18,mess);
                line = stream.readLine();
                while(line.indexOf("$") < 0)
                {
                    if(line.trimmed() != "")
                    {
                        fields = line.trimmed().split(QRegExp("\\t+"));
                        id_str = fields.at(0).trimmed();
                        id = fields.at(0).toInt();
                        for(i=0; i<prot->tests.size(); i++)
                        {
                            p_Test = prot->tests.at(i);
                            if(id_str == QString::fromStdString(p_Test->header.ID_Test))
                            {
                                Load_HLAChannel(p_Test, line);
                                break;
                            }
                        }
                    }
                    line = stream.readLine();
                }
                continue;
            }

            //--- Information about Cp_ALL_Package algorithms: ---------------------
            if(line.indexOf("$Cp_ALL_Package algorithms:$") >= 0)
            {
                if(Callback) Callback(pobj,19,mess);
                line = stream.readLine();
                while(line.indexOf("$") < 0)
                {
                    if(line.trimmed() != "")
                    {
                        fields = line.trimmed().split(QRegExp("\\t+"));
                        id_str = fields.at(0).trimmed();
                        id = fields.at(0).toInt();
                        for(i=0; i<prot->tests.size(); i++)
                        {
                            p_Test = prot->tests.at(i);
                            if(id_str == QString::fromStdString(p_Test->header.ID_Test) && fields.size() > 1)
                            {
                                text = fields.at(1);
                                p_PreferenceTest = new rt_Preference;
                                p_PreferenceTest->name = "FormID";
                                p_PreferenceTest->value = text.toStdString();
                                p_Test->preference_Test.push_back(p_PreferenceTest);

                                p_Test->header.comment = "DTReport";
                                break;
                            }
                        }
                    }
                    line = stream.readLine();
                }
                continue;
            }

            //--- Information about Cp_ALL_Package property: ---------------------
            if(line.indexOf("$Cp_ALL_Package property:$") >= 0)
            {
                if(Callback) Callback(pobj,20,mess);
                line = stream.readLine();
                while(line.indexOf("$") < 0)
                {
                    if(line.trimmed() != "")
                    {
                        fields = line.trimmed().split(QRegExp("\\t+"));
                        id_str = fields.at(0).trimmed();
                        id = fields.at(0).toInt();
                        for(i=0; i<prot->tests.size(); i++)
                        {
                            p_Test = prot->tests.at(i);
                            if(id_str == QString::fromStdString(p_Test->header.ID_Test) && fields.size() > 1)
                            {
                                text = fields.at(1);
                                p_PreferenceTest = new rt_Preference;
                                p_PreferenceTest->name = "ReportSettings";
                                p_PreferenceTest->value = text.toStdString();
                                p_Test->preference_Test.push_back(p_PreferenceTest);
                                break;
                            }
                        }
                    }
                    line = stream.readLine();
                }
                continue;
            }

            //--- Information about tubes -------------------------------------
            if(line.indexOf("Information about tubes:") >= 0)
            {
                if(Callback) Callback(pobj,20,mess);
                prot->active_Channels = 0x00;
                prot->count_Tubes = 0;
                prot->color_tube.clear();
                prot->enable_tube.clear();

                map.clear();
                line = stream.readLine();
                while(line.indexOf("$") < 0 && !stream.atEnd())
                {
                    if(line.trimmed() != "")
                    {
                        fields = line.trimmed().split(QRegExp("\\s+"));
                        key = (fields.at(4).toInt(&ok, 16) & 0xfff00) >> 8;
                        //qDebug() << key << line;
                        if(key > 0)
                        {
                            map.insert(key,line);       // sorted list for key
                        }

                        key = fields.at(1).toInt(&ok) & 0xff;   // enable tubes
                        prot->enable_tube.push_back(key);       //

                        if(key)
                        {
                            color = fields.at(5).toInt(&ok, 16);// color tubes
                            color.getRgb(&r,&g,&b);             //
                            key = QColor(b,g,r).rgb();          //
                            prot->color_tube.push_back(key);    //
                        }
                        else
                        {
                            color = QColor(Qt::gray);
                            key = color.rgb();
                            prot->color_tube.push_back(key);
                        }

                        prot->count_Tubes = fields.at(0).toInt();
                    }
                    line = stream.readLine();
                }
                prot->count_Tubes++;

                //qDebug() << "prot->count_Tubes: " << prot->count_Tubes;


                for(it_map = map.begin(); it_map != map.end(); ++it_map)
                {
                    line = it_map.value();
                    fields = line.trimmed().split(QRegExp("\\s+"));

                    if(fields.at(6) == "-1") fields.replace(6, "0");

                    //... Groups ..............................................
                    text = fields.at(7);
                    if(hash.contains(text)) p_Group = hash.value(text);
                    else
                    {
                      p_Group = new rt_GroupSamples();
                      p_Group->Unique_NameGroup = text.toStdString();
                      p_Group->ID_Group = QString::number(hash.count()).toStdString();
                      hash.insert(text, p_Group);
                      prot->Plate.groups.push_back(p_Group);
                    }

                    //... Samples .............................................
                    k = fields.at(4).toInt(&ok,16) & 0x01;
                    if(it_map == map.begin() || k != new_sample)
                    {
                        id_sample++;
                        p_Sample = new rt_Sample();
                        p_Group->samples.push_back(p_Sample);
                        p_Sample->ID_Sample = QString::number(id_sample).toStdString();
                        p_Sample->Unique_NameSample = fields.at(7).trimmed().toStdString();
                        p_Sample->ID_Test = fields.at(6).trimmed().toStdString();
                        p_Sample->p_Test = prot->tests.at(QString::fromStdString(p_Sample->ID_Test).toInt());

                        id_tube = 0;
                    }
                    new_sample = k;

                    //... Tubes ...............................................
                    p_Tube = new rt_Tube();
                    p_Sample->tubes.push_back(p_Tube);
                    p_Tube->ID_Tube = id_tube;
                    p_Tube->Unique_NameTube = QString::number(id_tube++).toStdString();
                    p_Tube->pos = fields.at(0).toInt();
                    color = fields.at(5).toInt(&ok, 16);
                    color.getRgb(&r,&g,&b);
                    p_Tube->color = QColor(b,g,r).rgb();

                    //... Channels ............................................
                    active_ch = fields.at(2).toInt(&ok, 16);

                    ch = 0;
                    if(fields.size() > 8) ch = fields.at(8).toInt(&ok, 16);
                    num_ch = 0;

                    switch(p_Sample->p_Test->header.Type_analysis)
                    {

                    case 0x2000:                            // Biocenose

                        for(i=0; i<MAX_CHANNELS; i++)
                        {
                            if(i > 3) j = 0;
                            else j = ch & (0xff << i*8);

                            if(j)
                            {
                                p_Channel = new rt_Channel();
                                p_Channel->ID_Channel = i;
                                p_Tube->channels.push_back(p_Channel);
                                p_TubeTest = p_Sample->p_Test->tubes.at(p_Tube->ID_Tube);
                                p_ChannelTest = p_TubeTest->channels.at(num_ch);
                                p_Channel->Unique_NameChannel = p_ChannelTest->name;

                                num_ch++;
                            }
                        }
                        break;


                    case 0x0400:                            // HLA_DRB
                    case 0x0410:                            // HLA_DQ
                    case 0x4000:                            // HPV
                    case 0x3000:                            // q+
                    case 0x0600:                            // RHD
                    case 0x0700:                            // Gender
                    case 0x5000:                            // AZF

                    case 0x20000:                           // Cp_ALL

                        if(ch == 0) ch++;
                        p_TubeTest = p_Sample->p_Test->tubes.at(ch-1);
                        for(i=0; i<p_TubeTest->channels.size(); i++)
                        {
                            p_ChannelTest = p_TubeTest->channels.at(i);
                            p_Channel = new rt_Channel();
                            p_Tube->channels.push_back(p_Channel);
                            p_Channel->ID_Channel = p_ChannelTest->ID_Channel;
                            p_Channel->Unique_NameChannel = p_ChannelTest->name;
                        }

                        break;

                    default:                                // Other tests

                        for(i=0; i<MAX_CHANNELS; i++)
                        {
                            j = active_ch & (0xf << i*4);
                            if(j)
                            {
                                p_Channel = new rt_Channel();
                                p_Tube->channels.push_back(p_Channel);
                                p_Channel->ID_Channel = i;
                                p_Channel->Unique_NameChannel = fluor_name[i].toStdString();
                            }
                        }
                        break;

                    }

                    //... Active Channels .....................................
                    prot->active_Channels |= active_ch;

                    //... active Tests ........................................
                    k = fields.at(6).toInt();
                    if(!active_tests.contains(k)) active_tests.append(k);

                    //qDebug() << "map: (i) " << line;
                }
                //qDebug() << "map: ok";

                //... necessary to delete inactive tests ......................
                k = prot->tests.size() - 1;
                for(i=k; i>=0; i--)
                {
                    p_Test = prot->tests.at(i);
                    if(!active_tests.contains(QString::fromStdString(p_Test->header.ID_Test).toInt()))
                    {
                        delete prot->tests.at(i);
                        prot->tests.erase(prot->tests.begin()+i);
                    }
                }

                //...
                active_tests.clear();
                for(i=0; i<prot->Plate.groups.size(); i++)
                {
                    p_Group = prot->Plate.groups.at(i);
                    for(j=0; j<p_Group->samples.size(); j++)
                    {
                        p_Sample = p_Group->samples.at(j);

                        for(k=0; k<prot->tests.size(); k++)
                        {
                            p_Test = prot->tests.at(k);
                            if(p_Test->header.ID_Test == p_Sample->ID_Test)
                            {
                                if(p_Test->tubes.size() == 0)
                                {
                                    for(l=0; l<p_Sample->tubes.size(); l++)
                                    {
                                        p_Tube = p_Sample->tubes.at(l);

                                        p_TubeTest = new rt_TubeTest();
                                        p_TubeTest->ID_Tube = l;
                                        p_Test->tubes.push_back(p_TubeTest);
                                        for(m=0; m<p_Tube->channels.size(); m++)
                                        {
                                            p_Channel = p_Tube->channels.at(m);
                                            p_ChannelTest = new rt_ChannelTest();
                                            p_TubeTest->channels.push_back(p_ChannelTest);
                                            p_ChannelTest->ID_Channel = p_Channel->ID_Channel;
                                            p_ChannelTest->name = p_Channel->Unique_NameChannel;
                                        }
                                    }
                                }
                                break;
                            }
                        }
                    }
                }
                //.............................................................

                continue;
            }

            //--- Device ------------------------------------------------------
            if(line.indexOf("$Device RT322:$") >= 0)
            {

                text = line.mid(15).trimmed();
                text.replace("(","");
                text.replace(")","");
                //qDebug() << "ser name: " << text;
                if(text.trimmed().isEmpty()) text = "...";
                prot->SerialName = text.toStdString();
                line = stream.readLine();

                continue;
            }

            //--- CrossTalk_Spectral ------------------------------------------


            if(line.indexOf("$CrossTalk_Spectral:$") >= 0)
            {

                prot->COEFF_Spectral.clear();
                line = stream.readLine();
                while(line.indexOf("$") < 0)
                {
                    if(line.trimmed() != "")
                    {
                        prot->COEFF_Spectral.push_back(line.toStdString());
                    }
                    line = stream.readLine();
                }
                continue;
            }

            //--- CrossTalk_UnequalChannel ------------------------------------

            if(line.indexOf("$CrossTalk_UnequalChannel:$") >= 0)
            {
                prot->COEFF_UnequalCh.clear();
                line = stream.readLine();
                while(line.indexOf("$") < 0)
                {
                    if(line.trimmed() != "")
                    {
                        prot->COEFF_UnequalCh.push_back(line.toStdString());
                    }
                    line = stream.readLine();
                }
                continue;
            }

            //--- CrossTalk_Optic ---------------------------------------------

            if(line.indexOf("$CrossTalk_Optic:$") >= 0)
            {
                prot->COEFF_Optic.clear();
                line = stream.readLine();
                while(line.indexOf("$") < 0)
                {
                    if(line.trimmed() != "")
                    {
                        prot->COEFF_Optic.push_back(line.toStdString());
                    }
                    line = stream.readLine();
                }
                continue;
            }

            //--- Results of optical measurements -----------------------------
            if(line.indexOf("Results of optical measurements:") >= 0)
            {
                if(Callback) Callback(pobj,50,mess);
                //... reserve measurements ....................................
                int count_ActiveCh = 0;
                for(i=0; i<MAX_CHANNELS; i++)
                {
                    if(prot->active_Channels & (0x0f << i*4)) count_ActiveCh++;
                }
                prot->meas.reserve(count_ActiveCh*2*(prot->count_PCR + prot->count_MC));
                //.............................................................

                do
                {
                    line = stream.readLine();
                    if(line.trimmed().length() == 0) continue;

                    fields = line.trimmed().split(QRegExp("\\s+"));
                    col = fields.count();

                    //... Only active channels ................................
                    k = fields[2].toInt(&ok,16) & 0x03;
                    k |= (fields[2].toInt(&ok,16) & 0x100) >> 6;
                    k = prot->active_Channels & (0x0f << k*4);
                    if(k == 0) continue;
                    //.........................................................

                    p_Measurement = new rt_Measurement();
                    prot->meas.push_back(p_Measurement);

                    p_Measurement->measurements.reserve(prot->count_Tubes);

                    for(i=0; i<col; i++)
                    {
                        value = fields[i].toInt(&ok,10);
                        if(i == 2) value = fields[i].toInt(&ok,16);

                        switch(i)
                        {
                        case 0: break;
                        case 1: p_Measurement->fn = div(value,5*2).quot;            break;
                        case 2: p_Measurement->type_meas = (value & 0x0c) >> 2;
                                p_Measurement->optical_channel = (value & 0x03);
                                p_Measurement->optical_channel |= (value & 0x100) >> 6;
                                p_Measurement->num_exp = (value & 0x60) >> 5;
                                                                                    break;
                        case 3: p_Measurement->block_number = value;                break;
                        case 4: p_Measurement->cycle_rep = value;                   break;
                        case 5: p_Measurement->exp_value = value;                   break;
                        case 6: p_Measurement->blk_exp = value;                     break;

                        default:    p_Measurement->measurements.push_back(value);   break;
                        }
                    }
                }
                while(!stream.atEnd());

                break;
            }

            line = stream.readLine();
        }

        file.close();
        if(Callback) Callback(pobj,0,"");

        //... Clear containers:
        map.clear();
        hash.clear();
        active_tests.clear();
        fields.clear();
    }

    return(res);
}


//-----------------------------------------------------------------------------
//--- Transpose RawData
//-----------------------------------------------------------------------------
int Transpose_RawData(void *pobj, callback_process* Callback, rt_Protocol *prot, QString mess)
{
    int i,j,k,m,id;
    bool saturation;
    short value, current_expo;
    double value_expo, expo_0, coef_expo, dvalue;    
    int percent;
    QString text;
    int mc_real;

    rt_Measurement *p_meas;

    QVector<short> value_pcr, value_mc;
    QVector<double> temp_mc;


    int count_ch = 0;                                           // count active channels
    for(i=0; i<MAX_CHANNELS; i++)
    {
        if(prot->active_Channels & (0x0f << i*4)) count_ch++;
    }
    int count_simple = 2;                                       // count exposition
    int count_tube = prot->count_Tubes;                         // count tubes
    int count_meas = div(prot->meas.size(), count_simple * count_ch).quot; // count meas

    //qDebug() << "tubes = " << count_tube;

    prot->PCR_RawData.clear();
    prot->X_PCR.clear();
    prot->MC_RawData.clear();
    prot->X_MC.clear();

    prot->PCR_RawData.reserve(count_meas*count_tube);
    prot->X_PCR.reserve(count_meas);
    prot->MC_RawData.reserve(count_meas*count_tube);
    prot->X_MC.reserve(count_meas);
    value_pcr.reserve(count_meas);
    value_mc.reserve(count_meas);


    //... Cycle for count active channels .....................................
    for(i=0; i<count_ch; i++)
    {
        if(Callback && count_ch > 0)
        {
            percent = (i+1)*100/count_ch;
            text = "Ch_" + QString::number(i) + " - " + mess;   //QObject::tr("Transpose_RawData");
            Callback(pobj,percent,text);
        }

        //... Cycle for count tubes ...........................................
        for(j=0; j<count_tube; j++)
        {
            //... Cycle for measurements ......................................
            current_expo = -1;

            for(m=0; m<count_simple; m++)
            {
                current_expo++;
                saturation = false;
                value_pcr.clear();
                value_mc.clear();                

                for(k=0; k<count_meas; k++)
                {
                    id = i*count_simple + k*count_simple*count_ch + current_expo;
                    p_meas = prot->meas.at(id);

                    if(k==0)
                    {
                        value_expo = p_meas->exp_value;
                        if(current_expo == 0) expo_0 = p_meas->exp_value;
                        if(value_expo > 0 && expo_0 > 0) coef_expo = expo_0/value_expo;
                        else coef_expo = 1.;
                    }

                    value = p_meas->measurements.at(j);
                    if(value & 0x8000) saturation = true;

                    if(saturation && current_expo < count_simple-1) break;

                    value &= 0xfff;
                    value -= p_meas->blk_exp;
                    dvalue = value * coef_expo;

                    switch(p_meas->type_meas)
                    {
                    case 1: value_pcr.append(dvalue);   break;
                    case 2: value_mc.append(dvalue);    break;
                    default:    break;
                    }
                }
                if(!saturation) break;
            }

            for(k=0; k<value_pcr.size(); k++) prot->PCR_RawData.push_back(value_pcr.at(k));

            if(prot->dT_mc < 0) // for negative dT_mc
            {
                std::reverse(value_mc.begin(), value_mc.end());
            }
            for(k=0; k<value_mc.size(); k++) prot->MC_RawData.push_back(value_mc.at(k));


            //qDebug() << "j=" << j << value_pcr.size() << prot->PCR_RawData.size();

            //if(Callback) Callback(pobj,j,"Transpose_RawData");
        }
    }

    prot->count_PCR = div(prot->PCR_RawData.size(), count_ch*count_tube).quot;
    prot->count_MC = div(prot->MC_RawData.size(), count_ch*count_tube).quot;

    //... X_PCR ...............................................................
    for(i=0; i<prot->count_PCR; i++) prot->X_PCR.push_back(i+1);

    //... X_MC ................................................................
    for(i=0; i<prot->count_MC; i++) prot->X_MC.push_back(prot->T_initial + prot->dT_mc*i);

    // for negative dT_mc: ...
    if(prot->dT_mc < 0)
    {
       prot->dT_mc = qAbs(prot->dT_mc);
       prot->T_initial -= prot->dT_mc * prot->count_MC;
       std::reverse(prot->X_MC.begin(), prot->X_MC.end());
    }
    //qDebug() << "MC: " << prot->T_initial << prot->dT_mc << QVector<double>::fromStdVector(prot->X_MC);
    // ...


    if(Callback) Callback(pobj,0,"");
    //.........................................................................

    if((prot->count_PCR > 0 && prot->count_PCR < 2) || (prot->count_MC > 0 && prot->count_MC < 5)) return(-1);

    return(0);
}

//-----------------------------------------------------------------------------
//--- Filtered RawData
//-----------------------------------------------------------------------------
int Filtered_RawData(void *pobj, callback_process* Callback, rt_Protocol *prot, QString mess)
{
    int i,j;
    int percent;
    QString text;
    int count;
    int sts;
    bool sts_filter = true;
    rt_Preference *preference;
    double rho = 30;
    bool ok;

    int num = 3;

    //qDebug() << "Filtered_RawData Start: " << QDateTime::currentDateTime().toString("h:mm:ss:zzz ");

    for(i=0; i<prot->preference_Pro.size(); i++)
    {
        preference = prot->preference_Pro.at(i);
        if(preference->name == USE_DIGITFILTER) {if(preference->value != "yes") sts_filter = false; break;}
        if(preference->name == PARAM_SPLINECUBE)
        {
            text = QString::fromStdString(preference->value);
            rho = text.toDouble(&ok);
            if(!ok) rho = 30;
        }
    }
    rho = -5 + 3.*rho/100.;
    rho = pow(10., rho);


    prot->PCR_Filtered.clear();
    prot->PCR_Filtered.reserve(prot->PCR_RawData.size());

    QVector<double> y;
    QVector<double> x,s,ds,d2s;

    for(i=0; i<prot->count_PCR + 2*num; i++) x.append(i);

    if(prot->count_PCR > 0) count = div(prot->PCR_RawData.size(), prot->count_PCR).quot;
    else count = 0;

    //... Cycle for count (active channels * count tubes ......................
    for(i=0; i<count; i++)
    {
        if(Callback && count > 0)
        {
            percent = (i+1)*100/count;
            text = QString::number(percent) + "% - " + mess;    //QObject::tr("Filtered PCR_RawData");
            Callback(pobj,percent,text);
        }

        s.fill(0,prot->count_PCR + 2*num);
        ds.fill(0,prot->count_PCR + 2*num);
        d2s.fill(0,prot->count_PCR + 2*num);

        y = QVector<double>::fromStdVector(prot->PCR_RawData).mid(i*prot->count_PCR, prot->count_PCR);

        if(sts_filter) FilterData_ByMedian(y, 2);
        //rho = 0.00002;
        //...
        for(j=0; j<num; j++)  y.push_back(y.last());
        for(j=0; j<num; j++)  y.push_front(y.first());
        //...

        SplineCube(x, y, s, ds, d2s, sts, rho);

        //...
        s.remove(0,num);
        s.remove(prot->count_PCR, num);
        ds.remove(0,num);
        ds.remove(prot->count_PCR, num);
        d2s.remove(0,num);
        d2s.remove(prot->count_PCR, num);
        //...

        for(j=0; j<prot->count_PCR; j++) prot->PCR_Filtered.push_back(s.at(j));
    }

    //qDebug() << "Filtered_RawData Stop: " << QDateTime::currentDateTime().toString("h:mm:ss:zzz ");

    //.........................................................................
    if(Callback) Callback(pobj,0,"");
    //.........................................................................

    return(0);
}

//-----------------------------------------------------------------------------
//--- Criterion_Structure
//-----------------------------------------------------------------------------
void Criterion_Structure(rt_Protocol *prot)
{
    int i,j,m;
    bool ok;
    int pos;

    //QMap<rt_Test*, Criterion_PCR*> *Criterion = (QMap<rt_Test*, Criterion_PCR*>*)cri;
    //QVector<rt_Test*> *PCR_Tests = (QVector<rt_Test*>*)p;

    Criterion_Test *criterion;
    rt_GroupSamples *group;
    rt_Sample *sample;
    rt_Tube *tube;
    rt_Test *ptest;
    rt_Preference *preference;

    prot->Criterion.push_back(new Criterion_Test());    // add item for disable tubes

    for(i=0; i<prot->tests.size(); i++)
    {
        ptest = prot->tests.at(i);
        criterion = new Criterion_Test();
        criterion->ptest = ptest;

        for(j=0; j<ptest->preference_Test.size(); j++)
        {
            preference = ptest->preference_Test.at(j);

            if(preference->name == CRITERION_POSRES) criterion->criterion_PosResult = QString::fromStdString(preference->value).toInt(&ok)/10.;
            if(preference->name == CRITERION_VALIDITY) criterion->criterion_Validity = QString::fromStdString(preference->value).toInt(&ok);
            if(preference->name == USE_AFF)
            {                
                if(preference->value == "no" || preference->value == "False") criterion->use_AFF = false;
                else criterion->use_AFF = true;
            }
            if(preference->name == CRITERION_AFF) criterion->criterion_AFF = QString::fromStdString(preference->value).toInt(&ok);
            if(preference->name == CRITERION_AFF_MIN) criterion->criterion_AFF_MIN = QString::fromStdString(preference->value).toInt(&ok);
            if(preference->name == CRITERION_AFF_MAX) criterion->criterion_AFF_MAX = QString::fromStdString(preference->value).toInt(&ok);
            if(preference->name == SIGMOIDVAL_MIN) criterion->sigmoid_ValidityMin = QString::fromStdString(preference->value).toInt(&ok);
            if(preference->name == SIGMOIDVAL_MAX) criterion->sigmoid_ValidityMax = QString::fromStdString(preference->value).toInt(&ok);
            if(preference->name == USE_THRESHOLD)
            {
                if(preference->value == "no" || preference->value == "False") criterion->use_threshold = false;
                else criterion->use_threshold = true;
            }
            if(preference->name == VALUE_THRESHOLD) criterion->value_threshold = QString::fromStdString(preference->value).toInt(&ok);
            if(preference->name == EXPOSURE) criterion->exposure = preference->value;
            if(preference->name == MC_FWHM_BORDER) criterion->fwhm_border = QString::fromStdString(preference->value).toInt(&ok);
            if(preference->name == MC_PEAKS_BORDER) criterion->peaks_border = QString::fromStdString(preference->value).toInt(&ok);

        }

        prot->Criterion.push_back(criterion);
    }

    // Plate_Tests
    prot->Plate_Tests.resize(prot->count_Tubes, NULL);

    for(i=0; i<prot->Plate.groups.size(); i++)
    {
        group = prot->Plate.groups.at(i);
        for(j=0; j<group->samples.size(); j++)
        {
            sample = group->samples.at(j);
            ptest = sample->p_Test;
            for(m=0; m<sample->tubes.size(); m++)
            {
                tube = sample->tubes.at(m);
                pos = tube->pos;
                prot->Plate_Tests.at(pos) = ptest;
            }
        }
    }
}

//-----------------------------------------------------------------------------
//--- Bace anf Fitted Data
//-----------------------------------------------------------------------------
int Bace_Data(void *pobj, callback_process* Callback, rt_Protocol *prot, QString mess)
{
    int i,j,k=0,m;
    int percent;
    QString text;    
    POINT_TAKEOFF p_OFF;
    POINT_TAKEOFF *pp_OFF;
    SPLINE_CP     *p_SplineCp;    
    QPointF P;
    short value_ff;
    double threshold_Coef = 10.;    // 10 rms
    QVector<double> threshold_test;

    QVector<double> Norm_Cp,Norm_Ct;
    QVector<double> Temp;
    double dval_cp, dval_ct, norm_cp, norm_ct;
    double val_cp, val_ct;

    //qDebug() << "Bace_Data Start: " << QDateTime::currentDateTime().toString("h:mm:ss:zzz ");

    // Load Criterion of Tests
    Criterion_Test *criterion;
    rt_Test *ptest;
    //QMap<rt_Test*, Criterion_PCR*> Criterion;
    //QVector<rt_Test*> PCR_Tests(prot->count_Tubes);

    Criterion_Structure(prot);
    for(i=0; i<prot->Criterion.size(); i++)
    {
        criterion = prot->Criterion.at(i);
        threshold_test.append((double)criterion->value_threshold);
    }
    if(threshold_test.size())
    {
        threshold_Coef = qMax(threshold_Coef, *std::max_element(threshold_test.begin(),threshold_test.end()));
    }
    //...

    for(i=0; i<prot->list_PointsOff.size(); i++)
    {
       pp_OFF = prot->list_PointsOff.at(i);
       delete pp_OFF;
    }
    prot->list_PointsOff.clear();

    for(i=0; i<prot->list_SplineCp.size(); i++)
    {
       p_SplineCp = prot->list_SplineCp.at(i);
       delete p_SplineCp;
    }
    prot->list_SplineCp.clear();

    prot->PCR_Bace.clear();
    prot->PCR_Bace.reserve(prot->PCR_RawData.size());
    QVector<double> y,s;
    QVector<double> threshold;
    QVector<double> Temp_CpSpline;

    if(prot->count_PCR  == 0) return(-1);

    int count = div(prot->PCR_RawData.size(), prot->count_PCR).quot;

    prot->PCR_FinishFlash.clear();
    prot->PCR_FinishFlash.reserve(count);

    prot->NormCp_Value.clear();
    prot->NormCt_Value.clear();

    Temp_CpSpline.reserve(count);

    //qDebug() << "Raw, Filtered: " << QVector<double>::fromStdVector(prot->PCR_Filtered).mid(0, prot->count_PCR);

    //qDebug() << "Start Bace&Fitting";
    //... Cycle for count (active channels * count tubes ......................
    for(i=0; i<count; i++)
    {
        if(Callback && count > 0)
        {
            percent = (i+1)*100/count;
            text = QString::number(percent) + "% - " + mess;    //QObject::tr("Bace&Fitting Data");
            Callback(pobj,percent,text);
        }

        ptest = prot->Plate_Tests.at(div(i,prot->count_Tubes).rem);
        /*for(j=0; j<prot->Criterion.size(); j++)
        {
            criterion = prot->Criterion.at(j);
            if(criterion->ptest == ptest) break;
        }*/

        y.clear();
        s.clear();
        y.reserve(prot->count_PCR);
        s.reserve(prot->count_PCR);
        y = QVector<double>::fromStdVector(prot->PCR_Filtered).mid(i*prot->count_PCR, prot->count_PCR);

        //qDebug() << "start" << criterion->criterion_PosResult;
        p_OFF = Analysis_PCR_Ex(y,s,NULL, criterion->criterion_PosResult);
        //qDebug() << "stop";

        pp_OFF = new POINT_TAKEOFF;
        *pp_OFF = p_OFF;

        prot->list_PointsOff.push_back(pp_OFF);        

        for(j=0; j<prot->count_PCR; j++) prot->PCR_Bace.push_back(s.at(j));


        //--- Spline Fitting (alternative)

        Temp_CpSpline.clear();

        //Temp_CpSpline = QVector<double>::fromStdVector(prot->PCR_Bace).mid(i*prot->count_PCR, prot->count_PCR);

        //for(j=0; j<prot->count_PCR; j++) Temp_CpSpline.append(prot->PCR_Bace.at(i*prot->count_PCR+j));
        //qDebug() << "Temp_CpSpline: " << Temp_CpSpline;       

        //P = CP_SplineFitting(Temp_CpSpline);
        P.setX(1.);
        P.setY(1.);

        p_SplineCp = new SPLINE_CP;
        p_SplineCp->Cp = P.x();
        p_SplineCp->F_Cp = P.y();
        prot->list_SplineCp.push_back(p_SplineCp);        


        //--- analysis of finish flash ---        
        value_ff = Analysis_FinishFlash(y);
        prot->PCR_FinishFlash.push_back(value_ff);        
        //qDebug() << "analysis of finish flash: " << i << value_ff << y;

        //--- threshold ---        
        if(p_OFF.valid) threshold.push_back(p_OFF.sigma);
        if(div(i+1,prot->count_Tubes).rem == 0)
        {
            qSort(threshold);            
            if(threshold.size()) prot->Threshold_Ct.push_back(threshold.last()*threshold_Coef);
            else prot->Threshold_Ct.push_back(1.);
            threshold.clear();
            Calculate_Ct(prot,k);
            k++;

            //qDebug() << "threshold = " << prot->Threshold_Ct.at(k-1);
        }

        //--- normalization ---
        Temp.clear();
        Temp = QVector<double>::fromStdVector(prot->PCR_Bace).mid(prot->PCR_Bace.size()-3-1, 3);
        dval_ct = std::accumulate(Temp.begin(), Temp.end(), 0.)/Temp.size();
        Norm_Ct.append(dval_ct);

        Norm_Cp.append(p_OFF.Fluor_Cp);        
    }
    //qDebug() << "Stop Bace&Fitting";

    for(i=0; i<prot->PCR_Bace.size(); i++) prot->PCR_Bace.at(i) += 1.;

    prot->NormCp_Value.reserve(prot->PCR_Bace.size());
    prot->NormCt_Value.reserve(prot->PCR_Bace.size());
    norm_cp = *std::max_element(Norm_Cp.begin(), Norm_Cp.end());
    norm_ct = *std::max_element(Norm_Ct.begin(), Norm_Ct.end());
    k = 0;
    for(i=0; i<count; i++)
    {
        if(Norm_Cp.at(i) <= 0. || norm_cp <= 0.) dval_cp = 0.;
        else dval_cp = norm_cp/Norm_Cp.at(i);
        if(Norm_Ct.at(i) <= 0. || norm_ct <= 0.) dval_ct = 0.;
        else dval_ct = norm_ct/Norm_Ct.at(i);

        for(j=0; j<prot->count_PCR; j++)
        {
            val_cp = prot->PCR_Bace.at(k) * dval_cp;
            if(val_cp <= 0.) val_cp = 1.;
            val_ct = prot->PCR_Bace.at(k) * dval_ct;
            if(val_ct <= 0.) val_ct = 1.;
            prot->NormCp_Value.push_back(val_cp);
            prot->NormCt_Value.push_back(val_ct);
            k++;
        }
    }
    Norm_Ct.clear();
    Norm_Cp.clear();


    //.........................................................................
    if(Callback) Callback(pobj,0,"");
    //.........................................................................

    //qDebug() << "Bace_Data Stop: " << QDateTime::currentDateTime().toString("h:mm:ss:zzz ");

    return(0);

}

//-----------------------------------------------------------------------------
//--- dF/dT_MC Data
//-----------------------------------------------------------------------------
int dFdT_MCData(void *pobj, callback_process* Callback, rt_Protocol *prot, QString mess, bool negative)
{
    int i,j;
    int percent;
    QString text;
    QVector<double> x,y,s,dy,d2y,x_temp;
    int sts;
    int count;
    bool sts_filter = true;
    rt_Preference *preference;
    double rho = 30;
    double rho_plus = rho + 1.;
    bool ok;
    int len;
    double min,max;

    int num = 5;

    //qDebug() << "dFdT_MCData Start: " << QDateTime::currentDateTime().toString("h:mm:ss:zzz ");

    for(i=0; i<prot->preference_Pro.size(); i++)
    {
        preference = prot->preference_Pro.at(i);
        if(preference->name == USE_DIGITFILTER) {if(preference->value != "yes") sts_filter = false; continue;}
        if(preference->name == PARAM_SPLINECUBE)
        {
            text = QString::fromStdString(preference->value);
            rho = text.toDouble(&ok);
            if(!ok) rho = 30;
        }
    }
    rho = -5 + 3.*rho/100.;
    rho = pow(10., rho);

    //qDebug() << "rho: " << rho;


    QVector<double> Two_peaks_X(2);
    QVector<double> Two_peaks_Y(2);

    prot->MC_dF_dT.clear();
    prot->MC_TPeaks.clear();
    prot->MC_TPeaks_Y.clear();
    prot->MC_Filtered.clear();
    prot->MC_dF_dT.reserve(prot->MC_RawData.size());
    prot->MC_Filtered.reserve(prot->MC_RawData.size());
    prot->MC_TPeaks.reserve(prot->MC_RawData.size()*2);
    prot->MC_TPeaks_Y.reserve(prot->MC_RawData.size()*2);

    if(prot->count_MC > 0) count = div(prot->MC_RawData.size(), prot->count_MC).quot;
    else count = 0;

    for(i=0; i<prot->count_MC + 2*num; i++) x.append(prot->T_initial + (i-num)*prot->dT_mc);
    x_temp = x.mid(num, prot->count_MC);

    //... Cycle for count (active channels * count tubes ......................
    for(i=0; i<count; i++)
    {
        if(Callback && count > 0)
        {
            percent = (i+1)*100/count;
            text = QString::number(percent) + "% - " + mess;    //QObject::tr("Filtered & dF/Dt Melting Curve Data");
            Callback(pobj,percent,text);
        }
        s.fill(0.,prot->count_MC + 2*num);
        dy.fill(0.,prot->count_MC + 2*num);
        d2y.fill(0.,prot->count_MC + 2*num);
        Two_peaks_X.fill(0.,2);
        Two_peaks_Y.fill(0.,2);

        y = QVector<double>::fromStdVector(prot->MC_RawData).mid(i*prot->count_MC, prot->count_MC);

        if(sts_filter) FilterData_ByMedian(y, 2);

        //...
        for(j=0; j<num; j++)  y.push_back(y.at(prot->count_MC-1));
        for(j=0; j<num; j++)  y.push_front(y.at(0));
        //...

        SplineCube(x, y, s, dy, d2y, sts, rho);

        //qDebug() << "x,y: " << x.size() << x << dy;

        //...
        s.remove(0,num);
        s.remove(prot->count_MC, num);
        dy.remove(0,num);
        dy.remove(prot->count_MC, num);
        d2y.remove(0,num);
        d2y.remove(prot->count_MC, num);
        //...


        //len = y.size() - 1;
        //for(j=0; j<5; j++) {s.replace(j, y.at(j)); s.replace(len-j, y.at(len-j));}      // краевой эффект
        //for(j=0; j<5; j++) {dy.replace(j, dy.at(5)); dy.replace(len-j, dy.at(len-5));}



        for(j=0; j<prot->count_MC; j++) prot->MC_Filtered.push_back(s.at(j));

        if(negative)
        {
            for(j=0; j<s.size(); j++) dy.replace(j, dy.at(j)*(-1.));
            for(j=0; j<dy.size(); j++) d2y.replace(j, d2y.at(j)*(-1.));
        }


        //...
        /*y = dy;
        FilterData_ByMedian(y, 1);
        SplineCube(x, y, dy, d2y, s, sts, rho_plus);
        */


        Find_TemperaturePeaks(Two_peaks_X, Two_peaks_Y, dy, d2y, x_temp);

        //qDebug() << "Two_peaks_X: " << Two_peaks_X;

        for(j=0; j<prot->count_MC; j++) prot->MC_dF_dT.push_back(dy.at(j));
        for(j=0; j<2; j++) prot->MC_TPeaks.push_back(Two_peaks_X.at(j));
        for(j=0; j<2; j++) prot->MC_TPeaks_Y.push_back(Two_peaks_Y.at(j));        
    }

    if(Callback) Callback(pobj,0,"");

    //qDebug() << "dFdT_MCData Stop: " << QDateTime::currentDateTime().toString("h:mm:ss:zzz ");


    return(0);
}
//-----------------------------------------------------------------------------
//--- Calculate_Ct
//-----------------------------------------------------------------------------
void Calculate_Ct(rt_Protocol *prot, int ch)
{
    int i;
    POINT_TAKEOFF *pp_OFF;
    QVector<double> y;

    int count_tb = prot->count_Tubes;
    int count = prot->list_PointsOff.size();
    int count_meas = prot->count_PCR;
    double threshold = prot->Threshold_Ct.at(ch);

    if(div(count,count_tb).quot < ch) return;       // error situation

    for(i=0; i<count_tb; i++)
    {
        pp_OFF = prot->list_PointsOff.at(i + count_tb*ch);
        if(pp_OFF->valid)
        {
            y = QVector<double>::fromStdVector(prot->PCR_Bace).mid((i + count_tb*ch)*count_meas, count_meas);
            pp_OFF->real_ct = Threshold_Curve(y,threshold);
        }
        else pp_OFF->real_ct = 0.;

    }
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Check(void *pobj, callback_process* Callback)
{
    int i=0,j;
    int percent, count_tube = 100;
    QString text;



    for(j=0; j<100; j++)
    {
    if(Callback)
    {
        percent = j*100/(count_tube-1);
        text = "Ch_" + QString::number(i) + ": (" +
               QString::number(j) + ") - Transpose_RawData";
        Callback(pobj,percent,text);
    }
    }

    if(Callback) Callback(pobj,0,"ok");
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
QString Convert_IndexToName(int pos, int col)
{
    QString text;

    text = QChar(0x41 + div(pos,col).quot);
    text += QString::number(div(pos,col).rem + 1);

    return(text);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
int Convert_NameToIndex(QString name, int col)
{
    QChar a = name.at(0);
    int ascii = a.toLatin1();
    QString text = name.mid(1);
    int num = text.toInt();
    int pos = 0;

    if(ascii < 0x41 || ascii > 0x41+16) return(0);

    pos = num + (ascii-0x41)*col - 1;

    return(pos);
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Load_HLAChannel(rt_Test *p_test, QString line)
{
    int i,j, k;
    QStringList fields, names;
    QString text;
    int id = -1;
    int count;
    bool ok;

    rt_TubeTest     *tube_test;
    rt_ChannelTest  *channel_test;

    if(line.length() == 0) return;

    fields = line.trimmed().split(QRegExp("\\t+"));
    if(fields.size() < 3) return;

    id = fields.at(1).toInt();
    if(id < 0) return;

    count = id;

    text = fields.at(2);
    names = text.trimmed().split(QRegExp("~~"));

    for(i=0; i<count; i++)
    {
        tube_test = new rt_TubeTest();
        tube_test->ID_Tube = i;
        p_test->tubes.push_back(tube_test);

        fields.clear();
        fields = names.at(i).split(";");

        for(j=0; j<4; j++)
        {
           if(fields.size() <= j) break;
           text = fields.at(j).trimmed();
           if(text == "-") continue;
           channel_test = new rt_ChannelTest();
           channel_test->ID_Channel = j;
           tube_test->channels.push_back(channel_test);
           channel_test->name = text.toStdString();
        }
    }
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Load_BiocenoseChannel(rt_Test *p_test, QString line)
{
    int i,j, k;
    QStringList fields, names;
    QString text;
    int id = -1;
    int count;
    bool ok;

    rt_TubeTest     *tube_test;
    rt_ChannelTest  *channel_test;

    if(line.length() == 0) return;    

    fields = line.trimmed().split(QRegExp("\\s+"));
    id = fields.at(1).toInt();
    if(id < 0) return;

    switch(id)
    {
    case 0:
    case 1:
    case 2:
                count = pow(2,2+id);    break;

    case 3:
                count = 24;             break;

    default:    count = 0;  break;
    }


    //template_test = new rt_TemplateTest();
    //template_test->ID_Template = 0;
    //p_test->templates.push_back(template_test);

    text = fields.at(24+2);
    names = text.trimmed().split(QRegExp(";"));

    for(i=0; i<count; i++)
    {
        tube_test = new rt_TubeTest();
        tube_test->ID_Tube = i;
        p_test->tubes.push_back(tube_test);

        id = fields.at(i+2).toInt(&ok, 16);
        for(j=0; j<4; j++)
        {
            k = (id & (0xff << 8*j)) >> 8*j;
            if(!k) continue;
            channel_test = new rt_ChannelTest();
            channel_test->ID_Channel = j;
            tube_test->channels.push_back(channel_test);
            channel_test->name = names.at(k-1).toStdString();
        }
    }
}

//-----------------------------------------------------------------------------
//--- FindNameByTest
//-----------------------------------------------------------------------------
QString FindNameByTest(rt_Test *ptest, int id_tube, int id_channel)
{
    int i,j;
    QString res = "";
    QString fluor_name[COUNT_CH] = FLUOR_NAME;

    rt_TubeTest     *tube_test;
    rt_ChannelTest  *channel_test;

    for(i=0; i<ptest->tubes.size(); i++)
    {
        tube_test = ptest->tubes.at(i);
        if(tube_test->ID_Tube == id_tube)
        {
            for(j=0; j<tube_test->channels.size(); j++)
            {
                channel_test = tube_test->channels.at(j);
                if(channel_test->ID_Channel == id_channel)
                {
                    res = QString::fromStdString(channel_test->name);
                    return(res);
                }
            }
        }
    }    
    if(res.trimmed() == "" && id_channel < COUNT_CH) res = fluor_name[id_channel];
    return(res);
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Sleep(int ms)
{
    QEventLoop loop;
    QTimer::singleShot(ms, &loop, SLOT(quit()));
    loop.exec();
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
QString GetRandomString(int count_symbol)
{
    const QString possibleCharacters("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
    const int randomStringLength = count_symbol; //12; // assuming you want random strings of 12 characters
    int index;
    QTime time = QTime::currentTime();
    //qsrand((uint)time.msec());
    qsrand((uint)time.msecsSinceStartOfDay());

    QString randomString;
    for(int i=0; i<randomStringLength; ++i)
    {
        index = qrand() % possibleCharacters.length();
        QChar nextChar = possibleCharacters.at(index);
        randomString.append(nextChar);
    }

    return(randomString);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
int CrossTalk_Correction(rt_Protocol *prot)
{
    int i,j,k,m;
    int num;
    int id;
    int index, count_meas;
    QString text;
    QStringList list;
    QStringList list_fluor;
    double dval;
    double val_1,val_2;
    double coeff;
    bool ok;
    bool sts_optic = true;
    bool sts_spectr = true;

    rt_Measurement  *meas;
    rt_Preference   *preference;

    int count_tubes = prot->count_Tubes;
    QString fluor_name[COUNT_CH] = FLUOR_NAME;  // in real protocol fluor_name may be another (R6G,...)

    for(i=0; i<COUNT_CH; i++) list_fluor << fluor_name[i];

    QVector<double> vec_temp;
    QVector<double> vec_Expo;

    QVector<double> coeff_spectr;
    QVector<double> coeff_optics;
    QVector<double> coeff_crosstalk;

    QVector<double> coeff_optics_temp;

    vector<double> *measure;

    for(i=0; i<prot->preference_Pro.size(); i++)
    {
        preference = prot->preference_Pro.at(i);

        if(preference->name == USE_CROSSTALK) {if(preference->value != "yes") sts_spectr = false; continue;}
        if(preference->name == USE_OPTICALCORRECTION) {if(preference->value != "yes") sts_optic = false; continue;}
    }


    // 1. Coeff Spectr
    k = 0;
    coeff_spectr.reserve(COUNT_CH*COUNT_CH);
    coeff_spectr.fill(0.,COUNT_CH*COUNT_CH);
    for(i=0; i<prot->COEFF_Spectral.size(); i++)
    {
        text = QString::fromStdString(prot->COEFF_Spectral.at(i));
        list = text.trimmed().split(QRegExp("\\s+"));
        if(list_fluor.contains(list.at(0)) && list.size() > 2)
        {
            text = list.at(1);
            id = text.toInt(&ok);

            for(j=2; j<list.size(); j++)
            {
                text = list.at(j);
                dval = text.toDouble(&ok);
                if(!ok) dval = 0;

                if(!sts_spectr) dval = 0;
                if(!sts_spectr && id == (j-2)) dval = 1.;

                coeff_spectr.replace((j-2)*COUNT_CH + k, dval);
            }
            k++;
        }
    }
    //qDebug() << "Coeff Spectr: " << coeff_spectr.size() << coeff_spectr;

    // 2. Coeff Optics
    coeff_optics.reserve(count_tubes*COUNT_CH);
    for(i=0; i<prot->COEFF_Optic.size(); i++)
    {
        text = QString::fromStdString(prot->COEFF_Optic.at(i));
        list = text.trimmed().split(QRegExp("\\s+"));
        for(j=0; j<list.size(); j++)
        {
            text = list.at(j);
            dval = text.toDouble(&ok);
            if(!ok) dval = 0;
            if(!sts_optic) dval = 1.;
            coeff_optics.append(dval);
        }
    }
    if(prot->COEFF_Optic.size() == count_tubes)
    {
        coeff_optics_temp.reserve(count_tubes*COUNT_CH);
        coeff_optics_temp = coeff_optics;
        coeff_optics.clear();
        coeff_optics.reserve(count_tubes*COUNT_CH);
        for(i=0; i<COUNT_CH; i++)
        {
            for(j=0; j<count_tubes; j++)
            {
                coeff_optics.append(coeff_optics_temp.at(j*COUNT_CH + i));
            }
        }
        coeff_optics_temp.clear();
    }

    //qDebug() << "Coeff Optics: " << prot->COEFF_Optic.size() << coeff_optics.size() << coeff_optics;


    // 3. Coeff CrossTalk
    coeff_crosstalk.reserve(count_tubes*COUNT_CH*COUNT_CH);
    for(i=0; i<count_tubes; i++)
    {
        for(j=0; j<COUNT_CH; j++)
        {
            //val_1 = coeff_optics.at(i*COUNT_CH + j);
            val_1 = coeff_optics.at(i + j*count_tubes);

            for(k=0; k<COUNT_CH; k++)
            {
                //val_2 = coeff_optics.at(i*COUNT_CH + k);
                val_2 = coeff_optics.at(i + k*count_tubes);

                if(val_1 == 0. || val_2 == 0.) dval = 0.;
                else
                {
                    dval = val_1/val_2 * coeff_spectr.at(j*COUNT_CH + k);
                    if(j != k) dval *= (-1.);
                }
                coeff_crosstalk.append(dval);
            }
        }
    }
    //qDebug() << "Coeff CrossTalk: " << coeff_crosstalk.size() << coeff_crosstalk.mid(0,10);

    // 3. CrossTalk Compensation
    int count_pcr = prot->count_PCR;
    int count_mc = prot->count_MC;
    int count_ch = 0;

    m = 0;
    k = 0;
    for(i=0; i<COUNT_CH; i++)   //load vec_Expo
    {
        meas = prot->meas.at(i*2);

        while(meas->optical_channel > m)
        {
            vec_Expo.append(0.);
            m++;
        }
        if(m == meas->optical_channel) vec_Expo.append(meas->exp_value);
        if(m > meas->optical_channel)
        {
            while(vec_Expo.size() < COUNT_CH)
            {
                vec_Expo.append(0.);
            }
            break;
        }
        m++;
    }
    //qDebug() << "vec_Expo: " << vec_Expo;

    for(i=0; i<COUNT_CH; i++) if(prot->active_Channels & (0x0f<<(i*4))) count_ch++;

    //qDebug() << "count_ch: " << count_ch << prot->active_Channels;

    //... for pcr & mc measurements ...
    index = 0;
    while(index < 2)
    {
        switch(index)
        {
        case 0:     count_meas = count_pcr;
                    measure = &(prot->PCR_RawData);
                    break;

        case 1:     count_meas = count_mc;
                    measure = &(prot->MC_RawData);
                    break;
        }

        for(i=0; i<count_meas; i++)
        {
            //if(!sts_spectr) break;      // !!! exit if sts_spectr == false
            if(!sts_spectr && !sts_optic) break;

            for(j=0; j<count_tubes; j++)
            {
                vec_temp.clear();
                m = 0;
                for(k=0; k<COUNT_CH; k++)
                {
                    if(prot->active_Channels & (0x0f<<(k*4)))
                    {
                        vec_temp.append(measure->at(i + j*count_meas + m*count_meas*count_tubes));
                        m++;
                    }
                    else vec_temp.append(0.);
                }

                num = 0;
                for(k=0; k<COUNT_CH; k++)
                {
                    if((prot->active_Channels & (0x0f<<(k*4))) == 0) continue;

                    dval = 0.;
                    for(m=0; m<COUNT_CH; m++)
                    {
                        if(vec_Expo.at(m) == 0.) continue;
                        coeff = coeff_crosstalk.at(j*COUNT_CH*COUNT_CH + k*COUNT_CH + m);

                        dval += vec_Expo.at(k)/vec_Expo.at(m) * vec_temp.at(m) * coeff;
                    }
                    //qDebug() << i << j << k << dval;
                    //coeff = coeff_optics.at(k + j*COUNT_CH);
                    coeff = coeff_optics.at(k*count_tubes + j);
                    if(coeff == 0.) coeff = 1.;
                    measure->at(i + j*count_meas + num*count_meas*count_tubes) = dval/coeff;
                    num++;
                }
            }
        }
        index++;
    }

    return(0);
}




