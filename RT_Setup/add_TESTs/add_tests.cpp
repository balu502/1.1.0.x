#include "add_tests.h"

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Add_TESTs::Add_TESTs(QWidget *parent): QDialog(parent)
{
    //QPalette palette;
    QFontMetrics fm(qApp->font());

    readCommonSettings();
    setFont(qApp->font());

    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowTitle(tr("Add Samples/Tests"));
    //setWindowIcon(QIcon(":/images/add_sample.png"));
    setFixedSize(620,520);

    QVBoxLayout *layout = new QVBoxLayout();
    this->setLayout(layout);
    layout->setMargin(2);

    Tab_ADD = new QTabWidget(this);

    //... samples ...
    Box_samples = new QGroupBox(this);
    Box_samples->setObjectName("Transparent");
    QVBoxLayout *layout_samples = new QVBoxLayout();
    Box_samples->setLayout(layout_samples);
    Label_SAMPLE = new QLabel(tr("Sample:"), this);
    Edit_Sample = new QLineEdit(this);
    QHBoxLayout *layout_01 = new QHBoxLayout();
    layout_01->addWidget(Label_SAMPLE,0,Qt::AlignLeft);
    layout_01->addWidget(Edit_Sample,1);
    Tree_Tests = new QTreeWidget(this);
    Tree_Tests->setFont(qApp->font());
    QHBoxLayout *layout_02 = new QHBoxLayout();
    sample_Kind = new QRadioButton(tr("sample"), this);
    Kplus_Kind = new QRadioButton(tr("C+"), this);
    Kminus_Kind = new QRadioButton(tr("C-"), this);
    layout_02->addWidget(sample_Kind,0,Qt::AlignLeft);
    layout_02->addWidget(Kplus_Kind,0,Qt::AlignLeft);
    layout_02->addWidget(Kminus_Kind,0,Qt::AlignLeft);
    layout_02->addStretch(1);
    sample_Kind->setChecked(true);
    layout_samples->addLayout(layout_01);
    layout_samples->addWidget(Tree_Tests, 1);
    layout_samples->addLayout(layout_02);
    //...

    //... Container ...
    Box_containers = new QGroupBox(this);
    Box_containers->setObjectName("Transparent");
    QVBoxLayout *layout_containers = new QVBoxLayout();
    Box_containers->setLayout(layout_containers);
    Label_CONTAINER = new QLabel(tr("Sample:"), this);
    Edit_Container = new QLineEdit(this);
    Label_CONTAINER_plus = new QLabel("*", this);
    Count_Container = new QSpinBox(this);
    Count_Container->setRange(1,384);
    Count_Container->setSingleStep(1);
    Count_Container->setValue(1);
    Count_Container->setFont(QFont("Times New Roman", 14, QFont::Bold));
    QHBoxLayout *layout_01_container = new QHBoxLayout();
    layout_01_container->addWidget(Label_CONTAINER,0,Qt::AlignLeft);
    layout_01_container->addWidget(Edit_Container,1);
    layout_01_container->addWidget(Label_CONTAINER_plus,0,Qt::AlignRight);
    layout_01_container->addWidget(Count_Container,0,Qt::AlignRight);
    Tree_Container = new QTreeWidget(this);
    Tree_Container->setFont(qApp->font());
    QHBoxLayout *layout_02_container = new QHBoxLayout();
    sample_container_Kind = new QRadioButton(tr("sample"), this);
    Kplus_container_Kind = new QRadioButton(tr("C+"), this);
    Kminus_container_Kind = new QRadioButton(tr("C-"), this);
    layout_02_container->addWidget(sample_container_Kind,0,Qt::AlignLeft);
    layout_02_container->addWidget(Kplus_container_Kind,0,Qt::AlignLeft);
    layout_02_container->addWidget(Kminus_container_Kind,0,Qt::AlignLeft);
    layout_02_container->addStretch(1);
    sample_container_Kind->setChecked(true);
    layout_containers->addLayout(layout_01_container);
    layout_containers->addWidget(Tree_Container, 1);
    layout_containers->addLayout(layout_02_container);
    //...


    Box_tests = new QGroupBox(this);
    Box_tests->setObjectName("Transparent");

    QVBoxLayout *layout_box = new QVBoxLayout();

    Box_tests->setLayout(layout_box);
    QHBoxLayout *layout_0 = new QHBoxLayout();
    QHBoxLayout *layout_1 = new QHBoxLayout();
    QHBoxLayout *layout_2 = new QHBoxLayout();
    QHBoxLayout *layout_3 = new QHBoxLayout();
    QHBoxLayout *layout_4 = new QHBoxLayout();
    QHBoxLayout *layout_5 = new QHBoxLayout();
    QHBoxLayout *layout_6 = new QHBoxLayout();
    QHBoxLayout *layout_7 = new QHBoxLayout();
    QHBoxLayout *layout_8 = new QHBoxLayout();
    QHBoxLayout *layout_9 = new QHBoxLayout();
    QHBoxLayout *layout_10 = new QHBoxLayout();
    //QHBoxLayout *layout_11 = new QHBoxLayout();

    Label_Name = new QLabel(tr("Test:"), this);
    Label_Type = new QLabel(tr("Analysis type:"), this);
    //Label_Comments = new QLabel(tr("Description:"), this);
    Combo_Tests = new QComboBox(this);
    Combo_Tests->setStyleSheet("background-color: #ffffff; selection-background-color: #d7d7ff; selection-color: black;");
    //Line_Type = new QLineEdit(this);
    Combo_Type = new QComboBox(this);
    Combo_Type->setStyleSheet("background-color: #ffffff; selection-background-color: #d7d7ff; selection-color: black;");
    //Text_Comments = new QTextEdit(this);
    Label_Samples = new QLabel(tr("Count of Samples:"), this);
    Label_countTubes = new QLabel("(x1)", this);
    Count_Samples = new QSpinBox(this);
    Count_Samples->setRange(1,384);
    Count_Samples->setSingleStep(1);
    Count_Samples->setValue(1);
    //palette.setColor(QPalette::HighlightedText,Qt::black);
    //palette.setColor(QPalette::Base,QColor(255,255,225));
    //palette.setColor(QPalette::Highlight,QColor(180,180,255));
    //Count_Samples->setPalette(palette);
    Count_Samples->setFont(QFont("Times New Roman", 14, QFont::Bold));
    Count_Double = new QSpinBox(this);
    Count_Double->setRange(1,384);
    Count_Double->setSingleStep(1);
    Count_Double->setValue(1);
    //Count_Double->setPalette(palette);
    Count_Double->setFont(QFont("Times New Roman", 10, QFont::Bold));
    Label_Double = new QLabel(tr("Doubles:"), this);
    Label_Kpos = new QLabel(tr("Count of C+:"), this);
    Label_countKpos = new QLabel("(x1)", this);
    Count_Kpos = new QSpinBox(this);
    Count_Kpos->setRange(0,384);
    Count_Kpos->setSingleStep(1);
    Count_Kpos->setValue(0);
    //Count_Kpos->setPalette(palette);
    Count_Kpos->setFont(QFont("Times New Roman", 10, QFont::Bold));
    Label_Kneg = new QLabel(tr("Count of C-:"), this);
    Label_countKneg = new QLabel("(x1)", this);
    Count_Kneg = new QSpinBox(this);
    Count_Kneg->setRange(0,384);
    Count_Kneg->setSingleStep(1);
    Count_Kneg->setValue(0);
    //Count_Kneg->setPalette(palette);
    Count_Kneg->setFont(QFont("Times New Roman", 10, QFont::Bold));

    Label_St = new QLabel(tr("Count of Standart:"), this);
    Label_countSt = new QLabel("(x1)", this);
    Count_St = new QSpinBox(this);
    Count_St->setRange(0,384);
    Count_St->setSingleStep(1);
    Count_St->setValue(0);
    //Count_St->setPalette(palette);
    Count_St->setFont(QFont("Times New Roman", 10, QFont::Bold));
    Label_DoubleSt = new QLabel(tr("Doubles:"), this);
    Count_DoubleSt = new QSpinBox(this);
    Count_DoubleSt->setRange(1,384);
    Count_DoubleSt->setSingleStep(1);
    Count_DoubleSt->setValue(1);
    //Count_DoubleSt->setPalette(palette);
    Count_DoubleSt->setFont(QFont("Times New Roman", 10, QFont::Bold));

    Label_St->setDisabled(true);
    Label_countSt->setDisabled(true);
    Count_St->setDisabled(true);
    Label_DoubleSt->setDisabled(true);
    Count_DoubleSt->setDisabled(true);

    List_Tests = new QTreeWidget(this);
    List_Tests->setFont(qApp->font());
    List_Tests->setSelectionMode(QAbstractItemView::SingleSelection);
    //Label_List = new QLabel(tr("List of tests:"), this);

    //Label_ListTypes = new QLabel(tr("List of Types:"), this);
    //List_Types = new QListWidget(this);

    // StyleSheets
    //if(style != "fusion") Combo_Tests->setStyleSheet("selection-background-color: #d7d7ff; selection-color: black;");
    //Combo_Tests->setFont(QFont("Times New Roman", 10, QFont::Bold));
    //if(style != "fusion") Combo_Type->setStyleSheet("selection-background-color: #d7d7ff; selection-color: black;");
    //Combo_Type->setFont(QFont("Times New Roman", 10, QFont::Bold));
    //List_Types->setStyleSheet("selection-background-color: #b4b4ff; selection-color: black;");
    //List_Types->setFont(QFont("Times New Roman", 10, QFont::Bold));

    //layout_11->addWidget(Label_ListTypes,0,Qt::AlignLeft);
    //layout_11->addWidget(List_Types,0,Qt::AlignRight);
    layout_0->addWidget(Label_Name,0,Qt::AlignLeft);
    layout_0->addWidget(Combo_Tests,0,Qt::AlignRight);
    layout_1->addWidget(Label_Type,0,Qt::AlignLeft);
    layout_1->addWidget(Combo_Type,0,Qt::AlignRight);

    Label_Name->setVisible(false);
    Combo_Tests->setVisible(false);
    Label_Type->setVisible(false);
    Combo_Type->setVisible(false);
    //layout_2->addWidget(Label_Comments,0,Qt::AlignLeft);
    //layout_2->addWidget(Text_Comments,0,Qt::AlignRight);
    //layout_2->addWidget(Label_List,0,Qt::AlignLeft);
    layout_2->addWidget(List_Tests,1);
    layout_3->addWidget(Label_Samples,0,Qt::AlignLeft);
    layout_4->addWidget(Count_Samples,0,Qt::AlignLeft);
    layout_4->addWidget(Label_countTubes,0,Qt::AlignLeft);
    layout_4->addStretch(1);
    layout_4->setSpacing(2);
    layout_3->addLayout(layout_4);
    layout_3->addStretch(1);    
    layout_3->addWidget(Label_Double,1,Qt::AlignRight);
    layout_3->addWidget(Count_Double,0,Qt::AlignRight);
    layout_5->addWidget(Label_Kpos,0,Qt::AlignLeft);
    layout_6->addWidget(Count_Kpos,0,Qt::AlignLeft);
    layout_6->addWidget(Label_countKpos,0,Qt::AlignLeft);
    layout_6->addStretch(1);
    layout_6->setSpacing(2);
    layout_5->addLayout(layout_6);
    layout_5->addStretch(1);
    layout_7->addWidget(Label_Kneg,0,Qt::AlignLeft);
    layout_8->addWidget(Count_Kneg,0,Qt::AlignLeft);
    layout_8->addWidget(Label_countKneg,0,Qt::AlignLeft);
    layout_8->addStretch(1);
    layout_8->setSpacing(2);
    layout_7->addLayout(layout_8);
    layout_7->addStretch(1);
    layout_9->addWidget(Label_St,0,Qt::AlignLeft);
    layout_10->addWidget(Count_St,0,Qt::AlignLeft);
    layout_10->addWidget(Label_countSt,0,Qt::AlignLeft);
    layout_10->addStretch(1);
    layout_10->setSpacing(2);
    layout_9->addLayout(layout_10);
    layout_9->addStretch(1);
    layout_9->addWidget(Label_DoubleSt,1,Qt::AlignRight);
    layout_9->addWidget(Count_DoubleSt,0,Qt::AlignRight);

    //layout_box->addLayout(layout_11);
    layout_box->addLayout(layout_1);
    layout_box->addLayout(layout_0);
    layout_box->addLayout(layout_2);
    layout_box->addLayout(layout_3);
    layout_box->addLayout(layout_5);
    layout_box->addLayout(layout_7);
    layout_box->addLayout(layout_9);

    //List_Types->setFixedWidth(250);

    Combo_Tests->setFixedWidth(250);
    Combo_Tests->setFixedHeight(22);
    Combo_Type->setFixedWidth(250);
    //Text_Comments->setFixedWidth(250);
    //Text_Comments->setReadOnly(true);
    //Text_Comments->setEnabled(false);

    int fixed_W = fm.width(Label_Samples->text()) * 1.1;

    Label_Samples->setFixedWidth(fixed_W);
    Label_Kpos->setFixedWidth(fixed_W);
    Label_Kneg->setFixedWidth(fixed_W);
    Label_St->setFixedWidth(fixed_W);

    Tab_ADD->addTab(Box_tests,tr("TEST with samples"));
    Tab_ADD->addTab(Box_samples,tr("SAMPLE with tests"));
    Tab_ADD->addTab(Box_containers,tr("Container"));

    QHBoxLayout *ctrl_layout = new QHBoxLayout();
    add_test = new QPushButton(tr("Add"), this);
    ok_button = new QPushButton("Ok", this);
    ok_button->setVisible(false);
    cancel_button = new QPushButton(tr("Close"), this);
    ctrl_layout->addStretch(1);
    ctrl_layout->addWidget(add_test,0,Qt::AlignRight);
    ctrl_layout->addWidget(ok_button,0,Qt::AlignRight);
    ctrl_layout->addWidget(cancel_button,0,Qt::AlignRight);
    ctrl_layout->setSpacing(2);

    /*main_progress = new QProgressBar(this);
    main_progress->setStyleSheet("QProgressBar:horizontal {border: 1px solid gray; border-radius: 3px; background: #E8E8E8; padding: 2px;}"
                "QProgressBar::chunk:horizontal {background: qlineargradient(x1: 0, y1: 0.1, x2: 1, y2: 0.1, stop: 0 #C4FFC4, stop: 1 lime);}");
    main_progress->setFixedHeight(14);
    main_progress->setRange(0,100);*/

    layout->addSpacing(10);
    layout->addWidget(Tab_ADD,1);
    //layout->addWidget(Box_tests,1);
    layout->addLayout(ctrl_layout);
    //layout->addWidget(main_progress);

    connect(cancel_button, SIGNAL(clicked(bool)), this, SLOT(close()));
    connect(ok_button, SIGNAL(clicked(bool)), this, SLOT(AddClose_Test()));
    connect(add_test, SIGNAL(clicked(bool)), this, SLOT(Add_Test()));
    connect(List_Tests, SIGNAL(itemSelectionChanged()), this, SLOT(select_test()));
    connect(List_Tests, SIGNAL(itemSelectionChanged()), this, SLOT(CheckState_AddButton()));
    connect(Combo_Tests, SIGNAL(currentIndexChanged(int)), this, SLOT(change_test(int)));    
    connect(Combo_Type, SIGNAL(currentIndexChanged(int)), this, SLOT(change_typeTest(int)));
    connect(Tab_ADD, SIGNAL(currentChanged(int)), this, SLOT(change_TabPage(int)));
    connect(Tab_ADD, SIGNAL(currentChanged(int)), this, SLOT(CheckState_AddButton()));
    connect(Edit_Sample, SIGNAL(editingFinished()), this, SLOT(change_NameSample()));
    connect(Edit_Container, SIGNAL(editingFinished()), this, SLOT(change_NameSample_forContainer()));
    connect(this, SIGNAL(signal_addSample(QString,QVector<rt_Test*>*,int,int)), this, SLOT(Increment_Sample(QString)));
    connect(Tree_Tests, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(CheckState_AddButton()));
    connect(Tree_Container, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(CheckState_AddButton()));

    add_test->setFocusPolicy(Qt::NoFocus);
    ok_button->setFocusPolicy(Qt::NoFocus);
    cancel_button->setFocusPolicy(Qt::NoFocus);

    label_gif = new QLabel(this);
    label_gif->setFixedSize(48,48);
    label_gif->raise();
    obj_gif = new QMovie(":/images/wait_1.gif");
    label_gif->setMovie(obj_gif);
    label_gif->setVisible(false);
    int w = width();
    int h = height();
    label_gif->move(w/2 - label_gif->width()/2, h/2 - label_gif->height()/2);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Add_TESTs::~Add_TESTs()
{
    Tab_ADD->blockSignals(true);
    List_Tests->blockSignals(true);
    Tree_Tests->blockSignals(true);
    Tree_Container->blockSignals(true);

    current_ListTests.clear();
    delete Box_tests;

    label_gif->setVisible(false);
    obj_gif->stop();
    delete obj_gif;
    delete label_gif;

    delete add_test;
    delete ok_button;
    delete cancel_button;
}
//-----------------------------------------------------------------------------
//--- readCommonSettings()
//-----------------------------------------------------------------------------
void Add_TESTs::readCommonSettings()
{
    QString text;
    QString dir_path = qApp->applicationDirPath();
    QSettings *CommonSettings = new QSettings(dir_path + "/tools/ini/preference.ini", QSettings::IniFormat);

    CommonSettings->beginGroup("Common");

        //... Language ...
        text = CommonSettings->value("language","ru").toString();
        if(translator.load(":/translations/add_" + text + ".qm"))
        {
            qApp->installTranslator(&translator);
        }

        //... Style ...
        text = CommonSettings->value("style","xp").toString();
        style = text.trimmed();

    CommonSettings->endGroup();
    delete CommonSettings;

    logo = QIcon("logotype.png");
    logonull = QIcon("logotype_null.png");
}

//-----------------------------------------------------------------------------
//--- AddClose_Test()
//-----------------------------------------------------------------------------
void Add_TESTs::AddClose_Test()
{
    int id = Combo_Tests->currentIndex();
    if(id == -1) return;
    rt_Test *ptest = current_ListTests.at(id);
    int count = Count_Samples->value();
    int doubles = Count_Double->value();
    int Kpos = Count_Kpos->value();
    int Kneg = Count_Kneg->value();
    int St = Count_St->value();
    int copies_St = Count_DoubleSt->value();
    count = count*doubles + Kpos + Kneg + St*copies_St;

    int tab = Tab_ADD->currentIndex();

    if(!add_action)
    {
        label_gif->setVisible(true);
        obj_gif->start();
        switch(tab)
        {
        default:    // Test
        case 0:
                    //qDebug() << "emit signal_addTest";
                    emit signal_addTest(ptest, count, doubles, Kpos, Kneg, St*copies_St, copies_St);
                    break;

        case 1:     // Sample
                    LoadListForSample(ListTests_ForSample);
                    emit signal_addSample(Edit_Sample->text(), &ListTests_ForSample, 1, 0);
                    break;

        case 2:     // Container
                    //LoadListForContainer(ListTests_ForSample);
                    //emit signal_addSample(Edit_Container->text(), &ListTests_ForSample);
                    break;

        }
    }

    close();
}
//-----------------------------------------------------------------------------
//--- Add_Test()
//-----------------------------------------------------------------------------
void Add_TESTs::Add_Test()
{
    rt_Test *ptest = NULL;
    int id = Combo_Tests->currentIndex();
    //if(id == -1) return;
    if(id >= 0) ptest = current_ListTests.at(id);
    int count = Count_Samples->value();
    int doubles = Count_Double->value();
    int Kpos = Count_Kpos->value();
    int Kneg = Count_Kneg->value();
    int St = Count_St->value();
    int copies_St = Count_DoubleSt->value();
    count = count*doubles + Kpos + Kneg + St*copies_St;

    int kind = 0;

    add_test->setDisabled(true);
    label_gif->setVisible(true);
    obj_gif->start();

    int tab = Tab_ADD->currentIndex();
    switch(tab)
    {
    default:    // Test
    case 0:
                if(ptest) emit signal_addTest(ptest, count, doubles, Kpos, Kneg, St*copies_St, copies_St);
                else {label_gif->setVisible(false); obj_gif->stop();}
                break;

    case 1:     // Sample
                if(Kplus_Kind->isChecked()) kind = 1;
                if(Kminus_Kind->isChecked()) kind = 2;
                LoadListForSample(ListTests_ForSample);
                emit signal_addSample(Edit_Sample->text(), &ListTests_ForSample, 1, kind);
                sample_Kind->setChecked(true);
                break;

    case 2:     // Container
                if(Kplus_container_Kind->isChecked()) kind = 1;
                if(Kminus_container_Kind->isChecked()) kind = 2;
                LoadListForContainer(ListTests_ForSample);
                emit signal_addSample(Edit_Container->text(), &ListTests_ForSample, Count_Container->value(), kind);
                sample_container_Kind->setChecked(true);
                break;
    }

    add_action = true;
    change_TabPage(tab);
}
//-----------------------------------------------------------------------------
//--- showEvent
//-----------------------------------------------------------------------------
void Add_TESTs::showEvent(QShowEvent *event)
{
    int i;
    int id;
    int method_ID;
    bool bace_tests = true;
    QStringList list_test;
    QString text;
    rt_Test *p_test;
    QIcon logotype;

    add_action = false;
    bool sts = TESTs->size();

    label_gif->setVisible(false);
    obj_gif->stop();

    // Load Type of tests ...
    Combo_Type->clear();
    Combo_Type->blockSignals(true);
    Combo_Type->setCurrentIndex(-1);
    foreach (QString name, *map_research)
    {
        method_ID = map_research->key(name);
        if(method_ID >= 0x20000 && bace_tests)
        {
            bace_tests = false;
            Combo_Type->addItem(logonull, "-----");
        }
        Combo_Type->addItem(logonull, name);
    }
    Combo_Type->addItem(logonull, "-----");
    Combo_Type->addItem(logonull, tr("All types of the test"));
    for(i=0; i<Combo_Type->count(); i++)
    {
        text = Combo_Type->itemText(i);
        if(text.contains("---")) Combo_Type->setItemData(i,0,Qt::UserRole - 1);
    }
    Combo_Type->setCurrentIndex(Combo_Type->count()-1);
    Combo_Type->blockSignals(false);

    // Load Tests ...

    list_test.clear();
    current_ListTests.clear();

    Combo_Tests->clear();
    Combo_Tests->blockSignals(true);


    ok_button->setEnabled(sts);
    add_test->setEnabled(false);
    Count_Samples->setEnabled(sts);
    Count_Double->setEnabled(sts);
    Count_Kpos->setEnabled(sts);
    Count_Kneg->setEnabled(sts);

    if(sts)
    {
        for(i=0; i<TESTs->size(); i++)
        {            
            p_test = TESTs->at(i);
            //if(p_test->header.Type_analysis == -1) continue;

            //qDebug() << "header.Hash: " << QString::fromStdString(p_test->header.Hash);

            current_ListTests.append(p_test);
            //list_test.append(QString::fromStdString(p_test->header.Name_Test));
            if(p_test->header.Hash == "ok") logotype = logo;
            else logotype = logonull;
            text = QString::fromStdString(p_test->header.Name_Test);
            if(map_TestTranslate->size()) text = map_TestTranslate->value(text, text);
            Combo_Tests->addItem(logotype, text);

        }        
        //Combo_Tests->addItems(list_test);
        id = 0;
    }
    else id = -1;

    change_test(id);
    Combo_Tests->setCurrentIndex(-1);
    Combo_Tests->blockSignals(false);
    Combo_Tests->setFocus();

    QStringList pp;
    pp << tr("List of Tests") << " " << " ";

    List_Tests->clear();
    List_Tests->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    List_Tests->setColumnCount(3);
    List_Tests->setHeaderLabels(pp);
    List_Tests->setColumnHidden(2,true);
    List_Tests->header()->setStretchLastSection(false);
    List_Tests->header()->resizeSection(1, 50);
    List_Tests->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    List_Tests->header()->setSectionResizeMode(1, QHeaderView::Fixed);

    LoadTests_Tree(List_Tests, false);
    select_test();

    // Samples Tab
    Tree_Tests->clear();
    Tree_Tests->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    Tree_Tests->setColumnCount(3);
    Tree_Tests->setHeaderLabels(pp);
    Tree_Tests->setColumnHidden(2,true);
    Tree_Tests->header()->setStretchLastSection(false);
    Tree_Tests->header()->resizeSection(1, 50);
    Tree_Tests->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    Tree_Tests->header()->setSectionResizeMode(1, QHeaderView::Fixed);

    LoadTests_Tree(Tree_Tests, true);    

    // Containers
    Load_Containers();
}

//-----------------------------------------------------------------------------
//--- Load_Containers()
//-----------------------------------------------------------------------------
void Add_TESTs::Load_Containers()
{
    QTreeWidgetItem *item;
    QTreeWidgetItem *item_ch;
    QTreeWidgetItem *item_root;
    rt_Test *p_test;
    rt_Test *p;
    rt_Preference *property;
    int type;
    QString text, name_test;
    QStringList list;

    QStringList pp;
    pp << tr("List of Containers") << " " << " ";

    Tree_Container->clear();
    Tree_Container->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    Tree_Container->setColumnCount(3);
    Tree_Container->setHeaderLabels(pp);
    Tree_Container->setColumnHidden(2,true);
    Tree_Container->header()->setStretchLastSection(false);
    Tree_Container->header()->resizeSection(1, 50);
    Tree_Container->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    Tree_Container->header()->setSectionResizeMode(1, QHeaderView::Fixed);


    item_root = new QTreeWidgetItem(Tree_Container);
    item_root->setText(0, tr("Containers"));
    item_root->setFlags(item_root->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsAutoTristate);
    item_root->setCheckState(0, Qt::Unchecked);
    item_root->setExpanded(true);

    QMap<QString, rt_Test*> map_Containers;
    foreach(p_test, *TESTs)
    {
        type = p_test->header.Type_analysis;
        if(type != 0x0033) continue;

        map_Containers.insert(QString::fromStdString(p_test->header.Name_Test), p_test);
    }

    foreach(p_test, map_Containers.values())
    {
        //type = p_test->header.Type_analysis;
        //if(type != 0x0033) continue;

        item = new QTreeWidgetItem(item_root);
        name_test = QString::fromStdString(p_test->header.Name_Test);
        if(map_TestTranslate->size()) name_test = map_TestTranslate->value(name_test,name_test);
        item->setText(0, name_test);
        //item->setText(1, QString::fromStdString(p_test->header.version));
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsAutoTristate);
        item->setCheckState(0, Qt::Unchecked);
        item->setExpanded(true);

        foreach(property, p_test->preference_Test)
        {
            if(property->name == "Checked_Tests")
            {
                text = QString::fromStdString(property->value);
                list = text.split("\r\n");

                foreach(text, list)
                {
                    foreach(p, *TESTs)
                    {
                        if(p->header.ID_Test == text.toStdString())
                        {
                            item_ch = new QTreeWidgetItem(item);
                            name_test = QString::fromStdString(p->header.Name_Test);
                            if(map_TestTranslate->size()) name_test = map_TestTranslate->value(name_test,name_test);
                            item_ch->setText(0, name_test);
                            item_ch->setText(1, QString::fromStdString(p->header.version));
                            item_ch->setText(2, QString::number((int)p));
                            item_ch->setCheckState(0, Qt::Unchecked);
                            item_ch->setFlags(item_ch->flags() | Qt::ItemIsUserCheckable);

                            if(p->header.Hash == "ok") item_ch->setIcon(0, logo);
                            else item_ch->setIcon(0, logonull);

                            break;
                        }
                    }
                }
                break;
            }
        }
    }
    map_Containers.clear();


    if(Edit_Container->text().trimmed().isEmpty()) Edit_Container->setText(tr("Sample_1"));
    if(Tab_ADD->currentIndex() == 2)
    {
        Edit_Container->selectAll();
        //Edit_Container->setFocus();
    }
}

