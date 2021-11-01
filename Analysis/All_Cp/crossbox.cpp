#include "crossbox.h"

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
CrossBox::CrossBox(QWidget *parent):
    QGroupBox(parent)
{
    QList<int> list_size;

    setObjectName("Transparent");

    QHBoxLayout *main_layout = new QHBoxLayout;
    main_layout->setMargin(0);
    main_layout->setSpacing(6);
    setLayout(main_layout);

    main_spl = new QSplitter(Qt::Horizontal, this);
    main_spl->setHandleWidth(6);
    main_spl->setChildrenCollapsible(false);
    main_layout->addWidget(main_spl);

    //... CrossTable ...
    box_Table = new QGroupBox(tr("   CrossTable Samples&&Tests: "),this);
    box_Table->setObjectName("Transparent");
    QVBoxLayout *layout_table = new QVBoxLayout;
    box_Table->setLayout(layout_table);

    CrossTable = new QTableWidget(0,0, this);
    CrossTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    //CrossTable->setSelectionBehavior(QAbstractItemView::SelectColumns);
    CrossTable->setSelectionMode(QAbstractItemView::SingleSelection);
    CrossTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    CrossTable->setContextMenuPolicy(Qt::CustomContextMenu);
    CrossTable->setFont(qApp->font());
    CrossTable->horizontalHeader()->setFont(qApp->font());
    CrossTable->verticalHeader()->setFont(qApp->font());
    Delegate = new Cross_ItemDelegate;
    Delegate->mc_rejime = false;
    CrossTable->setItemDelegate(Delegate);
    connect(CrossTable, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu_CrossTable()));
    connect(CrossTable, SIGNAL(currentCellChanged(int,int,int,int)), this, SLOT(SelelectCell_CrossTable(int,int)));

    layout_table->setMargin(4);
    layout_table->addSpacing(20);
    layout_table->addWidget(CrossTable);
    //layout_table->addStretch();

    //... Tests ...
    box_Test = new QGroupBox(tr("   Tests:"), this);
    box_Test->setObjectName("Transparent");
    QVBoxLayout *layout_test = new QVBoxLayout;
    box_Test->setLayout(layout_test);

    CrossTests = new QTableWidget(0,0, this);
    CrossTests->setSelectionMode(QAbstractItemView::SingleSelection);
    CrossTests->setEditTriggers(QAbstractItemView::NoEditTriggers);
    CrossTests->setContextMenuPolicy(Qt::CustomContextMenu);
    CrossTests->setFont(qApp->font());
    CrossTests->horizontalHeader()->setFont(qApp->font());
    CrossTests->verticalHeader()->setFont(qApp->font());
    Delegate_Test = new CrossTest_ItemDelegate;
    CrossTests->setItemDelegate(Delegate_Test);

    layout_test->setMargin(4);
    layout_test->addSpacing(20);
    layout_test->addWidget(CrossTests);
    //layout_test->addStretch(1);

    //main_spl->addWidget(CrossTests);
    //main_spl->addWidget(CrossTable);
    main_spl->addWidget(box_Test);
    main_spl->addWidget(box_Table);

    //... Actions ...
    copy_to_clipboard = new QAction(QIcon(":/images/copy_clipboard.png"),tr("copy to ClipBoard"), this);
    copy_to_excelfile = new QAction(QIcon(":/images/copy_excel.png"),tr("copy to Excel"), this);
    connect(copy_to_clipboard, SIGNAL(triggered(bool)), this, SLOT(to_ClipBoard()));
    connect(copy_to_excelfile, SIGNAL(triggered(bool)), this, SLOT(to_Excel()));

    choice_Cp = new QAction(tr("Cp"), this);
    choice_Cp->setCheckable(true);
    choice_Ct = new QAction(tr("Ct"), this);
    choice_Ct->setCheckable(true);
    choice_Peaks = new QAction(tr("Temperature Peaks"), this);
    choice_Peaks->setCheckable(true);
    choice_rejime = new QActionGroup(this);
    choice_rejime->addAction(choice_Cp);
    choice_rejime->addAction(choice_Ct);
    choice_rejime->addAction(choice_Peaks);
    choice_Cp->setChecked(true);
    create_FileTempCorrection = new QAction(tr("Create a file of the temperature correction"), this);
    create_FileTempCorrection->setEnabled(false);
    connect(choice_rejime, SIGNAL(triggered(QAction*)), this, SLOT(reFill_CrossInfo(QAction*)));
    connect(create_FileTempCorrection, SIGNAL(triggered(bool)), this, SLOT(Create_FileCorrection()));

    output_Mode = "Cp=";
    prot = NULL;

    list_size << 100 << 500;
    main_spl->setSizes(list_size);

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
CrossBox::~CrossBox()
{
    delete Delegate;
    delete CrossTable;
    delete CrossTests;

    delete box_Test;
    delete box_Table;
    delete main_spl;

    Test_Num.clear();
    qDeleteAll(Test_Names.values());
    Test_Names.clear();
    Test_Offset.clear();

    Sample_Test.clear();
    Sample_Names.clear();
    qDeleteAll(Sample_Cp.values());
    Sample_Cp.clear();
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void CrossBox::Fill_CrossInfo(rt_Protocol *P)
{
    int i,j,k,m,n;
    int num;
    int pos;
    QStringList header_Test, header_Sample;
    QStringList header;
    QString name_test, name_ch;
    QString name_sample;
    QString id_test, id_sample;
    QString text, str;
    QStringList *list_Cp, *test_names;
    QTableWidgetItem *item;
    int active_ch;
    QString fluor_name[COUNT_CH] = FLUOR_NAME;
    bool enable_sample;
    bool enable_pos;

    rt_Test         *test;
    rt_TubeTest     *tube;
    rt_ChannelTest  *channel;
    rt_GroupSamples *group;
    rt_Sample       *sample;
    rt_Tube         *Tube;
    rt_Channel      *Channel;    

    prot = P;
    if(!P) return;

    for(i = CrossTable->rowCount(); i>0; i--) CrossTable->removeRow(i-1);
    CrossTable->clear();
    for(i = CrossTests->rowCount(); i>0; i--) CrossTests->removeRow(i-1);
    CrossTests->clear();

    Test_Num.clear();
    qDeleteAll(Test_Names.values());
    Test_Names.clear();
    Test_Offset.clear();

    Sample_Num.clear();
    Sample_Test.clear();
    Sample_Names.clear();
    qDeleteAll(Sample_Cp.values());
    Sample_Cp.clear();

    //... Tests ...
    num = 0;
    for(i=0; i<prot->tests.size(); i++)
    {
        test = prot->tests.at(i);
        name_test = QString::fromStdString(test->header.Name_Test);
        if(map_TestTranslate->size()) name_test = map_TestTranslate->value(name_test, name_test);
        Test_Num.insert(i,QString::fromStdString(test->header.ID_Test));
        test_names = new QStringList();
        Test_Offset.insert(QString::fromStdString(test->header.ID_Test), num);
        active_ch = test->header.Active_channel;

        if(test->tubes.size() == 0)
        {
            for(j=0; j<COUNT_CH; j++)
            {
                if(active_ch & (1<<j*4))
                {
                    name_ch = fluor_name[j];
                    text = QString("%4/%5").arg(name_test).arg(name_ch);
                    header_Test.append(text);
                    test_names->append(text);
                    num++;
                }
            }
        }
        else
        {
            for(j=0; j<test->tubes.size(); j++)
            {
                tube = test->tubes.at(j);
                for(k=0; k<tube->channels.size(); k++)
                {
                    channel = tube->channels.at(k);
                    name_ch = QString::fromStdString(channel->name);
                    if(map_TestTranslate->size()) name_ch = map_TestTranslate->value(name_ch,name_ch);
                    text = QString("%4/%5").arg(name_test).arg(name_ch);
                    header_Test.append(text);
                    test_names->append(text);
                    num++;
                }
            }
        }

        Test_Names.insert(QString::fromStdString(test->header.ID_Test), test_names);
        //qDebug() << "test_names: " << *test_names;
    }
    //qDebug() << "Test_Nun: " << Test_Num;
    //qDebug() << "Test_Names: " << Test_Names;

    //... Samples ...
    num = 0;
    for(i=0; i<prot->Plate.groups.size(); i++)
    {
        group = prot->Plate.groups.at(i);
        for(j=0; j<group->samples.size(); j++)
        {
            sample = group->samples.at(j);
            name_sample = QString::fromStdString(sample->Unique_NameSample);
            header_Sample.append(name_sample + "   ");
            Sample_Num.insert(num, QString::fromStdString(sample->ID_Sample));
            num++;
            Sample_Test.insert(QString::fromStdString(sample->ID_Sample), QString::fromStdString(sample->ID_Test));
            list_Cp = new QStringList();
            Sample_Cp.insert(QString::fromStdString(sample->ID_Sample), list_Cp);

            enable_sample = false;

            for(k=0; k<sample->tubes.size(); k++)
            {
                Tube = sample->tubes.at(k);
                enable_pos = prot->enable_tube.at(Tube->pos);
                if(enable_pos) enable_sample = true;

                for(m=0; m<Tube->channels.size(); m++)
                {
                    Channel = Tube->channels.at(m);
                    str = "";
                    for(n=0; n<Channel->result_Channel.size(); n++)
                    {
                        text = QString::fromStdString(Channel->result_Channel.at(n));
                        if(text.startsWith(output_Mode))
                        {
                            pos = text.indexOf("=");
                            str = text.mid(pos+1);                            
                            break;
                        }
                    }
                    if(!enable_pos) str = "";
                    list_Cp->append(str);
                }
            }

            //qDebug() << "Sample: " << QString::fromStdString(sample->Unique_NameSample) << enable_sample << *list_Cp;

            if(!enable_sample)  // remove last element
            {
                header_Sample.removeLast();
                num--;
                Sample_Num.remove(num);
                Sample_Cp.remove(QString::fromStdString(sample->ID_Sample));
                list_Cp->clear();
                delete list_Cp;
            }

        }
    }
   //qDebug() << "Samples: " << Sample_Num << Sample_Test << Sample_Names << Sample_Cp << header_Sample;


    //... CrossTest ...
    CrossTests->setRowCount(header_Test.size());
    CrossTests->setColumnCount(1);
    header << tr(" ");
    CrossTests->setHorizontalHeaderLabels(header);
    CrossTests->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    CrossTests->horizontalHeader()->setFont(qApp->font());
    for(i=0; i<CrossTests->rowCount(); i++)
    {
        //CrossTests->setRowHeight(i, 20);
        item = new QTableWidgetItem();
        CrossTests->setItem(i,0, item);
        item->setText(header_Test.at(i));
    }
    CrossTests->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    CrossTests->verticalHeader()->setFont(qApp->font());


    //... CrossTable ...
    CrossTable->setRowCount(header_Sample.size());
    CrossTable->setColumnCount(header_Test.size());
    CrossTable->setVerticalHeaderLabels(header_Sample);    
    CrossTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    for(i=0; i<CrossTable->rowCount(); i++)
    {
        //CrossTable->setRowHeight(i, 20);
        for(j=0; j<CrossTable->columnCount(); j++)
        {
            item = new QTableWidgetItem();
            CrossTable->setItem(i,j, item);
            item->setText("");
        }
    }
    CrossTable->horizontalHeader()->setMinimumSectionSize(40);
    CrossTable->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    //... Load Result ...
    //qDebug() << "Load_Result:" << Sample_Num.values().size() << Sample_Num << header_Sample.size();
    for(i=0; i<Sample_Num.values().size(); i++)
    {
        id_sample = Sample_Num.values().at(i);
        id_test = Sample_Test.value(id_sample);
        test_names = Test_Names.value(id_test);
        list_Cp = Sample_Cp.value(id_sample);
        num = Test_Offset.value(id_test);
        for(j=0; j<test_names->size(); j++)
        {
            //str = test_names->at(j);
            //k = header_Test.indexOf(str);
            //if(k < 0) continue;
            if(j >= list_Cp->size()) break;

            item = CrossTable->item(i,j+num);
            item->setText(list_Cp->at(j));
        }
    }

    //qDebug() << "the end";

    //CrossTable->resizeRowsToContents();
    //CrossTests->resizeRowsToContents();

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void CrossBox::Create_FileCorrection()
{
    int i,j;
    int id;
    QTableWidgetItem *item;
    QString text, str;
    QString filename = "";

    QVector<QString> list;

    list.fill("0,0", prot->count_Tubes);

    filename = QFileDialog::getSaveFileName(NULL, tr("Save to File(*.tcr)..."),
                                            "", tr("File correction (*.tcr)"));

    if(filename.isEmpty()) return;


    id = CrossTable->currentColumn();
    str = "";

    j = 0;
    for(i=0; i<CrossTable->rowCount(); i++)
    {
        item = CrossTable->item(i,id);
        text = item->text();

        text.replace(".",",");
        list.replace(j, text);
        j++;
    }

    QFile file(filename);
    QTextStream out(&file);

    if(file.open(QFile::WriteOnly | QFile::Text))
    {
        foreach(text, list)
        {
            if(!str.isEmpty()) str += "\n";
            str += text;
        }

        out << str;

        file.close();
    }
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void CrossBox::reFill_CrossInfo(QAction *action)
{
    bool state_mc = false;
    Delegate->mc_rejime = false;

    if(action == choice_Cp) output_Mode = "Cp=";
    if(action == choice_Ct) output_Mode = "Ct=";
    if(action == choice_Peaks) {output_Mode = "mc_peaks="; Delegate->mc_rejime = true; state_mc = true;}

    if(prot) Fill_CrossInfo(prot);

    create_FileTempCorrection->setEnabled(state_mc);
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
QString CrossBox::CrossTable_ToDigits()
{
    int i,j,k;    
    QString text, str = "";
    QString sample_name;
    QTableWidgetItem *item;
    QStringList list;
    double dvalue;
    bool ok;

    str = "Tests:";
    for(i=0; i<CrossTests->rowCount(); i++)
    {
        item = CrossTests->item(i,0);
        str += "\t" + item->text();
    }
    str += "\r\n";
    str += "Samples: \r\n";

    for(i=0; i<CrossTable->rowCount(); i++)
    {
        sample_name = CrossTable->verticalHeaderItem(i)->text();
        for(j=0; j<CrossTable->columnCount(); j++)
        {
            item = CrossTable->item(i,j);
            text = item->text();

            if(!Delegate->mc_rejime) // only for
            {
                dvalue = text.toDouble(&ok);
                if(!ok) dvalue = 0.0;
                text = QString::number(dvalue, 'f', 1);
            }

            text.replace(".",",");            
            list.append(text);
        }

        if(list.size())
        {            
            for(k=0; k<list.size(); k++)
            {
                if(k == 0) str += sample_name + "\t";
                str += list.at(k) + "\t";
            }
            str += "\r\n";

        }
        else str += "\r\n";

        list.clear();
    }

    return(str);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void CrossBox::SelelectCell_CrossTable(int row, int col)
{
    int i;
    int pos = 0;
    QTableWidgetItem *item;


    for(i=0; i<CrossTable->columnCount(); i++)
    {
        item = CrossTable->item(row,i);
    }


    //emit sSelect_Curve(pos);
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void CrossBox::contextMenu_CrossTable()
{
    QMenu menu;
    menu.setStyleSheet("QMenu::item:selected {background-color: #d7d7ff; color: black;}");
    menu.setFont(qApp->font());

    menu.addAction(copy_to_clipboard);
    menu.addAction(copy_to_excelfile);
    menu.addSeparator()->setText(tr("Output mode"));
    menu.addAction(choice_Cp);
    menu.addAction(choice_Ct);
    menu.addAction(choice_Peaks);
    menu.addSeparator();
    menu.addAction(create_FileTempCorrection);



    menu.exec(QCursor::pos());

    menu.clear();
}

//-----------------------------------------------------------------------------
//--- to_Excel()
//-----------------------------------------------------------------------------
void CrossBox::to_Excel()
{
    QString filename = "";
    QByteArray ba;

    QString text = CrossTable_ToDigits();
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
//--- to_ClipBoard()
//-----------------------------------------------------------------------------
void CrossBox::to_ClipBoard()
{
    QString text = CrossTable_ToDigits();

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(text);
}

//-----------------------------------------------------------------------------
//--- Draw Cp Table
//-----------------------------------------------------------------------------
void Cross_ItemDelegate::paint(QPainter *painter,
                               const QStyleOptionViewItem &option,
                               const QModelIndex &index) const
{    
    QRect rect;
    QString text;    
    QFont font = qApp->font();
    double dvalue;
    bool ok;

    QStyleOptionViewItem  viewOption(option);
    viewOption.state &= ~QStyle::State_HasFocus;    // disable state HasFocus
    QStyledItemDelegate::paint(painter, viewOption, index);

    rect = option.rect;

    text = index.data().toString();

    //----------------------

    //__1. Background
    int row = index.row();
    if(option.state & QStyle::State_Selected)
    {
        painter->fillRect(option.rect, QColor(215,215,255));
    }
    else
    {
        painter->fillRect(option.rect, QColor(245,245,245));
        //if(div(row,2).rem)  painter->fillRect(option.rect, QColor(240,240,240));
        //else painter->fillRect(option.rect, QColor(245,245,245));
    }
    //-----------------------


    //__2. Data
    painter->setPen(QPen(Qt::black,1,Qt::SolidLine));
    //font = painter->font();
    font.setBold(false);
    painter->setFont(font);
    //qDebug() << "font: " << font;

    text = index.data().toString();
    if(text.trimmed().isEmpty()) return;

    if(!mc_rejime)
    {
        dvalue = text.toDouble(&ok);
        if(!ok || dvalue == 0.0) text = "-";
        else
        {
            text = QString::number(dvalue, 'f', 1);
        }
    }

    if(option.state & QStyle::State_Selected) {;}
    else  painter->fillRect(option.rect, QColor(255,255,255));

    painter->drawText(rect, Qt::AlignHCenter | Qt::AlignVCenter, text);

}
//-----------------------------------------------------------------------------
//--- Draw Cross Test
//-----------------------------------------------------------------------------
void CrossTest_ItemDelegate::paint(QPainter *painter,
                                   const QStyleOptionViewItem &option,
                                   const QModelIndex &index) const
{
    QRect rect;
    QString text;
    QFont font = qApp->font();
    double dvalue;
    bool ok;

    QStyleOptionViewItem  viewOption(option);
    viewOption.state &= ~QStyle::State_HasFocus;    // disable state HasFocus
    QStyledItemDelegate::paint(painter, viewOption, index);

    rect = option.rect;

    text = index.data().toString();

    //----------------------

    //__1. Background
    int row = index.row();
    if(option.state & QStyle::State_Selected)
    {
        painter->fillRect(option.rect, QColor(215,215,255));
    }
    else
    {
        painter->fillRect(option.rect, QColor(255,255,255));
        //if(div(row,2).rem)  painter->fillRect(option.rect, QColor(240,240,240));
        //else painter->fillRect(option.rect, QColor(245,245,245));
    }
    //-----------------------


    //__2. Data
    painter->setPen(QPen(Qt::black,1,Qt::SolidLine));
    //font = painter->font();
    font.setBold(false);
    painter->setFont(font);

    text = index.data().toString();
    if(text.trimmed().isEmpty()) return;

    painter->drawText(rect, Qt::AlignLeft | Qt::AlignVCenter, text);

}
