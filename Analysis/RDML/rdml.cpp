#include "rdml.h"


//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
extern "C" RDML_EXPORT bool __stdcall RDML_2_RT(char* p_str)
{
    int i,j,k,m,n,l;
    bool res = false;
    QString fn(p_str);
    QString text;
    rt_Protocol *prot = NULL;
    rt_Measurement *measure;
    rt_Test *ptest;

    QStringList list;
    list << "dye" << "thermalCyclingConditions" << "experiment";

    QStringList list_Program;
    list_Program << "XPRG 0 35 0" << "XLEV 9400 30 0 0 0 0" << "XLEV 6400 15 0 0 0 40" << "XSAV bio_rad";


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

        prot = new rt_Protocol();

        //--- временно
        prot->count_PCR = 40;
        prot->count_MC = 0;
        count_meas = prot->count_PCR + prot->count_MC;
        prot->count_Tubes = 96;
        prot->color_tube.clear();
        prot->enable_tube.clear();
        color = QColor(Qt::gray);
        key = color.rgb();
        for(n=0; n<prot->count_Tubes; n++)
        {
            prot->color_tube.push_back(key);
            prot->enable_tube.push_back(0);
        }
        //---

        for(i=0; i<root.childNodes().size(); i++)
        {
            child = root.childNodes().at(i);
            text = child.nodeName();

            switch(list.indexOf(text))
            {
            case 0:                     // dye
                    text = child.attributes().namedItem("id").nodeValue();
                    qDebug() << "dye: " << text;
                    k = -1;
                    if(text.contains("fam",Qt::CaseInsensitive)) k = 0;
                    if(text.contains("hex",Qt::CaseInsensitive)) k = 1;
                    if(text.contains("rox",Qt::CaseInsensitive)) k = 2;
                    if(text.contains("cy5",Qt::CaseInsensitive)) k = 3;
                    if(text.contains("cy5-5",Qt::CaseInsensitive)) k = 4;

                    if(text.contains("SYBRGreen",Qt::CaseInsensitive)) k = 1;

                    if(k >= 0)
                    {
                        vec_Value = new QVector<int>;
                        map_Value.insert(k, vec_Value);
                    }

                    prot->active_Channels += 0x01 << (4*k);
                    break;

            case 1:                     // thermalCyclingConditions
                    foreach(text, list_Program)
                    {
                        prot->program.push_back(text.toStdString());
                    }

                    break;

            case 2:                     // experiment

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

                        if(k < 0) continue;

                        vec_Value = map_Value.value(k);
                        //vec_Value->fill(1, count_meas*prot->count_Tubes);
                        //qDebug() << "fill" << k << vec_Value->size() << count_meas << prot->count_Tubes;

                        for(m=0; m<item.childNodes().size(); m++)
                        {
                            node = item.childNodes().at(m);
                            /*if(node.nodeName() == "pcrFormat" && first_it)
                            {
                                row = node.firstChildElement("rows").text().toInt();
                                col = node.firstChildElement("columns").text().toInt();
                                prot->count_Tubes = col*row;
                                prot->color_tube.clear();
                                prot->enable_tube.clear();
                                color = QColor(Qt::gray);
                                key = color.rgb();
                                for(n=0; n<prot->count_Tubes; n++)
                                {
                                    prot->color_tube.push_back(key);
                                    prot->enable_tube.push_back(0);
                                }

                                first_it = false;
                            }*/

                            if(node.nodeName() != "react") continue;
                            if(!map_Value.keys().contains(k)) break;                            

                            id = node.attributes().namedItem("id").nodeValue().toInt();


                            prot->enable_tube.at(id-1) = 1;


                            data = node.firstChildElement("data");

                            for(l=0; l<data.childNodes().size(); l++)
                            {
                                adp = data.childNodes().at(l);
                                if(adp.nodeName() != "adp") continue;

                                cyc = adp.firstChildElement("cyc");
                                fluor = adp.firstChildElement("fluor");

                                num = cyc.toElement().text().toInt();
                                value = fluor.toElement().text().toDouble();

                                num = (num-1) + (id-1)*count_meas;
                                //qDebug() << "num, value: " << num << value << vec_Value;

                                //vec_Value->replace((num-1) + (id-1)*count_meas, value);
                                vec_Value->append(value);
                            }                            
                        }

                        qDebug() << "Value: " << j << k << map_Value.value(k)->size();// << *map_Value.value(k);
                    }

                    break;

            default:        break;
            }
        }

        //... create measurements ...
        qDebug() << "enable: " << QVector<short>::fromStdVector(prot->enable_tube);

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
                    measure->type_meas = 1;         //
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
        RDML_LoadSourceSet("rdml/RDML_Protocol_96.trt", prot);


        prot->state = mData;

        qDebug() << "Prot: " << prot->active_Channels << prot->count_Tubes;
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
void RDML_LoadSourceSet(QString fn, rt_Protocol *p)
{
    int i;

    QDomDocument    doc;
    QDomElement     root;
    QDomNode        child;
    QDomNode        node;
    rt_GroupSamples *group;

    QFile file(fn);

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
    }
}