//-----------------------------------------------------------------------------
//--- LoadTests_ToSamplesTab()
//-----------------------------------------------------------------------------
void Add_TESTs::LoadTests_Tree(QTreeWidget *tree, bool checked)
{
    int i,j, id;

    int type;
    QString text, str;
    QString name_test;
    bool find;
    bool sector_BasicTests = true;

    rt_Test *p_test;
    QTreeWidgetItem *item;    
    QTreeWidgetItem *item_cur;
    QTreeWidgetItem *item_parent;

    QMultiMap<int, rt_Test*> TESTs_Temporary;
    QStringList list;
    QList<QTreeWidgetItem*> list_item;

    QFont f = qApp->font();    
    f.setItalic(true);
    QBrush b(Qt::red);

    QMap<QString, rt_Test*> temp_Map;
    QMultiMap<QString, rt_Test*> TESTs_dtr;
    QString catalog;
    QVector<rt_Test*> List_all;


    for(i=0; i<TESTs->size(); i++)
    {
        p_test = TESTs->at(i);
        type = p_test->header.Type_analysis;
        if(type == 0x0033) continue;            // Container

        //TESTs_Temporary.insert(type, p_test);
        if(type < 0x2000) TESTs_Temporary.insert(type, p_test);
        else
        {
            catalog = QString::fromStdString(p_test->header.Catalog);
            TESTs_dtr.insert(catalog, p_test);
        }
    }


    foreach(p_test, TESTs_Temporary.values())
    {
        List_all.append(p_test);
    }
    foreach(text, TESTs_dtr.keys())
    {
        if(list.contains(text)) continue;
        else list.append(text);

        temp_Map.clear();
        QMultiMap<QString, rt_Test*>::iterator it = TESTs_dtr.find(text);
        while(it != TESTs_dtr.end() && it.key() == text)
        {
            p_test = (rt_Test*)it.value();
            temp_Map.insert(QString::fromStdString(p_test->header.Name_Test), p_test);
            ++it;
        }

        foreach(p_test, temp_Map.values())
        {
            List_all.append(p_test);
        }
    }


    for(i=0; i<List_all.size(); i++)
    {
        //p_test = TESTs_Temporary.values().at(i);
        p_test = List_all.at(i);

        type = p_test->header.Type_analysis;
        text = map_research->value(type,"...");
        if(text == "...") text = tr("Unknown");

        // Temporary
        str = QString::fromStdString(p_test->header.Catalog);
        if(!str.isEmpty()) text = str;
        if(type == 0) text = tr("Simple method");


        if(type >= 0x2000 && sector_BasicTests && i)
        {
            sector_BasicTests = false;
            item = new QTreeWidgetItem(tree);
            item->setText(0, "-----");
            item->setDisabled(true);
        }

        list.clear();
        list = text.split("\\");
        //qDebug() << QString::fromStdString(p_test->header.Name_Test) << list;

        list_item.clear();
        for(j=0; j<tree->topLevelItemCount(); j++)
        {
            list_item.append(tree->topLevelItem(j));
        }

        id = 0;
        foreach(text, list)
        {            
            find = false;
            foreach(item, list_item)
            {
                if(item->text(0) == text)
                {
                    find = true;
                    break;
                }
            }

            if(find)
            {
                item_cur = item;
            }
            else
            {
                if(!id) item_cur = new QTreeWidgetItem(tree);
                else    item_cur = new QTreeWidgetItem(item_parent);
                if(text == tr("Unknown"))
                {
                    item_cur->setFont(0,f);
                    item_cur->setForeground(0,b);
                }
                item_cur->setText(0, text);
                if(checked)
                {
                    item_cur->setCheckState(0, Qt::Unchecked);
                    item_cur->setFlags(item_cur->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsAutoTristate);
                }
            }

            item_parent = item_cur;
            list_item.clear();
            for(j=0; j<item_cur->childCount(); j++)
            {
                list_item.append(item_cur->child(j));
            }

            id++;
        }
        item = new QTreeWidgetItem(item_cur);

        name_test = QString::fromStdString(p_test->header.Name_Test);
        if(map_TestTranslate->size()) name_test = map_TestTranslate->value(name_test, name_test);

        item->setText(0, name_test);
        item->setText(1, QString::fromStdString(p_test->header.version));
        item->setText(2, QString::number((int)p_test));
        if(checked)
        {
            item->setCheckState(0, Qt::Unchecked);
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        }

        // Icons
        if(p_test->header.Hash == "ok") item->setIcon(0, logo);
        else item->setIcon(0, logonull);
        //...
    }
    TESTs_Temporary.clear();
    List_all.clear();

    if(tree == Tree_Tests)
    {
        if(Edit_Sample->text().trimmed().isEmpty()) Edit_Sample->setText(tr("Sample_1"));        
        if(Tab_ADD->currentIndex() == 1) {Edit_Sample->selectAll(); Edit_Sample->setFocus();}
    }
}

