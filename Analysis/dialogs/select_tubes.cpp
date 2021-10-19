
#include "select_tubes.h"


//-----------------------------------------------------------------------------
//--- Constructor
//-----------------------------------------------------------------------------
Select_tubes::Select_tubes(const QString& title, QWidget* parent, Qt::WindowFlags flags)
    : QDockWidget(title, parent, flags)
{
    readCommonSettings();
    setFont(qApp->font());

    Color_Buttons << QColor(255,0,0) << QColor(255,128,0) << QColor(255,255,0) << QColor(0,255,0) << QColor(0,128,255) << QColor(0,0,255) << QColor(255,0,255);

    main_widget = parent;
    mTitleBar = new DockWidgetTitleBar(this);
    setTitleBarWidget(mTitleBar);

    create_TabBox();
    setWidget(Tab_Select);
    setAllowedAreas(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
    //setAllowedAreas(Qt::NoDockWidgetArea);

    create_ColorButtons();

    p_prot = NULL;
    dock_area = Qt::TopDockWidgetArea;

    connect(Select_Grid,SIGNAL(cellClicked(int,int)),this,SLOT(cellSelected(int,int)));
    //connect(Select_Grid->horizontalHeader(),SIGNAL(sectionPressed(int)),this,SLOT(columnSelected(int)));
    //connect(Select_Grid->verticalHeader(),SIGNAL(sectionPressed(int)),this,SLOT(rowSelected(int)));
    connect(header_hor, SIGNAL(sSectionsSelect(QVector<int>)), this, SLOT(ColumnsSelect(QVector<int>)));
    connect(header_ver, SIGNAL(sSectionsSelect(QVector<int>)), this, SLOT(RowsSelect(QVector<int>)));


    connect(mTitleBar->aSelect, SIGNAL(changed()), this, SLOT(slot_SelectChange()));
    connect(mTitleBar->aNumerate, SIGNAL(changed()), this, SLOT(slot_NumerateChange()));
    connect(mTitleBar, SIGNAL(close_Signal()), this, SLOT(slot_HideTabWidget()));

    connect(Select_Grid->clear_plate, SIGNAL(triggered(bool)), this, SLOT(ClearPlate()));
    connect(Select_Grid->back_InitialState, SIGNAL(triggered(bool)), this, SLOT(InitialState()));
    //connect(Select_Grid, SIGNAL(color_Rejime(bool)), mTitleBar, SLOT(change_ColorRejime(bool)));
    connect(Select_Grid, SIGNAL(color_Rejime(bool)), this, SLOT(restore_Cursor()));
    connect(Select_Grid, SIGNAL(color_Rejime(bool)), this, SLOT(slot_ColorRejime(bool)));

    connect(this,SIGNAL(dockLocationChanged(Qt::DockWidgetArea)), this,SLOT(change_dock(Qt::DockWidgetArea)));


    //connect(Test_tree,SIGNAL(itemChanged(QTreeWidgetItem*,int)),
    //        Test_tree,SLOT(click_item(QTreeWidgetItem*,int)));


    Select_Grid->selected = false;
    Select_Grid->moving = false;

    //
    cursor_FREE = QCursor(QPixmap(":/images/cursor_null.png"));

    //setCursor(cursor_FREE);
    //unsetCursor();
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Select_tubes::~Select_tubes()
{
    //delete first_ColorButton;
}
//-----------------------------------------------------------------------------
//--- readCommonSettings()
//-----------------------------------------------------------------------------
void Select_tubes::readCommonSettings()
{
    QString text;
    QString dir_path = qApp->applicationDirPath();
    QSettings *CommonSettings = new QSettings(dir_path + "/tools/ini/preference.ini", QSettings::IniFormat);

    CommonSettings->beginGroup("Common");

        //... Language ...
        text = CommonSettings->value("language","ru").toString();
        if(translator.load(":/translations/select_" + text + ".qm"))
        {
            qApp->installTranslator(&translator);
        }
        //... Style ...
        text = CommonSettings->value("style","xp").toString();
        StyleApp = text;

    CommonSettings->endGroup();
    delete CommonSettings;

    logo = QIcon("logotype.png");
    logonull = QIcon("logotype_null.png");
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Select_tubes::create_TabBox()
{
    QFont f = qApp->font();
    f.setBold(false);
    f.setPointSize(f.pointSize()-2);

// 1.--- Select Grid ---
    Select_Grid = new TableWidget(0,0,this);
    Select_Grid->setSelectionMode(QAbstractItemView::NoSelection);
    Select_Grid->setEditTriggers(QAbstractItemView::NoEditTriggers);
    Delegate = new ItemDelegate();
    Delegate->style = StyleApp;
    Select_Grid->setItemDelegate(Delegate);
    Select_Grid->address = this;
    Select_Grid->setFont(f);
    Select_Grid->setContextMenuPolicy(Qt::CustomContextMenu);
    Delegate->A1 = &Select_Grid->A1;
    Delegate->numeration = false;
    Delegate->enable_initial = &EnableTube_Initial;
    Select_Grid->mTitleBar = mTitleBar;
    header_hor = new MyHeader(Qt::Horizontal, this);
    header_hor->setFont(f);
    Select_Grid->setHorizontalHeader(header_hor);
    header_ver = new MyHeader(Qt::Vertical, this);
    header_ver->setFont(f);
    Select_Grid->setVerticalHeader(header_ver);
    Select_Grid->ColorTube_Current = &ColorTube_Current;

    //Horizontal_Header = new MyHorizontalHeaderView(Qt::Horizontal, Select_Grid);
    //Select_Grid->setHorizontalHeader(Horizontal_Header);

    //Horizontal_Header->moving = false;
    //Horizontal_Header->selected = false;

    connect(Select_Grid, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu_SelectGrid()));


    //Select_Grid->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    //Select_Grid->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    //header = new MyHeader(Qt::Horizontal, Select_Grid);
    //Select_Grid->setHorizontalHeader(header);


    connect(Select_Grid->corner_btn, SIGNAL(clicked()), this, SLOT(all_Selected()));


    Color_Box = new QGroupBox();
    Color_Box->setObjectName("Transparent");
    Color_Box->setFixedSize(24,180);
    Color_Box->setVisible(false);
    QVBoxLayout *fluor_layout = new QVBoxLayout();
    Color_Box->setLayout(fluor_layout);
    fluor_layout->setMargin(0);
    fluor_layout->setSpacing(1);


    first_ColorButton = new Color_ToolButton(QColor(255,0,0), this);
    second_ColorButton = new Color_ToolButton(QColor(255,128,0), this);
    third_ColorButton = new Color_ToolButton(QColor(255,255,0), this);
    fourth_ColorButton = new Color_ToolButton(QColor(0,255,0), this);
    fifth_ColorButton = new Color_ToolButton(QColor(0,128,255), this);
    sixth_ColorButton = new Color_ToolButton(QColor(0,0,255), this);
    seventh_ColorButton = new Color_ToolButton(QColor(255,0,255), this);

    color_History = new QToolButton(this);
    color_History->setIcon(QIcon(":/images/back_16.png"));
    color_History->setDisabled(true);

    fluor_layout->addWidget(first_ColorButton);
    fluor_layout->addWidget(second_ColorButton);
    fluor_layout->addWidget(third_ColorButton);
    fluor_layout->addWidget(fourth_ColorButton);
    fluor_layout->addWidget(fifth_ColorButton);
    fluor_layout->addWidget(sixth_ColorButton);
    fluor_layout->addWidget(seventh_ColorButton);
    fluor_layout->addWidget(color_History);

    Select_Box = new QGroupBox();    
    Select_Box->setObjectName("Transparent");
    layout_select = new QHBoxLayout();
    layout_select->setMargin(0);
    layout_select->setSpacing(1);
    Select_Box->setLayout(layout_select);

    layout_select->addWidget(Select_Grid,1);
    layout_select->addWidget(Color_Box, 1, Qt::AlignTop);

    Fluor_Buttons = new QButtonGroup(this);    
    Fluor_Buttons->setExclusive(true);
    Fluor_Buttons->addButton(first_ColorButton,0);
    Fluor_Buttons->addButton(second_ColorButton,1);
    Fluor_Buttons->addButton(third_ColorButton,2);
    Fluor_Buttons->addButton(fourth_ColorButton,3);
    Fluor_Buttons->addButton(fifth_ColorButton,4);
    Fluor_Buttons->addButton(sixth_ColorButton,5);
    Fluor_Buttons->addButton(seventh_ColorButton,6);

    first_ColorButton->setCheckable(true);
    second_ColorButton->setCheckable(true);
    third_ColorButton->setCheckable(true);
    fourth_ColorButton->setCheckable(true);
    fifth_ColorButton->setCheckable(true);
    sixth_ColorButton->setCheckable(true);
    seventh_ColorButton->setCheckable(true);

    connect(Fluor_Buttons, SIGNAL(buttonClicked(int)), this, SLOT(slot_FluorButton(int)));
    connect(color_History, SIGNAL(clicked(bool)), this, SLOT(ColorHistory()));


// 2.--- Test&Sample Select ---
    SampleTest_Box = new QGroupBox();
    SampleTest_Box->setObjectName("Transparent");
    layout_box = new QVBoxLayout();
    //layout_OnOff = new QHBoxLayout();
    SampleTest_Box->setLayout(layout_box);
    box_spl = new QSplitter(Qt::Horizontal);
    box_spl->setHandleWidth(6);
    box_spl->setChildrenCollapsible(true);

    Sample_tree = new TreeWidget();
    Sample_tree->setFont(qApp->font());
    Sample_tree->setHeaderLabel(tr("Samples"));
    Test_tree = new TreeWidget();
    Test_tree->setFont(qApp->font());
    Test_tree->setHeaderLabel(tr("Tests"));

    box_spl->addWidget(Test_tree);
    box_spl->addWidget(Sample_tree);

/*
    checkedALL_Test = new QPushButton(QIcon(":/images/Checked_ALL.png"),"",NULL);
    checkedNONE_Test = new QPushButton(QIcon(":/images/Checked_NONE.png"),"",NULL);
    checkedALL_Sample = new QPushButton(QIcon(":/images/Checked_ALL.png"),"",NULL);
    checkedNONE_Sample = new QPushButton(QIcon(":/images/Checked_NONE.png"),"",NULL);

    layout_OnOff->addWidget(checkedALL_Test, 0, Qt::AlignLeft);
    layout_OnOff->addWidget(checkedNONE_Test, 0, Qt::AlignLeft);    
    layout_OnOff->addStretch(1);
    layout_OnOff->addWidget(checkedALL_Sample, 0, Qt::AlignRight);
    layout_OnOff->addWidget(checkedNONE_Sample, 0, Qt::AlignRight);
*/
    connect(Sample_tree, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(apply_change()));
    connect(Test_tree, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(apply_change()));
    //connect(checkedALL_Sample, SIGNAL(clicked()), this, SLOT(selectALL_Sample()));
    //connect(checkedNONE_Sample, SIGNAL(clicked()), this, SLOT(selectNONE_Sample()));
    //connect(checkedALL_Test, SIGNAL(clicked()), this, SLOT(selectALL_Test()));
    //connect(checkedNONE_Test, SIGNAL(clicked()), this, SLOT(selectNONE_Test()));

    layout_box->addWidget(box_spl);
    //layout_box->addLayout(layout_OnOff);
    //layout_box->setSpacing(2);
    layout_box->setMargin(2);


// 3.--- Main Tab ---
    Tab_Select = new QTabWidget();

    Tab_Select->addTab(SampleTest_Box, tr("Test&&Sample"));
    Tab_Select->addTab(Select_Box, tr("Select tubes"));


//...
    Select_Grid->color_buttons = Fluor_Buttons;
    Select_Grid->color_history = color_History;

}
//-----------------------------------------------------------------------------
//--- ColorHistory()
//-----------------------------------------------------------------------------
void Select_tubes::ColorHistory()
{
    int pos, color;

    color_History->setDisabled(true);
    enable_event.enable_tubes.clear();

    foreach(pos, Select_Grid->map_ColorHistory.keys())
    {
        color = Select_Grid->map_ColorHistory.value(pos);

        p_prot->color_tube.at(pos) = color;
        ColorTube_Current.replace(pos, color);
        enable_event.enable_tubes.push_back((short)pos);
    }

    QApplication::sendEvent(main_widget, &enable_event);
    Select_Grid->viewport()->update();
}

//-----------------------------------------------------------------------------
//--- slot_FluorButton(int)
//-----------------------------------------------------------------------------
void Select_tubes::slot_FluorButton(int id)
{    
    Color_ToolButton *button = (Color_ToolButton*)Fluor_Buttons->button(id);
    QColor color = button->color;
    unsigned int val = (color.rgba() | 0xff000000);
    QString str_color = QString::number(val,16);
    QString Style = QString("QToolButton::checked {background-color: #%1; border: 1px solid black}").arg(str_color);

    //qDebug() << "str_color: " << str_color;


    /*QPalette pal = button->palette();
    pal.setColor(QPalette::Background, Color_Buttons.at(id));
    button->setPalette(pal);*/

    button->setStyleSheet(Style);

    for(int i=0; i<Fluor_Buttons->buttons().size(); i++)
    {
        if(i == id) continue;
        button = (Color_ToolButton*)Fluor_Buttons->button(i);
        color = button->color;
        color.setAlpha(60);
        val = color.rgba();
        str_color = QString::number(val,16);
        //qDebug() << "str_color:__ " << str_color;
        Style = QString("QToolButton {background-color: #%1; border: 1px solid gray}").arg(str_color);
        button->setStyleSheet(Style);
    }
}
//-----------------------------------------------------------------------------
//--- slot_ColorRejime(bool)
//-----------------------------------------------------------------------------
void Select_tubes::slot_ColorRejime(bool state)
{
    Color_Box->setVisible(state);
    if(state && Fluor_Buttons->checkedId() < 0)
    {
        Fluor_Buttons->button(0)->setChecked(true);
        emit Fluor_Buttons->buttonClicked(0);
    }
    if(state != mTitleBar->aSelect->isChecked())
    {
        mTitleBar->aSelect->blockSignals(true);
        mTitleBar->aSelect->setChecked(state);
        mTitleBar->aSelect->blockSignals(false);
    }
}

//-----------------------------------------------------------------------------
//--- slot_SelectChange()
//-----------------------------------------------------------------------------
void Select_tubes::slot_SelectChange()
{
    bool color_state = mTitleBar->aSelect->isChecked();

    if(color_state) Tab_Select->setCurrentIndex(1);

    Select_Grid->select_rejime->blockSignals(true);
    Select_Grid->color_select->setChecked(color_state);
    Select_Grid->position_select->setChecked(!color_state);
    Select_Grid->select_rejime->blockSignals(false);

    emit Select_Grid->color_Rejime(color_state);
}
//-----------------------------------------------------------------------------
//--- slot_NumerateChange()
//-----------------------------------------------------------------------------
void Select_tubes::slot_NumerateChange()
{
    bool num_state = mTitleBar->aNumerate->isChecked();

    if(num_state) Tab_Select->setCurrentIndex(1);

    Select_Grid->numeration_rejime->setChecked(num_state);
    Delegate->numeration = Select_Grid->numeration_rejime->isChecked();
    Select_Grid->viewport()->update();
}
//-----------------------------------------------------------------------------
//--- slot_HideTabWidget()
//-----------------------------------------------------------------------------
void Select_tubes::slot_HideTabWidget()
{
    bool state = Tab_Select->isVisible();

    //Tab_Select->setVisible(!state);
}

//-----------------------------------------------------------------------------
//--- create_ColorButtons()
//-----------------------------------------------------------------------------
void Select_tubes::create_ColorButtons()
{
    //first_ColorButton = new Color_Button(Qt::red, this);
}

//-----------------------------------------------------------------------------
//--- contextMenu_SelectGrid
//-----------------------------------------------------------------------------
void Select_tubes::contextMenu_SelectGrid()
{
    QMenu menu;

    menu.setStyleSheet("QMenu::item:selected {background-color: #d7d7ff; color: black;}");
    menu.setFont(qApp->font());

    menu.addAction(Select_Grid->clear_plate);
    menu.addAction(Select_Grid->back_InitialState);
    menu.addSeparator()->setText(tr("Selection rejime"));
    menu.addAction(Select_Grid->position_select);
    menu.addAction(Select_Grid->color_select);
    menu.addSeparator();
    menu.addAction(Select_Grid->numeration_rejime);

    menu.exec(QCursor::pos());
    Delegate->numeration = Select_Grid->numeration_rejime->isChecked();

    menu.clear();

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Select_tubes::clear_SelectGrid()
{
    Select_Grid->clear();
    Select_Grid->setColumnCount(0);
    Select_Grid->setRowCount(0);
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Select_tubes::load_SelectGrid(rt_Protocol *p)
{
    int i,j,k;
    int pos;    
    QStringList header;
    QTableWidgetItem* item;
    int col,row;

    p_prot = NULL;
    Select_Grid->clear();
    Test_tree->clear();
    Sample_tree->clear();

    if(p == NULL) return;
    p_prot = p;


    // 0. --- Position in Protocol
    PositionInProtocol(p_prot);

    // 1. --- Load Select Structure ---

    p->Plate.PlateSize(p->count_Tubes, row, col);
    Delegate->get_prot(p);

    Select_Grid->setColumnCount(col);                           // count of column
    Select_Grid->setRowCount(row);                              // ...      row

    for(i=0; i<row; i++)
    {
        for(j=0; j<col; j++)
        {            
            pos = i*col + j;
            item = new QTableWidgetItem();
            Select_Grid->setItem(i,j,item);
            if(p->enable_tube.at(pos)) item->setText("1");
            else item->setText("0");
        }
    }

    for(i=0; i<col; i++) header.append(QString::number(i+1));   // Column Header    
    Select_Grid->setHorizontalHeaderLabels(header);
    for(i=0; i<col; i++) Select_Grid->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Stretch);
    Select_Grid->horizontalHeader()->setFont(qApp->font());

    header.clear();

    for(i=0; i<row; i++) header.append(QChar(0x41 + i));        // Row Header    
    Select_Grid->setVerticalHeaderLabels(header);
    for(i=0; i<row; i++) Select_Grid->verticalHeader()->setSectionResizeMode(i, QHeaderView::Stretch);
    Select_Grid->verticalHeader()->setFont(qApp->font());

    header.clear();

    // 1.1 calculate A1
    item = Select_Grid->item(0,0);
    Select_Grid->A1 = Select_Grid->visualItemRect(item);


    // 2. --- Load Test&Sample Select Structure ---

    Test_tree->blockSignals(true);
    Sample_tree->blockSignals(true);
    Fill_Tests();
    Fill_Samples();
    Test_tree->blockSignals(false);
    Sample_tree->blockSignals(false);

    // 3. --- Fill Initial Structure ---

    EnableTube_Initial.clear();
    ColorTube_Initial.clear();
    ColorTube_Current.clear();

    for(i=0; i<p->count_Tubes; i++)
    {
        EnableTube_Initial.append(p->enable_tube.at(i));
        ColorTube_Initial.append(p->color_tube.at(i));
        ColorTube_Current.append(p->color_tube.at(i));
    }

    // 4. --- Fill Active Tubes Array ---

    Protocol_ActiveTubes.clear();

    for(i=0; i<p->Plate.groups.size(); i++)
    {
        rt_GroupSamples *group = p->Plate.groups.at(i);
        for(j=0; j<group->samples.size(); j++)
        {
            rt_Sample *sample = group->samples.at(j);
            for(k=0; k<sample->tubes.size(); k++)
            {
                rt_Tube *tube = sample->tubes.at(k);
                pos = tube->pos;
                Protocol_ActiveTubes.append(pos);
            }
        }
    }    

}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Select_tubes::Fill_Samples()
{
    int i,j;
    int count_sample, count_groups;
    QString text;
    QStringList list_samples;
    QTreeWidgetItem *item;
    QTreeWidgetItem *item_main;
    rt_GroupSamples *source;
    rt_Sample *sample;

    if(p_prot == NULL) return;   

    Sample_tree->blockSignals(true);

    Sample_tree->clear();
    Sample_tree->setColumnCount(1);
    Sample_tree->setHeaderLabel(tr("Samples"));
    Sample_tree->header()->setFont(qApp->font());

    item_main = new QTreeWidgetItem(Sample_tree);
    item_main->setFlags(item_main->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsAutoTristate);
    item_main->setCheckState(0, Qt::Checked);
    item_main->setText(0, tr("All samples:"));


    count_groups = p_prot->Plate.groups.size();

    for(i=0; i<count_groups; i++)
    {
        source = p_prot->Plate.groups.at(i);
        count_sample = source->samples.size();

        for(j=0; j<count_sample; j++)
        {
            sample = source->samples.at(j);
            text = QString::fromStdString(sample->Unique_NameSample);
            if(list_samples.contains(text.trimmed())) continue;
            list_samples.append(text);
            item = new QTreeWidgetItem(item_main);
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(0, Qt::Checked);
            item->setText(0, text);
        }
    }

    item_main->setExpanded(true);

    Sample_tree->blockSignals(false);    
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Select_tubes::Fill_Tests()
{
    int i,j,k;
    int count_test;
    int count_tube;
    int count_channel;
    QString text, name_str;
    QTreeWidgetItem *item_main;
    QTreeWidgetItem *item;
    QTreeWidgetItem *item_child;

    rt_Test         *test;
    rt_TubeTest     *tube;
    rt_ChannelTest  *channel;

    if(p_prot == NULL) return;
    count_test = p_prot->tests.size();

    Test_tree->blockSignals(true);

    QStringList header_list;
    header_list << tr("Tests") << " ";

    Test_tree->clear();
    Test_tree->setColumnCount(2);
    Test_tree->setHeaderLabels(header_list);
    Test_tree->header()->setMinimumSectionSize(1);
    Test_tree->header()->setStretchLastSection(false);
    Test_tree->header()->resizeSection(1, 50);
    Test_tree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    Test_tree->header()->setSectionResizeMode(1, QHeaderView::Fixed);

    Test_tree->header()->setFont(qApp->font());

    item_main = new QTreeWidgetItem(Test_tree);
    item_main->setFlags(item_main->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsAutoTristate);
    item_main->setCheckState(0, Qt::Checked);
    item_main->setText(0, tr("All tests:"));    

    for(i=0; i<count_test; i++)
    {
        test = p_prot->tests.at(i);
        text = QString::fromStdString(test->header.Name_Test);
        item = new QTreeWidgetItem(item_main);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(0, Qt::Checked);
        if(map_TestTranslate->size()) text = map_TestTranslate->value(text, text);
        item->setText(0, text);
        item->setText(1, QString::fromStdString(test->header.version));
        if(test->header.Hash == "ok") item->setIcon(0, logo);
        else item->setIcon(0, logonull);

        count_tube = test->tubes.size();
        if(count_tube < 2) continue;

        item->setFlags(item->flags() | Qt::ItemIsAutoTristate);

        for(j=0; j<count_tube; j++)
        {
            tube = test->tubes.at(j);
            count_channel = tube->channels.size();
            text = "";
            for(k=0; k<count_channel; k++)
            {
                channel = tube->channels.at(k);
                if(text.length()) text += "; ";
                name_str = QString::fromStdString(channel->name);
                if(map_TestTranslate->size()) name_str = map_TestTranslate->value(name_str, name_str);
                text += name_str;
            }
            item_child = new QTreeWidgetItem(item);
            item_child->setFlags(item_child->flags() | Qt::ItemIsUserCheckable);
            item_child->setCheckState(0, Qt::Checked);
            item_child->setText(0, text);
            item_child->setIcon(0, logonull);
        }

        item->setExpanded(true);
    }

    item_main->setExpanded(true);

    Test_tree->blockSignals(false);
}


//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Select_tubes::cellSelected(int row, int col)
{
    QTableWidgetItem* item;
    QColor color;

    if(p_prot == NULL) return;

    int pos = col + row * Select_Grid->columnCount();

    if(Select_Grid->color_select->isChecked())
    {
        //color = QColorDialog::getColor(Qt::gray, this, tr("Select Color"));
        Color_ToolButton *button = (Color_ToolButton*)Fluor_Buttons->checkedButton();
        color = button->color;
        if(color.isValid())
        {
            Select_Grid->map_ColorHistory.clear();
            color_History->setDisabled(false);
            Select_Grid->map_ColorHistory.insert(pos, p_prot->color_tube.at(pos));
            p_prot->color_tube.at(pos) = color.rgb();
            ColorTube_Current.replace(pos, color.rgb());

            //qDebug() << "color: " << pos << p_prot->color_tube.at(pos);
        }
    }
    else
    {
        p_prot->enable_tube[pos] = !p_prot->enable_tube[pos] & 0x01;

        item = Select_Grid->item(row,col);
        if(p_prot->enable_tube[pos]) item->setText("1");    // call repaint cell
        else item->setText("0");
    }

    // ...

    enable_event.enable_tubes.clear();
    //enable_event.enable_tubes.resize(1);
    enable_event.enable_tubes.push_back((short)pos);
    QApplication::sendEvent(main_widget, &enable_event);

    Select_Grid->viewport()->update();
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Select_tubes::start_ColumnSelect(int col)
{
    /*Column_Selected.clear();

    Column_Selected.append(col);

    qDebug() << "start_ColumnSelect: " << Column_Selected;
    */
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Select_tubes::add_ColumnSelect(int col)
{
    //if(!Column_Selected.contains(col)) Column_Selected.append(col);
    //qDebug() << "add_ColumnSelect: " << Column_Selected;
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Select_tubes::columnMoved(int col)
{
    columnSelected(col);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Select_tubes::rowMoved(int row)
{
    rowSelected(row);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Select_tubes::ColumnsSelect(QVector<int> vec_column)
{
    int i;
    int pos;
    QTableWidgetItem* item;
    int row, col, nCol;
    QColor color;

    if(p_prot == NULL) return;

    p_prot->Plate.PlateSize(p_prot->count_Tubes, row, col);

    if(Select_Grid->color_select->isChecked())
    {
        //color = QColorDialog::getColor(Qt::gray, this, tr("Select Color"));
        Color_ToolButton *button = (Color_ToolButton*) Fluor_Buttons->checkedButton();
        color = button->color;

        Select_Grid->map_ColorHistory.clear();
        color_History->setDisabled(false);
    }

    enable_event.enable_tubes.clear();

    foreach(nCol, vec_column)
    {
        for(i=0; i<row; i++)
        {
            pos = i*col + nCol;

            if(Protocol_ActiveTubes.indexOf((short)pos) < 0) continue;
            //if(ID_Methods.at(pos) != p_prot->current_Method && !p_prot->enable_tube.at(pos)) continue;

            if(Select_Grid->color_select->isChecked())
            {
                if(color.isValid())
                {
                    Select_Grid->map_ColorHistory.insert(pos, p_prot->color_tube.at(pos));
                    p_prot->color_tube.at(pos) = color.rgb();
                    ColorTube_Current.replace(pos, color.rgb());
                }
            }
            else
            {
                p_prot->enable_tube[pos] = !p_prot->enable_tube[pos] & 0x01;
                item = Select_Grid->item(i,nCol);
                if(p_prot->enable_tube[pos]) item->setText("1");    // call repaint cell
                else item->setText("0");                            // ...
            }

            enable_event.enable_tubes.append((short)pos);
        }
    }

    QApplication::sendEvent(main_widget, &enable_event);

    //qDebug() << main_widget << enable_event.enable_tubes;
    Select_Grid->viewport()->update();

}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Select_tubes::columnSelected(int nCol)
{
    int i;
    int pos;
    QTableWidgetItem* item;
    int row, col;
    QColor color;

    if(p_prot == NULL) return;

    p_prot->Plate.PlateSize(p_prot->count_Tubes, row, col);


    if(Select_Grid->color_select->isChecked())
    {
        //color = QColorDialog::getColor(Qt::gray, this, tr("Select Color"));
        Color_ToolButton *button = (Color_ToolButton*) Fluor_Buttons->checkedButton();
        color = button->color;

        Select_Grid->map_ColorHistory.clear();
        color_History->setDisabled(false);
    }

    enable_event.enable_tubes.clear();

    for(i=0; i<row; i++)
    {
        pos = i*col + nCol;

        if(Protocol_ActiveTubes.indexOf((short)pos) < 0) continue;
        //if(ID_Methods.at(pos) != p_prot->current_Method && !p_prot->enable_tube.at(pos)) continue;

        if(Select_Grid->color_select->isChecked())
        {
            if(color.isValid())
            {
                Select_Grid->map_ColorHistory.insert(pos, p_prot->color_tube.at(pos));
                p_prot->color_tube.at(pos) = color.rgb();
                ColorTube_Current.replace(pos, color.rgb());
            }
        }
        else
        {
            p_prot->enable_tube[pos] = !p_prot->enable_tube[pos] & 0x01;
            item = Select_Grid->item(i,nCol);
            if(p_prot->enable_tube[pos]) item->setText("1");    // call repaint cell
            else item->setText("0");                            // ...
        }

        enable_event.enable_tubes.append((short)pos);
    }

    QApplication::sendEvent(main_widget, &enable_event);

    //qDebug() << main_widget << enable_event.enable_tubes;
    Select_Grid->viewport()->update();
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Select_tubes::RowsSelect(QVector<int> vec_row)
{
    int i;
    int pos;
    QTableWidgetItem* item;
    int row, col, nRow;
    QColor color;

    if(p_prot == NULL) return;

    p_prot->Plate.PlateSize(p_prot->count_Tubes, row, col);

    if(Select_Grid->color_select->isChecked())
    {
        //color = QColorDialog::getColor(Qt::gray, this, tr("Select Color"));
        Color_ToolButton *button = (Color_ToolButton*) Fluor_Buttons->checkedButton();
        color = button->color;

        Select_Grid->map_ColorHistory.clear();
        color_History->setDisabled(false);
    }

    enable_event.enable_tubes.clear();

    foreach(nRow, vec_row)
    {
        for(i=0; i<col; i++)
        {
            pos = nRow*col + i;

            if(Protocol_ActiveTubes.indexOf((short)pos) < 0) continue;
            //if(ID_Methods.at(pos) != p_prot->current_Method && !p_prot->enable_tube.at(pos)) continue;

            if(Select_Grid->color_select->isChecked())
            {
                if(color.isValid())
                {
                    Select_Grid->map_ColorHistory.insert(pos, p_prot->color_tube.at(pos));
                    p_prot->color_tube.at(pos) = color.rgb();
                    ColorTube_Current.replace(pos, color.rgb());
                }
            }
            else
            {
                p_prot->enable_tube[pos] = !p_prot->enable_tube[pos] & 0x01;
                item = Select_Grid->item(nRow,i);
                if(p_prot->enable_tube[pos]) item->setText("1");    // call repaint cell
                else item->setText("0");                            // ...

            }

            enable_event.enable_tubes.append((short)pos);
        }
    }

    QApplication::sendEvent(main_widget, &enable_event);
    Select_Grid->viewport()->update();

}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Select_tubes::rowSelected(int nRow)
{
    int i;
    int pos;
    QTableWidgetItem* item;
    int row, col;
    QColor color;

    if(p_prot == NULL) return;

    p_prot->Plate.PlateSize(p_prot->count_Tubes, row, col);

    if(Select_Grid->color_select->isChecked())
    {
        //color = QColorDialog::getColor(Qt::gray, this, tr("Select Color"));
        Color_ToolButton *button = (Color_ToolButton*) Fluor_Buttons->checkedButton();
        color = button->color;

        Select_Grid->map_ColorHistory.clear();
        color_History->setDisabled(false);
    }

    enable_event.enable_tubes.clear();
    for(i=0; i<col; i++)
    {
        pos = nRow*col + i;

        if(Protocol_ActiveTubes.indexOf((short)pos) < 0) continue;
        //if(ID_Methods.at(pos) != p_prot->current_Method && !p_prot->enable_tube.at(pos)) continue;

        if(Select_Grid->color_select->isChecked())
        {
            if(color.isValid())
            {
                Select_Grid->map_ColorHistory.insert(pos, p_prot->color_tube.at(pos));
                p_prot->color_tube.at(pos) = color.rgb();
                ColorTube_Current.replace(pos, color.rgb());
            }
        }
        else
        {
            p_prot->enable_tube[pos] = !p_prot->enable_tube[pos] & 0x01;
            item = Select_Grid->item(nRow,i);
            if(p_prot->enable_tube[pos]) item->setText("1");    // call repaint cell
            else item->setText("0");                            // ...

        }

        enable_event.enable_tubes.append((short)pos);
    }

    //qDebug() << "send row: " << enable_event.enable_tubes;

    QApplication::sendEvent(main_widget, &enable_event);
    Select_Grid->viewport()->update();
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Select_tubes::all_Selected()
{
    int i,j;
    int pos;
    QTableWidgetItem* item;
    int row, col;
    QColor color;

    if(p_prot == NULL) return;

    p_prot->Plate.PlateSize(p_prot->count_Tubes, row, col);

    if(Select_Grid->color_select->isChecked())
    {
        //color = QColorDialog::getColor(Qt::gray, this, tr("Select Color"));
        Color_ToolButton *button = (Color_ToolButton*) Fluor_Buttons->checkedButton();
        color = button->color;

        Select_Grid->map_ColorHistory.clear();
        color_History->setDisabled(false);
    }

    enable_event.enable_tubes.clear();
    //enable_event.enable_tubes.resize(row*col);

    for(i=0; i<row; i++)
    {
        for(j=0; j<col; j++)
        {
            pos = j + i*col;

            if(Protocol_ActiveTubes.indexOf((short)pos) < 0) continue;

            enable_event.enable_tubes.append((short)pos);

            //if(ID_Methods.at(pos) != p_prot->current_Method && !p_prot->enable_tube.at(pos)) continue;

            if(Select_Grid->color_select->isChecked())
            {
                if(color.isValid())
                {
                    Select_Grid->map_ColorHistory.insert(pos, p_prot->color_tube.at(pos));
                    p_prot->color_tube.at(pos) = color.rgb();
                    ColorTube_Current.replace(pos, color.rgb());
                }
            }
            else
            {
                p_prot->enable_tube[pos] = !p_prot->enable_tube[pos] & 0x01;
                item = Select_Grid->item(i,j);
                if(p_prot->enable_tube[pos]) item->setText("1");    // call repaint cell
                else item->setText("0");
            }
        }
    }
    QApplication::sendEvent(main_widget, &enable_event);
    Select_Grid->viewport()->update();
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Select_tubes::ClearPlate()
{
    int i,j;
    int pos;
    QTableWidgetItem* item;
    int row, col;

    if(p_prot == NULL) return;

    p_prot->Plate.PlateSize(p_prot->count_Tubes, row, col);

    enable_event.enable_tubes.clear();

    for(i=0; i<row; i++)
    {
        for(j=0; j<col; j++)
        {
            pos = j + i*col;
            enable_event.enable_tubes.append((short)pos);

            p_prot->enable_tube[pos] = false;

            item = Select_Grid->item(i,j);
            item->setText("0");
        }
    }
    QApplication::sendEvent(main_widget, &enable_event);

    Select_Grid->position_select->setChecked(true);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Select_tubes::InitialState()
{
    int i,j;
    int pos;
    QTableWidgetItem* item;
    int row, col;

    if(p_prot == NULL) return;

    p_prot->Plate.PlateSize(p_prot->count_Tubes, row, col);

    enable_event.enable_tubes.clear();

    ColorTube_Current.clear();

    for(i=0; i<row; i++)
    {
        for(j=0; j<col; j++)
        {
            pos = j + i*col;
            enable_event.enable_tubes.append((short)pos);

            p_prot->enable_tube[pos] = EnableTube_Initial.at(pos);
            p_prot->color_tube[pos] = ColorTube_Initial.at(pos);
            ColorTube_Current.append(ColorTube_Initial.at(pos));

            item = Select_Grid->item(i,j);
            if(p_prot->enable_tube[pos]) item->setText("1");    // call repaint cell
            else item->setText("0");
        }
    }

    QApplication::sendEvent(main_widget, &enable_event);

    Select_Grid->position_select->setChecked(true);
    Select_Grid->select_rejime->triggered(Select_Grid->position_select);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void ItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QColor color,color_inv;
    int r,g,b;
    double value;
    QVector<QString> *vec;
    QString text;
    int rad;
    int x,y;
    double X,Y,R;
    int pos;
    int min_value = qMin(option.rect.width(), option.rect.height());
    int min_value_Pro = qMin(A1->width(),A1->height());

    if(abs(min_value_Pro - min_value) > 3)
    {
        min_value_Pro = min_value;
        *A1 = option.rect;
    }

    QFont f = painter->font();
    QFont f_new = f;
    f_new.setPointSize(f.pointSize()-0);

    //R = (double)min_value/2. * 0.80;
    R = (double)min_value_Pro/2. * 0.90;
    X = (double)option.rect.x() + (double)option.rect.width()/2. - R;
    Y = (double)option.rect.y() + (double)option.rect.height()/2. - R;

    rad = (int)(R+0.5);
    x = (int)(X+0.5);
    y = (int)(Y+0.5);

    QPalette pal = option.palette;
    QStyleOptionViewItem  viewOption(option);

    viewOption.state &= ~QStyle::State_HasFocus;    // disable state HasFocus

    QStyledItemDelegate::paint(painter, viewOption, index);

    painter->setRenderHint(QPainter::Antialiasing);

    //__1. Background
    if(style == "fusion") painter->fillRect(option.rect, QColor(255,255,255));
    else painter->fillRect(option.rect, QColor(255,255,225));

    //__2. Enable Tube
    pos = index.column() + index.row() * index.model()->columnCount();

    //qDebug() << pos << index.column() << index.row() << min_value_Pro << min_value; //index.model()->columnCount();

    if(pos >= 0 && pos < p_prot->count_Tubes)
    {
        if(p_prot->enable_tube.at(pos))
        {
            color = p_prot->color_tube.at(pos);
            //color.getRgb(&r,&g,&b);
            //color = QColor(b,g,r).rgb();
            painter->setPen(QPen(Qt::black,1,Qt::SolidLine));
            painter->setBrush(QBrush(color, Qt::SolidPattern));
            painter->drawEllipse(x,y,2*rad,2*rad);
        }

        if(enable_initial->at(pos))
        {
            if(numeration)
            {
                painter->setFont(f_new);
                if(!p_prot->enable_tube.at(pos)) color = Qt::white;
                color.getRgb(&r,&g,&b);
                value = 0.3*(double)(r) + 0.59*(double)(g) + 0.11*(double)(b);
                if(value < 128) color = Qt::white;
                else color = Qt::black;

                painter->setPen(QPen(color));
                vec = p_prot->Map_Reserve.value("PositionInProtocol", NULL);
                if(vec != NULL)
                {
                    text = vec->at(pos).trimmed();
                    if(text.length())
                    {
                        painter->drawText(option.rect, Qt::AlignCenter, text);
                    }
                }
                painter->setFont(f);
            }
        }
    }
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
TableWidget::TableWidget(int rows, int cols, QWidget *parent)
            : QTableWidget(rows, cols, parent)
{
    QAbstractButton* btn = findChild<QAbstractButton*>();
    if(btn) corner_btn = btn;

    select_rejime = new QActionGroup(this);
    position_select = new QAction(tr("position rejime"), this);
    position_select->setCheckable(true);
    color_select = new QAction(tr("color rejime"), this);
    color_select->setCheckable(true);
    clear_plate = new QAction(QIcon(":/images/clear_16.png"), tr("clear Plate"), this);
    back_InitialState = new QAction(tr("back to initial state"), this);
    select_rejime->addAction(position_select);
    select_rejime->addAction(color_select);
    position_select->setChecked(true);
    numeration_rejime = new QAction(tr("numeration"), this);
    numeration_rejime->setCheckable(true);

    cursor_ColorFill = QCursor(QPixmap(":/images/cursor_ColorFill.png"));

    connect(select_rejime, SIGNAL(triggered(QAction*)), this, SLOT(change_SelectRejime(QAction*)));

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void TableWidget::change_SelectRejime(QAction *select)
{
    bool state = false;

    if(select == position_select) setCursor(Qt::ArrowCursor);
    if(select == color_select) {setCursor(cursor_ColorFill); state = true;}

    emit color_Rejime(state);
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
TableWidget::~TableWidget()
{
    delete position_select;
    delete color_select;
    delete select_rejime;
    delete clear_plate;
    delete numeration_rejime;
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void TableWidget::mousePressEvent(QMouseEvent *event)
{
    QTableWidget::mousePressEvent(event);

    if(event->button() == Qt::LeftButton)
    {
        point_0 = event->pos();
        point_1 = point_0;

        selected = true;
        coord_0.setX(currentColumn());
        coord_0.setY(currentRow());
    }
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void TableWidget::mouseReleaseEvent(QMouseEvent *event)
{
    int i,j;
    int pos;
    EnableTubes *enable;
    rt_Protocol *p;
    QTableWidgetItem* item;
    bool zoom = false;
    int row, col;
    QColor color;

    if(selected && moving) zoom = true;
    selected = false;
    moving = false;
    if(event->button() == Qt::LeftButton) QTableWidget::mouseReleaseEvent(event);

    coord_1.setX(currentColumn());
    coord_1.setY(currentRow());    

    if(zoom && coord_0 != coord_1)
    {
        p = ((Select_tubes*)(this->address))->p_prot;
        enable  = &((Select_tubes*)(this->address))->enable_event;

        enable->enable_tubes.clear();

        int min_row = qMin(coord_0.y(),coord_1.y());
        int min_col = qMin(coord_0.x(),coord_1.x());
        int max_row = qMax(coord_0.y(),coord_1.y());
        int max_col = qMax(coord_0.x(),coord_1.x());

        if(color_select->isChecked())
        {
            //color = QColorDialog::getColor(Qt::gray, this, tr("Select Color"));
            Color_ToolButton *button = (Color_ToolButton*)color_buttons->checkedButton();
            color = button->color;

            map_ColorHistory.clear();
            color_history->setDisabled(false);
        }

        p->Plate.PlateSize(p->count_Tubes, row, col);

        for(i=min_row; i<=max_row; i++)
        {
            for(j=min_col; j<=max_col; j++)
            {
                pos = j + i*col;

                if(color_select->isChecked())       // Color
                {
                    if(color.isValid())
                    {
                        map_ColorHistory.insert(pos, p->color_tube.at(pos));
                        p->color_tube.at(pos) = color.rgb();
                        ColorTube_Current->replace(pos, color.rgb());
                    }
                }
                else                                // Position
                {
                    p->enable_tube[pos] = !p->enable_tube[pos];
                    item = this->item(i,j);
                    if(p->enable_tube[pos]) item->setText("1");
                    else item->setText("0");
                }

                enable->enable_tubes.append((short)pos);
            }
        }

        QApplication::sendEvent(((Select_tubes*)(this->address))->main_widget, enable);
    }

    viewport()->update();
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Select_tubes::Previous_EnableTubes(QVector<short> *prev_enable)
{
    int i,j;

    QTableWidgetItem* item;
    int row, col;
    int pos;

    enable_event.enable_tubes.clear();

    p_prot->Plate.PlateSize(p_prot->count_Tubes, row, col);

    for(i=0; i<row; i++)
    {
        for(j=0; j<col; j++)
        {
            pos = j + i*col;

            p_prot->enable_tube[pos] = prev_enable->at(pos);
            item = Select_Grid->item(i,j);
            if(p_prot->enable_tube[pos]) item->setText("1");
            else item->setText("0");

            enable_event.enable_tubes.append((short)pos);
        }
    }
    Tab_Select->setCurrentIndex(1);

    QApplication::sendEvent(main_widget, &enable_event);
    Select_Grid->viewport()->update();
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void TableWidget::mouseMoveEvent(QMouseEvent *event)
{
    QTableWidget::mouseMoveEvent(event);

    if(selected)
    {
        point_1 = event->pos();
        moving = true;
        viewport()->update();
    }
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void TableWidget::paintEvent(QPaintEvent *e)
{
    QTableWidget::paintEvent(e);

    if(selected && moving)
    {
        QPen pen(Qt::black,1,Qt::DashLine);
        QPainter painter_table(viewport());

        painter_table.setPen(pen);
        painter_table.drawRect(QRect(point_0,point_1));
    }
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void TableWidget::resizeEvent(QResizeEvent *event)
{
    QTableWidget::resizeEvent(event);

    if(this->rowCount() > 0 && columnCount() > 0)
    {
        QTableWidgetItem *item = this->item(0,0);
        A1 = this->visualItemRect(item);
    }
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void TreeWidget::click_item(QTreeWidgetItem *item, int col)
{
    int i;
    int count;
    bool sts_check = false;
    bool sts_uncheck = false;   

    blockSignals(true);

    Qt::CheckState state = item->checkState(0);

    if(item->parent())
    {        
        count = item->parent()->childCount();
        for(i=0; i<count; i++)
        {
            if(item->parent()->child(i)->checkState(0) == Qt::Checked) sts_check = true;
            else sts_uncheck = true;
        }
        if(!sts_check || !sts_uncheck) item->parent()->setCheckState(0,state);
        else item->parent()->setCheckState(0,Qt::PartiallyChecked);
    }
    else
    {        
        count = item->childCount();
        for(i=0; i<count; i++) item->child(i)->setCheckState(0, state);
    }

    blockSignals(false);
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Select_tubes::paintEvent(QPaintEvent* event)
{
        QDockWidget::paintEvent(event);        

        if(isFloating() && style()->objectName().compare( "Oxygen", Qt::CaseInsensitive ) != 0)
        {
                QRect rect = this->rect().adjusted( 0, 0, -1, -1 );

                QPainter painter( this );
                painter.setPen( QColor(Qt::black));//QColor( 145, 142, 142 ) );
                painter.setBrush( Qt::NoBrush );
                painter.drawRect( rect );                
        }
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Select_tubes::mouseReleaseEvent(QMouseEvent *e)
{
    if(e->button() == Qt::LeftButton)
    {
        qDebug() << "mouseReleaseEvent: ";
    }
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Select_tubes::tree_apply_change(QTreeWidgetItem *item, int col)
{
        //Test_tree->click_item(item, col);

        apply_change();
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
bool Select_tubes::check_Mask()
{
    int i,j;
    QVector<short> current_maskTests;
    QVector<short> current_maskSamples;
    QTreeWidgetItem *item;
    QTreeWidgetItem *item_ch;

    // Samples
    for(i=0; i<Sample_tree->topLevelItem(0)->childCount(); i++)
    {
        item = Sample_tree->topLevelItem(0)->child(i);
        current_maskSamples.push_back((short)item->checkState(0));
    }
    //qDebug() << "current_maskSamples: " << current_maskSamples;

    // Tests
    for(i=0; i<Test_tree->topLevelItem(0)->childCount(); i++)
    {
        item = Test_tree->topLevelItem(0)->child(i);
        current_maskTests.push_back((short)item->checkState(0));
        for(j=0; j<item->childCount(); j++)
        {
            item_ch = item->child(j);
            current_maskTests.push_back((short)item_ch->checkState(0));
        }
    }
    //qDebug() << "current_maskTests: " << current_maskTests;

    if(qEqual(current_maskTests.begin(), current_maskTests.end(), mask_Tests.begin()) &&
       qEqual(current_maskSamples.begin(), current_maskSamples.end(), mask_Samples.begin()))
    {
        return(true);
    }
    else
    {
        mask_Tests.clear();
        mask_Samples.clear();

        mask_Tests = current_maskTests.mid(0);
        mask_Samples = current_maskSamples.mid(0);
        //qDebug() << "mask: " << mask_Tests << mask_Samples;
    }

    return(false);
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Select_tubes::apply_change()
{
    int i,j,k,m,l;
    QString text;
    QString name_test;
    int pos;    
    QTreeWidgetItem *item;
    QTreeWidgetItem *item_ch;
    QTreeWidgetItem *item_main_S = Sample_tree->topLevelItem(0);
    QTreeWidgetItem *item_main_T = Test_tree->topLevelItem(0);
    rt_GroupSamples *source;
    rt_Sample       *sample;
    rt_Tube         *tube;

    bool control;
    rt_Preference   *preference;
    int control_countTubes;
    bool ok;
    int num;
    int count;

    enable_event.enable_tubes.clear();

    if(p_prot == NULL) return;
    //qDebug() << "apply_change()";

    if(check_Mask()) return;

    for(i=0; i<p_prot->enable_tube.size(); i++) p_prot->enable_tube[i] = false;    // all false


    for(l=0; l<item_main_S->childCount(); l++)
    {
        item = item_main_S->child(l);
        text = item->text(0);
        if(item->checkState(0) == Qt::Unchecked) continue;

        for(i=0; i<p_prot->Plate.groups.size(); i++)
        {
            source = p_prot->Plate.groups.at(i);
            for(m=0; m<source->samples.size(); m++)
            {
                sample = source->samples.at(m);
                if(text == QString::fromStdString(sample->Unique_NameSample))
                {
                    // sample is control? //
                    control = false;
                    control_countTubes = sample->tubes.size();
                    foreach(preference, sample->preference_Sample)
                    {
                        if(preference->name == "kind")
                        {
                            if(preference->value == "ControlPositive" || preference->value == "ControlNegative")
                            {
                                control = true;
                            }
                            break;
                        }
                    }
                    if(control)
                    {
                        foreach(preference, sample->preference_Sample)
                        {
                            if(preference->name == "ControlTubesCount")
                            {
                                num = QString::fromStdString(preference->value).toInt(&ok);
                                if(ok) control_countTubes = num;
                                break;
                            }
                        }
                    }
                    //...

                    name_test = QString::fromStdString(sample->p_Test->header.Name_Test);
                    if(map_TestTranslate->size()) name_test = map_TestTranslate->value(name_test, name_test);

                    for(j=0; j<item_main_T->childCount(); j++)
                    {
                        item = item_main_T->child(j);
                        if(item->childCount())
                        {
                            if(item->checkState(0) != Qt::Unchecked &&
                               item->text(0) == name_test)// &&
                               //sample->tubes.size() == item->childCount())
                            {
                                count = sample->tubes.size();
                                if(control) count = control_countTubes;
                                for(k=0; k<count; k++)
                                {
                                    if(k >= item->childCount()) break;
                                    item_ch = item->child(k);
                                    if(item_ch->checkState(0) != Qt::Unchecked)
                                    {
                                        tube = sample->tubes.at(k);
                                        pos = tube->pos;
                                        p_prot->enable_tube[pos] = true;
                                    }
                                }
                            }
                        }
                        else
                        {
                            if(item->checkState(0) != Qt::Unchecked &&
                               item->text(0) == name_test &&
                               sample->tubes.size() == 1)
                            {
                                tube = sample->tubes.at(0);
                                pos = tube->pos;
                                p_prot->enable_tube[pos] = true;
                            }
                        }
                    }
                }
            }
        }
    }

    //qDebug() << "apply_change():" << QVector<short>::fromStdVector(p_prot->enable_tube);
    //--- ReDraw Chart ---
    for(i=0; i<p_prot->count_Tubes; i++) enable_event.enable_tubes.append(i);
    QApplication::sendEvent(main_widget, &enable_event);
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Select_tubes::selectALL_Sample()
{
    int i;
    QTreeWidgetItem *item;
    QTreeWidgetItem *item_main;

    if(p_prot == NULL) return;

    Sample_tree->blockSignals(true);

    item_main = Sample_tree->topLevelItem(0);
    int count = item_main->childCount();
    //int count = Sample_tree->topLevelItemCount();

    for(i=0; i<count; i++)
    {
        //item = Sample_tree->topLevelItem(i);
        item = item_main->child(i);
        item->setCheckState(0, Qt::Checked);
    }

    Sample_tree->blockSignals(false);

    apply_change();

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Select_tubes::selectNONE_Sample()
{
    int i;
    QTreeWidgetItem *item;
    QTreeWidgetItem *item_main;

    if(p_prot == NULL) return;

    Sample_tree->blockSignals(true);

    item_main = Sample_tree->topLevelItem(0);
    int count = item_main->childCount();
    //int count = Sample_tree->topLevelItemCount();

    for(i=0; i<count; i++)
    {
        //item = Sample_tree->topLevelItem(i);
        item = item_main->child(i);
        item->setCheckState(0, Qt::Unchecked);
    }

    Sample_tree->blockSignals(false);

    apply_change();

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Select_tubes::selectALL_Test()
{
    int i,j;
    int count_child;
    QTreeWidgetItem *item_main;
    QTreeWidgetItem *item;
    QTreeWidgetItem *item_child;    

    if(p_prot == NULL) return;

    Test_tree->blockSignals(true);

    item_main = Test_tree->topLevelItem(0);
    int count = item_main->childCount();
    //int count = Test_tree->topLevelItemCount();

    for(i=0; i<count; i++)
    {
        //item = Test_tree->topLevelItem(i);
        item = item_main->child(i);
        item->setCheckState(0, Qt::Checked);

        count_child = item->childCount();
        for(j=0; j<count_child; j++)
        {
            item_child = item->child(j);
            item_child->setCheckState(0, Qt::Checked);
        }
    }

    Test_tree->blockSignals(false);

    apply_change();
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Select_tubes::selectNONE_Test()
{
    int i,j;
    int count_child;
    QTreeWidgetItem *item_main;
    QTreeWidgetItem *item;
    QTreeWidgetItem *item_child;    

    if(p_prot == NULL) return;

    Test_tree->blockSignals(true);

    item_main = Test_tree->topLevelItem(0);
    int count = item_main->childCount();
    //int count = Test_tree->topLevelItemCount();

    for(i=0; i<count; i++)
    {
        //item = Test_tree->topLevelItem(i);
        item = item_main->child(i);
        item->setCheckState(0, Qt::Unchecked);

        count_child = item->childCount();
        for(j=0; j<count_child; j++)
        {
            item_child = item->child(j);
            item_child->setCheckState(0, Qt::Unchecked);
        }
    }

    Test_tree->blockSignals(false);

    apply_change();
}
//-------------------------------------------------------------------------------
//--- restore_Cursor()
//-------------------------------------------------------------------------------
void Select_tubes::restore_Cursor()
{
    QCursor cur;
    QPoint pos;


    //QApplication::restoreOverrideCursor();

    Select_Grid->viewport()->setCursor(cursor_FREE);
    Select_Grid->setCursor(cursor_FREE);
    Select_Grid->horizontalHeader()->setCursor(cursor_FREE);

    mTitleBar->setCursor(cursor_FREE);




    cur = this->cursor();
    pos = cur.pos();
    cur.setPos(mapToGlobal(QPoint(-1,-1)));
    //cur.setPos(10,10);
    qApp->processEvents();
    cur.setPos(pos);


    if(Select_Grid->position_select->isChecked())
    {
        Select_Grid->viewport()->setCursor(Qt::ArrowCursor);
        Select_Grid->setCursor(Qt::ArrowCursor);
        Select_Grid->horizontalHeader()->setCursor(Qt::ArrowCursor);
        Select_Grid->verticalHeader()->setCursor(Qt::ArrowCursor);
    }
    else
    {
        Select_Grid->viewport()->setCursor(Select_Grid->cursor_ColorFill);
        Select_Grid->setCursor(Select_Grid->cursor_ColorFill);
        Select_Grid->horizontalHeader()->setCursor(Select_Grid->cursor_ColorFill);
        Select_Grid->verticalHeader()->setCursor(Select_Grid->cursor_ColorFill);
    }


    mTitleBar->setCursor(Qt::ArrowCursor);

    qApp->processEvents();

}


//-------------------------------------------------------------------------------
//--- PositionInProtocol
//-------------------------------------------------------------------------------
void Select_tubes::PositionInProtocol(rt_Protocol *p)
{
    int pos_prot = 0;
    int pos;
    if(p == NULL) return;

    rt_GroupSamples *group;
    rt_Sample       *sample;
    rt_Tube         *tube;

    QVector<QString> *vec_PosProtocol = new QVector<QString>();
    vec_PosProtocol->resize(p->count_Tubes);
    vec_PosProtocol->fill("");

    foreach(group, p->Plate.groups)
    {
        foreach(sample, group->samples)
        {
            foreach(tube, sample->tubes)
            {
                pos_prot++;
                pos = tube->pos;
                vec_PosProtocol->replace(pos, QString::number(pos_prot));
            }
        }
    }
    if(!p->Map_Reserve.contains("PositionInProtocol"))
    {
       p->Map_Reserve.insert("PositionInProtocol", vec_PosProtocol);
    }
}
/*
//-------------------------------------------------------------------------------
//---
//-------------------------------------------------------------------------------
void MyHorizontalHeaderView::mouseReleaseEvent(QMouseEvent *event)
{
    qDebug() << "mouseReleaseEvent from HeaderView" << Column_Selected;

    selected = false;
    moving = false;
    if(event->button() == Qt::LeftButton) QHeaderView::mouseReleaseEvent(event);

    Column_Selected.clear();
}
//-------------------------------------------------------------------------------
//---
//-------------------------------------------------------------------------------
void MyHorizontalHeaderView::mousePressEvent(QMouseEvent *event)
{
    QHeaderView::mousePressEvent(event);

    if(event->button() == Qt::LeftButton)
    {
        selected = true;
        Column_Selected.clear();

    }

    qDebug() << "mousePressEvent: " << selected;
}
//-------------------------------------------------------------------------------
//---
//-------------------------------------------------------------------------------
void MyHorizontalHeaderView::mouseMoveEvent(QMouseEvent *event)
{
    QHeaderView::mouseMoveEvent(event);

    if(selected)
    {
        moving = true;
        //viewport()->update();
    }

    qDebug() << "mouseMoveEvent: " << selected << moving;
}
//-------------------------------------------------------------------------------
//---
//-------------------------------------------------------------------------------
void MyHorizontalHeaderView::paintSection(QPainter *painter, const QRect &rect, int logicalIndex)
{
    qDebug() << "paintSection: ";

    //if(!rect.isValid()) return;

    painter->save();
    QHeaderView::paintSection(painter,rect,logicalIndex);
    painter->restore();

    //if(selected && moving)
    //{
    painter->save();
        painter->fillRect(rect, QColor(100,100,100));
        if(!Column_Selected.contains(logicalIndex)) Column_Selected.append(logicalIndex);
    painter->restore();
    //}
}
*/
//-------------------------------------------------------------------------------
//---
//-------------------------------------------------------------------------------
void MyHeader::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
{
    QString text;
    QRect rect_new; // = rect;
    rect_new.setX(rect.x()-1);
    rect_new.setY(rect.y()-1);
    rect_new.setWidth(rect.width());
    rect_new.setHeight(rect.height());

    if(orientation() == Qt::Horizontal && logicalIndex == 0)
    {
        rect_new.setX(rect.x());
        rect_new.setWidth(rect.width()-1);
    }

    QPen pen(Qt::black,1,Qt::SolidLine);
    pen.setColor(QColor(50,50,50));

    if(!rect.isValid()) return;

    //qDebug() << "paint: " << logicalIndex << selected;
    painter->save();
    //QHeaderView::paintSection(painter,rect,logicalIndex);
    painter->restore();

    if(logicalIndex >= 0)
    {
        if(selected && Section_Selected.contains(logicalIndex)) painter->fillRect(rect, QColor(210,210,210));
        else painter->fillRect(rect, QColor(255,255,255));
        painter->setPen(pen);
        painter->drawRect(rect_new);
        if(orientation() == Qt::Horizontal) text = QString::number(logicalIndex+1);
        else text = QChar(0x41 + logicalIndex);
        painter->drawText(rect, Qt::AlignCenter, text);
    }
}
//-------------------------------------------------------------------------------
//---
//-------------------------------------------------------------------------------
void MyHeader::mouseReleaseEvent(QMouseEvent *event)
{
    QHeaderView::mouseReleaseEvent(event);
    //qDebug() << "mouseReleaseEvent: " << selected << moving << Section_Selected;

    if(event->button() == Qt::LeftButton && Section_Selected.size())
    {
        emit sSectionsSelect(Section_Selected);
    }

    selected = false;
    moving = false;
    Section_Selected.clear();
    viewport()->update();
}
//-------------------------------------------------------------------------------
//---
//-------------------------------------------------------------------------------
void MyHeader::mousePressEvent(QMouseEvent *event)
{
    int id = -1;

    QHeaderView::mousePressEvent(event);

    Section_Selected.clear();

    if(event->button() == Qt::LeftButton)
    {
        selected = true;
        id = logicalIndexAt(event->pos());
        Section_Selected.append(id);

        viewport()->update();
    }


    //qDebug() << "mousePressEvent: " << id << selected << moving << Section_Selected;
}
//-------------------------------------------------------------------------------
//---
//-------------------------------------------------------------------------------
void MyHeader::mouseMoveEvent(QMouseEvent *event)
{
    int id = -1;
    QHeaderView::mouseMoveEvent(event);

    id = logicalIndexAt(event->pos());
    //qDebug() << "mouseMoveEvent " << id << selected;

    if(selected)
    {
        if(id >= 0 && !Section_Selected.contains(id))
        {
            Section_Selected.append(id);
            viewport()->update();
        }
    }
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
/*void Color_Button::paintEvent(QPaintEvent *e)
{
    //QColor color;

    if(isChecked()) color.setAlpha(255);
    else color.setAlpha(100);

    //QToolButton::paintEvent(e);

    QPainter painter(this);

    QRect rect = painter.viewport();
    //rect.setX(2);
    //rect.setY(3);
    //rect.setWidth(rect.width() - 6);
    //rect.setHeight(rect.height() - 4);

    //painter.setBrush(QBrush(Qt::red, Qt::SolidPattern));
    //painter.drawRect(rect);

    painter.fillRect(rect, color);
    if(isChecked()) painter.drawRect(rect);

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Color_Button::mouseDoubleClickEvent(QMouseEvent *e)
{
    QPushButton::mouseDoubleClickEvent(e);

    //qDebug() << "mouseDoubleClickEvent: ";
    QColor color_temp = QColorDialog::getColor(color, this, tr("Select Color"));
    if(color_temp.isValid()) color = color_temp;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Color_Button::mouseReleaseEvent(QMouseEvent *e)
{
    QPushButton::mouseReleaseEvent(e);

    if(e->button() == Qt::RightButton)
    {
        QColor color_temp = QColorDialog::getColor(color, this, tr("Select Color"));
        if(color_temp.isValid()) color = color_temp;
    }
}*/
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Color_ToolButton::mouseDoubleClickEvent(QMouseEvent *e)
{
    QToolButton::mouseDoubleClickEvent(e);

    //qDebug() << "mouseDoubleClickEvent: ";

    QColor color_temp = QColorDialog::getColor(color, this, tr("Select Color"));
    if(color_temp.isValid())
    {
        color = color_temp;
        unsigned int val = color.rgba();
        QString str_color = QString::number(val,16);

        if(isChecked()) setStyleSheet(QString("QToolButton::checked {background-color: #%1; border: 1px solid black}").arg(str_color));
        else
        {
            color.setAlpha(60);
            val = color.rgba();
            str_color = QString::number(val,16);
            setStyleSheet(QString("QToolButton {background-color: #%1; border: 1px solid gray}").arg(str_color));
        }
    }
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Color_ToolButton::mouseReleaseEvent(QMouseEvent *e)
{
    QToolButton::mouseReleaseEvent(e);

    if(e->button() == Qt::RightButton)
    {
        QColor color_temp = QColorDialog::getColor(color, this, tr("Select Color"));
        if(color_temp.isValid())
        {
            color = color_temp;
            unsigned int val = color.rgba();
            QString str_color = QString::number(val,16);

            if(isChecked()) setStyleSheet(QString("QToolButton::checked {background-color: #%1; border: 1px solid black}").arg(str_color));
            else
            {
                color.setAlpha(60);
                val = color.rgba();
                str_color = QString::number(val,16);
                setStyleSheet(QString("QToolButton {background-color: #%1; border: 1px solid gray}").arg(str_color));
            }
        }
    }
}
