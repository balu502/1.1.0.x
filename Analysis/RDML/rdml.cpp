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

    QStringList list;
    list << "dye" << "thermalCyclingConditions" << "experiment";

    QColor color;
    int key, count_meas, id;
    int col = 0, row = 0;
    bool first_it = true;
    int num, value;


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


        // временно
        prot->count_PCR = 40;
        prot->count_MC = 0;
        count_meas = prot->count_PCR + prot->count_MC;

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

                        for(m=0; m<item.childNodes().size(); m++)
                        {
                            node = item.childNodes().at(m);
                            if(node.nodeName() == "pcrFormat" && first_it)
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
                            }

                            if(node.nodeName() != "react") continue;

                            if(!map_Value.keys().contains(k)) break;

                            vec_Value = map_Value.value(k);
                            //vec_Value->fill(1, count_meas*prot->count_Tubes);

                            id = node.attributes().namedItem("id").nodeValue().toInt();

                            data = node.firstChildElement("data");

                            for(l=0; l<data.childNodes().size(); l++)
                            {
                                adp = data.childNodes().at(l);
                                if(adp.nodeName() != "adp") continue;

                                cyc = adp.firstChildElement("cyc");
                                fluor = adp.firstChildElement("fluor");

                                num = cyc.toElement().text().toInt();
                                value = fluor.toElement().text().toDouble();
                                qDebug() << "num, value: " << num << value << id;

                                //vec_Value->replace((num-1) + (id-1)*count_meas, value);
                                vec_Value->append(value);

                            }
                        }
                    }

                    break;

            default:        break;
            }

        }



        prot->state = mData;

        qDebug() << "Prot: " << prot->active_Channels << prot->count_Tubes;
        qDebug() << "Value: " << map_Value.value(0)->size() << count_meas << *map_Value.value(0);

        //---------
        res = true;
        //---------

        SaveAsXML(NULL, NULL, prot, "rdml/rdml.rt", "");

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


    return(res);
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------

RDML::RDML()
{
}