//-----------------------------------------------------------------------------
//--- LoadTests_ToSamplesTab()
//-----------------------------------------------------------------------------
/*void Add_TESTs::LoadTests_ToSamplesTab()
{
    int i;
    int type;
    QString text;
    rt_Test *p_test;
    QIcon logotype;
    bool sector_BasicTests = true;
    QStringList type_id;

    QTreeWidgetItem *item;
    QTreeWidgetItem *item_child;

    QMultiMap<int, rt_Test*> TESTs_Temporary;

    QFont f = qApp->font();
    f.setItalic(true);
    QBrush b (Qt::red);

    QStringList pp;
    pp << tr("List of Tests");
    Tree_Tests->clear();
    Tree_Tests->setColumnCount(2);
    Tree_Tests->setColumnWidth(0, 400);
    Tree_Tests->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    Tree_Tests->setHeaderLabels(pp);

    for(i=0; i<TESTs->size(); i++)
    {
        p_test = TESTs->at(i);
        type = p_test->header.Type_analysis;
        TESTs_Temporary.insert(type, p_test);        
    }

    for(i=0; i<TESTs->size(); i++)
    {
        p_test = TESTs_Temporary.values().at(i);
        type = p_test->header.Type_analysis;
        text = map_research->value(type,"...");
        if(text == "...") text = tr("Unknown");


        //qDebug() << "Tests: " << i << QString::fromStdString(p_test->header.Name_Test) << type << text;

        if(type >= 0x2000 && sector_BasicTests && i)
        {
            sector_BasicTests = false;
            item = new QTreeWidgetItem(Tree_Tests);
            item->setText(0, "-----");
            item->setDisabled(true);
        }

        if(!type_id.contains(text))
        {
            type_id.append(text);
            item = new QTreeWidgetItem(Tree_Tests);
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsAutoTristate);
            item->setCheckState(0, Qt::Unchecked);
            if(text == tr("Unknown"))
            {
                item->setFont(0,f);
                item->setForeground(0,b);
            }
            item->setText(0, text);
        }

        item = Tree_Tests->findItems(text, Qt::MatchContains, 0).at(0);
        item_child = new QTreeWidgetItem(item);
        item_child->setFlags(item_child->flags() | Qt::ItemIsUserCheckable);
        item_child->setCheckState(0, Qt::Unchecked);
        text = QString::fromStdString(p_test->header.Name_Test);
        item_child->setText(0,text);        
        item_child->setText(1, QString::number((int)p_test));

        // Icons
        if(p_test->header.Hash == "ok") item_child->setIcon(0, logo);
        else item_child->setIcon(0, logonull);
        //...
    }

    if(Edit_Sample->text().trimmed().isEmpty()) Edit_Sample->setText(tr("Sample_1"));
    Edit_Sample->selectAll();
    if(Tab_ADD->currentIndex() == 1) Edit_Sample->setFocus();

    TESTs_Temporary.clear();
}*/
//-----------------------------------------------------------------------------
//--- LoadListForContainer
//-----------------------------------------------------------------------------
void Add_TESTs::LoadListForContainer(QVector<rt_Test *> &list)
{
    bool ok;
    rt_Test *p_test;
    QTreeWidgetItemIterator it(Tree_Container);
    QString text;

    list.clear();

    while(*it)
    {
        if((*it)->isDisabled()) {++it; continue;}

        text = (*it)->text(2);
        if((*it)->checkState(0) == Qt::Checked && text.length())
        {
            p_test = (rt_Test*)(text.toInt(&ok));
            if(ok) list.append(p_test);
        }
        ++it;
    }
}

