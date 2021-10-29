#include "analyser_quality.h"

extern "C" ANALYSER_QUALITYSHARED_EXPORT Analysis_Interface* __stdcall createAnalyser_plugin()
{
    return(new Analyser_Quality());
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Analyser_Quality::Analyser_Quality()
{
    MainBox = NULL;
    ru_Lang = false;
    readCommonSettings();
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void* Analyser_Quality::Create_Win(void *pobj, void *main)
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

    Results_Table = new Quality_TableWidget();
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
void Analyser_Quality::Destroy()
{
    delete this;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_Quality::Destroy_Win()
{

    if(MainBox != NULL)
    {
        qDeleteAll(Samples_Quality);
        Samples_Quality.clear();
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
void Analyser_Quality::Show()
{
    MainBox->show();
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_Quality::GetInformation(QVector<QString> *info)
{
    info->append("0x0002");
    info->append(tr("Quality_PCR"));
}
//-----------------------------------------------------------------------------
//--- readCommonSettings()
//-----------------------------------------------------------------------------
void Analyser_Quality::readCommonSettings()
{
    QString text;
    QString dir_path = qApp->applicationDirPath();
    QSettings *CommonSettings = new QSettings(dir_path + "/tools/ini/preference.ini", QSettings::IniFormat);

    CommonSettings->beginGroup("Common");

        //... Language ...
        text = CommonSettings->value("language","ru").toString();

        if(translator.load(":/translations/analyser_quality_" + text + ".qm"))
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
void Analyser_Quality::Analyser(rt_Protocol *prot)
{
    int i,j,k,m;
    int id;
    QString text,str;
    QString name_res, value_res;
    double dvalue;
    QStringList list;
    bool use_Ct;
    bool ok;

    rt_GroupSamples *group;
    rt_Sample       *sample;
    rt_Tube         *tube;
    rt_Channel      *channel;
    rt_Preference   *property;
    sample_Quality  *sample_Q;

    rt_TubeTest     *tube_test;
    rt_ChannelTest  *channel_test;
    QVector<short>  Type;
    bool is_IC;
    QMap<int,int> map_Specific;
    QMap<int,int> map_IC;
    int result_Specific, result_IC;

    qDeleteAll(Samples_Quality);
    Samples_Quality.clear();

    for(i=0; i<prot->Plate.groups.size(); i++)
    {
        group = prot->Plate.groups.at(i);
        for(j=0; j<group->samples.size(); j++)
        {
            sample = group->samples.at(j);
            if(sample->p_Test->header.Type_analysis == 0x0002)  // Quality tests
            {
                sample_Q = new sample_Quality();
                Samples_Quality.append(sample_Q);
                sample_Q->unique_name = QString::fromStdString(sample->Unique_NameSample);      // unique_name
                sample_Q->ptest = sample->p_Test;                                               // ptest
                tube = sample->tubes.at(0);
                sample_Q->pos = tube->pos;                                                      // pos

                map_Specific.clear();
                map_IC.clear();

                // use Cp or Ct ...
                use_Ct = false;
                foreach(property, sample_Q->ptest->preference_Test)
                {
                    if(property->name == "use_Threshold")
                    {
                        if(property->value == "True") use_Ct = true;
                        break;
                    }
                }
                // Specific or IC
                Type.clear();
                tube_test = sample_Q->ptest->tubes.at(0);
                foreach(channel_test, tube_test->channels)
                {
                    foreach(property, channel_test->preference_ChannelTest)
                    {
                        is_IC = false;
                        if(property->name == "specific&ic")
                        {
                            if(property->value == "IC") is_IC = true;
                            break;
                        }
                    }
                    if(is_IC) Type.append(1);   // IC
                    else Type.append(0);        // Specific
                }
                sample_Q->Type = Type;
                //

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
                            sample_Q->Cp.append(dvalue);

                            if(sample_Q->Type.at(k) == 0) map_Specific.insert(k,(int)dvalue);
                            else  map_IC.insert(k,(int)dvalue);

                            break;
                        }
                    }
                }

                // Quality Analysis: ...
                result_Specific = -1;
                result_IC = -1;
                if(map_Specific.size()) result_Specific = 0;
                foreach(m, map_Specific.values()) {if(m>0) result_Specific = 1;}
                if(map_IC.size()) result_IC = 0;
                foreach(m, map_IC.values()) {if(m>0) result_IC = 1;}

                sample_Q->result = "?";

                if(result_Specific == 1)                        sample_Q->result = "+";
                if(result_Specific == 0 && result_IC == 1)      sample_Q->result = "-";
                if(result_Specific == 0 && result_IC == -1)     sample_Q->result = "-";
                //...???...
                if(result_Specific == 0 && result_IC == 0)      sample_Q->result = "?";
                if(result_Specific == -1 && result_IC == -1)    sample_Q->result = "?";
                if(result_Specific == -1)                       sample_Q->result = "?";

                // Quality_Result to tube_Results
                str = QString("Quality_Result=%1").arg(sample_Q->result);
                tube->result_Tube.push_back(str.toStdString());
            }
        }
    }

    Prot = prot;
    Fill_ResultsTable();
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_Quality::Enable_Tube(QVector<short> *e)
{
    Fill_ResultsTable();
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_Quality::Select_Tube(int pos)
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
void Analyser_Quality::Save_Results(char *fname)
{
    int i,j,k,m;
    bool ok;
    bool save = false;

    QString text(fname);
    QString ID_Test,str;
    int method;
    int pos;
    sample_Quality  *sample_Q;

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

    QVector<QString> Quality_Tests;
    QVector<string> Y;

    // 1. Read & Edit Info
    QFile file(text);
    if(file.exists() && file.open(QIODevice::ReadOnly))
    {
        if(doc.setContent(&file))
        {
            file.close();

            root = doc.documentElement();

            // Tests with method == 0x0002
            tests = root.firstChildElement("tests");
            if(!tests.isNull())
            {
                for(i=0; i<tests.childNodes().size(); i++)
                {
                    node = tests.childNodes().at(i);
                    ID_Test = node.firstChildElement("IDTest").text();
                    method = node.firstChildElement("method").text().toInt(&ok);
                    if(method == 0x0002) Quality_Tests.append(ID_Test);
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
                        if(Quality_Tests.contains(ID_Test))
                        {
                            save = true;
                            tubes = sample.firstChildElement("tubes");
                            for(k=0; k<tubes.childNodes().size(); k++)
                            {
                                item_tubes = tubes.childNodes().at(k);
                                pos = item_tubes.firstChildElement("pos").text().toInt(&ok);
                                foreach(sample_Q, Samples_Quality)
                                {
                                    if(sample_Q->pos == pos)
                                    {
                                        Y.clear();
                                        str = "Quality_Result=" + sample_Q->result;
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
void Analyser_Quality::contextMenu_ResultsTable()
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
void Analyser_Quality::to_ClipBoard_ByColumn()
{
    QString text = ResultsTable_ToDigits_ByColumn();

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(text);
}
//-----------------------------------------------------------------------------
//--- to_ClipBoard()
//-----------------------------------------------------------------------------
void Analyser_Quality::to_ClipBoard()
{
    QString text = ResultsTable_ToDigits();

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(text);
}
//-----------------------------------------------------------------------------
//--- to_Excel()
//-----------------------------------------------------------------------------
void Analyser_Quality::to_Excel()
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
QString Analyser_Quality::ResultsTable_ToDigits_ByColumn()
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
QString Analyser_Quality::ResultsTable_ToDigits()
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
void Analyser_Quality::Fill_ResultsTable()
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
    sample_Quality   *sample_Q;

    bool status_background = false;

    int row_plate,col_plate;
    Prot->Plate.PlateSize(Prot->count_Tubes,row_plate,col_plate);

    header << tr("Pos") << tr("Name") << tr("Cp(Ct)") << tr("Quality") << tr("Result");
    Results_Table->clear();
    Results_Table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    Results_Table->setColumnCount(5);
    Results_Table->setHorizontalHeaderLabels(header);
    Results_Table->verticalHeader()->hide();
    Results_Table->setColumnWidth(0,50);    // pos
    Results_Table->setColumnWidth(1,70);    // name sample
    Results_Table->setColumnWidth(2,100);    // Cp
    Results_Table->setColumnWidth(3,40);    // Quality result
    Results_Table->setColumnWidth(4,90);    // Common result
    for(i=0; i<Results_Table->columnCount(); i++) Results_Table->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Fixed);
    Results_Table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    Results_Table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);


    foreach(sample_Q, Samples_Quality)
    {
        if(!Prot->enable_tube.at(sample_Q->pos)) continue;
        count += sample_Q->Cp.size();
    }

    Results_Table->setRowCount(count);

    Samples_Background.clear();
    Samples_Background.reserve(count);

    // Load data
    foreach(sample_Q, Samples_Quality)
    {
        pos = sample_Q->pos;
        if(!Prot->enable_tube.at(pos)) continue;

        status_background = !status_background;

        act_channels = sample_Q->ptest->header.Active_channel;
        count = sample_Q->Cp.size();
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
                            item->setText(Convert_IndexToName(sample_Q->pos, col_plate));
                            break;
                case 1:         // Name
                            text = QString("%1 (%2)").arg(sample_Q->unique_name).arg(QString::fromStdString(sample_Q->ptest->header.Name_Test));
                            item->setText(text);
                            break;
                case 2:         // Cp
                            text = QString::number(sample_Q->Cp.at(i),'f',1);
                            if(sample_Q->Cp.at(i) == 0.) text = "-";
                            text += QString("~%1").arg(id_ch);
                            item->setText(text);
                            break;
                case 3:         // Quality value
                            text = "-";
                            if(sample_Q->Cp.at(i) > 0.) text = "+";
                            if(sample_Q->Type.at(i) == 1) text += tr("(IC)");
                            item->setText(text);
                            break;
                case 4:         // Result
                            item->setText(sample_Q->result);
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
        Results_Table->setSpan(row_current-count,4,count,1);
    }
}
//-----------------------------------------------------------------------------
//--- ResultsTable_ChangedCurrentCell(int, int, int, int)
//-----------------------------------------------------------------------------
void Analyser_Quality::ResultsTable_ChangedCurrentCell(int cur_row, int cur_col, int prev_row, int prev_col)
{
    QTableWidgetItem *item;
    QString text;
    int pos = 0;
    int count_col,count_row;

    if(!Prot || cur_row < 0 || cur_col < 0) return;

    int count_tubes = Prot->count_Tubes;
    Prot->Plate.PlateSize(count_tubes, count_row, count_col);

    if(cur_col > 0)
    {
        Results_Table->setCurrentCell(cur_row,0);
    }

    item = Results_Table->item(cur_row,0);
    text = item->text();
    pos = Convert_NameToIndex(text,count_col);
    Select_Curve(pos);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_Quality::Select_Curve(int pos)
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
    int i,j;
    QRect rect;
    QString text, str;
    int row,col;
    int id;
    QColor color;
    QFont f = qApp->font();
    painter->setFont(f);
    int font_size = f.pointSize();
    bool ok;
    QFontMetrics fm = painter->fontMetrics();
    double dvalue;
    QStringList list;
    double coef_conc = 1.;
    double dmax, width_conc;
    QFont TimesFont("Times", 14, QFont::Bold);
    QColor bg_color[COUNT_CH] = {0xD8E2F1,0xD8E9D8,0xFEF0CB,0xFFE6FF,0xFFE6FF};
    QPixmap pixmap(":/images/fam_flat.png");

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

    case 3:     str = text.left(1);
                f.setBold(true);
                painter->setFont(f);
                painter->drawText(rect, Qt::AlignCenter, str);
                str = text.mid(1);
                if(str.length())
                {
                    f.setBold(false);
                    font_size -= 2;
                    f.setPointSize(font_size);
                    painter->setFont(f);
                    painter->setPen(Qt::gray);
                    painter->drawText(rect, Qt::AlignRight | Qt::AlignVCenter, str);
                    painter->setPen(Qt::black);
                }
                break;

    case 4:     if(text.contains("+"))  color = QColor(255, 128, 128);
                if(text.contains("-"))  color = QColor(200, 255, 200);
                if(text.contains("?"))  color = QColor(255, 255, 128);
                painter->fillRect(option.rect, color);
                font_size += 4;
                f.setPointSize(font_size);
                f.setBold(true);
                f.setFamily("Times New Roman");
                painter->setFont(f);
                painter->drawText(rect, Qt::AlignCenter, text);
                break;

    default:    break;
    }

}
