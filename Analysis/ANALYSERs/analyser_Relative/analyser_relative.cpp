#include "analyser_relative.h"

extern "C" ANALYSER_RELATIVESHARED_EXPORT Analysis_Interface* __stdcall createAnalyser_plugin()
{
    return(new Analyser_Relative());
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Analyser_Relative::Analyser_Relative()
{
    MainBox = NULL;
    ru_Lang = false;
    readCommonSettings();
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void* Analyser_Relative::Create_Win(void *pobj, void *main)
{
    int i;
    QString fluor_name[COUNT_CH] = FLUOR_NAME;
    QFont f = qApp->font();

    QWidget *parent = (QWidget*)pobj;
    main_widget = (QWidget*)main;

    MainBox = new QGroupBox(parent);
    MainBox->setFont(f);
    MainBox->setObjectName("Transparent");
    QVBoxLayout *main_layout = new QVBoxLayout;
    main_layout->setMargin(0);
    main_layout->setSpacing(4);
    MainBox->setLayout(main_layout);

    Results_Table = new Relative_TableWidget();
    Results_Table->setContextMenuPolicy(Qt::CustomContextMenu);
    Results_Table->setSelectionBehavior(QAbstractItemView::SelectRows);
    Results_Table->setSelectionMode(QAbstractItemView::SingleSelection);
    results_Delegate = new ResultsItemDelegate();
    Results_Table->setItemDelegate(results_Delegate);
    results_Delegate->background = &Samples_Background;
    main_layout->addWidget(Results_Table);

    copy_to_clipboard = new QAction(QIcon(":/images/copy_clipboard.png"),tr("copy to ClipBoard"), this);
    copy_to_excelfile = new QAction(QIcon(":/images/copy_excel.png"),tr("copy to Excel"), this);
    copy_to_clipboard_on_column = new QAction(QIcon(":/images/copy_clipboard.png"),tr("copy to ClipBoard by column"), this);

    connect(Results_Table, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu_ResultsTable()));
    connect(Results_Table, SIGNAL(currentCellChanged(int,int,int,int)), this, SLOT(ResultsTable_ChangedCurrentCell(int,int,int,int)));
    connect(copy_to_clipboard, SIGNAL(triggered(bool)), this, SLOT(to_ClipBoard()));
    connect(copy_to_excelfile, SIGNAL(triggered(bool)), this, SLOT(to_Excel()));
    connect(copy_to_clipboard_on_column, SIGNAL(triggered(bool)), this, SLOT(to_ClipBoard_ByColumn()));

    return(MainBox);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_Relative::Destroy()
{
    delete this;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_Relative::Destroy_Win()
{

    if(MainBox != NULL)
    {
        qDeleteAll(Samples_Relative);
        Samples_Relative.clear();
        Samples_Background.clear();

        delete results_Delegate;
        Results_Table->clear();
        delete Results_Table;

        delete MainBox;
        MainBox = NULL;
    }
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_Relative::Show()
{
    MainBox->show();
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_Relative::GetInformation(QVector<QString> *info)
{
    info->append("0x0003");
    info->append(tr("Relative_PCR"));
}
//-----------------------------------------------------------------------------
//--- readCommonSettings()
//-----------------------------------------------------------------------------
void Analyser_Relative::readCommonSettings()
{
    QString text;
    QString dir_path = qApp->applicationDirPath();
    QSettings *CommonSettings = new QSettings(dir_path + "/tools/ini/preference.ini", QSettings::IniFormat);

    CommonSettings->beginGroup("Common");

        //... Language ...
        text = CommonSettings->value("language","ru").toString();

        if(translator.load(":/translations/analyser_relative_" + text + ".qm"))
        {
            qApp->installTranslator(&translator);
            if(text == "ru") ru_Lang = true;
        }
        //... Style ...
        text = CommonSettings->value("style","xp").toString();
        StyleApp = text;

    CommonSettings->endGroup();
    delete CommonSettings;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_Relative::Analyser(rt_Protocol *prot)
{
    int i,j,k,m;
    int id;
    QString text,str;
    QString name_res, value_res;
    double dvalue;
    QStringList list;
    bool use_Ct;
    bool ok;
    double value_tar;
    double value_ref;


    rt_GroupSamples *group;
    rt_Sample       *sample;
    rt_Tube         *tube;
    rt_Channel      *channel;
    rt_Preference   *property;
    rt_Test         *ptest;
    sample_Relative  *sample_R;


    rt_TubeTest     *tube_test;
    rt_ChannelTest  *channel_test;
    QVector<short>  Type;
    bool is_Ref;
    double max_value;

    QVector<double> vec_double;
    QMultiMap<rt_Test*, sample_Relative*> map_Samples;

    qDeleteAll(Samples_Relative);
    Samples_Relative.clear();

    for(i=0; i<prot->Plate.groups.size(); i++)
    {
        group = prot->Plate.groups.at(i);
        for(j=0; j<group->samples.size(); j++)
        {
            sample = group->samples.at(j);
            if(sample->p_Test->header.Type_analysis == 0x0003)  // Relative tests
            {
                ptest = sample->p_Test;

                sample_R = new sample_Relative();
                Samples_Relative.append(sample_R);
                sample_R->unique_name = QString::fromStdString(sample->Unique_NameSample);      // unique_name
                sample_R->ptest = sample->p_Test;                                               // ptest
                sample_R->sample = sample;                                                      // sample
                tube = sample->tubes.at(0);
                sample_R->pos = tube->pos;                                                      // pos

                //qDebug() << "Sample: " << sample_R->unique_name  << sample_R->pos;

                //map_Specific.clear();
                //map_IC.clear();

                // use Cp or Ct ...
                use_Ct = false;
                foreach(property, sample_R->ptest->preference_Test)
                {
                    if(property->name == "use_Threshold")
                    {
                        if(property->value == "True") use_Ct = true;
                        break;
                    }
                }
                // Target or Reference
                Type.clear();
                tube_test = sample_R->ptest->tubes.at(0);
                foreach(channel_test, tube_test->channels)
                {
                    foreach(property, channel_test->preference_ChannelTest)
                    {
                        is_Ref = false;
                        if(property->name == "target&reference")
                        {
                            //qDebug() << "property->name == target&reference" << QString::fromStdString(property->value);
                            if(property->value == "Reference") is_Ref = true;
                            break;
                        }
                    }
                    if(is_Ref) Type.append(1);  // Reference
                    else Type.append(0);        // Target
                }
                sample_R->Type = Type;
                //qDebug() << "   Type: " << sample_R->Type;
                //

                // Cp(Ct)
                for(k=0; k<tube->channels.size(); k++)
                {
                    channel = tube->channels.at(k);
                    for(m=0; m<channel->result_Channel.size(); m++)
                    {
                        text = QString::fromStdString(channel->result_Channel.at(m));
                        list = text.split("=");
                        name_res = QString(list.at(0)).trimmed();
                        value_res = QString(list.at(1)).trimmed();

                        if((name_res == "Ct" && use_Ct) || (name_res == "Cp" && !use_Ct))
                        {
                            dvalue = value_res.toDouble(&ok);
                            if(!ok) dvalue = 0.;
                            sample_R->Cp.append(dvalue);

                            //if(sample_R->Type.at(k) == 0) map_Specific.insert(k,(int)dvalue);
                            //else  map_IC.insert(k,(int)dvalue);

                            break;
                        }
                    }
                }
                //qDebug() << "   Cp: " << sample_R->Cp;

                // Control (Standart) sample ???
                sample_R->type_Sample = 0;  // Target sample
                foreach(property, sample->preference_Sample)
                {
                    if(property->name == "kind")
                    {
                        if(property->value == "Standart") sample_R->type_Sample = 1;
                        break;
                    }
                }
                //qDebug() << "   Type: " << sample_R->type_Sample;

                // DCT_tar
                if(!sample_R->Type.contains(1)) {sample_R->valid = false; continue;}

                vec_double.clear();
                for(k=0; k<sample_R->Type.size(); k++)
                {
                    if(sample_R->Type.at(k) == 1)
                    {
                        value_ref = sample_R->Cp.at(k);
                        if(value_ref <= 0.) value_ref = 0.;
                        vec_double.append(value_ref);
                    }
                }
                if(vec_double.contains(0.)) {sample_R->valid = false; continue;}
                value_ref = Find_MeanValue(vec_double);
                for(k=0; k<sample_R->Type.size(); k++)
                {
                    if(sample_R->Type.at(k) == 1) dvalue = 0;
                    else dvalue = sample_R->Cp.at(k) - value_ref;
                    sample_R->DCT_tar.append(dvalue);
                }

                //
                map_Samples.insert(ptest, sample_R);
                //qDebug() << "   DCT_tar: " << sample_R->DCT_tar;
            }
        }
    }

    //qDebug() << "map:" << map_Samples.size() << map_Samples;

    max_value = 0.;
    foreach(ptest, map_Samples.uniqueKeys())
    {
        vec_double.clear();
        foreach(sample_R, map_Samples.values(ptest))
        {
            id = 0;
            //qDebug() << "sample_R->type_Sample == 1 valid:" << sample_R->type_Sample << sample_R->valid <<sample_R->pos;
            if(sample_R->type_Sample == 1 && sample_R->valid)
            {
                if(vec_double.isEmpty()) vec_double.fill(0, sample_R->DCT_tar.size());
                for(i=0; i<vec_double.size(); i++)
                {
                    vec_double.replace(i, vec_double.at(i) + sample_R->DCT_tar.at(i));
                    id++;
                }
            }
        }
        if(id)
        {
            for(i=0; i<vec_double.size(); i++)
            {
                vec_double.replace(i, vec_double.at(i)/id);     // DCT ref
            }
        }
        //qDebug() << "vec_double: " << id << vec_double;

        foreach(sample_R, map_Samples.values(ptest))
        {
            sample_R->result = "ok";
            if(!sample_R->valid) {sample_R->result = "???"; continue;}
            if(vec_double.isEmpty()) {sample_R->result = "?"; continue;}
            if(sample_R->type_Sample == 1) {sample_R->result = tr("Control"); continue;}

            for(i=0; i<sample_R->DCT_tar.size(); i++)
            {
                sample_R->DDCT.append(sample_R->DCT_tar.at(i) - vec_double.at(i));
            }

            //qDebug() << "   DDCT: " << sample_R->DCT_tar;

            dvalue = *std::min_element(sample_R->DDCT.begin(), sample_R->DDCT.end());
            if(dvalue < max_value) max_value = dvalue;

            // Quality_Result to tube_Results
            str = "";
            i = 0;
            //qDebug() << "   Type: " << sample_R->Type << sample_R->DDCT;
            foreach(dvalue, sample_R->DDCT)
            {
                if(!str.isEmpty()) str += "\t";
                if(sample_R->Type.at(i) == 1) str += "-";
                else str += QString::number(dvalue,'f', 1);
                i++;
            }
            if(str.isEmpty()) str = "?";
            str = QString("Relative_Result=%1").arg(str);
            sample_R->sample->tubes.at(0)->result_Tube.push_back(str.toStdString());
            //qDebug() << "   str: " << str;
        }
    }
    //qDebug() << "max_value: " << max_value;
    results_Delegate->max_value = max_value;

    vec_double.clear();
    map_Samples.clear();

    Prot = prot;
    Fill_ResultsTable();
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_Relative::Enable_Tube(QVector<short> *e)
{
    Fill_ResultsTable();
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_Relative::Select_Tube(int pos)
{
    int i;
    int col,row;
    Prot->Plate.PlateSize(Prot->count_Tubes,row,col);

    QString str;
    QString text = Convert_IndexToName(pos,col);

    //... Result ...
    for(i=0; i<Results_Table->rowCount(); i++)
    {
        str = Results_Table->item(i,0)->text();

        if(str.trimmed() == text.trimmed())
        {
            Results_Table->setCurrentCell(i,0);
            break;
        }
    }
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_Relative::Save_Results(char *fname)
{
    int i,j,k,m;
    bool ok;
    bool save = false;

    QString text(fname);
    QString ID_Test,str;
    int method;
    int pos;
    double dvalue;
    sample_Relative  *sample_R;

    QDomDocument    doc;
    QDomElement     root;
    QDomNode        tests;
    QDomNode        node;
    QDomNode        sourceSet;
    QDomNode        item_source;
    QDomNode        samples;
    QDomNode        sample;
    QDomNode        tubes;
    QDomNode        item_tubes;
    QDomElement     tube_result;
    QDomElement     quality_result;

    QVector<QString> Relative_Tests;
    QVector<string> Y;

    // 1. Read & Edit Info
    QFile file(text);
    if(file.exists() && file.open(QIODevice::ReadOnly))
    {
        if(doc.setContent(&file))
        {
            file.close();

            root = doc.documentElement();

            // Tests with method == 0x0003
            tests = root.firstChildElement("tests");
            if(!tests.isNull())
            {
                for(i=0; i<tests.childNodes().size(); i++)
                {
                    node = tests.childNodes().at(i);
                    ID_Test = node.firstChildElement("IDTest").text();
                    method = node.firstChildElement("method").text().toInt(&ok);
                    if(method == 0x0003) Relative_Tests.append(ID_Test);
                }
            }

            // sourveSet
            sourceSet = root.firstChildElement("sourceSet");
            if(!sourceSet.isNull())
            {
                for(i=0; i<sourceSet.childNodes().size(); i++)
                {
                    item_source = sourceSet.childNodes().at(i);
                    samples = item_source.firstChildElement("samples");
                    for(j=0; j<samples.childNodes().size(); j++)
                    {
                        sample = samples.childNodes().at(j);
                        ID_Test = sample.firstChildElement("IDTest").text();
                        if(Relative_Tests.contains(ID_Test))
                        {
                            save = true;
                            tubes = sample.firstChildElement("tubes");
                            for(k=0; k<tubes.childNodes().size(); k++)
                            {
                                item_tubes = tubes.childNodes().at(k);
                                pos = item_tubes.firstChildElement("pos").text().toInt(&ok);
                                foreach(sample_R, Samples_Relative)
                                {
                                    if(sample_R->pos == pos)
                                    {
                                        Y.clear();
                                        str = "";
                                        foreach(dvalue, sample_R->DDCT)
                                        {
                                            if(!str.isEmpty()) str += "\t";
                                            str += QString::number(dvalue,'f', 1);
                                        }
                                        if(str.isEmpty()) str = "?";

                                        str = "Relative_Result=" + str;
                                        Y.append(str.toStdString());
                                        quality_result = MakeElementResults(doc,"tube_results",&Y);
                                        tube_result = item_tubes.firstChildElement("tube_results");
                                        if(tube_result.isNull()) item_tubes.appendChild(quality_result);
                                        else item_tubes.replaceChild(quality_result, tube_result);
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        else file.close();

        // 2. Write info
        if(file.open(QIODevice::WriteOnly) && save)
        {
            QTextStream(&file) << doc.toString();
            file.close();
        }
    }

}
//-----------------------------------------------------------------------------
//--- contextMenu_CpTable
//-----------------------------------------------------------------------------
void Analyser_Relative::contextMenu_ResultsTable()
{
    QMenu menu;
    menu.setStyleSheet("QMenu::item:selected {background-color: #d7d7ff; color: black;}");

    menu.addAction(copy_to_clipboard);
    menu.addAction(copy_to_excelfile);
    menu.addSeparator();
    menu.addAction(copy_to_clipboard_on_column);

    menu.exec(QCursor::pos());

    menu.clear();

}
//-----------------------------------------------------------------------------
//--- to_ClipBoard_ByColumn()
//-----------------------------------------------------------------------------
void Analyser_Relative::to_ClipBoard_ByColumn()
{
    QString text = ResultsTable_ToDigits_ByColumn();

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(text);
}
//-----------------------------------------------------------------------------
//--- to_ClipBoard()
//-----------------------------------------------------------------------------
void Analyser_Relative::to_ClipBoard()
{
    QString text = ResultsTable_ToDigits();

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(text);
}
//-----------------------------------------------------------------------------
//--- to_Excel()
//-----------------------------------------------------------------------------
void Analyser_Relative::to_Excel()
{
    QString filename = "";
    QByteArray ba;

    QString text = ResultsTable_ToDigits();
    text.replace(QString("\t"), QString(";"));

    filename = QFileDialog::getSaveFileName(NULL, tr("Save to Excel(*.csv)..."),
                                            "ToExcel.csv", tr("Excel (*.csv)"));

    QFile file(filename);

    if(!filename.isEmpty())
    {
        if(file.open(QIODevice::WriteOnly))
        {            
            if(ru_Lang)
            {
                QTextCodec *codec1 = QTextCodec::codecForName("CP1251");
                qDebug() << text;
                ba = codec1->fromUnicode(text);
                file.write(ba);
                file.close();
            }
            else
            {
                QTextStream(&file) << text;
                file.close();
            }
        }
    }
}
//-----------------------------------------------------------------------------
//--- CpTable_ToDigits_ByColumn()
//-----------------------------------------------------------------------------
QString Analyser_Relative::ResultsTable_ToDigits_ByColumn()
{
    int i,j,k;
    int num,id;
    QString text, str = "";
    QString pos, pos_prev = "";
    QString sample_str = "";
    QTableWidgetItem *item;
    QStringList list;
    QStringList Vec;
    bool new_tube;
    QMap<int,QString> Cp_map;
    QString Result = "";

    for(i=0; i<Results_Table->rowCount(); i++)
    {
        for(j=0; j<Results_Table->columnCount(); j++)
        {
            item = Results_Table->item(i,j);
            text = item->text();

            //qDebug() << "i,j: " << i << j << text;
            //continue;

            switch(j)
            {
            case 0:
                    pos = text;
                    if(pos != pos_prev)
                    {
                        new_tube = true;

                        if(sample_str.size())
                        {
                            foreach(text, Cp_map.values())
                            {
                                sample_str += QString("%1\t").arg(text);
                            }
                            sample_str += Result;

                            str += sample_str + "\r\n";
                        }
                        for(k=0; k<COUNT_CH; k++)
                        {
                            Cp_map.insert(k,"-");
                        }
                        sample_str.clear();
                        sample_str = QString("%1\t").arg(pos);
                    }
                    else new_tube = false;
                    pos_prev = pos;
                    break;

            case 1:
                    if(new_tube) sample_str += QString("%1\t").arg(text);
                    break;

            case 2:
                    text.replace(".",",");
                    list = text.split("~");
                    id = QString(list.at(1)).toInt();
                    Cp_map.insert(id, list.at(0));
                    break;

            case 4:
                    Result = text;
                    break;

            default:    break;
            }
        }
    }

    foreach(text, Cp_map.values())
    {
        sample_str += QString("%1\t").arg(text);
    }
    sample_str += Result;


    str += sample_str + "\r\n";

    return(str);
}
//-----------------------------------------------------------------------------
//--- CpTable_ToDigits()
//-----------------------------------------------------------------------------
QString Analyser_Relative::ResultsTable_ToDigits()
{
    int i,j,k;
    int num;
    QString text, str = "";
    QTableWidgetItem *item;
    QStringList list;
    QStringList Vec;


    for(i=0; i<Results_Table->rowCount(); i++)
    {
        for(j=0; j<Results_Table->columnCount(); j++)
        {
            item = Results_Table->item(i,j);
            text = item->text();

            switch(j)
            {
            default:    Vec.append(text);
                        break;

            case 2:     list = text.split("~");
                        if(list.size() > 1) Vec.append(QString(list.at(1)));
                        else Vec.append("...");
                        text = list.at(0);
                        text.replace(".",",");
                        Vec.append(text);
                        break;

            case 3:     //list = text.split("~");
                        //text = list.at(0);
                        //text.replace(".",",");
                        num = text.indexOf("(");
                        if(num >= 0) text = text.mid(0,num);
                        Vec.append(text);
                        break;
            }
        }

        if(Vec.size())
        {
            for(j=0; j<Vec.size(); j++)
            {
                if(j) str += "\t";
                str += Vec.at(j);
            }
            str += "\r\n";
        }
        else str += "\r\n";

        Vec.clear();
    }

    return(str);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_Relative::Fill_ResultsTable()
{
    int i,j,k;
    int id_ch;
    double value;
    int act_channels;
    int row_current = 0;
    QString text;
    QStringList header;
    int count = 0;
    int pos;
    QTableWidgetItem *item;
    sample_Relative   *sample_R;

    bool status_background = false;

    int row_plate,col_plate;
    Prot->Plate.PlateSize(Prot->count_Tubes,row_plate,col_plate);

    QString rel_label = "DDCt";
    rel_label[0] = QChar(0x394);
    rel_label[1] = QChar(0x394);

    QString result_label = QString("2^(-%1)").arg(rel_label);

    header << tr("Pos") << tr("Name") << tr("Cp(Ct)") << rel_label << result_label;
    Results_Table->clear();
    Results_Table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    Results_Table->setColumnCount(5);
    Results_Table->setHorizontalHeaderLabels(header);
    Results_Table->verticalHeader()->hide();
    Results_Table->setColumnWidth(0,50);    // pos
    Results_Table->setColumnWidth(1,70);    // name sample
    Results_Table->setColumnWidth(2,100);    // Cp
    Results_Table->setColumnWidth(3,100);    // Relative result
    Results_Table->setColumnWidth(4,90);    // Histogramm result
    for(i=0; i<Results_Table->columnCount(); i++) Results_Table->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Fixed);
    Results_Table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    Results_Table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);


    foreach(sample_R, Samples_Relative)
    {
        if(!Prot->enable_tube.at(sample_R->pos)) continue;
        count += sample_R->Cp.size();
    }

    Results_Table->setRowCount(count);

    Samples_Background.clear();
    Samples_Background.reserve(count);

    // Load data
    foreach(sample_R, Samples_Relative)
    {
        pos = sample_R->pos;
        if(!Prot->enable_tube.at(pos)) continue;

        status_background = !status_background;

        act_channels = sample_R->ptest->header.Active_channel;
        count = sample_R->Cp.size();
        for(i=0; i<count; i++)
        {
            k = 0;
            id_ch = 0;
            for(j=0; j<COUNT_CH; j++)
            {
               if(act_channels & (0xf << 4*j))
               {
                    if(i == k)
                    {
                        id_ch = j;
                        break;
                    }
                    k++;
               }
            }

            for(j=0; j<Results_Table->columnCount(); j++)
            {
                item = new QTableWidgetItem();
                //item = new QTableWidgetItem();
                switch(j)
                {
                case 0:         // Pos
                            item->setText(Convert_IndexToName(sample_R->pos, col_plate));
                            break;

                case 1:         // Name
                            text = QString("%1 (%2)").arg(sample_R->unique_name).arg(QString::fromStdString(sample_R->ptest->header.Name_Test));
                            item->setText(text);
                            break;

                case 2:         // Cp
                            text = QString::number(sample_R->Cp.at(i),'f',1);
                            if(sample_R->Cp.at(i) == 0.) text = "-";
                            text += QString("~%1").arg(id_ch);
                            item->setText(text);
                            break;

                case 3:         // Relative value
                            if(sample_R->result == "ok")
                            {
                                if(sample_R->Type.at(i) == 1) text = tr("(Reference)");
                                else text = QString::number(sample_R->DDCT.at(i),'f',1);
                            }
                            else
                            {
                                text = sample_R->result;
                            }
                            item->setText(text);
                            break;

                case 4:         // Result Hostogramm
                            if(sample_R->result == "ok")
                            {
                                if(sample_R->Type.at(i) == 1) text = "";
                                else
                                {
                                    text = QString::number(sample_R->DDCT.at(i),'f',1);
                                    text += QString("~%1").arg(id_ch);
                                }
                            }
                            else text = "";
                            if(sample_R->result.contains(tr("Control")) && sample_R->Type.at(i) == 0)
                            {
                                text = QString("%1~%2").arg(tr("Control")).arg(id_ch);
                            }
                            item->setText(text);
                            break;

                default:        // default
                            text = "";
                            break;
                }
                Results_Table->setItem(row_current, j, item);
            }
            Samples_Background.append((short)status_background);
            Results_Table->setRowHeight(row_current, 20);
            row_current++;
        }
        Results_Table->setSpan(row_current-count,0,count,1);
        Results_Table->setSpan(row_current-count,1,count,1);
        //Results_Table->setSpan(row_current-count,4,count,1);
    }
}
//-----------------------------------------------------------------------------
//--- ResultsTable_ChangedCurrentCell(int, int, int, int)
//-----------------------------------------------------------------------------
void Analyser_Relative::ResultsTable_ChangedCurrentCell(int cur_row, int cur_col, int prev_row, int prev_col)
{
    QTableWidgetItem *item;
    QString text;
    int pos = 0;
    int count_col,count_row;

    if(!Prot || cur_row < 0 || cur_col < 0) return;

    //qDebug() << "ChangedCurrentCell" << cur_col << cur_row;

    int count_tubes = Prot->count_Tubes;
    Prot->Plate.PlateSize(count_tubes, count_row, count_col);

    if(cur_col > 0)
    {
        Results_Table->setCurrentCell(cur_row,0);
        //QTimer::singleShot(10, this, &Analyser_Relative::SelectNewCell);
    }

    item = Results_Table->item(cur_row,0);
    text = item->text();
    pos = Convert_NameToIndex(text,count_col);
    Select_Curve(pos);

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_Relative::SelectNewCell()
{
    int cur_row = Results_Table->currentRow();

    Results_Table->setCurrentCell(cur_row, 0);

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_Relative::Select_Curve(int pos)
{
    event_plot.pos_plot = pos;
    QApplication::sendEvent(main_widget, &event_plot);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void ResultsItemDelegate::paint(QPainter *painter,
                               const QStyleOptionViewItem &option,
                               const QModelIndex &index) const
{
    //int i,j;
    QRect rect;
    QString text, str;
    int row,col;
    int id;
    double dvalue, Max_value;
    double null_value;
    QColor color;
    QFont f = qApp->font();
    painter->setFont(f);
    int font_size = f.pointSize();
    bool ok;
    double coef, width;
    QFontMetrics fm = painter->fontMetrics();    
    QStringList list;    
    QFont TimesFont("Times", 14, QFont::Bold);
    QColor bg_color[COUNT_CH] = {0xD8E2F1,0xD8E9D8,0xFEF0CB,0xFFE6FF,0xFFD7D7};
    QPixmap pixmap(":/images/fam_flat.png");

    painter->setPen(QColor(Qt::black));

    QStyleOptionViewItem  viewOption(option);
    viewOption.state &= ~QStyle::State_HasFocus;    // disable state HasFocus
    QStyledItemDelegate::paint(painter, viewOption, index);

    rect = option.rect;

    text = index.data().toString();
    row = index.row();
    col = index.column();

    //... Background ...
    //
    if(background->at(row)) color = QColor(255,255,255);
    else color = QColor(235,235,235);

    if(option.state & QStyle::State_Selected) color = QColor(215,215,255);
    painter->fillRect(option.rect, color);

    //... Data ...
    switch(col)
    {
    case 0:     painter->drawText(rect, Qt::AlignCenter, text);
                break;

    case 1:     painter->drawText(rect, Qt::AlignCenter, text);
                break;

    case 2:     list.clear();
                list = text.split("~");
                text = list.at(0);

                id = QString(list.at(1)).toInt(&ok);
                switch(id)
                {
                case 0:  pixmap.load(":/images/flat/fam_flat.png");    break;
                case 1:  pixmap.load(":/images/flat/hex_flat.png");    break;
                case 2:  pixmap.load(":/images/flat/rox_flat.png");    break;
                case 3:  pixmap.load(":/images/flat/cy5_flat.png");    break;
                case 4:  pixmap.load(":/images/flat/cy55_flat.png");   break;
                default: pixmap.load(":/images/flat/disable_flat.png");   break;
                }
                //color = bg_color[id];
                //painter->fillRect(rect.left(), rect.top(), 10, rect.height(), color);
                painter->drawPixmap( rect.left(), rect.top() + (rect.height()-8)/2 - 2, pixmap);
                //painter->drawPixmap(0, option.rect.y() + 22*K*i + 2 + dy,pixmap);
                rect.setLeft(rect.left() + 10);

                painter->drawText(rect, Qt::AlignCenter, text);
                break;

    case 3:                
                if(text.contains(tr("Reference")) || text.contains(tr("Control")))
                {
                    f.setBold(false);
                    font_size -= 2;
                    f.setPointSize(font_size);
                    painter->setFont(f);
                    painter->setPen(Qt::gray);
                    painter->drawText(rect, Qt::AlignCenter, text);
                    painter->setPen(Qt::black);
                    break;
                }

                painter->drawText(rect, Qt::AlignCenter, text);
                break;

    case 4:
                if(!text.isEmpty())
                {
                    list.clear();
                    list = text.split("~");
                    text = list.at(0);

                    id = QString(list.at(1)).toInt(&ok);
                    if(!ok || id >= COUNT_CH) id = 0;
                    color = bg_color[id];
                    if(text.contains(tr("Control"))) color = QColor(127,127,127);

                    dvalue = text.toDouble(&ok);
                    if(!ok) dvalue = 0.;

                    dvalue = pow(2,-dvalue);
                    Max_value = pow(2,-max_value);

                //if(dvalue > 0. && max_value > 0)
                //{
                    coef = dvalue/Max_value;
                    width = rect.width()*coef - 20;
                    if(width < 0) width = 0;
                    painter->fillRect(rect.left() + 5, rect.top() + 3, width, rect.height() - 7, color);
                    painter->drawRect(rect.left() + 5, rect.top() + 3, width, rect.height() - 7);
                //}
                }
                Max_value = pow(2,-max_value);
                coef = 1./Max_value;
                width = rect.width()*coef - 20;
                if(width < 0) width = 0;
                painter->setPen(QColor(Qt::red));
                painter->drawLine(rect.left() + 5 + width, rect.top(), rect.left() + 5 + width, rect.bottom());

                break;

    default:    break;
    }

}