//-----------------------------------------------------------------------------
//--- LoadListForSample
//-----------------------------------------------------------------------------
void Add_TESTs::LoadListForSample(QVector<rt_Test *> &list)
{
    int i,j;
    bool ok;
    rt_Test *p_test;
    QTreeWidgetItem *item;
    QTreeWidgetItem *item_child;
    QTreeWidgetItemIterator it(Tree_Tests);
    QString text;

    list.clear();

    while(*it)
    {
        if((*it)->isDisabled()) {++it; continue;}

        text = (*it)->text(2);
        if((*it)->checkState(0) == Qt::Checked && text.length())
        {
            p_test = (rt_Test*)(text.toInt(&ok));
            if(ok) list.append(p_test);
        }
        ++it;
    }

    /*for(i=0; i<Tree_Tests->topLevelItemCount(); i++)
    {
        item = Tree_Tests->topLevelItem(i);
        if(item->isDisabled()) continue;

        for(j=0; j<item->childCount(); j++)
        {
            item_child = item->child(j);
            if(item_child->checkState(0) == Qt::Checked)
            {
                p_test = (rt_Test*)(item_child->text(2).toInt(&ok));
                if(ok) list.append(p_test);
            }
        }
    }*/
}

//-----------------------------------------------------------------------------
//--- change_typeTest
//-----------------------------------------------------------------------------
void Add_TESTs::change_typeTest(int id)
{
    int i, index;
    int id_method;
    rt_Test *ptest;
    QIcon logotype;

    current_ListTests.clear();
    //add_test->setDisabled(true);
    ok_button->setDisabled(true);

    Combo_Tests->blockSignals(true);
    Combo_Tests->clear();

    QString text = Combo_Type->itemText(id);
    id_method = map_research->key(text, -1);

    for(i=0; i<TESTs->size(); i++)
    {
        ptest = TESTs->at(i);
        //if(ptest->header.Type_analysis == -1) continue;

        if(id_method == ptest->header.Type_analysis || id_method == -1)
        {
            current_ListTests.append(ptest);
            if(ptest->header.Hash == "ok") logotype = logo;
            else logotype = logonull;            
            Combo_Tests->addItem(logotype, QString::fromStdString(ptest->header.Name_Test));            
        }
    }
    if(current_ListTests.size())
    {
        //add_test->setDisabled(false);
        ok_button->setDisabled(false);
        if(add_action) ok_button->setDisabled(true);
        index = Combo_Tests->currentIndex();
        change_test(index);
    }
    Combo_Tests->blockSignals(false);
    Combo_Tests->setFocus();
}
//-----------------------------------------------------------------------------
//--- select_test()
//-----------------------------------------------------------------------------
void Add_TESTs::select_test()
{
    int i;
    QString name;
    QList<QTreeWidgetItem *> list;
    QTreeWidgetItem *item;
    rt_Test *p_test;

    //add_test->setDisabled(true);
    ok_button->setDisabled(true);

    list = List_Tests->selectedItems();
    if(list.isEmpty()) return;

    item = list.at(0);
    //if(item->childCount()) return;
    //if(item->text(2).isEmpty()) return;



    int id = -1;
    bool state = false;



    for(i=0; i<current_ListTests.size(); i++)
    {
        p_test = current_ListTests.at(i);
        name = QString::fromStdString(p_test->header.Name_Test);
        if(map_TestTranslate->size()) name = map_TestTranslate->value(name,name);
        if(name == item->text(0))
        {
            id = i;
            break;
        }
    }

    Combo_Tests->setCurrentIndex(id);

    if(id < 0) state = true;

    //add_test->setDisabled(state);
    ok_button->setDisabled(state);

    if(add_action) ok_button->setDisabled(true);

    //qDebug() << "item: " << id;
    //change_test(id);

}

