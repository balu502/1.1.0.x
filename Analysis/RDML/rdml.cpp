#include "rdml.h"


//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
extern "C" RDML_EXPORT bool __stdcall RDML_2_RT(char* p_str)
{
    int i,j,k,m,n,l;
    bool res = false;
    QString fn(p_str);
    QString text, str;
    rt_Protocol *prot = NULL;
    rt_Measurement *measure;
    rt_Test *ptest;

    QStringList list;
    list << "dye" << "thermalCyclingConditions" << "experiment" << "experimenter" << "id" << "dateUpdated";

    QStringList list_Program;
    //list_Program << "XPRG 0 35 0" << "XLEV 9400 30 0 0 0 0" << "XLEV 6400 15 0 0 0 40" << "XSAV bio_rad";


    QColor color;
    int key, count_meas, id;
    int col = 0, row = 0;
    bool first_it = true;
    int num;
    int value;


    QDomDocument    doc;
    QDomElement     root;
    QDomElement     experiment;
    QDomNode        child;
    QDomNode        node;
    QDomNode        item;
    QDomNode        data;
    QDomNode        adp;
    QDomNode        cyc;
    QDomNode        fluor;

    QDomElement     run_item;
    QDomElement     ch_item;
    QDomElement     rows_item;
    QDomElement     columns_item;
    QDomElement     react_item;
    QDomElement     data_item;
    QDomNode        ch;


    QMap<int, QVector<int>*> map_Value;
    QVector<int> *vec_Value;

    QFile file(fn);

    if(!file.exists()) return(false);

    while(1)
  {

    if(file.exists() && file.open(QIODevice::ReadOnly))
    {
        if(!doc.setContent(&file))
        {
            file.close();
            return(false);
        }
        file.close();

        root = doc.documentElement();
        if(root.nodeName() != "rdml") break;        
        if(!root.hasAttribute("version") || !root.hasAttribute("xmlns:rdml") || !root.hasAttribute("xmlns")) break;

        prot = new rt_Protocol();

        // Comments...
        rt_Preference   *pro_Comments = new rt_Preference();
        pro_Comments->name = "Comments";
        prot->preference_Pro.push_back(pro_Comments);
        //...

        for(i=0; i<root.childNodes().size(); i++)
        {
            child = root.childNodes().at(i);
            text = child.nodeName();

            switch(list.indexOf(text))
            {
            case 5:                     // dateUpdated
                    pro_Comments->value += "\r\n" + child.toElement().text().toStdString();
                    break;

            case 4:                     // id
                    prot->name = child.firstChildElement("publisher").text().toStdString();
                    break;

            case 3:                     // experimenter

                    text = child.attributes().namedItem("id").nodeValue();
                    str = child.firstChildElement("lastName").text();
                    prot->owned_by = QString("%1_%2").arg(text).arg(str).toStdString();

                    break;

            case 0:                     // dye
                    text = child.attributes().namedItem("id").nodeValue();
                    //qDebug() << "dye: " << text;
                    k = -1;
                    if(text.contains("fam",Qt::CaseInsensitive)) k = 0;
                    if(text.contains("hex",Qt::CaseInsensitive)) k = 1;
                    if(text.contains("rox",Qt::CaseInsensitive)) k = 2;
                    if(text.contains("cy5",Qt::CaseInsensitive)) k = 3;
                    if(text.contains("cy5-5",Qt::CaseInsensitive)) k = 4;
                    if(text.contains("cy5_5",Qt::CaseInsensitive)) k = 4;

                    if(text.contains("SYBRGreen",Qt::CaseInsensitive)) k = 1;

                    if(k >= 0)
                    {
                        vec_Value = new QVector<int>;
                        map_Value.insert(k, vec_Value);
                    }

                    prot->active_Channels += 0x01 << (4*k);
                    break;

            case 1:                     // thermalCyclingConditions: Programm of amplification
                    str = "";
                    RDML_LoadProgram(&child, str);
                    list_Program = str.split("\r\n");

                    foreach(text, list_Program)
                    {
                        prot->program.push_back(text.toStdString());
                    }

                    break;

            case 2:                     // experiment

                    // 1.
                    run_item = child.firstChildElement("run");
                    ch_item = run_item.firstChildElement("pcrFormat");
                    rows_item = ch_item.firstChildElement("rows");
                    row = rows_item.text().toInt();
                    columns_item = ch_item.firstChildElement("columns");
                    col = columns_item.text().toInt();
                    prot->count_Tubes = row * col;

                    prot->color_tube.clear();
                    prot->enable_tube.clear();
                    color = QColor(Qt::gray);
                    key = color.rgb();
                    for(n=0; n<prot->count_Tubes; n++)
                    {
                        prot->color_tube.push_back(key);
                        prot->enable_tube.push_back(0);
                    }

                    prot->count_MC = 0;
                    prot->count_PCR = 0;

                    react_item = run_item.firstChildElement("react");
                    data_item = react_item.firstChildElement("data");
                    for(j=0; j<data_item.childNodes().size(); j++)
                    {
                        ch = data_item.childNodes().at(j);
                        if(ch.nodeName() == "adp") prot->count_PCR++;
                        if(ch.nodeName() == "mdp") prot->count_MC++;
                    }
                    count_meas = prot->count_PCR + prot->count_MC;

                    pro_Comments->value += "\r\n" + run_item.firstChildElement("instrument").text().toStdString();

                    // 2.
                    for(j=0; j<child.childNodes().size(); j++)
                    {
                        item = child.childNodes().at(j);
                        if(item.nodeName() != "run") continue;

                        text = item.attributes().namedItem("id").nodeValue();
                        k = -1;
                        if(text.contains("fam",Qt::CaseInsensitive)) k = 0;
                        if(text.contains("hex",Qt::CaseInsensitive)) k = 1;
                        if(text.contains("rox",Qt::CaseInsensitive)) k = 2;
                        if(text.contains("cy5",Qt::CaseInsensitive)) k = 3;
                        if(text.contains("cy5-5",Qt::CaseInsensitive)) k = 4;
                        if(text.contains("cy5_5",Qt::CaseInsensitive)) k = 4;


                        if(k < 0) continue;

                        vec_Value = map_Value.value(k);

                        for(m=0; m<item.childNodes().size(); m++)
                        {
                            node = item.childNodes().at(m);

                            if(node.nodeName() != "react") continue;
                            if(!map_Value.keys().contains(k)) break;                            

                            id = node.attributes().namedItem("id").nodeValue().toInt();


                            prot->enable_tube.at(id-1) = 1;


                            data = node.firstChildElement("data");

                            for(l=0; l<data.childNodes().size(); l++)
                            {
                                adp = data.childNodes().at(l);
                                if(adp.nodeName() != "adp" && adp.nodeName() != "mdp") continue;

                                fluor = adp.firstChildElement("fluor");                                
                                value = fluor.toElement().text().toDouble();

                                //if(adp.nodeName() == "adp") value |= 0x10fff;
                                //if(adp.nodeName() == "mdp") value |= 0x20fff;

                                vec_Value->append(value);
                            }                            
                        }
                        //qDebug() << "Value: " << j << k << map_Value.value(k)->size();// << *map_Value.value(k);
                    }

                    break;

            default:        break;
            }
        }

        //... create measurements ...
        //qDebug() << "enable: " << QVector<short>::fromStdVector(prot->enable_tube);

        for(i=0; i<count_meas; i++)
        {
            for(j=0; j<COUNT_CH; j++)
            {
                if(!map_Value.keys().contains(j)) continue;
                vec_Value = map_Value.value(j);

                for(k=0; k<2; k++)
                {
                    measure = new rt_Measurement();
                    prot->meas.push_back(measure);

                    measure->fn = i;

                    if(i < prot->count_PCR) measure->type_meas = 1;         //
                    else measure->type_meas = 2;                            //

                    measure->block_number = 1;      //
                    measure->cycle_rep = 1;         //
                    measure->optical_channel = j;
                    measure->num_exp = k;
                    measure->blk_exp = 0;           //
                    measure->exp_value = 1000/(1+k*4);

                    id = 0;
                    for(m=0; m<prot->enable_tube.size(); m++)
                    {
                        if(prot->enable_tube.at(m) == 0)
                        {
                            value = 0;
                        }
                        else
                        {
                            value = vec_Value->at(i+id*count_meas);                            
                            id++;
                        }
                        if(value < 0) value = 0;
                        if(k) value /= 5;
                        measure->measurements.push_back(value);
                    }

                    //qDebug() << "Meas:" << QVector<short>::fromStdVector(measure->measurements);
                }
            }
        }

        //... create test ...
        ptest = new rt_Test();
        prot->tests.push_back(ptest);
        RDML_LoadTest("rdml/rdml_test.xml", ptest);

        //... create SourceSet ...
        RDML_LoadSourceSet(prot);

        //... change protocol properties ...
        for(i=0; i<prot->preference_Pro.size(); i++)
        {
            rt_Preference *preference = prot->preference_Pro.at(i);
            if(preference->name == USE_CROSSTALK || preference->name == USE_OPTICALCORRECTION)
            {
                preference->value = "no";
            }
        }


        prot->state = mData;



        //qDebug() << "Prot: " << prot->active_Channels << prot->count_Tubes;
        //qDebug() << "Va = lue: " << map_Value.value(0)->size() << count_meas << *map_Value.value(0);

        //---------
        res = true;
        //---------


        SaveAsXML(NULL, NULL, prot, "rdml/rdml.rt", "");

        qDebug() << "rdml: save ok";

    }
    break;
  }

    doc.clear();
    if(prot)
    {
        prot->Clear_Protocol();
        delete prot;
    }
    foreach(vec_Value, map_Value.values())
    {
        vec_Value->clear();
        delete vec_Value;
    }
    map_Value.clear();

    qDebug() << "rdml: clear ok";


    return(res);
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------

RDML::RDML()
{
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void RDML_LoadTest(QString fn, rt_Test *p)
{
    QDomDocument    doc_test;
    QDomElement     root_test;
    QDomNode        node;

    QFile file(fn);

    if(file.exists() && file.open(QIODevice::ReadOnly))
    {
        if(!doc_test.setContent(&file))
        {
            file.close();
            return;
        }
        file.close();

        root_test = doc_test.documentElement();

        node = root_test.childNodes().at(0);

        LoadXML_Test(node, p);
    }
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void RDML_LoadSourceSet(rt_Protocol *p)
{
    int i,j;
    int id = 0;

    QDomDocument    doc;
    QDomElement     root;
    QDomNode        child;
    QDomNode        node;
    rt_GroupSamples *group;
    rt_Sample       *sample;
    rt_Tube         *tube;
    rt_Channel      *channel;
    short           state;

    rt_Test         *ptest;
    ptest = p->tests.at(0);     // simple test...


    for(i=0; i<p->count_Tubes; i++)
    {
        if(p->enable_tube.at(i) == 0) continue;

        group = new rt_GroupSamples();
        group->Unique_NameGroup = QString("%1_%2").arg(QObject::tr("Sample")).arg(id+1).toStdString();
        {Sleep(5); group->ID_Group = GetRandomString(10).toStdString();}
        p->Plate.groups.push_back(group);

        sample = new rt_Sample();
        sample->Unique_NameSample = group->Unique_NameGroup;
        {Sleep(5); sample->ID_Sample = GetRandomString(10).toStdString();}
        sample->ID_Test = "-1";
        sample->p_Test = ptest;
        group->samples.push_back(sample);

        tube = new rt_Tube();
        tube->ID_Tube = 0;
        tube->Unique_NameTube = "0";
        tube->pos = i;
        sample->tubes.push_back(tube);

        for(j=0; j<COUNT_CH; j++)
        {
            if(!(p->active_Channels & (0x0f << 4*j))) continue;

            channel = new rt_Channel();
            channel->ID_Channel = j;
            tube->channels.push_back(channel);
        }

        id++;
    }


    /*QFile file(fn);

    if(file.exists() && file.open(QIODevice::ReadOnly))
    {
        if(!doc.setContent(&file))
        {
            file.close();
            return;
        }
        file.close();

        root = doc.documentElement();
        child = root.firstChildElement("sourceSet");

        if(child.isNull()) return;

        for(i=0; i<child.childNodes().size(); i++)
        {
             node = child.childNodes().at(i);
             group = new rt_GroupSamples();
             p->Plate.groups.push_back(group);
             LoadXML_Group(node, group, p);
        }
    }*/
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void RDML_LoadProgram(QDomNode *child, QString &s)
{
    int i,id,key;
    QString text, name_pro, str;
    QDomNode item;
    QDomElement nr;
    QDomElement temperature;
    QDomElement temperature_item;
    QDomElement duration_item;
    QDomElement loop;
    QDomElement measure;
    QDomElement temperatureChange;
    QDomElement goto_item;
    QDomElement repeat_item;
    QStringList list;
    QMap<int,QString> map_item;
    double temp_1, temp_2;

    int temp_value, duration_value, temp_incr, type_meas, goto_value, repeat_value;


    name_pro = child->attributes().namedItem("id").nodeValue();

    for(i=0; i<child->childNodes().size(); i++)
    {
        item = child->childNodes().at(i);
        text = item.nodeName();
        if(text != "step") continue;

        nr = item.firstChildElement("nr");
        id = nr.text().toInt();

        loop = item.firstChildElement("loop");
        if(!loop.isNull())
        {
            goto_item = loop.firstChildElement("goto");
            goto_value = goto_item.text().toInt();
            repeat_item = loop.firstChildElement("repeat");
            repeat_value = repeat_item.text().toInt();

            text = QString("LOOP %1 %2").arg(goto_value).arg(repeat_value);
            map_item.insert(id, text);

            continue;
        }

        temperature = item.firstChildElement("temperature");
        temperature_item = temperature.firstChildElement("temperature");
        duration_item = temperature.firstChildElement("duration");

        temp_value = temperature_item.text().toInt() * 100;
        duration_value = duration_item.text().toInt();

        temp_incr = 0;
        type_meas = 0;

        measure = temperature.firstChildElement("measure");
        if(!measure.isNull())
        {
            text = measure.text();
            if(text.contains("real time")) type_meas = 1;
            if(text.contains("meltcurve")) type_meas = 2;
        }
        temperatureChange = temperature.firstChildElement("temperatureChange");
        if(!temperatureChange.isNull())
        {
            temp_incr = temperatureChange.text().toDouble() * 100;
        }

        text = QString("XLEV %1 %2 %3 0 0 %4").arg(temp_value).arg(duration_value).arg(temp_incr).arg(type_meas);

        map_item.insert(id, text);
    }
    //qDebug() << "map: " << map_item;

    for(id=0; id<map_item.keys().size(); id++)
    {
        key = map_item.keys().at(id);
        text = map_item.value(key);
        if(text.contains("LOOP"))
        {
            list = text.split(" ");
            repeat_value = list.at(2).toInt();
            goto_value = list.at(1).toInt();

            str = "";
            while(goto_value < key)
            {
                if(!str.isEmpty()) str += "\r\n";
                str += map_item.value(goto_value);
                map_item.insert(goto_value, "");

                goto_value++;
            }
            str += QString("\r\nXCYC %1").arg(repeat_value);
            map_item.insert(key, str);
        }

        // melting curve
        if(text.contains("XLEV"))
        {
            list = text.split(" ");
            temp_incr = list.at(3).toInt();
            type_meas = list.at(6).toInt();
            if(temp_incr != 0 && type_meas == 2)
            {
                temp_1 = list.at(1).toDouble()/100.;
                str = map_item.value(key-1);
                list = str.split(" ");
                temp_2 = list.at(1).toDouble()/100.;

                repeat_value = fabs(temp_1 - temp_2)/fabs(temp_incr/100.);

                text += QString("\r\nXCYC %1").arg(repeat_value);
                map_item.insert(key, text);
            }
        }
    }
    //qDebug() << "map_new: " << map_item;

    str = "XPRG 0 35 0";
    foreach(text, map_item.values())
    {
        if(text.isEmpty()) continue;
        if(!str.isEmpty())  str += "\r\n";

        str += text;
        if(text.contains("XLEV") && !text.contains("XCYC"))
        {
            str += QString("\r\nXCYC 1");
        }
    }
    str += QString("\r\nXSAV %1").arg(name_pro);
    s = str;

}