//-----------------------------------------------------------------------------
//--- change_test
//-----------------------------------------------------------------------------
void Add_TESTs::change_test(int id)
{
    int i;
    int count;
    rt_Test *p_test;
    rt_Preference *property;
    QString text;
    QStringList list;
    int count_st = 0;
    int count_copies_st = 0;
    bool ok;

    //qDebug() << "change test: " << id;

    Count_Samples->setValue(1);
    Count_Double->setValue(1);
    Count_Kpos->setValue(0);
    Count_Kneg->setValue(0);
    Count_St->setValue(0);
    Count_DoubleSt->setValue(1);

    //Line_Type->setText("");
    Label_St->setEnabled(false);
    Count_St->setEnabled(false);
    Label_countSt->setEnabled(false);
    Count_DoubleSt->setEnabled(false);
    Label_DoubleSt->setEnabled(false);

    if(id < 0)
    {
        Label_countTubes->setText("(x1)");
        Label_countKpos->setText("(x1)");
        Label_countKneg->setText("(x1)");
        Label_countSt->setText("(x1)");
    }
    else
    {
        //p_test = TESTs->at(id);
        //qDebug() << "list: " << current_ListTests.size();
        p_test = current_ListTests.at(id);
        count = p_test->tubes.size();
        //qDebug() << "info: " << QString::fromStdString(p_test->header.Name_Test) << p_test->tubes.size();
        text = QString("(x%1)").arg(count);
        Label_countTubes->setText(text);
        Label_countKpos->setText(text);
        Label_countKneg->setText(text);
        Label_countSt->setText(text);
        text = QString::fromStdString(p_test->header.comment);
        if(text.contains("\r\nDTReport ="))
        {
            QStringList list = text.split("=");
            if(list.size() > 1)
            {
                text = list.at(1);
                //Line_Type->setText(text.trimmed());
            }
        }
        // check count Control+- for each test
        foreach(property, p_test->preference_Test)
        {
            if(property->name == "ControlTubesCount")
            {
                text = QString::fromStdString(property->value);
                count = text.toInt(&ok);
                if(ok)
                {
                    text = QString("(x%1)").arg(count);
                    Label_countKpos->setText(text);
                    Label_countKneg->setText(text);
                }
                continue;
            }
            if(property->name == "PosControlCount")
            {
                text = QString::fromStdString(property->value);
                count = text.toInt(&ok);
                if(ok)
                {
                    Count_Kpos->setValue(count);
                }
                continue;
            }
            if(property->name == "NegControlCount")
            {
                text = QString::fromStdString(property->value);
                count = text.toInt(&ok);
                if(ok)
                {
                    Count_Kneg->setValue(count);
                }
                continue;
            }
        }

        // comments about test
        //Text_Comments->setText(QString::fromStdString(p_test->header.comment));
        //if(p_test->header.Type_analysis > 0x100) Text_Comments->setText("");

        // Info about Standarts
        for(i=0; i<p_test->preference_Test.size(); i++)
        {
            property = p_test->preference_Test.at(i);
            if(property->name == "Standarts Information")
            {
                text = QString::fromStdString(property->value);
                list = text.split("\t");
                if(list.size() >= 5)
                {
                    count_st = QString(list.at(0)).toInt(&ok);
                    count_copies_st = QString(list.at(1)).toInt(&ok);
                    if(count_st >= 2 && count_copies_st >= 1)
                    {
                        Count_St->setValue(count_st);
                        Count_DoubleSt->setValue(count_copies_st);
                        Label_St->setEnabled(true);
                        Count_St->setEnabled(true);
                        Label_countSt->setEnabled(true);
                        Count_DoubleSt->setEnabled(true);
                        Label_DoubleSt->setEnabled(true);
                    }
                }
                break;
            }
        }
    }
}
//-----------------------------------------------------------------------------
//--- change_TabPage(int page)
//-----------------------------------------------------------------------------
void Add_TESTs::change_TabPage(int page)
{
    bool sts = false;

    switch(page)
    {
    default:    break;

    case 0:     if(Combo_Tests->currentIndex() >= 0) sts = true;
                break;

    case 1:     if(!Edit_Sample->text().trimmed().isEmpty())  sts = true;
                //Edit_Sample->setFocus();
                break;

    case 2:     if(!Edit_Container->text().trimmed().isEmpty())  sts = true;
                //Edit_Container->setFocus();
                break;
    }

    //add_test->setEnabled(sts);
    ok_button->setEnabled(sts);
    if(add_action) ok_button->setDisabled(true);

    if(page == 1 && sts)
    {
        //Edit_Sample->setFocus();
        Edit_Sample->selectAll();
    }
    if(page == 2 && sts)
    {
        //Edit_Container->setFocus();
        Edit_Container->selectAll();
    }
}
//-----------------------------------------------------------------------------
//--- change_NameSample()
//-----------------------------------------------------------------------------
void Add_TESTs::change_NameSample()
{
    change_TabPage(1);
}
//-----------------------------------------------------------------------------
//--- change_NameSample_forContainer()
//-----------------------------------------------------------------------------
void Add_TESTs::change_NameSample_forContainer()
{
    change_TabPage(2);
}

//-----------------------------------------------------------------------------
//--- Increment_Sample()
//-----------------------------------------------------------------------------
void Add_TESTs::Increment_Sample(QString name)
{
    if(name.isEmpty()) return;

    int i;
    QString text;
    QChar symbol;
    int last = -1;
    int first = -1;
    int number;
    bool ok;
    QTreeWidgetItem *item;
    bool checked = false;

    switch(Tab_ADD->currentIndex())
    {
        case 0: break;

        case 1: for(i=0; i<Tree_Tests->topLevelItemCount(); i++)
                {
                    item = Tree_Tests->topLevelItem(i);
                    if(item->checkState(0) != Qt::Unchecked) {checked = true; break;}
                }
                break;

        case 2: item = Tree_Container->topLevelItem(0);
                if(item->checkState(0) != Qt::Unchecked) checked = true;
                break;
    }



    if(!checked) return;

    for(i=name.size()-1; i>=0; i--)
    {
        symbol = name.at(i);
        if(symbol.isDigit())
        {
            if(last < 0) last = i;
            first = i;
        }
        else
        {
            if(first >= 0) break;
        }
    }

    if(first >= 0)
    {
        number = name.mid(first, last-first+1).toInt(&ok);
        if(!ok) name = QString("%1_1").arg(name);
        else
        {
            number++;
            text = "";
            if(last < name.size()-1) text = name.mid(last+1);
            name = QString("%1%2%3").arg(name.mid(0,first)).arg(number).arg(text);
        }
    }
    else name = QString("%1_1").arg(name);

    switch(Tab_ADD->currentIndex())
    {
    case 1: Edit_Sample->setText(name);
            Edit_Sample->selectAll();
            //Edit_Sample->setFocus();
            break;

    case 2: Edit_Container->setText(name);
            Edit_Container->selectAll();
            //Edit_Container->setFocus();
            break;

    default:    break;
    }
}

//-----------------------------------------------------------------------------
//--- CheckState_AddButton()
//-----------------------------------------------------------------------------
void Add_TESTs::CheckState_AddButton()
{
    int i;
    QList<QTreeWidgetItem *> list;
    QTreeWidgetItem *item;
    bool state = false;


    int id = Tab_ADD->currentIndex();

    switch(id)
    {
    case 0:
            list = List_Tests->selectedItems();
            if(list.isEmpty()) break;
            item = list.at(0);
            if(item->childCount() == 0) state = true;
            break;

    case 1:
            for(i=0; i<Tree_Tests->topLevelItemCount(); i++)
            {
                item = Tree_Tests->topLevelItem(i);
                if(item->checkState(0) != Qt::Unchecked) {state = true; break;}
            }
            break;

    case 2:
            for(i=0; i<Tree_Container->topLevelItemCount(); i++)
            {
                item = Tree_Container->topLevelItem(i);
                if(item->checkState(0) != Qt::Unchecked) {state = true; break;}
            }
            break;

    default:    break;

    }

    add_test->setEnabled(state);
}


