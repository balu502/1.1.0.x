#include "analyser_hrm.h"

extern "C" ANALYSER_HRMSHARED_EXPORT Analysis_Interface* __stdcall createAnalyser_plugin()
{
    return(new Analyser_HRM());
}


//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Analyser_HRM::Analyser_HRM()
{
    MainBox = NULL;
    ru_Lang = false;
    readCommonSettings();

    first_cycle = true;
    act_Channel = 0;
    current_History = NULL;

    Init_Colors();
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_HRM::Init_Colors()
{

    Color_Cluster.clear();
    Color_Cluster << Qt::red << Qt::darkGreen << Qt::blue << Qt::yellow << Qt::magenta << Qt::cyan << Qt::black << Qt::darkCyan;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_HRM::Default_Colors()
{
    int i;
    QColor color;
    Legend  *legend;
    QString qss;

    Init_Colors();

    i = 0;
    foreach(color, Color_Cluster)
    {
        legend = List_Legend.at(i);

        if(color.isValid())
        {
            qss = QString("background-color: %1; border: 1px solid black;").arg(color.name());
            legend->color_legend->setStyleSheet(qss);
            legend->color_legend->update();
        }
        i++;
    }

    slot_ChangeGroup();
    Apply_legend->setDisabled(true);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_HRM::Apply_Colors()
{
    int i;
    QColor color;
    Legend  *legend;

    i = 0;
    foreach(legend, List_Legend)
    {
        color = legend->color_legend->palette().color(QPalette::Window);
        Color_Cluster.replace(i, color);
        i++;
    }

    //slot_reAnalysis();
    slot_ChangeGroup();
    Apply_legend->setDisabled(true);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_HRM::Check_ChangeColor(int id)
{
    QColor color;
    Legend  *legend = List_Legend.at(id);

    color = legend->color_legend->palette().color(QPalette::Window);
    if(color != Color_Cluster.at(id))  Apply_legend->setDisabled(false);
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void* Analyser_HRM::Create_Win(void *pobj, void *main)
{
    int i;    
    QFont f = qApp->font();
    QColor bg = QColor(250,250,250); //Qt::white; //QColor(0,40,0);
    QStringList header;
    QList<int> list, list_main;
    QColor color;
    Legend  *legend;
    QString qss;

    QFont f_Labels("Times New Roman", 14, QFont::Bold);

    QWidget *parent = (QWidget*)pobj;
    main_widget = (QWidget*)main;

    list << main_widget->width()*0.7 << main_widget->width()*0.3;
    list_main << main_widget->height()*0.4 << main_widget->height()*0.6;

    MainBox = new QGroupBox(parent);
    MainBox->setFont(f);
    MainBox->setObjectName("Transparent");
    QVBoxLayout *main_layout = new QVBoxLayout;
    main_layout->setMargin(0);
    main_layout->setSpacing(4);
    MainBox->setLayout(main_layout);

    main_spl = new QSplitter(Qt::Vertical, MainBox);
    main_spl->setHandleWidth(10);
    main_spl->setChildrenCollapsible(false);
    main_layout->addWidget(main_spl);
    graph_spl = new QSplitter(Qt::Horizontal, MainBox);
    graph_spl->setHandleWidth(10);
    graph_spl->setChildrenCollapsible(false);
    control_spl = new QSplitter(Qt::Horizontal, MainBox);
    control_spl->setHandleWidth(10);
    control_spl->setChildrenCollapsible(false);
    main_spl->addWidget(graph_spl);
    main_spl->addWidget(control_spl);
    main_spl->setSizes(list_main);

    Tab_Graph = new QTabWidget(parent);
    Graph_Claster = new QGroupBox(parent);
    Graph_Claster->setObjectName("Transparent");
    Tab_Control = new QTabWidget(parent);
    Tab_Result = new QTabWidget(parent);

    //... plot: RawData and ...
    Hrm_RawData = new HrmPlot(true, true, bg, tr("Primary curve"), parent);
    Hrm_dFdT = new HrmPlot(true, true, bg, tr("dF/dT curve"), parent);
    Hrm_RawData_Border = new HrmPlot(true, true, bg, tr("Cropped curve"), parent);
    Hrm_Norm = new HrmPlot(true, true, bg, tr("Normalized curve"), parent);
    Hrm_DY_Norm = new HrmPlot(true, true, bg, tr("Normalized Melt curve"), parent);
    Hrm_Diff = new HrmPlot(true, true, bg, tr("Difference Curve"), parent);
    Tab_Graph->addTab(Hrm_RawData, "1");
    Tab_Graph->addTab(Hrm_dFdT, "2");
    Tab_Graph->addTab(Hrm_RawData_Border, "3");
    Tab_Graph->addTab(Hrm_Norm, "4");
    Tab_Graph->addTab(Hrm_DY_Norm, "5");
    Tab_Graph->addTab(Hrm_Diff, "6");
    Tab_Graph->setCurrentWidget(Hrm_DY_Norm);
    Tab_Graph->setFont(f_Labels);

    curve_BorderLeft = NULL;
    curve_BorderRight = NULL;

    //Hrm_Diff->setAxisAutoScale(QwtPlot::xBottom, false);
    //Hrm_Diff->setAxisScale(QwtPlot::xBottom, 0, 1);
    Hrm_Norm->setAxisAutoScale(QwtPlot::yLeft, false);
    Hrm_Norm->setAxisScale(QwtPlot::yLeft, 0, 1);
    Hrm_DY_Norm->setAxisAutoScale(QwtPlot::yLeft, false);
    Hrm_DY_Norm->setAxisScale(QwtPlot::yLeft, 0, 1);


    //... Cluster Plot ...
    Cluster_Plot = new HrmPlot(false, false, QColor(225,245,216), "", parent);
    Cluster_Plot->Shape_Cluster = &Shape_Cluster;
    //disconnect(Cluster_Plot->copy_to_clipboard_as_image, SIGNAL(triggered(bool)), 0, 0);
    //connect(Cluster_Plot->copy_to_clipboard_as_image, SIGNAL(triggered(bool))), Cluster_Plot, SLOT( )
    Cluster_Plot->enableAxis(QwtPlot::yLeft, true);
    Cluster_Plot->enableAxis(QwtPlot::xBottom, true);
    //Cluster_Plot->setAxisMaxMajor(QwtPlot::xBottom, 0);
    //Cluster_Plot->setAxisMaxMinor(QwtPlot::xBottom, 0);
    QVBoxLayout *cluster_layout = new QVBoxLayout;
    cluster_layout->setMargin(0);
    Graph_Claster->setLayout(cluster_layout);
    cluster_layout->addWidget(Cluster_Plot);
    //Cluster_Plot->grid->setVisible(false);
    QFont ff = qApp->font();
    ff.setBold(false);
    ff.setPointSize(ff.pointSize() - 4);
    Cluster_Plot->setAxisFont(QwtPlot::yLeft, ff);
    Cluster_Plot->setAxisFont(QwtPlot::xBottom, ff);


    QVBoxLayout *plot_V_layout = new QVBoxLayout();
    QHBoxLayout *plot_H1_layout = new QHBoxLayout();
    QHBoxLayout *plot_H2_layout = new QHBoxLayout();
    plot_V_layout->setMargin(1);
    plot_H1_layout->setMargin(1);
    plot_H1_layout->setSpacing(4);
    plot_H2_layout->setMargin(1);

    Cluster_Plot->canvas()->setLayout(plot_V_layout);
    plot_V_layout->addLayout(plot_H1_layout);
    plot_V_layout->addLayout(plot_H2_layout);

    Label_AxisY = new QLabel("D", parent);
    Label_AxisX = new QLabel("M", parent);
    Label_CountClusters = new QLabel("K = ...", parent);
    Label_ResultClustering = new QLabel("(100%)", parent);
    Label_AxisY->setFont(f_Labels);
    Label_AxisX->setFont(f_Labels);
    Label_CountClusters->setFont(f_Labels);
    f_Labels.setBold(false);
    Label_ResultClustering->setFont(f_Labels);
    plot_H1_layout->addWidget(Label_AxisY, 1, Qt::AlignLeft | Qt::AlignTop);
    plot_H1_layout->addWidget(Label_CountClusters, 0, Qt::AlignRight | Qt::AlignTop);
    plot_H1_layout->addWidget(Label_ResultClustering, 0, Qt::AlignRight | Qt::AlignTop);
    plot_H2_layout->addWidget(Label_AxisX, 1, Qt::AlignRight | Qt::AlignBottom);

    QFont ft = qApp->font();
    ft.setPointSize(f.pointSize()+2);
    ft.setBold(true);
    QwtText title;
    title.setFont(ft);
    title.setText(tr("Cluster Analysis"));
    Cluster_Plot->setTitle(title);


    //... Results ...
    header << tr("Identificator") << tr("T_peaks,°C") << tr("Group") << tr("%");
    Sample_Results = new QTableWidget(parent);
    Sample_Results->setSelectionBehavior(QAbstractItemView::SelectRows);
    Sample_Results->setSelectionMode(QAbstractItemView::SingleSelection);
    Sample_Results->setEditTriggers(QAbstractItemView::NoEditTriggers);
    Sample_Results->setContextMenuPolicy(Qt::CustomContextMenu);
    Sample_Results->setColumnCount(4);
    Sample_Results->setHorizontalHeaderLabels(header);    
    Sample_Results->setColumnWidth(1,80);
    Sample_Results->setColumnWidth(2,80);
    Sample_Results->setColumnWidth(3,40);
    Sample_Results->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
    Sample_Results->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
    Sample_Results->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);    
    Sample_Results->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    Samples_Delegate = new Samples_ItemDelegate();
    Samples_Delegate->border_quality = &Border_Quality;
    Sample_Results->setItemDelegate(Samples_Delegate);
    Samples_Delegate->map_reference = &HRM_Reference;

    header.clear();
    header << tr("Identificator") << tr("G_type") << tr("color") << tr("Sample");
    Groups_Results = new QTableWidget(parent);
    Groups_Results->setSelectionBehavior(QAbstractItemView::SelectRows);
    Groups_Results->setSelectionMode(QAbstractItemView::SingleSelection);
    //Groups_Results->setEditTriggers(QAbstractItemView::NoEditTriggers);
    Groups_Results->setContextMenuPolicy(Qt::CustomContextMenu);
    Groups_Results->setColumnCount(4);
    Groups_Results->setHorizontalHeaderLabels(header);
    Groups_Results->setColumnWidth(0,140);
    Groups_Results->setColumnWidth(1,80);
    Groups_Results->setColumnWidth(2,50);
    Groups_Results->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
    Groups_Results->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
    Groups_Results->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    Groups_Results->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    Groups_Delegate = new Groups_ItemDelegate();
    Groups_Results->setItemDelegate(Groups_Delegate);

    Tab_Result->addTab(Sample_Results, "Samples");
    Tab_Result->addTab(Groups_Results, "Groups");
    active_Table = NULL;

    //... Control Box ...
    Scroll_Box = new QScrollArea(parent);
    Control_Box = new QGroupBox(parent);
    Control_Box->setObjectName("Transparent");
    Scroll_Box->setWidget(Control_Box);
    Scroll_Box->setWidgetResizable(true);
    //Control_Box->setMinimumHeight(150);
    QHBoxLayout *control_layout = new QHBoxLayout();
    control_layout->setSpacing(10);

    ClusterParameters_Box = new QGroupBox(parent);
    ClusterParameters_Box->setObjectName("Transparent");
    QVBoxLayout *parameters_layout = new QVBoxLayout();
    parameters_layout->setMargin(0);
    ClusterParameters_Box->setLayout(parameters_layout);

    Parameters_Titul = new QLabel(tr("1. Parameters of clustering"), parent);
    parameters_layout->addWidget(Parameters_Titul, 0, Qt::AlignTop);
    QHBoxLayout *parameters_layout_Hor = new QHBoxLayout();
    //parameters_layout_Hor->setMargin(10);
    parameters_layout->addLayout(parameters_layout_Hor);
    QVBoxLayout *parameters_layout_V1 = new QVBoxLayout();
    QVBoxLayout *parameters_layout_V2 = new QVBoxLayout();
    parameters_layout_Hor->addSpacing(15);
    parameters_layout_Hor->addLayout(parameters_layout_V1);
    parameters_layout_Hor->addLayout(parameters_layout_V2);

    Auto_clustering = new QRadioButton(tr("auto"), parent);
    Depth_clustering = new QRadioButton(tr("Depth clustering"), parent);
    Count_clustering = new QRadioButton(tr("Count clustering"), parent);
    Reference_clustering = new QRadioButton(tr("Reference clustering"), parent);
    Reference_clustering->setDisabled(true);
    Auto_clustering->setChecked(true);
    //Depth_clustering->setChecked(true);
    //Count_clustering->setChecked(true);
    Group_clustering = new QButtonGroup(parent);
    Group_clustering->addButton(Auto_clustering, 0);
    Group_clustering->addButton(Depth_clustering, 1);
    Group_clustering->addButton(Count_clustering, 2);
    Group_clustering->addButton(Reference_clustering, 3);
    parameters_layout_V1->addWidget(Auto_clustering);
    parameters_layout_V1->addWidget(Depth_clustering);
    parameters_layout_V1->addWidget(Count_clustering);
    parameters_layout_V1->addWidget(Reference_clustering);

    Depth_Clusters = new QSlider(Qt::Horizontal, parent);
    Depth_Clusters->setMinimum(0);
    Depth_Clusters->setMaximum(100);
    Depth_Clusters->setValue(0);
    Depth_Clusters->setDisabled(true);
    Count_Clusters = new QSpinBox(parent);
    Count_Clusters->setRange(1,100);
    Count_Clusters->setValue(3);
    Count_Clusters->setFixedWidth(60);
    Count_Clusters->setDisabled(true);
    Count_Clusters->setFont(f_Labels);

    parameters_layout_V2->addStretch(1);
    parameters_layout_V2->addWidget(Depth_Clusters);
    parameters_layout_V2->addWidget(Count_Clusters);
    parameters_layout_V2->addStretch(1);


    // Temperature_Box
    Temperature_Box = new QGroupBox(parent);
    Temperature_Box->setObjectName("Transparent");
    QVBoxLayout *temperature_layout = new QVBoxLayout();
    temperature_layout->setMargin(0);
    Temperature_Box->setLayout(temperature_layout);

    Temperature_Titul = new QLabel(tr("2. Temperatures Borders"), parent);
    temperature_layout->addWidget(Temperature_Titul, 0, Qt::AlignTop);
    QHBoxLayout *temperature_layout_Hor = new QHBoxLayout();
    temperature_layout->addLayout(temperature_layout_Hor);
    QVBoxLayout *temperature_layout_V1 = new QVBoxLayout();
    QHBoxLayout *temperature_layout_H2 = new QHBoxLayout();
    temperature_layout_Hor->addSpacing(15);
    temperature_layout_Hor->addLayout(temperature_layout_V1);
    temperature_layout_Hor->addLayout(temperature_layout_H2);

    Auto_Temperature = new QRadioButton(tr("auto"), parent);
    Manual_Temperature = new QRadioButton(tr("manual"), parent);
    Auto_Temperature->setChecked(true);
    Group_Temperature = new QButtonGroup(parent);
    Group_Temperature->addButton(Auto_Temperature, 0);
    Group_Temperature->addButton(Manual_Temperature, 1);
    temperature_layout_V1->addWidget(Auto_Temperature);
    temperature_layout_V1->addWidget(Manual_Temperature);
    Left_Border = new QDoubleSpinBox(parent);
    Right_Border = new QDoubleSpinBox(parent);
    Left_Border->setFont(f_Labels);
    Right_Border->setFont(f_Labels);
    Left_Border->setDisabled(true);
    Right_Border->setDisabled(true);
    temperature_layout_H2->addWidget(Left_Border, 0, Qt::AlignBottom);
    temperature_layout_H2->addWidget(Right_Border, 0, Qt::AlignBottom);
    Left_Border->setDecimals(1);
    Left_Border->setSingleStep(0.1);
    Right_Border->setDecimals(1);
    Right_Border->setSingleStep(0.1);
    Left_Border->setSuffix(" °C");
    Right_Border->setSuffix(" °C");

    // TemperatureCorrect_Box
    TemperatureCorrect_Box = new QGroupBox(parent);
    TemperatureCorrect_Box->setObjectName("Transparent");
    QVBoxLayout *correct_layout = new QVBoxLayout();
    correct_layout->setMargin(0);
    TemperatureCorrect_Box->setLayout(correct_layout);

    TemperatureCorrect_Titul = new QLabel(tr("3. Temperatures Correction"), parent);
    correct_layout->addWidget(TemperatureCorrect_Titul, 0, Qt::AlignTop);
    QHBoxLayout *correct_layout_Hor = new QHBoxLayout();
    correct_layout->addLayout(correct_layout_Hor);
    Use_TemperatureCorrect = new QCheckBox(tr("use correction"), parent);
    Use_TemperatureCorrect->setDisabled(true);   
    correct_layout_Hor->addSpacing(15);
    correct_layout_Hor->addWidget(Use_TemperatureCorrect, 1, Qt::AlignLeft);
    Label_FileCorrect = new QLabel(parent);
    Label_FileCorrect->setFrameStyle(QFrame::Panel | QFrame::StyledPanel);
    Label_FileCorrect->setLineWidth(0);
    Label_FileCorrect->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    Label_FileCorrect->setVisible(false);
    //Label_FileCorrect->setText("here: file correction...");
    Open_FileCorrect = new QToolButton(parent);
    Open_FileCorrect->setIcon(QIcon(":/images/open_32"));
    Surface_3D = new QToolButton(parent);
    Surface_3D->setIcon(QIcon(":/images/surface_3D"));
    Surface_3D->setVisible(false);
    correct_layout_Hor->addWidget(Surface_3D, 0, Qt::AlignRight);
    correct_layout_Hor->addWidget(Label_FileCorrect, 0, Qt::AlignRight);
    correct_layout_Hor->addWidget(Open_FileCorrect, 0, Qt::AlignRight);

    // TemperatureNorm_Box
    TemperatureNorm_Box = new QGroupBox(parent);
    TemperatureNorm_Box->setObjectName("Transparent");
    QVBoxLayout *norm_layout = new QVBoxLayout();
    norm_layout->setMargin(0);
    TemperatureNorm_Box->setLayout(norm_layout);

    TemperatureNorm_Titul = new QLabel(tr("4. Temperature Normalization"), parent);
    norm_layout->addWidget(TemperatureNorm_Titul, 0, Qt::AlignTop);
    QHBoxLayout *norm_layout_Hor = new QHBoxLayout();
    norm_layout->addLayout(norm_layout_Hor);
    Use_TemperatureNorm = new QCheckBox(tr("use normalization"), parent);
    Use_TemperatureNorm->setChecked(true);
    norm_layout_Hor->addSpacing(15);
    norm_layout_Hor->addWidget(Use_TemperatureNorm, 1, Qt::AlignLeft);

    TemperatureNorm_Box->setVisible(false);     // !!! NOT VISIBLE


    // Select_Grid
    Select_Grid = new TableWidget(0,0,parent);
    Select_Grid->setSelectionMode(QAbstractItemView::NoSelection);
    Select_Grid->setEditTriggers(QAbstractItemView::NoEditTriggers);    
    Select_Grid->Curve_Results = &Curve_Results;
    Delegate = new ItemDelegate();
    Delegate->style = StyleApp;
    Delegate->Curve_Results = &Curve_Results;
    Delegate->map_reference = &HRM_Reference;
    Select_Grid->setItemDelegate(Delegate);
    Select_Grid->address = this;
    //f.setPointSize(f.pointSize()-4);
    Select_Grid->setFont(ff);
    Select_Grid->setContextMenuPolicy(Qt::CustomContextMenu);
    Delegate->A1 = &Select_Grid->A1;
    Delegate->numeration = false;

    // Select Group
    Label_Groups = new QLabel(tr("1. Selecting a group:"), parent);
    Select_Group = new QGroupBox(parent);
    QVBoxLayout *groups_layout = new QVBoxLayout();
    Select_Group->setLayout(groups_layout);
    Groups = new QComboBox(parent);
    Groups->setStyleSheet("background-color: #ffffff; selection-background-color: #d7d7ff; selection-color: black;");
    f = qApp->font();
    f.setPointSize(f.pointSize()+2);
    Groups->setFont(f);
    Label_Fluors = new QLabel(tr("2. Selecting a channel:"), parent);
    Fluors = new QComboBox(parent);
    Fluors->setStyleSheet("background-color: #ffffff; selection-background-color: #d7d7ff; selection-color: black;");
    Fluors->setFont(f);

    // Control Tool
    Control_Tool = new QToolBar(tr("Control_Tool"), parent);
    Control_Tool->setOrientation(Qt::Horizontal);
    Numerate_tool = new QAction(QIcon(":/images/Numerate_small.png"), tr("Numerate tubes"), this);
    Numerate_tool->setCheckable(true);
    Control_Tool->addAction(Numerate_tool);

    groups_layout->addSpacing(15);
    groups_layout->addWidget(Label_Groups);
    groups_layout->addWidget(Groups);
    groups_layout->addSpacing(10);
    groups_layout->addWidget(Label_Fluors);
    groups_layout->addWidget(Fluors);

    groups_layout->addStretch(1);
    groups_layout->addWidget(Control_Tool, 1, Qt::AlignBottom);

    connect(Group_clustering, SIGNAL(buttonClicked(int)), this, SLOT(slot_ChangeRejimeClustering(int)));
    connect(Depth_Clusters, SIGNAL(valueChanged(int)), this, SLOT(slot_ChangeParamCluster()));
    connect(Count_Clusters, SIGNAL(valueChanged(int)), this, SLOT(slot_ChangeParamCluster()));
    connect(Sample_Results, SIGNAL(cellClicked(int,int)), this, SLOT(slot_SampleResults_clicked(int,int)));
    connect(Sample_Results, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu_SampleResults()));
    connect(Groups_Results, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu_GroupsResults()));
    connect(Group_Temperature, SIGNAL(buttonClicked(int)), this, SLOT(slot_ChangeTemperatureBorders(int)));
    connect(Left_Border, SIGNAL(valueChanged(double)), this, SLOT(slot_reAnalysis()));
    connect(Right_Border, SIGNAL(valueChanged(double)), this, SLOT(slot_reAnalysis()));
    connect(Groups, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_ChangeGroup()));
    connect(Fluors, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_ChangeGroup()));
    connect(Select_Grid, SIGNAL(sReanalysis()), this, SLOT(slot_ChangeSelectGrid()));
    connect(Open_FileCorrect, SIGNAL(clicked(bool)), this, SLOT(slot_OpenFileCorrection()));
    connect(Select_Grid, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu_SelectGrid()));
    connect(Select_Grid->reference_rejime, SIGNAL(triggered(bool)), this, SLOT(change_Reference()));
    connect(Select_Grid->clear_AllReference, SIGNAL(triggered(bool)), this, SLOT(clear_ALLReferences()));
    connect(Use_TemperatureCorrect, SIGNAL(clicked(bool)), this, SLOT(Use_TempCorrection(bool)));
    connect(Surface_3D, SIGNAL(clicked(bool)), this, SLOT(View_3D()));
    connect(Numerate_tool, SIGNAL(toggled(bool)), this, SLOT(Numerate_Plate()));
    connect(Use_TemperatureNorm, SIGNAL(clicked(bool)), this, SLOT(Use_TempNormalization(bool)));

    Control_Box->setLayout(control_layout);
    QVBoxLayout *control_1_layout = new QVBoxLayout();
    //QVBoxLayout *control_2_layout = new QVBoxLayout();

    control_layout->addLayout(control_1_layout);
    //control_layout->addLayout(control_2_layout);
    //control_2_layout->addWidget(Select_Group);
    control_1_layout->addWidget(ClusterParameters_Box); //, 0, Qt::AlignTop);
    control_1_layout->addWidget(Temperature_Box); //, 0, Qt::AlignTop);
    control_1_layout->addWidget(TemperatureCorrect_Box);
    control_1_layout->addWidget(TemperatureNorm_Box);
    control_1_layout->addStretch(1);

    Control_Group = new QGroupBox(parent);
    Control_Group->setObjectName("Transparent");
    QHBoxLayout *main_control_layout = new QHBoxLayout();
    main_control_layout->setMargin(0);
    Control_Group->setLayout(main_control_layout);
    main_control_layout->addWidget(Select_Group); //, 0, Qt::AlignLeft);
    main_control_layout->addWidget(Tab_Control); //, 0, Qt::AlignRight);

    Scroll_Legend = new QScrollArea(parent);
    Legend_Box = new QGroupBox(parent);
    Legend_Box->setObjectName("Transparent");
    Scroll_Legend->setWidget(Legend_Box);
    Scroll_Legend->setWidgetResizable(true);
    QHBoxLayout *legend_layout = new QHBoxLayout();
    legend_layout->setMargin(1);
    legend_layout->setSpacing(2);
    Legend_Box->setLayout(legend_layout);
    ButtonsColor_Box = new QGroupBox(parent);
    ControlColor_Box = new QGroupBox(parent);
    legend_layout->addWidget(ButtonsColor_Box);
    legend_layout->addWidget(ControlColor_Box);

    QVBoxLayout *legend_1_layout = new QVBoxLayout();
    QVBoxLayout *legend_2_layout = new QVBoxLayout();
    ButtonsColor_Box->setLayout(legend_1_layout);
    ControlColor_Box->setLayout(legend_2_layout);
    legend_1_layout->setSpacing(2);
    Default_legend = new QPushButton(tr("Default"), parent);
    Apply_legend = new QPushButton(tr("Apply"), parent);
    Apply_legend->setDisabled(true);
    Default_legend->setStyleSheet("height: 28px; width: 100px;");
    Apply_legend->setStyleSheet("height: 28px; width: 100px;");
    /*const QSize BUTTON_SIZE = QSize(150, 35);
    Default_legend->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    Apply_legend->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    Default_legend->setFixedSize(BUTTON_SIZE);
    Apply_legend->setFixedSize(BUTTON_SIZE);
    */
    legend_2_layout->addWidget(Default_legend, 0, Qt::AlignRight);
    legend_2_layout->addWidget(Apply_legend, 0, Qt::AlignRight);
    legend_2_layout->addStretch(1);

    connect(Default_legend, SIGNAL(clicked(bool)), this, SLOT(Default_Colors()));
    connect(Apply_legend, SIGNAL(clicked(bool)), this, SLOT(Apply_Colors()));

    i = 0;
    foreach(color, Color_Cluster)
    {
        legend = new Legend(i, parent);
        List_Legend.append(legend);
        if(color.isValid())
        {
            qss = QString("background-color: %1; border: 1px solid black;").arg(color.name());
            legend->color_legend->setStyleSheet(qss + "QPushButton::pressed {background-color: #fff;}");
        }

        connect(legend, SIGNAL(sChange_color(int)), this, SLOT(Check_ChangeColor(int)));

        legend_1_layout->addWidget(legend);
        i++;
    }
    legend_1_layout->addStretch(1);


    Tab_Control->addTab(Scroll_Box, tr("Control"));
    Tab_Control->addTab(Select_Grid, tr("Select"));
    Tab_Control->addTab(Scroll_Legend, tr("Color legend"));

    graph_spl->addWidget(Tab_Graph);
    graph_spl->addWidget(Graph_Claster);
    graph_spl->setSizes(list);
    control_spl->addWidget(Control_Group);
    control_spl->addWidget(Tab_Result);

    //... Actions ...
    copy_to_clipboard = new QAction(QIcon(":/images/copy_clipboard.png"),tr("copy to ClipBoard"), this);
    copy_to_excelfile = new QAction(QIcon(":/images/copy_excel.png"),tr("copy to Excel"), this);
    connect(copy_to_clipboard, SIGNAL(triggered(bool)), this, SLOT(to_ClipBoard()));
    connect(copy_to_excelfile, SIGNAL(triggered(bool)), this, SLOT(to_Excel()));

    Border_Quality = 75;
    Cp_Validity = 0;

    return(MainBox);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_HRM::Destroy()
{
    delete this;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_HRM::Destroy_Win()
{

    if(MainBox != NULL)
    {
        qDeleteAll(List_Legend);
        List_Legend.clear();

        qDeleteAll(Curve_Results.values());
        Curve_Results.clear();

        qDeleteAll(HRM_GROUPs.values());
        HRM_GROUPs.clear();

        qDeleteAll(HRM_HISTORY.values());
        HRM_HISTORY.clear();

        Clear_Clusters();
        Clear_ClusterMass();

        HRM_Reference.clear();

        delete Hrm_RawData;
        delete Hrm_dFdT;
        delete Hrm_RawData_Border;
        delete Hrm_Norm;
        delete Hrm_DY_Norm;
        delete Hrm_Diff;
        delete Cluster_Plot;

        X_Temp.clear();
        X_Norm.clear();
        qDeleteAll(Y_Norm);
        Y_Norm.clear();
        qDeleteAll(DY_Norm);
        DY_Norm.clear();
        qDeleteAll(Diff_Norm);
        Diff_Norm.clear();
        qDeleteAll(DY_Cluster);
        DY_Cluster.clear();
        qDeleteAll(DYmass_Cluster);
        DYmass_Cluster.clear();

        delete MainBox;
        MainBox = NULL;
    }
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_HRM::Show()
{
    MainBox->show();
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_HRM::GetInformation(QVector<QString> *info)
{
    info->append("0x0010");
    info->append(tr("HRM"));
}
//-----------------------------------------------------------------------------
//--- readCommonSettings()
//-----------------------------------------------------------------------------
void Analyser_HRM::readCommonSettings()
{
    QString text;
    QString dir_path = qApp->applicationDirPath();
    QSettings *CommonSettings = new QSettings(dir_path + "/tools/ini/preference.ini", QSettings::IniFormat);

    CommonSettings->beginGroup("Common");

        //... Language ...
        text = CommonSettings->value("language","ru").toString();

        if(translator.load(":/translations/analyser_hrm_" + text + ".qm"))
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
void Analyser_HRM::Analyser(rt_Protocol *prot)
{
    rt_GroupSamples *group;
    rt_Sample       *sample;
    rt_Tube         *tube;
    rt_Preference   *property;
    bool find;
    QString name_Copies;
    QString sample_ID;
    QString text;
    QVector<int> *hrm_group;
    QMap<QString, QString> map_copies;
    CURVE_RESULTS   *curve_results;
    HISTORY         *history;

    QString fluor_name[COUNT_CH] = FLUOR_NAME;
    QIcon icon;
    int first_ch = -1;
    int i;
    int col,raw;
    prot->Plate.PlateSize(prot->count_Tubes, raw, col);

    temperature_Correction.fill(0., prot->count_Tubes);  // Temperature Correction

    Prot = prot;
    qDeleteAll(HRM_GROUPs.values());
    HRM_GROUPs.clear();
    qDeleteAll(Curve_Results.values());
    Curve_Results.clear();

    //---
    Fluors->blockSignals(true);
    for(i=0; i<COUNT_CH; i++)
    {
        Fluors->addItem(fluor_name[i]);
        switch(i)
        {
        case 0:  icon.addFile(":/images/fam_flat.png");   break;
        case 1:  icon.addFile(":/images/hex_flat.png");   break;
        case 2:  icon.addFile(":/images/rox_flat.png");   break;
        case 3:  icon.addFile(":/images/cy5_flat.png");   break;
        case 4:  icon.addFile(":/images/cy55_flat.png");   break;
        default: icon.addFile(NULL);   break;
        }
        Fluors->setItemIcon(i,icon);
        if(!(Prot->active_Channels & (0x0f<<i*4)))
        {
           Fluors->setItemData(i,0,Qt::UserRole - 1);
        }
        if(first_ch < 0 && (prot->active_Channels & (0x0f<<i*4)))
        {
            first_ch = i;
        }
    }
    Fluors->setCurrentIndex(first_ch);
    act_Channel = Fluors->currentIndex();
    Fluors->blockSignals(false);
    //---

    foreach(group, Prot->Plate.groups)
    {
        foreach(sample, group->samples)
        {
            sample_ID = QString::fromStdString(sample->ID_Sample);
            if(sample->p_Test->header.Type_analysis == 0x0010)   // HRM research
            {
                // find K-
                find = false;
                foreach(property, sample->preference_Sample)
                {
                    if(property->name == "kind" && property->value == "ControlNegative") {find = true; break;}
                    //qDebug() << "property: " << QString::fromStdString(property->name) << QString::fromStdString(property->value);
                }
                if(find) continue;

                text = QString::fromStdString(sample->p_Test->header.Name_Test);
                if(HRM_GROUPs.keys().contains(text))
                {
                    hrm_group = HRM_GROUPs.value(text);
                }
                else
                {
                    hrm_group = new QVector<int>;
                    HRM_GROUPs.insert(text, hrm_group);
                }
                tube = sample->tubes.at(0);
                hrm_group->append(tube->pos);

                HRM_Reference.insert(tube->pos, 0);

                /*find = false;
                tube = sample->tubes.at(0);
                foreach(property, sample->preference_Sample)
                {
                    if(property->name == "copies")
                    {
                        name_Copies = QString::fromStdString(property->value);
                        if(HRM_GROUPs.keys().contains(name_Copies)) hrm_group = HRM_GROUPs.value(name_Copies);
                        else
                        {
                            hrm_group = new QVector<int>;
                            HRM_GROUPs.insert(name_Copies, hrm_group);
                        }

                        hrm_group->append(tube->pos);
                        find = true;

                        if(name_Copies == sample_ID)
                        {
                            text = QString("%1,...").arg(Convert_IndexToName(tube->pos, col));
                            map_copies.insert(sample_ID, text);
                        }
                        break;
                    }
                }
                if(!find)
                {
                    name_Copies = "unknown";
                    if(HRM_GROUPs.keys().contains(name_Copies)) hrm_group = HRM_GROUPs.value(name_Copies);
                    else
                    {
                        hrm_group = new QVector<int>;
                        HRM_GROUPs.insert(name_Copies, hrm_group);
                    }

                    hrm_group->append(tube->pos);
                }*/

                curve_results = new CURVE_RESULTS();
                curve_results->Identificator = QString::fromStdString(sample->Unique_NameSample);
                curve_results->enable = true;
                curve_results->pos = tube->pos;
                curve_results->sample = sample;
                Curve_Results.insert(tube->pos, curve_results);                
            }
        }
    }

    /*foreach(text, map_copies.keys())
    {
        hrm_group = HRM_GROUPs.value(text);
        HRM_GROUPs.insert(map_copies.value(text), hrm_group);
        HRM_GROUPs.remove(text);
    }*/

    foreach(text, HRM_GROUPs.keys())
    {
        history = new HISTORY();
        HRM_HISTORY.insert(text, history);
    }

    //qDebug() << "GROUPs: " << HRM_GROUPs;
    //foreach(hrm_group, HRM_GROUPs.values()) {qDebug() << "HRM_GROUPs: " << *hrm_group;}

    Groups->blockSignals(true);
    Groups->clear();

    foreach(hrm_group, HRM_GROUPs.values())
    {
        text = HRM_GROUPs.key(hrm_group);
        Groups->addItem(text);
        Groups->setCurrentText(text);
        Analysis_Group(hrm_group);
    }
    Groups->blockSignals(false);

    slot_ChangeGroup();
    if(Groups->currentIndex() != 0 && Groups->count() > 1)
    {
        Groups->setCurrentIndex(0);
    }

    //map_copies.clear();
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_HRM::Analysis_Group(QVector<int> *hrm_group)
{
    int i,j,k,m;
    int id,pos;
    double dx, dx_min, dx_max;
    QString text,str;
    QString name_res, value_res;
    double dvalue, dval;
    QStringList list;
    bool use_Ct;
    bool ok;
    QwtPlotCurve *curve, *curve_diff, *curve_cluster;
    double *x;
    double *y;
    int count_meas;
    int count_tubes;
    //int act_ch = 0;
    QColor color;
    QVector<double> max_Border;
    QVector<double> min_Border;
    double min_Temp, max_Temp, Tpeak, Fpeak;
    QVector<double> vec, *Y_vec;
    QVector<QPointF> *vec_P;
    CURVE_RESULTS   *curve_results;
    QVector<QPointF> Points;

    QPointF P;
    double value, mean_value, stdev;
    int id_border = -1;
    int count_meas_border;
    int id_rep = 1;


    while(1)
 {
    // 1. ... Group Samples ...
    HRM_Group.clear();
    HRM_Group = *hrm_group;
    hrm = hrm_group;

    //qDebug() << "HRM_Group_0: " << HRM_Group.size() << HRM_Group;


    for(i=HRM_Group.size()-1; i>=0; i--)
    {
        id = HRM_Group.at(i);
        curve_results = Curve_Results.value(id);
        if(!curve_results->enable) HRM_Group.remove(i);
    }


    //qDebug() << "HRM_Group: " << HRM_Group.size() << HRM_Group;



    // 2. ... Raw_Data ...
    count_meas = Prot->count_MC;
    count_tubes = Prot->count_Tubes;
    x = &Prot->X_MC.at(0);
    y = &Prot->MC_Filtered.at(count_meas*count_tubes*act_Channel);
    color = QColor(0,0,0);

    Hrm_RawData->Delete_Curves();
    if(curve_BorderLeft) delete curve_BorderLeft;
    if(curve_BorderRight) delete curve_BorderRight;
    foreach(i, HRM_Group)
    {
        curve_results = Curve_Results.value(i);     // only enable
        if(!curve_results->enable) continue;        //

        curve = new QwtPlotCurve();
        curve->setStyle(QwtPlotCurve::Lines);
        curve->setCurveAttribute(QwtPlotCurve::Fitted, true);
        curve->setPen(color, 1);
        curve->setSamples(x, y+count_meas*i, count_meas);
        curve->attach(Hrm_RawData);
        Hrm_RawData->list_Curves.append(curve);
    }
    curve_BorderLeft = new QwtPlotCurve();
    color = Qt::black; // QColor(0,255,0, 150);
    //color.setAlpha(150);
    curve_BorderLeft->setPen(color,1);
    curve_BorderRight = new QwtPlotCurve();
    color = Qt::black; // QColor(255, 0,0, 150);
    //color.setAlpha(150);
    curve_BorderRight->setPen(color,1);
    curve_BorderLeft->attach(Hrm_RawData);
    curve_BorderRight->attach(Hrm_RawData);
    curve_BorderLeft->setSamples(NULL);
    curve_BorderRight->setSamples(NULL);

    Hrm_RawData->replot();
    for(i=0; i<count_meas; i++) vec.append(x[i]);

    // 3. ... MinMax Temperatue Borders ...
    max_Border.clear();
    min_Border.clear();

    min_Temp = max_Temp = 0.;
    foreach(i, HRM_Group)
    {
        curve_results = Curve_Results.value(i);     // only enable
        if(!curve_results->enable) continue;        //

        //qDebug() << "mc_peaks: " << i << prot->MC_TPeaks.at(i*2) << prot->MC_TPeaks_Y.at(i*2);
        Tpeak = Prot->MC_TPeaks.at(i*2+act_Channel*count_tubes*2);
        Fpeak = Prot->MC_TPeaks_Y.at(i*2+act_Channel*count_tubes*2);

        //... temperature correction ...
        if(Use_TemperatureCorrect->isChecked())
        {
            if(temperature_Correction.size() == count_tubes)
            {
                pos = curve_results->pos;
                dval = temperature_Correction.at(0) - temperature_Correction.at(pos);
                Tpeak -= dval;
            }
        }
        //...

        curve_results = Curve_Results.value(i);
        curve_results->Tpeaks = Tpeak;

        if(Tpeak <= 0. || Fpeak <= 0.) continue;
        max_Border.append(Tpeak + 2.);
        min_Border.append(Tpeak - 2.);
    }


    if(Auto_Temperature->isChecked())
    {
        min_Temp = (double)*std::min_element(min_Border.begin(), min_Border.end());
        max_Temp = (double)*std::max_element(max_Border.begin(), max_Border.end());

        min_Temp = roundTo(min_Temp,1);
        max_Temp = roundTo(max_Temp,1);

        Left_Border->blockSignals(true);
        Right_Border->blockSignals(true);
        Left_Border->setValue(min_Temp);
        Right_Border->setValue(max_Temp);
        //Left_Border->setRange(min_Temp - 2., min_Temp + 2.);
        //Right_Border->setRange(max_Temp - 2., max_Temp + 2.);
        Left_Border->blockSignals(false);
        Right_Border->blockSignals(false);
    }
    else
    {
        min_Temp = Left_Border->value();
        max_Temp = Right_Border->value();
    }

    //qDebug() << "Min,Max: " << min_Temp << max_Temp;

    // 3.1 Draw min/max borders ...
    Points.clear();
    P.setX(min_Temp);
    P.setY(Hrm_RawData->axisInterval(QwtPlot::yLeft).minValue());
    Points.append(P);
    P.setY(Hrm_RawData->axisInterval(QwtPlot::yLeft).maxValue());
    Points.append(P);
    curve_BorderLeft->setSamples(Points);


    Points.clear();
    P.setX(max_Temp);
    Points.append(P);
    P.setY(Hrm_RawData->axisInterval(QwtPlot::yLeft).minValue());
    Points.append(P);
    curve_BorderRight->setSamples(Points);
    Hrm_RawData->replot();
    Points.clear();

    // 2.1 ... dF/dT chart ...
    x = &Prot->X_MC.at(0);
    y = &Prot->MC_dF_dT.at(count_meas*count_tubes*act_Channel);
    color = QColor(0,0,0);

    Hrm_dFdT->Delete_Curves();
    foreach(i, HRM_Group)
    {
        curve_results = Curve_Results.value(i);     // only enable
        if(!curve_results->enable) continue;        //

        curve = new QwtPlotCurve();
        curve->setStyle(QwtPlotCurve::Lines);
        curve->setCurveAttribute(QwtPlotCurve::Fitted, true);
        curve->setPen(color, 1);
        curve->setSamples(x, y+count_meas*i, count_meas);
        curve->attach(Hrm_dFdT);
        Hrm_dFdT->list_Curves.append(curve);
    }
    Hrm_dFdT->replot();


    // 4. ... Raw_Data + Temperature_Borders ...
    X_Temp.clear();
    y = &Prot->MC_Filtered.at(count_meas*count_tubes*act_Channel);
    //min_Temp = 73.;
    //max_Temp = 77.;
    if(min_Temp > 0. && max_Temp > 0.)
    {        
        for(i=0; i<vec.size(); i++)
        {
            value = vec.at(i);

            dx_min = fabs(value - min_Temp);
            dx_max = fabs(value - max_Temp);
            //qDebug() << "x_temp: " << i << value << min_Temp << max_Temp << dx_min << dx_max;
            if(value >= (min_Temp - 0.001) && value <= (max_Temp + 0.001))
            {
                X_Temp.append(value);
                if(id_border < 0) id_border = i;
            }
        }
         //qDebug() << "X_Temp: " << id_border << X_Temp.size() << X_Temp ;

         count_meas_border = X_Temp.size();

         Curve_NULL.clear();
         Curve_NULL.fill(0, count_meas_border);

         Hrm_RawData_Border->Delete_Curves();
         foreach(i, HRM_Group)
         {
             curve_results = Curve_Results.value(i);     // only enable
             if(!curve_results->enable) continue;        //

             curve = new QwtPlotCurve();
             curve->setStyle(QwtPlotCurve::Lines);
             curve->setCurveAttribute(QwtPlotCurve::Fitted, true);
             curve->setPen(color, 1);
             curve->setSamples(x+id_border, y+count_meas*i+id_border, count_meas_border);
             curve->attach(Hrm_RawData_Border);
             Hrm_RawData_Border->list_Curves.append(curve);
         }
         Hrm_RawData_Border->setAxisAutoScale(QwtPlot::xBottom, false);
         Hrm_RawData_Border->setAxisScale(QwtPlot::xBottom, X_Temp.first(), X_Temp.last());

         Hrm_RawData_Border->replot();
    }
    else break;

    // 5. ... Norm ...
    vec.clear();
    X_Norm.clear();
    qDeleteAll(Y_Norm);
    Y_Norm.clear();


    m = X_Temp.size();
    dx = 1./(m-1);
    for(i=0; i<m; i++)
    {
        if(Use_TemperatureNorm->isChecked()) X_Norm.append(X_Temp.at(i));
        else X_Norm.append(i*dx);
    }

    //qDebug() << "X_Temp: " << X_Temp.size() << X_Temp;
    //qDebug() << "X_Norm: " << X_Norm.size() << X_Norm;


    foreach(curve, Hrm_RawData_Border->list_Curves)
    {
        Y_vec = new QVector<double>;
        vec.clear();

        for(i=0; i<count_meas_border; i++)
        {
            vec.append(curve->sample(i).y() - curve->minYValue());
        }
        min_Temp = *std::min_element(vec.begin(),vec.end());
        max_Temp = *std::max_element(vec.begin(), vec.end());
        max_Temp = fabs(max_Temp - min_Temp);
        if(max_Temp == 0.) max_Temp = 1;
        foreach(value, vec)
        {
            Y_vec->append(value/max_Temp);
        }
        Y_Norm.append(Y_vec);
    }

    // Temperature correction
    if(Use_TemperatureCorrect->isChecked())
    {
        dvalue = fabs(X_Temp.last() - X_Temp.first());
        if(Use_TemperatureNorm->isChecked()) dvalue = 1.;
        Temperature_Correction(&HRM_Group, &X_Norm, &Y_Norm, &temperature_Correction, dvalue);
    }
    //...

    id = 0;
    Hrm_Norm->Delete_Curves();
    foreach(i, HRM_Group)
    {
        curve_results = Curve_Results.value(i);     // only enable
        if(!curve_results->enable) continue;        //

        Y_vec = Y_Norm.at(id);
        curve = new QwtPlotCurve();
        curve->setStyle(QwtPlotCurve::Lines);
        curve->setCurveAttribute(QwtPlotCurve::Fitted, true);
        curve->setPen(color, 1);
        curve->setSamples(X_Norm, *Y_vec);
        curve->attach(Hrm_Norm);
        Hrm_Norm->list_Curves.append(curve);
        id++;
    }
    if(Use_TemperatureNorm->isChecked())
    {
        Hrm_Norm->setAxisTitle(QwtPlot::xBottom, tr("Temperature,°C"));
        Hrm_Norm->setAxisAutoScale(QwtPlot::xBottom, false);
        Hrm_Norm->setAxisScale(QwtPlot::xBottom, X_Norm.first(), X_Norm.last());
    }
    else
    {
        Hrm_Norm->setAxisTitle(QwtPlot::xBottom, tr(" "));
        Hrm_Norm->setAxisAutoScale(QwtPlot::xBottom, true);
    }

    Hrm_Norm->replot();

    // 6. ... DY/DT Norm ...
    vec.clear();
    qDeleteAll(DY_Norm);
    DY_Norm.clear();

    foreach(i, HRM_Group)
    {
        curve_results = Curve_Results.value(i);     // only enable
        if(!curve_results->enable) continue;        //

        Y_vec = new QVector<double>;
        vec = QVector<double>::fromStdVector(Prot->MC_dF_dT).mid(count_meas*count_tubes*act_Channel + id_border + i*count_meas, count_meas_border);
        min_Temp = *std::min_element(vec.begin(),vec.end());
        id = 0;
        foreach(value, vec)
        {
            vec.replace(id, value - min_Temp);
            id++;
        }
        min_Temp = *std::min_element(vec.begin(),vec.end());
        max_Temp = *std::max_element(vec.begin(), vec.end());
        max_Temp = fabs(max_Temp - min_Temp);
        if(max_Temp == 0.) max_Temp = 1;
        foreach(value, vec)
        {
            Y_vec->append(value/max_Temp);
        }
        DY_Norm.append(Y_vec);
    }

    // Temperature correction
    if(Use_TemperatureCorrect->isChecked())
    {
        //dvalue = fabs(X_Temp.last() - X_Temp.first());
        Temperature_Correction(&HRM_Group, &X_Norm, &DY_Norm, &temperature_Correction, dvalue);
    }
    //...

    id = 0;
    Hrm_DY_Norm->Delete_Curves();
    foreach(i, HRM_Group)
    {
        curve_results = Curve_Results.value(i);     // only enable
        if(!curve_results->enable) continue;        //

        Y_vec = DY_Norm.at(id);
        curve = new QwtPlotCurve();
        curve->setStyle(QwtPlotCurve::Lines);
        curve->setCurveAttribute(QwtPlotCurve::Fitted, true);
        curve->setPen(color, 1);
        curve->setSamples(X_Norm, *Y_vec);
        curve->attach(Hrm_DY_Norm);
        Hrm_DY_Norm->list_Curves.append(curve);
        id++;
    }
    if(Use_TemperatureNorm->isChecked())
    {
        Hrm_DY_Norm->setAxisTitle(QwtPlot::xBottom, tr("Temperature,°C"));
        Hrm_DY_Norm->setAxisAutoScale(QwtPlot::xBottom, false);
        Hrm_DY_Norm->setAxisScale(QwtPlot::xBottom, X_Norm.first(), X_Norm.last());
    }
    else
    {
        Hrm_DY_Norm->setAxisTitle(QwtPlot::xBottom, tr(" "));
        Hrm_DY_Norm->setAxisAutoScale(QwtPlot::xBottom, true);
    }
    Hrm_DY_Norm->replot();
    //Tab_Graph->setCurrentWidget(Hrm_DY_Norm);

    // 7. ... Difference ...
    vec.clear();
    qDeleteAll(Diff_Norm);
    Diff_Norm.clear();


    QMap<double,int> map_mean, map_mean_1;
    double val;
    id = 0;
    // find reper curve
    foreach(curve, Hrm_Norm->list_Curves)
    {
        vec.append(curve->sample(count_meas_border/2).y());
        map_mean.insert(vec.at(id), id);

        //qDebug() << "y,x: " << id << curve->sample(count_meas_border/2).y() << curve->sample(count_meas_border/2).x();

        id++;
    }
    value = (map_mean.keys().last() + map_mean.keys().first())/2.;
    foreach(val, map_mean.keys())
    {
        id = map_mean.value(val);
        map_mean_1.insert(fabs(value - val), id);
    }
    val = map_mean_1.keys().at(0);
    id_rep = map_mean_1.value(val);

    //id = vec.size()/2;
    //value = map_mean.keys().at(id);
    //id_rep = map_mean.value(value);

    //id_rep = std::min_element(vec.begin(),vec.end()) - vec.begin();
    //qDebug() << "id_rep: " << id_rep << map_mean;
    map_mean.clear();
    map_mean_1.clear();

    QwtPlotCurve *curve_rep = Hrm_Norm->list_Curves.at(id_rep);
    Hrm_Diff->Delete_Curves();
    foreach(curve, Hrm_Norm->list_Curves)
    {
        curve_diff = new QwtPlotCurve();
        curve_diff->setStyle(QwtPlotCurve::Lines);
        curve_diff->setCurveAttribute(QwtPlotCurve::Fitted, true);
        curve_diff->setPen(color, 1);
        curve_diff->attach(Hrm_Diff);
        Hrm_Diff->list_Curves.append(curve_diff);

        Y_vec = new QVector<double>;
        for(i=0; i<count_meas_border; i++)
        {
            Y_vec->append(curve->sample(i).y() - curve_rep->sample(i).y());
        }
        Diff_Norm.append(Y_vec);

        curve_diff->setSamples(X_Norm, *Y_vec);
    }
    if(Use_TemperatureNorm->isChecked())
    {
        Hrm_Diff->setAxisTitle(QwtPlot::xBottom, tr("Temperature,°C"));
        Hrm_Diff->setAxisAutoScale(QwtPlot::xBottom, false);
        Hrm_Diff->setAxisScale(QwtPlot::xBottom, X_Norm.first(), X_Norm.last());
    }
    else
    {
        Hrm_Diff->setAxisTitle(QwtPlot::xBottom, tr(" "));
        Hrm_Diff->setAxisAutoScale(QwtPlot::xBottom, true);
    }
    Hrm_Diff->replot();


    // 8. ... Cluster Plot ...
    vec.clear();
    qDeleteAll(DY_Cluster);
    DY_Cluster.clear();
    qDeleteAll(DYmass_Cluster);
    DYmass_Cluster.clear();

    Cluster_Plot->Delete_Curves();

    id = 0;
    //qDebug() << "size points: " << Hrm_DY_Norm->list_Curves.size();
    foreach(curve, Hrm_DY_Norm->list_Curves)
    //foreach(curve, Hrm_Diff->list_Curves)
    {
        vec.clear();
        for(i=0; i<count_meas_border; i++)
        {
            vec.append(curve->sample(i).y());
        }
        mean_value = Find_MeanValue(vec);   //gsl_stats_mean(vec.data(), 1, vec.size());
        stdev = Find_StDeviation(vec);      //gsl_stats_sd(vec.data(), 1, vec.size());
        mean_value = Distance(&vec, &Curve_NULL);
        //mean_value = Accumulate(&vec);

        //stdev *= stdev;
        //mean_value *= mean_value;

        //stdev = gsl_stats_sd_m(vec.data(), 1, vec.size(), 0.);

        vec_P = new QVector<QPointF>;
        P.setX(mean_value);
        P.setY(stdev);
        vec_P->append(P);
        DY_Cluster.append(vec_P);
        //qDebug() << "DY_Cluster: " << P << vec;

        //if(id < Color_Cluster.size()) color = Color_Cluster.at(id);
        //else color = Qt::black;
        color = Qt::black;

        curve_cluster = new QwtPlotCurve();
        curve_cluster->setStyle(QwtPlotCurve::Dots);
        curve_cluster->setSymbol(new QwtSymbol(QwtSymbol::Ellipse,
                                               QBrush(color),
                                               QPen(color, 1),
                                               QSize(4,4)));
        curve_cluster->setSamples(*vec_P);
        curve_cluster->attach(Cluster_Plot);
        Cluster_Plot->list_Curves.append(curve_cluster);

        id++;
    }
    Cluster_Plot->setAxisAutoScale(QwtPlot::yLeft, true);
    Cluster_Plot->setAxisAutoScale(QwtPlot::xBottom, true);
    Cluster_Plot->replot();

    // 9. ... Create Cluster ...
    Clear_Clusters();
    Create_Cluster();

    // 10. ... Analysis ...

    if(Reference_clustering->isChecked())
    {
        Cluster_ReferenceAnalysis(List_CLUSTERs);
    }
    else
    {
        Cluster_Analysis(List_CLUSTERs);
        FinishAuto_Analysis();
    }

    Draw_ClusterMass(List_CLUSTERs);
    //Quality_Clustering(List_CLUSTERs, Curve_Results);
    Fill_SampleResults();
    Fill_GroupResults();
    //Select_Grid->viewport()->update();

    Cluster_Plot->setAxisAutoScale(QwtPlot::yLeft, false);
    Cluster_Plot->setAxisAutoScale(QwtPlot::xBottom, false);

    break;  //
 }

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_HRM::slot_OpenFileCorrection()
{
    QString dirName = qApp->applicationDirPath();
    QString selectedFilter;
    int pos;
    QString text;

    QString fileName = QFileDialog::getOpenFileName(NULL, tr("Open File Correction"),
                                                    dirName,
                                                    tr("File correction *.tcr"),
                                                    &selectedFilter);

    if(fileName.isEmpty()) return;

    QFileInfo fi(fileName);

    if(fi.exists())
    {
        pos = fi.filePath().indexOf(":/");
        text = fi.filePath();
        text.resize(pos+2);
        text +=  ".../" + fi.fileName();
        Label_FileCorrect->setText(text);
        Label_FileCorrect->setVisible(true);
        Surface_3D->setVisible(true);
        Use_TemperatureCorrect->setDisabled(false);
        file_Correction = fileName;

    }
    else
    {
        Label_FileCorrect->setVisible(false);
        Surface_3D->setVisible(false);
        Use_TemperatureCorrect->setDisabled(true);
        file_Correction = "";
    }

}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_HRM::slot_ChangeSelectGrid()
{
    int pos;
    QTableWidgetItem* item;
    CURVE_RESULTS *curve_results;
    int row, col;
    QVector<short> enable;

    foreach(pos, *hrm)
    {
        if(Curve_Results.keys().contains(pos))
        {
            curve_results = Curve_Results.value(pos);

            row = div(pos,Select_Grid->columnCount()).quot;
            col = div(pos,Select_Grid->columnCount()).rem;
            item = Select_Grid->item(row,col);
            if(item->text().trimmed() == "1") curve_results->enable = true;
            else
            {
                curve_results->enable = false;
            }
            enable.append(curve_results->enable);
        }
    }

    //qDebug() << "slot_ChangeSelectGrid(): " << enable.size() << enable;
    enable.clear();

    slot_reAnalysis();
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_HRM::slot_ChangeGroup()
{
    int i;
    QString str;
    QString text = Groups->currentText();
    QVector<int> *hrm_group = HRM_GROUPs.value(text);
    QVector<int> invalid_Tubes;
    int rejime;
    int count = 0;
    int pos, id;
    int id_act;
    int value;
    double dvalue;
    bool ok;
    rt_Test *ptest;
    rt_Preference *property;
    POINT_TAKEOFF *PointsOff;


    Border_Quality = 75;
    Cp_Validity = 0;
    foreach(ptest, Prot->tests)
    {
        //qDebug() << "test: " << QString::fromStdString(ptest->header.Name_Test);
        if(text == QString::fromStdString(ptest->header.Name_Test))
        {
            foreach(property, ptest->preference_Test)
            {
                if(property->name == "BORDER_QUALITY")
                {
                    str = QString::fromStdString(property->value);
                    value = str.toInt(&ok);
                    if(ok) Border_Quality = value;
                    continue;
                }
                if(property->name == "CP_VALIDITY")
                {
                    str = QString::fromStdString(property->value);
                    dvalue = str.toDouble(&ok);
                    if(ok) Cp_Validity = dvalue;
                    continue;
                }
            }
            break;
        }
    }

    // Cp Validity
    if(Cp_Validity > 0)
    {
        foreach(pos, *hrm_group)
        {
            PointsOff = Prot->list_PointsOff.at(pos);
            dvalue = roundTo(PointsOff->cp_Sigmoid, 1);
            //qDebug() <<"Cp: " << pos << dvalue << PointsOff->cp_Sigmoid << Cp_Validity;
            if(dvalue >= Cp_Validity || dvalue <= 0.)
            {
                invalid_Tubes.append(pos);
            }
        }
        foreach(pos, invalid_Tubes)
        {
            id = hrm_group->indexOf(pos);
            hrm_group->remove(id);
        }
    }

    // if curve has invalid peak
    invalid_Tubes.clear();
    act_Channel;

    foreach(pos, *hrm_group)
    {
        dvalue = Prot->MC_TPeaks.at(pos*2 + act_Channel*Prot->count_Tubes*2);
        if(dvalue <= 0.) invalid_Tubes.append(pos);
    }
    foreach(pos, invalid_Tubes)
    {
        id = hrm_group->indexOf(pos);
        hrm_group->remove(id);
    }

    //...

    foreach(str, HRM_HISTORY.keys())
    {
        //qDebug() << "History: " << str << HRM_HISTORY.value(str)->Param_Clustering << HRM_HISTORY.value(str)->depth_clustering <<
        //                                                                              HRM_HISTORY.value(str)->count_clustering;
    }

    current_History = HRM_HISTORY.value(text);

    if(current_History)
    {
        // Group_Temperature
        Group_Temperature->blockSignals(true);
        Left_Border->blockSignals(true);
        Right_Border->blockSignals(true);

        rejime = current_History->Temperature_Bordering;
        switch(rejime)
        {
        case 0:     // auto
                Auto_Temperature->setChecked(true);
                break;

        default:
        case 1: Manual_Temperature->setChecked(true);
                Left_Border->setValue(current_History->left_border);
                Right_Border->setValue(current_History->right_border);
                break;
        }
        Left_Border->setEnabled((bool)rejime);
        Right_Border->setEnabled((bool)rejime);

        Right_Border->blockSignals(false);
        Left_Border->blockSignals(false);
        Group_Temperature->blockSignals(false);

        // Clustreing
        Group_clustering->blockSignals(true);
        Depth_Clusters->blockSignals(true);
        Count_Clusters->blockSignals(true);
        Reference_clustering->blockSignals(true);

        rejime = current_History->Param_Clustering;
        Depth_Clusters->setEnabled(false);
        Count_Clusters->setEnabled(false);
        switch(rejime)
        {
        case 0:         // auto
                Auto_clustering->setChecked(true);
                break;

        case 1:         // depth
                Depth_clustering->setChecked(true);
                Depth_Clusters->setValue(current_History->depth_clustering);
                Count_Clusters->setValue(current_History->count_clustering);
                Depth_Clusters->setEnabled(true);
                break;

        case 2:         // count
                Count_clustering->setChecked(true);
                Depth_Clusters->setValue(current_History->depth_clustering);
                Count_Clusters->setValue(current_History->count_clustering);
                Count_Clusters->setEnabled(true);
                break;

        case 3:         // reference
                Reference_clustering->setChecked(true);
                break;
        }

        Count_Clusters->blockSignals(false);
        Depth_Clusters->blockSignals(false);
        Group_clustering->blockSignals(false);
        Reference_clustering->blockSignals(false);


        foreach (pos, *hrm_group)
        {
            if(HRM_Reference.value(pos)) count++;
        }
        Reference_clustering->setText(QString("%1 (%2)").arg(tr("Reference clustering")).arg(count));
        if(count) Reference_clustering->setDisabled(false);
        else Reference_clustering->setDisabled(true);
    }

    Load_SelectGrid(hrm_group);

    Analysis_Group(hrm_group);

    str = QString("%1:  %2").arg(tr("Select")).arg(text);
    Tab_Control->setTabText(1, str);
    str = QString("%1:  %2").arg(tr("Samples")).arg(text);
    Tab_Result->setTabText(0, str);
    str = QString("%1:  %2").arg(tr("Clusters")).arg(text);
    Tab_Result->setTabText(1, str);

}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_HRM::slot_reAnalysis()
{
    //qDebug() << "slot_reAnalysis(): ";

    current_History->left_border = Left_Border->value();
    current_History->right_border = Right_Border->value();

    QString text = Groups->currentText();
    QVector<int> *hrm_group = HRM_GROUPs.value(text);

    Analysis_Group(hrm_group);

}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_HRM::slot_ChangeTemperatureBorders(int rejime)
{
    bool state = true;
    if(rejime == 0) state = false;

    QString text = Groups->currentText();
    QVector<int> *hrm_group = HRM_GROUPs.value(text);

    Left_Border->setDisabled(!state);
    Right_Border->setDisabled(!state);

    Analysis_Group(hrm_group);

    current_History->Temperature_Bordering = rejime;
    current_History->left_border = Left_Border->value();
    current_History->right_border = Right_Border->value();
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_HRM::slot_ChangeRejimeClustering(int rejime)
{
    bool state = true;
    if(rejime == 0 || rejime == 3) state = false;

    Count_Clusters->setDisabled(true);
    Depth_Clusters->setDisabled(true);

    switch(rejime)
    {
    case 0: break;

    case 1: Depth_Clusters->setDisabled(false);
            break;

    case 2: Count_Clusters->setDisabled(false);
            break;

    case 3: break;

    default:
            break;
    }

    slot_ChangeParamCluster();

    current_History->Param_Clustering = rejime;
    current_History->depth_clustering = Depth_Clusters->value();
    current_History->count_clustering = Count_Clusters->value();
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_HRM::slot_SampleResults_clicked(int row, int col)
{
    int id, ID, i;
    int size;
    QColor color;
    QwtSymbol* symbol;
    QwtPlotCurve    *curve;
    QList<HrmPlot*> LIST;
    HrmPlot *Plot;
    CURVE_RESULTS *curve_result;


    LIST << Hrm_RawData << Hrm_dFdT << Hrm_RawData_Border << Hrm_Norm << Hrm_DY_Norm << Hrm_Diff;

    // 1. ... Cluster_Plot ...
    //qDebug() << "HRM_Group: " << HRM_Group << HRM_Group.size();
    id = 0;
    ID = 0;
    foreach(i, HRM_Group)
    {
        ID++;
        curve_result = Curve_Results.value(i);
        if(!curve_result->enable) continue;

        curve = Cluster_Plot->list_Curves.at(id);
        const QwtSymbol* sym = curve->symbol();
        symbol = const_cast<QwtSymbol*>(sym);
        if((ID-1) == row) {size = 8; color = Qt::black;}
        else {size = 4; color = Qt::black;}
        symbol->setBrush(color);
        symbol->setPen(color,1);
        symbol->setSize(size, size);
        id++;
    }
    Cluster_Plot->replot();

    // 2.
    foreach(Plot, LIST)
    {
        id = 0;
        ID = 0;
        foreach(i, HRM_Group)
        {
            ID++;
            curve_result = Curve_Results.value(i);
            if(!curve_result->enable) continue;

            curve = Plot->list_Curves.at(id);
            color = curve->pen().color();
            if((ID-1) == row) size = 3;
            else size = 1;
            curve->setPen(color, size);
            id++;
        }
        Plot->replot();
    }
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_HRM::slot_ChangeParamCluster()
{    
    Clear_Clusters();
    Create_Cluster();

    current_History->depth_clustering = Depth_Clusters->value();
    current_History->count_clustering = Count_Clusters->value();

    // 10. ... Analysis ...

    if(Reference_clustering->isChecked())           // only reference clustering
    {
        Cluster_ReferenceAnalysis(List_CLUSTERs);
    }
    else
    {
        Cluster_Analysis(List_CLUSTERs);
        FinishAuto_Analysis();
    }

    Draw_ClusterMass(List_CLUSTERs);
    //Quality_Clustering(List_CLUSTERs, Curve_Results);
    Fill_SampleResults();
    Fill_GroupResults();
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_HRM::Clear_ClusterMass()
{
    foreach(QwtPlotShapeItem *item, Shape_Cluster)
    {
        item->detach();
        delete item;
    }
    Shape_Cluster.clear();
    //Cluster_Plot->replot();
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_HRM::Fill_GroupResults()
{
    int i,j,k,id;
    QString text;
    QTableWidgetItem *item;
    int col,row;
    int ID;
    QStringList header;
    bool ok;
    double value;
    CLUSTER *cluster;
    CURVE_RESULTS *curve_results;
    int val;
    QList<int> list;

    Prot->Plate.PlateSize(Prot->count_Tubes, row, col);

    Groups_Results->clearContents();
    Groups_Results->clearSpans();
    Groups_Results->setRowCount(List_CLUSTERs.size());
    Groups_Results->setWordWrap(true);
    //Groups_Results->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);


    for(i=0; i<Groups_Results->rowCount(); i++)
    {
        cluster = List_CLUSTERs.at(i);

        for(j=0; j<Groups_Results->columnCount(); j++)
        {
            item = new QTableWidgetItem();
            //item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            Groups_Results->setItem(i,j,item);

            text = "";
            switch(j)
            {
            case 0:         // Identificator
                    text = QString("%1_%2").arg(tr("Cluster")).arg(i+1);
                    break;
            case 1:         // Genotype
                    text = "...";
                    break;
            case 2:         // color
                    val = cluster->color.rgb() & 0xffffff;
                    text = QString("%1").arg(val);
                    //qDebug() << "color: " << curve_results->color.rgb();
                    break;

            case 3:         // Samples
                    list.clear();
                    foreach(id, cluster->curves)
                    {
                        list.append(id);
                    }
                    qSort(list);
                    k = 0;
                    foreach(id, list)
                    {
                        if(!text.isEmpty()) text += ",";
                        else text += "   ";
                        if(div(k,10).rem == 0 && k) text += "\r\n   ";
                        text += Convert_IndexToName(id, col);
                        k++;
                    }

                    break;

            default:
                    text = "";
                    break;

            }
            item->setText(text);
        }
        Groups_Results->resizeRowToContents(i);
    }
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_HRM::Fill_SampleResults()
{
    int i,j,id;
    QString text;
    QTableWidgetItem *item;
    int col,row;
    int ID;
    QStringList header;
    bool ok;
    short state;
    double value;
    CLUSTER *cluster;
    CURVE_RESULTS *curve_results;
    int val;
    QString hrm_Results;

    Prot->Plate.PlateSize(Prot->count_Tubes, row, col);

    //... clear all results of samples ...
    rt_GroupSamples *group;
    rt_Sample       *sample;

    foreach(group, Prot->Plate.groups)
    {
        foreach(sample, group->samples)
        {
            sample->result_Sample.clear();
        }
    }
    //...

    Sample_Results->clearContents();
    Sample_Results->clearSpans();
    Sample_Results->setRowCount(HRM_Group.size());

    for(i=0; i<Sample_Results->rowCount(); i++)
    {
        hrm_Results.clear();

        ID = HRM_Group.at(i);
        curve_results = Curve_Results.value(ID);
        header.append(Convert_IndexToName(ID, col));
        hrm_Results += Convert_IndexToName(ID, col) + "\t";

        if(!curve_results->enable)
        {
            curve_results->Group = -1;
            curve_results->color = Qt::white;
            curve_results->Percent_Clustering = 0;
        }

        for(j=0; j<Sample_Results->columnCount(); j++)
        {
            item = new QTableWidgetItem();
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            Sample_Results->setItem(i,j,item);

            text = "";
            switch(j)
            {
            case 0:         // Identificator
                    text = curve_results->Identificator;                    
                    //if(HRM_Reference.value(ID,0)) text += " (ref.)";                    
                    break;
            case 1:         // Temperature Peaks
                    value = curve_results->Tpeaks;
                    if(value <= 0) text = "-";
                    else text = QString("%1").arg(value,0,'f',2);
                    break;
            case 2:         // Groups
                    val = curve_results->color.rgb() & 0xffffff;
                    text = QString("%1 %2").arg(curve_results->Group+1).arg(val);
                    //qDebug() << "color: " << curve_results->color.rgb();
                    break;

            case 3:         // Percent
                    value = curve_results->Percent_Clustering;
                    value *= 100.;
                    if(value < 0) value = 0.;
                    text = QString("%1").arg(value, 0, 'f', 0);
                    break;

            default:
                    text = "";
                    break;

            }
            item->setText(text);
            hrm_Results += text + "\t";
        }
        hrm_Results = QString("HRM_Results=%1").arg(hrm_Results);
        AddResult(curve_results->sample->result_Sample, hrm_Results, "HRM_Results=");
        qDebug() << "HRM_Results: " << hrm_Results;
    }    
    Sample_Results->setVerticalHeaderLabels(header);
    Sample_Results->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);


}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_HRM::Draw_ClusterMass(QVector<CLUSTER *> &list)
{
    QwtPlotCurve *curve_cluster;
    double mean_value, stdev;
    QVector<QPointF> *vec_P;
    QVector<double> *vec;
    QPointF P;
    QVector<double> X,Y;
    double Rx,Ry;
    QSizeF size;
    double Dx,Dy;
    int id = 0;
    int pos;
    QColor color;
    CURVE_RESULTS *curve_results;

    QVector<QPointF> vec_curve;

    Clear_ClusterMass();

    qDeleteAll(DYmass_Cluster);
    DYmass_Cluster.clear();

    //...
    /*
    foreach(curve_cluster, Cluster_Plot->list_Curves)
    {
        P = curve_cluster->sample(0);
        P.setX(P.x()*1.02);
        P.setY(P.y()*1.02);
        vec_curve.append(P);
        curve_cluster->setSamples(vec_curve);

        break;
    }
    Cluster_Plot->replot();
    */

    //...


    Dy = Cluster_Plot->axisInterval(QwtPlot::yLeft).maxValue() - Cluster_Plot->axisInterval(QwtPlot::yLeft).minValue();
    Dx = Cluster_Plot->axisInterval(QwtPlot::xBottom).maxValue() - Cluster_Plot->axisInterval(QwtPlot::xBottom).minValue();

    foreach(CLUSTER* cluster, list)
    {
        mean_value = Find_MeanValue(cluster->XY_mass);
        stdev = Find_StDeviation(cluster->XY_mass);
        mean_value = Distance(&cluster->XY_mass, &Curve_NULL);
        //mean_value = Accumulate(&cluster->XY_mass);
        //stdev *= stdev;
        //mean_value *= mean_value;
        //stdev = gsl_stats_sd_m(cluster->XY_mass.data(), 1, cluster->XY_mass.size(), 0.);
        //qDebug() << "XY_mass: " << mean_value << stdev << cluster->XY_mass;

        P.setX(mean_value);
        P.setY(stdev);
        vec_P = new QVector<QPointF>;
        vec_P->append(P);
        DYmass_Cluster.append(vec_P);

        //qDebug() << "DY_ClusterMASS: " << P << cluster->curves;

        X.clear();
        Y.clear();
        foreach(vec, cluster->XY)
        {
            mean_value = Find_MeanValue(*vec);
            stdev = Find_StDeviation(*vec);
            mean_value = Distance(vec, &Curve_NULL);
            //mean_value = Accumulate(vec);
            //stdev *= stdev;
            //mean_value *= mean_value;
            //stdev = gsl_stats_sd_m(vec->data(), 1, vec->size(), 0.);
            //qDebug() << ">>>: " << mean_value << stdev;

            X.append(fabs(P.x() - mean_value));
            Y.append(fabs(P.y() - stdev));
        }
        Rx = *std::max_element(X.begin(), X.end());
        Ry = *std::max_element(Y.begin(), Y.end());
        //qDebug() << "Rx,Ry: " << Rx << Ry << Dx << Dy;
        //if(Rx < Dx/100.) Rx = Dx/100.;
        //if(Ry < Dy/100.) Ry = Dy/100.;

        size.setWidth(Rx*2.5);
        size.setHeight(Ry*2.5);

        if(size.width() < Dx/20 && size.width() > 0) size.setWidth(Dx/6);
        if(size.height() < Dy/20 && size.height() > 0) size.setHeight(Dy/6);

        if(size.width() == 0) size.setWidth(Dx/20);
        if(size.height() == 0) size.setHeight(Dy/20);

        //if(Rx*4 > Dx) size.setWidth(Rx*3);
        //if(Ry*4 > Dy) size.setHeight(Ry*3);

        //qDebug() << "P, size: " << P << size;

        if(id <= Color_Cluster.size()) color = Color_Cluster.at(id);
        else color = Qt::darkGray;
        QColor fillColor = color;
        fillColor.setAlpha(100);

        QwtPlotShapeItem *item = new QwtPlotShapeItem();
        item->setShape(ShapeFactory::path(ShapeFactory::Ellipse, P, size));
        QPen pen(fillColor, 1);
        item->setPen(pen);
        item->setBrush(fillColor);
        item->attach(Cluster_Plot);

        Shape_Cluster.append(item);

        cluster->color = color;

        //qDebug() << "Cluster: " << id << color << cluster->curves.size() << cluster->curves;
        foreach(pos, cluster->curves)
        {
            if(Curve_Results.keys().contains(pos))
            {
                curve_results = Curve_Results.value(pos);
                curve_results->color = color;
                curve_results->Group = id;
                //qDebug() << "pos: " << pos << curve_results->color;
            }
        }

        ChangeColor_Curve(Hrm_Diff, cluster->curves, color);
        ChangeColor_Curve(Hrm_DY_Norm, cluster->curves, color);
        ChangeColor_Curve(Hrm_Norm, cluster->curves, color);
        ChangeColor_Curve(Hrm_RawData_Border, cluster->curves, color);
        ChangeColor_Curve(Hrm_dFdT, cluster->curves, color);
        ChangeColor_Curve(Hrm_RawData, cluster->curves, color);

        id++;
    }

    Cluster_Plot->replot();
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_HRM::ChangeColor_Curve(HrmPlot *plot, QVector<int> vec, QColor color)
{
    int i,id;

    i=0;
    foreach(QwtPlotCurve *curve, plot->list_Curves)
    {
        id = HRM_Group.at(i);
        if(vec.contains(id))
        {
            curve->setPen(color,1);
        }
        i++;
    }
    plot->replot();
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_HRM::Clear_Clusters()
{
    foreach(CLUSTER* cluster, List_CLUSTERs)
    {
        cluster->curves.clear();
        cluster->XY.clear();
        cluster->XY_mass.clear();
        delete cluster;
    }
    List_CLUSTERs.clear();
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_HRM::Create_Cluster()
{
    int id = 0;
    QVector<double> *Y_vec;

    foreach(Y_vec, DY_Norm) //Diff_Norm)
    {
        CLUSTER* cluster = new CLUSTER();

        cluster->XY.append(Y_vec);
        cluster->XY_mass = *Y_vec;
        cluster->curves.append(HRM_Group.at(id));
        List_CLUSTERs.append(cluster);
        id++;
    }
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_HRM::FinishAuto_Analysis()
{
    int i;
    int key, key_before, key_after;
    double value;
    int num;
    QVector<double> vec;

    if(Group_clustering->checkedId() != 0) return;
    if(Silhouette_Quality.values().size() == 0) return;

    foreach(value, Silhouette_Quality.values())
    {
        vec.append(value);
    }

    value = *std::max_element(vec.begin(), vec.end());
    num = Silhouette_Quality.key(value, 0);
    //qDebug() << "Silhouette_Quality: " << num << value << Silhouette_Quality << vec;

    vec.clear();
    for(i=1; i<InnerDistance_Quality.keys().size()-1; i++)
    {
        key = InnerDistance_Quality.keys().at(i);
        key_before = InnerDistance_Quality.keys().at(i-1);
        key_after = InnerDistance_Quality.keys().at(i+1);
        value = fabs(InnerDistance_Quality.value(key) - InnerDistance_Quality.value(key_after))/
                fabs(InnerDistance_Quality.value(key) - InnerDistance_Quality.value(key_before));
        vec.append(value);
    }

    //qDebug() << "InnerDistance_Quality: " << vec << InnerDistance_Quality;

    vec.clear();
    Count_Clusters->blockSignals(true);
    Count_Clusters->setValue(num);
    Count_Clusters->blockSignals(false);

    Clear_Clusters();
    Create_Cluster();
    Cluster_Analysis(List_CLUSTERs, num);

}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_HRM::Cluster_Analysis(QVector<CLUSTER*> &list, int num)
{
    int i;
    QPoint P_max, P_min;
    QPointF P;
    double Min_Dist, Max_Dist, min_Dist, max_Dist;
    double DIST;
    CLUSTER *cluster, *child_cluster;
    QVector<QVector<double>*> Distance;
    int id_curve, id;
    int count;
    double value;
    int val;
    bool stop;
    QVector<double> *vec;

    //qDebug() << "LIST_Clasters: " << list.size() << list;
    Silhouette_Quality.clear();
    InnerDistance_Quality.clear();

    // 0. Count_Clusters
    Count_Clusters->setRange(1, list.size());


    // 1. Calculate Min/Max
    Distance_Matrix(list, &Distance, &P_max, &P_min);
    min_Dist = Distance.at(P_min.x())->at(P_min.y());
    max_Dist = Distance.at(P_max.x())->at(P_max.y());
    Min_Dist = min_Dist;
    Max_Dist = max_Dist;

    /*if(first_cycle)
    {
        Depth_Clusters->blockSignals(true);
        //val = (int)((Min_Dist*5. - Min_Dist)/(Max_Dist - Min_Dist)*100.);
        val = 0; // 20%
        Depth_Clusters->setValue(val);
        Depth_Clusters->blockSignals(false);
        first_cycle = false;
    }*/

    DIST = Depth_Clusters->value()/100.*(Max_Dist - Min_Dist) + Min_Dist;
    //qDebug() << "min_max: " << Min_Dist << Max_Dist << DIST << Depth_Clusters->value();

    // 2. Integrate Clusters
    while(1)
    {
        stop = false;
        switch(Group_clustering->checkedId())
        {
        case 0:     // auto
                if(list.size() <= 1 || list.size() <= num) stop = true;
                break;

        case 1:     // Depth clustreing
                Count_Clusters->blockSignals(true);
                Count_Clusters->setValue(list.size());
                Count_Clusters->blockSignals(false);
                if(Min_Dist >= DIST || list.size() <= 1) stop = true;
                break;

        case 2:     // Count clustering
                Depth_Clusters->blockSignals(true);
                Depth_Clusters->setValue((Min_Dist - min_Dist)/(max_Dist - min_Dist) * 100.);
                if(list.size() == 1) Depth_Clusters->setValue(100);
                //qDebug() << "new min_max: " << Min_Dist << Max_Dist << Depth_Clusters->value();
                Depth_Clusters->blockSignals(false);
                if(Count_Clusters->value() >= list.size() || list.size() <= 1) stop = true;
                break;
        }

        if(stop) break;


        cluster = list.at(P_min.x());        
        child_cluster = list.at(P_min.y() + P_min.x() + 1);


        // a. add to cluster::curves , cluster::XY
        foreach(id_curve, child_cluster->curves)
        {
            cluster->curves.append(id_curve);
        }
        foreach(vec, child_cluster->XY)
        {
            cluster->XY.append(vec);
        }
        count = vec->size();

        // b. calculate new centre of mass cluster::XY_mass
        cluster->XY_mass.clear();

        for(i=0; i<count; i++)
        {
            value = 0;
            id = 0;
            foreach(vec, cluster->XY)
            {
                value += vec->at(i);
                id++;
            }
            value /= id;
            cluster->XY_mass.append(value);
        }

        // c. delete child_cluster
        list.remove(P_min.y() + P_min.x() + 1);
        delete child_cluster;

        // d. calculate distance again
        qDeleteAll(Distance);
        Distance.clear();
        Distance_Matrix(list, &Distance, &P_max, &P_min);
        Min_Dist = Distance.at(P_min.x())->at(P_min.y());
        Max_Dist = Distance.at(P_max.x())->at(P_max.y());

        val = Quality_Clustering(list, Curve_Results);
        Silhouette_Quality.insert(list.size(), val);        
        InnerDistance_Clustering(list, InnerDistance_Quality);
        //qDebug() << "Quality_Clustering: " << val << list.size();
    }

    //qDebug() << "Clustering: " << Silhouette_Quality << InnerDistance_Quality;

    qDeleteAll(Distance);
    Distance.clear();
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_HRM::Cluster_ReferenceAnalysis(QVector<CLUSTER*> &list)
{
    int i;
    int key, id;
    int count = 0;
    bool find;
    QVector<int> vec_ref, vec;
    CLUSTER *cluster_ref, *cluster;
    QVector<CLUSTER*> list_ref;
    QVector<double> vec_dist;
    QVector<double> *vec_temp;
    double value;

    Silhouette_Quality.clear();
    InnerDistance_Quality.clear();


    //... count reference ...
    foreach(key, HRM_Group)
    {
        if(HRM_Reference.value(key,0))
        {
            vec_ref.append(key);
            count++;
        }
        else vec.append(key);
    }
    //qDebug() << "Cluster_ReferenceAnalysis: count " << count << list.size();


    if(!count) {return;}

    //... only reference cluster ...
    foreach(key, vec_ref)
    {
        foreach(cluster_ref, list)
        {
            if(cluster_ref->curves.at(0) == key) list_ref.append(cluster_ref);
        }
    }

    foreach(key, vec)
    {
        foreach(cluster, list)
        {
            find = false;
            if(cluster->curves.at(0) == key)
            {
                find = true;
                break;
            }
        }

        if(!find) {qDebug() << "not find cluster! ";  return;}

        //... cycle for cluster_ref ...
        vec_dist.clear();
        foreach(cluster_ref, list_ref)
        {
            vec_dist.append(Distance(&cluster_ref->XY_mass, &cluster->XY_mass));
        }
        id = std::min_element(vec_dist.begin(),vec_dist.end()) - vec_dist.begin();
        cluster_ref = list_ref.at(id);

        cluster_ref->curves.append(key);
        cluster_ref->XY.append(cluster->XY.at(0));

        // b. calculate new centre of mass cluster_ref::XY_mass
        /*count = cluster_ref->XY_mass.size();
        cluster_ref->XY_mass.clear();

        for(i=0; i<count; i++)
        {
            value = 0;
            id = 0;
            foreach(vec_temp, cluster_ref->XY)
            {
                value += vec_temp->at(i);
                id++;
            }
            value /= id;
            cluster_ref->XY_mass.append(value);
        }*/

        //... remove ...
        id = list.indexOf(cluster);
        list.remove(id);
        delete cluster;
    }

    // calculate new centre of mass cluster_ref::XY_mass
    foreach(cluster, list)
    {
        count = cluster->XY_mass.size();
        for(i=0; i<count; i++)
        {
            value = 0;
            id = 0;
            foreach(vec_temp, cluster->XY)
            {
                value += vec_temp->at(i);
                id++;
            }
            value /= id;
            cluster->XY_mass.replace(i, value);
        }
    }


    i = Quality_Clustering(list, Curve_Results);
    Silhouette_Quality.insert(list.size(), i);
    InnerDistance_Clustering(list, InnerDistance_Quality);


    list_ref.clear();
    vec.clear();
    vec_ref.clear();
    vec_dist.clear();
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_HRM::InnerDistance_Clustering(QVector<CLUSTER *> &list, QMap<int, double> &map)
{
    int i,j,k, id;
    CLUSTER *cluster;
    double value = 0;
    double val;
    double mean_value, stdev;
    double mean, std;

    foreach(cluster, list)
    {
        mean_value = Find_MeanValue(cluster->XY_mass);
        stdev = Find_StDeviation(cluster->XY_mass);
        mean_value = Distance(&cluster->XY_mass, &Curve_NULL);
        //mean_value = Accumulate(&cluster->XY_mass);

        for(i=0; i<cluster->curves.size(); i++)
        {
            mean = Find_MeanValue(*cluster->XY.at(i));
            std = Find_StDeviation(*cluster->XY.at(i));
            mean = Distance(cluster->XY.at(i), &Curve_NULL);
            //mean = Accumulate(cluster->XY.at(i));

            val = sqrt(pow(mean-mean_value, 2) + pow(std-stdev,2));
            value += val;
        }
    }
    map.insert(list.size(), value);
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
int Analyser_HRM::Quality_Clustering(QVector<CLUSTER *> &list, QMap<int, CURVE_RESULTS *> &map)
{
    int i,j,k, id;
    CLUSTER         *cluster, *cluster_temp;
    CURVE_RESULTS   *curve_results;
    double a,b, value;
    QVector<double> vec, VEC;
    QVector<double> *vec1, *vec2;
    QVector<double> Silhouette_clusters;
    QVector<double> temp_vec;
    QColor color;

    j = 0;
    foreach(cluster, list)
    {
        if(j <= Color_Cluster.size()) color = Color_Cluster.at(j);
        else color = Qt::darkGray;


        vec.clear();
        foreach(cluster_temp, list)
        {
            if(cluster_temp == cluster) continue;
            vec.append(Distance(&cluster->XY_mass, &cluster_temp->XY_mass));
        }
        if(vec.size()) b = *std::min_element(vec.begin(), vec.end());
        else b = 1;

        temp_vec.clear();
        for(i=0; i<cluster->curves.size(); i++)
        {
            id = cluster->curves.at(i);
            a = Distance(cluster->XY.at(i), &cluster->XY_mass);
            if(vec.size()) value = (b - a)/qMax(b,a);
            else value = 1;
            curve_results = map.value(id);
            curve_results->Percent_Clustering = value;
            curve_results->Group = j;
            curve_results->color = color; //cluster->color;

            //temp_vec.append(value);
        }

        //value = Find_MeanValue(temp_vec);
        //Silhouette_clusters.append(value);
        //qDebug() << "color: " << j << cluster->color;
        j++;
    }
    vec.clear();
    VEC.clear();
    //... Summ Validity of Clustering ...
    /*for(i=0; i<list.size(); i++)
    {
        cluster = list.at(i);
        for(j=0; j<cluster->curves.size(); j++)
        {
            for(k=j+1; k<cluster->curves.size(); k++)
            {
                vec1 = cluster->XY.at(j);
                vec2 = cluster->XY.at(k);
                vec.append(Distance(vec1, vec2));
                //vec.append(Distance_DM(vec1, vec2));
            }
        }

        for(j=i+1; j<list.size(); j++)
        {
            cluster_temp = list.at(j);
            VEC.append(Distance(&cluster->XY_mass, &cluster_temp->XY_mass));
            //VEC.append(Distance_DM(&cluster->XY_mass, &cluster_temp->XY_mass));
        }
     }*/
    foreach(cluster, list)
    {
        for(id=0; id<cluster->curves.size(); id++)
        {
            for(k=0; k<cluster->curves.size(); k++)
            {
                if(id == k) continue;
                vec.append(Distance(cluster->XY.at(k), cluster->XY.at(id)));
            }
        }
        if(cluster->curves.size() == 1) // for "simple" cluster - with single curve
        {
            temp_vec.clear();
            foreach(cluster_temp, list)
            {
                if(cluster == cluster_temp) continue;
                temp_vec.append(Distance(cluster->XY.at(0), &cluster_temp->XY_mass));
            }
            value = *std::min_element(temp_vec.begin(), temp_vec.end());
            vec.append(value);
        }


        /*if(cluster->curves.size() > 1)
        {
            for(i=0; i<cluster->curves.size(); i++)
            {
                vec.append(Distance(cluster->XY.at(i), &cluster->XY_mass));
            }
        }*/


        foreach(cluster_temp, list)
        {
            if(cluster == cluster_temp) continue;
            VEC.append(Distance(&cluster_temp->XY_mass, &cluster->XY_mass));
        }
    }

    //qDebug() << "Quality_Clustering: " << list.size() << vec << VEC;

    //a = Find_MeanValue(vec);
    //b = Find_MeanValue(VEC);
    if(vec.size()) a = Find_MeanValue(vec); //a = *std::max_element(vec.begin(), vec.end());
    else a = 0;
    if(VEC.size()) b = *std::min_element(VEC.begin(), VEC.end());
    else b = 0;


    value = (b - a)/qMax(b,a);

    //...
    //value = Find_MeanValue(Silhouette_clusters);
    //...

    k = (value + 1)/2. * 100;
    if(k < 0) k = 0;
    //k = value * 100;

    //if(k >= 100) k = 0;
    //qDebug() << "Validity: " << k; // << vec << VEC;
    if(k > 0 && k < 100) Label_ResultClustering->setText(QString(" (%1%)").arg(k));
    else Label_ResultClustering->setText("");
    Label_CountClusters->setText(QString("K = %1").arg(list.size()));


    //qDebug() << "Map_size: " << map.size();

    vec.clear();
    VEC.clear();

    return(k);

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
double Analyser_HRM::Distance_DM(QVector<double> *A, QVector<double> *B)
{
    double mean_A, stdev_A;
    double mean_B, stdev_B;
    double value;

    mean_A = Find_MeanValue(*A);
    mean_B = Find_MeanValue(*B);
    stdev_A = Find_StDeviation(*A);
    stdev_B = Find_StDeviation(*B);

    value = sqrt(pow(mean_A - mean_B, 2.) + pow(stdev_A - stdev_B, 2.));

    return(value);
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
double Analyser_HRM::Distance(QVector<double> *A, QVector<double> *B)
{
    int i;
    double value , summ = 0.;

    if(A->size() != B->size()) return(0.);

    for(i=0; i<A->size(); i++)
    {
        value = pow(A->at(i) - B->at(i), 2);
        summ += value;
    }
    value = summ;
    value = sqrt(value);


    return(value);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
double Analyser_HRM::Accumulate(QVector<double> *A)
{
    double value, summ = 0.0;
    int coef;
    //std::accumulate(A->begin(), A->end(), 0.0);
    foreach(value, *A)
    {
        coef = 1;
        if(value < 0) coef = -1;
        summ += pow(value,2) * coef;
    }

    return(summ);
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_HRM::Distance_Matrix(QVector<CLUSTER *> &list, QVector<QVector<double>*> *matrix, QPoint *P_max, QPoint *P_min)
{
    int i,j,k;
    int count;
    double value, sum, max_val, min_val;
    double max_Value = 0;
    double min_Value = 100;
    int id_min, id_max;
    CLUSTER *cluster_main, *cluster;
    QVector<double> *vec;

    for(i=0; i<list.size(); i++)
    {
        vec = new QVector<double>;
        cluster_main = list.at(i);

        for(j=i+1; j<list.size(); j++)
        {
            cluster = list.at(j);

            /*
            count = cluster_main->XY_mass.size();
            sum = 0;
            for(k=0; k<count; k++)
            {
                value = pow(cluster_main->XY_mass.at(k) - cluster->XY_mass.at(k), 2);                
                sum += value;
            }
            value = sum;
            value = sqrt(value);
            */
            value = Distance(&cluster->XY_mass, &cluster_main->XY_mass);


            vec->append(value);

        }

        //qDebug() << "Dist: " << i << *vec << cluster_main->curves;
        matrix->append(vec);
        if(vec->size())
        {
            max_val = *std::max_element(vec->begin(), vec->end());
            min_val = *std::min_element(vec->begin(), vec->end());
            id_min = std::min_element(vec->begin(), vec->end()) - vec->begin();
            id_max = std::max_element(vec->begin(), vec->end()) - vec->begin();

            if(max_val > max_Value || !i) {max_Value = max_val; P_max->setX(i); P_max->setY(id_max);}
            if(min_val < min_Value || !i) {min_Value = min_val; P_min->setX(i); P_min->setY(id_min);}
        }
    }    

    //qDebug() << "MaxMin: " << *P_min << *P_max;
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_HRM::Enable_Tube(QVector<short> *e)
{

}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_HRM::Select_Tube(int pos)
{

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_HRM::Save_Results(char *fname)
{

}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_HRM::Load_SelectGrid(QVector<int> *hrm_group)
{
    int i,j,k;
    int pos;
    QStringList header;
    QString text;
    QTableWidgetItem* item;
    int col,row;
    CURVE_RESULTS *curve_results;

    //qDebug() << "hrm_group: " << *hrm_group;

    Select_Grid->clear();
    if(Prot == NULL) return;

    // 1. --- Load Select Structure ---
    Prot->Plate.PlateSize(Prot->count_Tubes, row, col);
    Delegate->get_prot(Prot);

    Select_Grid->setColumnCount(col);                           // count of column
    Select_Grid->setRowCount(row);                              // ...      row

    for(i=0; i<row; i++)
    {
        for(j=0; j<col; j++)
        {
            pos = i*col + j;            
            item = new QTableWidgetItem();
            Select_Grid->setItem(i,j,item);
            if(hrm_group->contains(pos))
            {
                text = "1";
                if(Curve_Results.keys().contains(pos))
                {
                    curve_results = Curve_Results.value(pos);
                    if(curve_results->enable) text = "1";
                    else text = "0";
                }
                item->setText(text);
            }
            else item->setText("-1");
        }
    }

    for(i=0; i<col; i++) header.append(QString::number(i+1));   // Column Header
    Select_Grid->setHorizontalHeaderLabels(header);
    for(i=0; i<col; i++) Select_Grid->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Stretch);

    header.clear();

    for(i=0; i<row; i++) header.append(QChar(0x41 + i));        // Row Header
    Select_Grid->setVerticalHeaderLabels(header);
    for(i=0; i<row; i++) Select_Grid->verticalHeader()->setSectionResizeMode(i, QHeaderView::Stretch);

    header.clear();

    // 1.1 calculate A1
    item = Select_Grid->item(0,0);
    Select_Grid->A1 = Select_Grid->visualItemRect(item);


    Select_Grid->repaint();
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_HRM::contextMenu_SelectGrid()
{
    QMenu menu;
    short state = 0;
    int col,row;
    Prot->Plate.PlateSize(Prot->count_Tubes, row, col);
    int pos = Select_Grid->currentColumn() + Select_Grid->currentRow()*col;
    if(HRM_Reference.contains(pos))
    {
        state = HRM_Reference.value(pos);
    }
    else return;

    menu.setStyleSheet("QMenu::item:selected {background-color: #d7d7ff; color: black;}");
    menu.addSeparator()->setText(tr(" reference rejime: "));
    menu.addAction(Select_Grid->reference_rejime);
    menu.addAction(Select_Grid->clear_AllReference);

    Select_Grid->reference_rejime->setChecked(state);


    menu.exec(QCursor::pos());
    //Delegate->numeration = Select_Grid->numeration_rejime->isChecked();

    menu.clear();
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_HRM::change_Reference()
{
    int col,row;
    Prot->Plate.PlateSize(Prot->count_Tubes, row, col);

    int pos = Select_Grid->currentColumn() + Select_Grid->currentRow()*col;
    short state = Select_Grid->reference_rejime->isChecked();

    HRM_Reference.insert(pos, state);    

    //qDebug() << "reference: " << HRM_Reference << HRM_Group;

    int count = 0;
    foreach (pos, HRM_Group)
    {
        if(HRM_Reference.value(pos)) count++;
    }
    Reference_clustering->setText(QString("%1 (%2)").arg(tr("Reference clustering")).arg(count));

    if(count == 0)
    {
        Reference_clustering->setDisabled(true);
        Auto_clustering->setChecked(true);
        emit Group_clustering->buttonClicked(0);
    }
    else
    {
        Reference_clustering->setDisabled(false);
        if(Reference_clustering->isChecked()) slot_ChangeParamCluster();
    }

    Select_Grid->repaint();
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_HRM::clear_ALLReferences()
{
    int key;

    foreach (key, HRM_Group)
    {
        HRM_Reference.insert(key,0);
    }

    Reference_clustering->setText(QString("%1 (%2)").arg(tr("Reference clustering")).arg(0));

    Reference_clustering->setDisabled(true);
    Auto_clustering->setChecked(true);
    emit Group_clustering->buttonClicked(0);

    Select_Grid->repaint();
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_HRM::Use_TempNormalization(bool state)
{
    slot_ChangeGroup();
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_HRM::Use_TempCorrection(bool state)
{
    int i;
    QFile file(file_Correction);
    QString text;
    QStringList list;
    double value, val;
    bool ok;

    temperature_Correction.clear();
    temperature_Correction.fill(0., Prot->count_Tubes);

    if(state)
    {
        if(!file.exists())
        {
            file_Correction = "";
            Label_FileCorrect->setText("");
            Use_TemperatureCorrect->blockSignals(true);
            Use_TemperatureCorrect->setChecked(false);
            Use_TemperatureCorrect->setDisabled(true);
            Use_TemperatureCorrect->blockSignals(false);
            return;
        }

        if(file.open(QFile::ReadOnly))
        {
            QTextStream in(&file);
            text = in.readAll();

            file.close();
        }

        text.replace(",",".");
        list = text.split("\r\n");

        //qDebug() << "Use_TempCorrection: " << list.size() << list;

        temperature_Correction.clear();
        temperature_Correction.fill(0., Prot->count_Tubes);
        val = QString(list.at(0)).toDouble(&ok);
        if(!ok) val = 0.;
        i = 0;
        foreach(text, list)
        {            
            value = text.toDouble(&ok);            
            if(!ok) value = 0.;
            value -= val;
            value = -value;
            temperature_Correction.replace(i, value);
            i++;
            if(i >= temperature_Correction.size()) break;
        }
    }
    else
    {

    }

    // Correction temperature peaks


    //qDebug() << "Use_TempCorrection: " << temperature_Correction.size() << temperature_Correction;

    slot_ChangeGroup();
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_HRM::View_3D()
{
    QProcess *vec = new QProcess(this);
    QString lang = "en";
    if(ru_Lang) lang = "ru";
    QString text = QString("surface.exe \"%1\" %2").arg(file_Correction).arg(lang);
    vec->startDetached(text);
    qDebug() << "view_3D: " << text;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_HRM::Numerate_Plate()
{
    bool num_state = Numerate_tool->isChecked();

    if(num_state) Tab_Control->setCurrentIndex(1);

    Delegate->numeration = num_state;
    Select_Grid->viewport()->update();
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_HRM::Temperature_Correction(QVector<int> *hrm_group,
                                          QVector<double> *X,
                                          QVector<QVector<double> *> *data,
                                          QVector<double> *temp_correct,
                                          double coef)
{
    int id, pos;
    double val;
    QVector<double> *vec;


    id = 0;
    foreach(pos, *hrm_group)
    {
        val = temp_correct->at(0) - temp_correct->at(pos);        // offset...

        vec = data->at(id);

        //qDebug() << "data_in " << id << val << coef << *vec;

        Interpolation_steffen(X, vec, val/coef);

        //qDebug() << "data_out: " << id << val << coef << *vec;

        id++;
    }
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_HRM::Interpolation_steffen(QVector<double> *X, QVector<double> *Y, double dx)
{
    int i;
    double *x;
    double *y;
    double x_i, value;

    const size_t N = X->size();

    x = X->data();
    y = Y->data();

    gsl_interp_accel *acc = gsl_interp_accel_alloc();
    gsl_spline *spline_cubic = gsl_spline_alloc(gsl_interp_cspline, N);
    //gsl_spline *spline_steffen = gsl_spline_alloc(gsl_interp_steffen, N);

    gsl_spline_init(spline_cubic, x, y, N);
    //gsl_spline_init(spline_steffen, x, y, N);

    for(i=0; i<Y->size(); i++)
    {
        x_i = X->at(i) + dx;
        if(x_i > X->last()) x_i = X->last();
        if(x_i < X->first()) x_i = X->first();

        value = gsl_spline_eval(spline_cubic, x_i, acc);
        Y->replace(i, value);
    }

    gsl_spline_free(spline_cubic);
    gsl_interp_accel_free(acc);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_HRM::contextMenu_SampleResults()
{
    active_Table = Sample_Results;

    QMenu menu;
    menu.setStyleSheet("QMenu::item:selected {background-color: #d7d7ff; color: black;}");
    menu.setFont(qApp->font());

    menu.addAction(copy_to_clipboard);
    menu.addAction(copy_to_excelfile);

    menu.exec(QCursor::pos());

    menu.clear();
    active_Table = NULL;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_HRM::contextMenu_GroupsResults()
{
    active_Table = Groups_Results;

    QMenu menu;
    menu.setStyleSheet("QMenu::item:selected {background-color: #d7d7ff; color: black;}");
    menu.setFont(qApp->font());

    menu.addAction(copy_to_clipboard);
    menu.addAction(copy_to_excelfile);

    menu.exec(QCursor::pos());

    menu.clear();
    active_Table = NULL;
}
//-----------------------------------------------------------------------------
//--- to_Excel()
//-----------------------------------------------------------------------------
void Analyser_HRM::to_Excel()
{
    QString filename = "";
    QByteArray ba;

    QString text = Table_ToDigits(active_Table);
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
void Analyser_HRM::to_ClipBoard()
{
    QString text = Table_ToDigits(active_Table);

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(text);
}
//-----------------------------------------------------------------------------
//--- to_ClipBoard()
//-----------------------------------------------------------------------------
QString Analyser_HRM::Table_ToDigits(QTableWidget *table)
{
    if(!table)  return("");

    int i,j;
    QTableWidgetItem *item;
    QStringList list, list_cluster;
    QString temp, pos, text, str;
    QString header_str = " ";    

    for(i=0; i<table->columnCount(); i++)
    {
        temp = table->horizontalHeaderItem(i)->text();
        header_str += "\t" + temp;
    }

    for(i=0; i<table->rowCount(); i++)
    {
        if(table == Sample_Results) pos = table->verticalHeaderItem(i)->text();
        if(table == Groups_Results) pos = QString("%1").arg(i+1);

        text = "";
        for(j=0; j<table->columnCount(); j++)
        {
            item = table->item(i,j);
            str = item->text();

            if(table == Sample_Results)
            {
                if(j == 1) str.replace(".",",");
                if(j == 2)
                {
                    list_cluster = str.split(QRegExp("\\s+"));
                    str = list_cluster.at(0);
                }
            }
            if(table == Groups_Results)
            {
                if(j == 2) str = QString("%1").arg(i+1);
                if(j == 3) {str.replace("\r\n",""); str.replace(" ","");}  // Samples
            }

            text += "\t" + str;
        }
        text = pos + text;
        list.append(text);
    }

    text = header_str + "\r\n" + list.join("\r\n");

    return(text);
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
HrmPlot::HrmPlot(bool visible_axis, bool visible_axis_title, QColor bg, QString titul, QWidget *parent):
    QwtPlot(parent)
{
    QwtText text_x, text_y;
    active_ch = 0;  // Fam

    canvas()->setCursor(Qt::ArrowCursor);
    setCanvasBackground(bg);
    //canvas()->installEventFilter(this);

    QFont f = qApp->font();
    f.setBold(false);
    f.setPointSize(f.pointSize()-2);
    setAxisFont(QwtPlot::yLeft, f);
    setAxisFont(QwtPlot::xBottom, f);
    //setAxisFont(QwtPlot::yLeft, f);
    //setAxisFont(QwtPlot::xBottom, f);

    grid = new QwtPlotGrid;
    grid->setMajorPen(QPen(QColor(100,100,100), 0, Qt::DotLine));
    grid->attach(this);

    setAxisAutoScale(QwtPlot::yLeft, true);
    setAxisAutoScale(QwtPlot::xBottom, true);

    if(!visible_axis)
    {
        enableAxis(QwtPlot::yLeft, false);
        enableAxis(QwtPlot::xBottom, false);
    }
    else
    {
        text_x.setText(tr("Temperature,°C"));
        text_y.setText(tr("Fluorescence"));
        if(titul.contains("dF/dT")) text_y.setText("dF/dT");
        if(visible_axis_title)
        {
            setAxisTitle(QwtPlot::xBottom, text_x);
            setAxisTitle(QwtPlot::yLeft, text_y);
        }

    }
    setMinimumSize(100,100);

    // titul
    label_caption = new QLabel(titul, parent);
    if(!titul.isEmpty())
    {
        QHBoxLayout *plot_H_layout = new QHBoxLayout();
        canvas()->setLayout(plot_H_layout);
        plot_H_layout->addWidget(label_caption, 1, Qt::AlignRight | Qt::AlignTop);
        f.setPointSize(f.pointSize()+4);
        f.setBold(true);
        label_caption->setFont(f);
    }

    //... Menu ...
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu_Plot()));

    //... Actions ...
    save_as_PNG = new QAction(QIcon(":/images/images.png"),tr("save as PNG_Image"), this);
    copy_to_clipboard_as_image = new QAction(QIcon(":/images/image_to_clipboard_24.png"),tr("copy to ClipBoard as Image"), this);
    connect(save_as_PNG, SIGNAL(triggered(bool)), this, SLOT(to_Image()));
    connect(copy_to_clipboard_as_image, SIGNAL(triggered(bool)), this, SLOT(to_ImageClipBoard()));

    //... plot image
    box_image = new QGroupBox(this);
    box_image->setStyleSheet("background: #FFFFFF; border: 1px solid transparent;");
    //box_image->setFixedSize(400,200);
    box_image->setVisible(false);
    box_image->lower();
    QVBoxLayout *layout_image = new QVBoxLayout();
    box_image->setLayout(layout_image);
    layout_image->setMargin(0);
    plot_image = new QwtPlot();
    grid_image = new QwtPlotGrid;
    grid_image->setMajorPen(QPen(Qt::gray, 0, Qt::DotLine));
    grid_image->attach(plot_image);
    layout_image->addWidget(plot_image);
    plot_image->setCanvasBackground(bg);

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
HrmPlot::~HrmPlot()
{
    delete grid;
    delete label_caption;

    Delete_Curves();

    qDeleteAll(listCurve_image.begin(), listCurve_image.end());
    listCurve_image.clear();
    delete grid_image;
    delete plot_image;
    delete box_image;
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void HrmPlot::Delete_Curves()
{    
    qDeleteAll(list_Curves.begin(), list_Curves.end());
    list_Curves.clear();
}

//-----------------------------------------------------------------------------
//--- contextMenu
//-----------------------------------------------------------------------------
void HrmPlot::contextMenu_Plot()
{
    QMenu menu;
    menu.setStyleSheet("QMenu::item:selected {background-color: #d7d7ff; color: black;}");
    menu.setFont(qApp->font());

    menu.addAction(copy_to_clipboard_as_image);
    menu.addAction(save_as_PNG);

    menu.exec(QCursor::pos());

    menu.clear();
}
//-----------------------------------------------------------------------------
//--- to_Image
//-----------------------------------------------------------------------------
void HrmPlot::to_Image()
{
    QString filename = "";

    filename = QFileDialog::getSaveFileName(this, tr("Save to Image..."),
                                            "image.png", tr("Images (*.png)"));
    if(!filename.isEmpty())
    {
        QPixmap pixmap(this->grab());
        pixmap.save(filename, "PNG");
    }
}
//-----------------------------------------------------------------------------
//--- to_ImageClipBoard()
//-----------------------------------------------------------------------------
void HrmPlot::to_ImageClipBoard()
{
    int i;
    QColor color;
    int count;
    QPointF P;
    QVector<QPointF> vec;
    QwtPlotCurve *curve;
    QwtPlotShapeItem *item;
    QFont font_plot = qApp->font();
    font_plot.setBold(false);

    bool cluster = false;

    if(this->title().text().contains(tr("Cluster Analysis"))) cluster = true;

    //qDebug() << "Cluster_Plot: " << cluster << this->title().text();

    QClipboard *clipboard = QApplication::clipboard();

    foreach(curve, list_Curves)
    {
        if(!curve->isVisible()) continue;

        curve_image = new QwtPlotCurve();

        if(cluster)
        {
            color = Qt::black;
            curve_image->setStyle(QwtPlotCurve::Dots);
            curve_image->setSymbol(new QwtSymbol(QwtSymbol::Ellipse,
                                                   QBrush(color),
                                                   QPen(color, 1),
                                                   QSize(4,4)));
        }
        else curve_image->setStyle(QwtPlotCurve::Lines);

        curve_image->attach(plot_image);
        listCurve_image.append(curve_image);


        curve_image->setPen(curve->pen());
        count = curve->dataSize();
        vec.clear();
        for(i=0; i<count; i++)
        {
            P = curve->sample(i);
            vec.append(P);
        }
        curve_image->setSamples(vec);
    }

    if(cluster)
    {
        foreach(item, *Shape_Cluster)
        {
           shape_item = new QwtPlotShapeItem();

           shape_item->setShape(item->shape());
           shape_item->setPen(item->pen());
           shape_item->setBrush(item->brush());

           shape_item->attach(plot_image);
           listShape_image.append(shape_item);
        }
    }

    if(cluster) plot_image->setTitle(this->title());
    else plot_image->setTitle(label_caption->text());
    plot_image->setFont(font_plot);
    plot_image->setAxisTitle(QwtPlot::xBottom, this->axisTitle(QwtPlot::xBottom));
    plot_image->setAxisTitle(QwtPlot::yLeft, this->axisTitle(QwtPlot::yLeft));
    plot_image->setAxisFont(QwtPlot::xBottom, font_plot);
    plot_image->setAxisFont(QwtPlot::yLeft, font_plot);


    box_image->setGeometry(this->rect());
    box_image->setVisible(true);
    plot_image->replot();
    replot();
    clipboard->setPixmap(box_image->grab());
    box_image->setVisible(false);
    replot();

    qDeleteAll(listCurve_image.begin(), listCurve_image.end());
    listCurve_image.clear();

    qDeleteAll(listShape_image.begin(), listShape_image.end());
    listShape_image.clear();

    //setCanvasBackground(QColor(255,255,255));
    //clipboard->setPixmap(canvas()->grab());
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
HISTORY::HISTORY(QWidget *parent)
{
    Param_Clustering = 0;       // auto
    depth_clustering = 0;
    count_clustering = 0;
    Temperature_Bordering = 0;  // auto
    left_border = 0.;
    right_border = 0.;
    Temperature_Correction = false;
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
CLUSTER::CLUSTER(QWidget *parent):
        QObject(parent)
{
    color = Qt::black;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
CLUSTER::~CLUSTER()
{    
    XY.clear();
    curves.clear();    
    XY_mass.clear();
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
CURVE_RESULTS::CURVE_RESULTS(QWidget *parent):
    QWidget(parent)
{
    Identificator = "";
    Tpeaks = 0.;
    Group = 0;
    enable = true;
    color = Qt::white;
    Percent_Clustering = 0.;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
CURVE_RESULTS::~CURVE_RESULTS()
{

}
//-----------------------------------------------------------------------------
//--- Draw Sample_Results
//-----------------------------------------------------------------------------
void Samples_ItemDelegate::paint(QPainter *painter,
                           const QStyleOptionViewItem &option,
                           const QModelIndex &index) const
{
    QRect rect;
    QString text, str;
    QFont font;
    QStringList list;
    QColor color;
    unsigned int val;
    QPixmap pixmap;
    bool ok;
    int num_group;

    QPalette pal = option.palette;
    QStyleOptionViewItem  viewOption(option);
    viewOption.state &= ~QStyle::State_HasFocus;    // disable state HasFocus
    QStyledItemDelegate::paint(painter, viewOption, index);

    rect = option.rect;

    //__1. Background
    int row = index.row();
    if(option.state & QStyle::State_Selected)
    {
        painter->fillRect(option.rect, QColor(215,215,255));
    }
    else
    {
        if(div(row,2).rem)  painter->fillRect(option.rect, QColor(255,255,255));
        else painter->fillRect(option.rect, QColor(245,245,245));
    }
    //-----------------------

    //__2. Data
    painter->setPen(QPen(Qt::black,1,Qt::SolidLine));
    font = painter->font();
    font.setBold(false);
    font.setItalic(false);
    painter->setFont(font);

    int col = index.column();
    text = index.data().toString();

    switch(col)
    {
    case 0:     // Identificator
            painter->drawText(option.rect, Qt::AlignHCenter | Qt::AlignVCenter, text);
            break;

    case 1:     // Tpeak
            painter->drawText(option.rect, Qt::AlignHCenter | Qt::AlignVCenter, text);
            break;

    case 2:     // Group
            list = text.split(" ");
            text = list.at(0);
            str = list.at(1);
            val = str.toInt();
            num_group = text.toInt(&ok);
            if(!ok) num_group = -1;
            if(num_group < 0) text = "-";
             //qDebug() << "color_paint: " << val;
            color = val;
            if(num_group >= 0)painter->fillRect(option.rect, color);
            painter->setPen(QPen(Qt::white,1,Qt::SolidLine));
            painter->drawText(option.rect, Qt::AlignHCenter | Qt::AlignVCenter, text);
            painter->setPen(QPen(Qt::black,1,Qt::SolidLine));
            break;

    case 3:     // %

            val = text.toInt(&ok);
            if(!ok) val = 0;
            if(val == 0) text = "-";
            if(val < *border_quality && val >= 0)
            {
                pixmap.load(":/images/Attention.png");
                painter->drawPixmap(rect.x() + rect.width() - 8,
                                    option.rect.y() + 1,
                                    pixmap);
            }
            if(val == 100) painter->setPen(QPen(Qt::gray,1,Qt::SolidLine));
            painter->drawText(option.rect, Qt::AlignHCenter | Qt::AlignVCenter, text);
            break;

    default:
            break;

    }
    painter->setPen(QPen(Qt::black,1,Qt::SolidLine));
}

//-----------------------------------------------------------------------------
//--- Draw Group_Results
//-----------------------------------------------------------------------------
void Groups_ItemDelegate::paint(QPainter *painter,
                           const QStyleOptionViewItem &option,
                           const QModelIndex &index) const
{
    QRect rect;
    QString text, str;
    QFont font;
    QStringList list;
    QColor color;
    unsigned int val;
    QPixmap pixmap;
    bool ok;
    int num_group;

    QPalette pal = option.palette;
    QStyleOptionViewItem  viewOption(option);
    viewOption.state &= ~QStyle::State_HasFocus;    // disable state HasFocus
    QStyledItemDelegate::paint(painter, viewOption, index);

    rect = option.rect;

    //__1. Background
    int row = index.row();
    if(option.state & QStyle::State_Selected)
    {
        painter->fillRect(option.rect, QColor(215,215,255));
    }
    else
    {
        if(div(row,2).rem)  painter->fillRect(option.rect, QColor(255,255,255));
        else painter->fillRect(option.rect, QColor(245,245,245));
    }
    //-----------------------

    //__2. Data
    painter->setPen(QPen(Qt::black,1,Qt::SolidLine));
    font = painter->font();
    font.setBold(false);
    font.setItalic(false);
    painter->setFont(font);

    int col = index.column();
    text = index.data().toString();

    switch(col)
    {
    case 0:     // Identificator
            painter->drawText(option.rect, Qt::AlignHCenter | Qt::AlignVCenter, text);
            break;

    case 1:     // Genotype
            painter->drawText(option.rect, Qt::AlignHCenter | Qt::AlignVCenter, text);
            break;

    case 2:     // color
            val = text.toInt(&ok);
            if(!ok) val = 0;
            color = val;
            painter->fillRect(option.rect, color);
            break;

    case 3:     // Samples
            painter->drawText(option.rect, Qt::AlignLeft | Qt::AlignVCenter, text);
            break;

    default:
            break;

    }
    painter->setPen(QPen(Qt::black,1,Qt::SolidLine));
}
//-----------------------------------------------------------------------------
//--- createEditor
//-----------------------------------------------------------------------------
QWidget* Groups_ItemDelegate::createEditor(QWidget *parent,
                                       const QStyleOptionViewItem &option,
                                       const QModelIndex &index) const
{
    QWidget *obj;
    QLineEdit *obj_le;
    int col = index.column();

    switch(col)
    {
    case 1:

            obj_le = new QLineEdit(parent);
            obj_le->setStyleSheet("QLineEdit {selection-background-color: rgb(215,215,255); selection-color: black;}");
            connect(obj_le, SIGNAL(editingFinished()), this, SLOT(Close_LineEditor()));
            obj = obj_le;
            break;

     default:

            obj = nullptr;
            break;

    }
    return(obj);

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Groups_ItemDelegate::Close_LineEditor()
{
    QLineEdit *editor = qobject_cast<QLineEdit *>(sender());
    if(editor)
    {
        emit commitData(editor);    // fix data: This signal must be emitted when the editor widget has completed editing the data,
                                    // and wants to write it back into the model.
        emit closeEditor(editor);
        //emit change_NameSample();
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
    reference_rejime = new QAction(tr("reference"), this);
    reference_rejime->setCheckable(true);
    clear_AllReference = new QAction(tr("clear all references"), this);

    cursor_ColorFill = QCursor(QPixmap(":/images/cursor_ColorFill.png"));

    //connect(select_rejime, SIGNAL(triggered(QAction*)), this, SLOT(change_SelectRejime(QAction*)));

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
/*void TableWidget::change_SelectRejime(QAction *select)
{
    bool state = false;

    if(select == position_select) setCursor(Qt::ArrowCursor);
    if(select == color_select) {setCursor(cursor_ColorFill); state = true;}

    emit color_Rejime(state);
}*/

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
TableWidget::~TableWidget()
{
    delete position_select;
    delete color_select;
    delete select_rejime;
    delete clear_plate;
    delete reference_rejime;
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
    QTableWidgetItem* item;
    bool zoom = false;
    QString text;
    bool reanalysis = false;

    QTableWidget::mouseReleaseEvent(event);

    if(selected && moving) zoom = true;
    selected = false;
    moving = false;
    if(event->button() == Qt::LeftButton)
    {

        coord_1.setX(currentColumn());
        coord_1.setY(currentRow());

        if(coord_0 == coord_1)      // single item
        {
            item = this->currentItem();
            text = item->text().trimmed();
            if(text == "1") {item->setText("0"); reanalysis = true;}
            if(text == "0") {item->setText("1"); reanalysis = true;}
        }

        if(zoom && coord_0 != coord_1)  // multi items
        {
            int min_row = qMin(coord_0.y(),coord_1.y());
            int min_col = qMin(coord_0.x(),coord_1.x());
            int max_row = qMax(coord_0.y(),coord_1.y());
            int max_col = qMax(coord_0.x(),coord_1.x());

            for(i=min_row; i<=max_row; i++)
            {
                for(j=min_col; j<=max_col; j++)
                {
                    item = this->item(i,j);
                    text = item->text().trimmed();
                    if(text == "1") {item->setText("0"); reanalysis = true;}
                    if(text == "0") {item->setText("1"); reanalysis = true;}
                }
            }
        }

        if(reanalysis) emit sReanalysis();
    }



    viewport()->update();
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

    if(rowCount() > 0 && columnCount() > 0)
    {
        QTableWidgetItem *item = this->item(0,0);
        A1 = this->visualItemRect(item);
    }
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void ItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QColor color;
    QString text;
    bool state_ref = false;
    short val;
    int rad;
    int x,y;
    double X,Y,R;
    double value;
    int r,g,b;
    int pos;
    QVector<QString> *vec;
    int min_value = qMin(option.rect.width(), option.rect.height());
    int min_value_Pro = qMin(A1->width(),A1->height());

    if(abs(min_value_Pro - min_value) > 3)
    {
        min_value_Pro = min_value;
        *A1 = option.rect;
    }

    QFont f = painter->font();
    QFont f_new = f;

    f_new.setPointSize(f.pointSize()+3);

    QRect rect = option.rect;
    QRect rect_new = rect;
    rect_new.setRect(rect.x()+4, rect.y()+4, rect.width()-8, rect.height()-8);

    //R = (double)min_value/2. * 0.80;
    R = (double)min_value_Pro/2.* 0.80;
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


    //__0. Position
    pos = index.column() + index.row() * index.model()->columnCount();
    if(map_reference->keys().contains(pos))
    {
        val = map_reference->value(pos);
        state_ref = (bool)val;
    }
    else state_ref = false;


    //__1. Background
    if(style == "fusion") painter->fillRect(option.rect, QColor(255,255,255));
    else painter->fillRect(option.rect, QColor(255,255,225));

    //__2. Data
    text = index.data().toString();    
    if(Curve_Results->keys().contains(pos))
    {
        color = Curve_Results->value(pos)->color;
    }
    else color = p_prot->color_tube.at(pos);
    //color = Qt::black;

    if(text.trimmed() == "1")
    {
        //color = p_prot->color_tube.at(pos);
        painter->setPen(QPen(Qt::black,1,Qt::SolidLine));
        painter->setBrush(QBrush(color, Qt::SolidPattern));
        if(state_ref) painter->drawRect(rect_new);
        else painter->drawEllipse(x,y,2*rad,2*rad);
    }
    else
    {
        if(text.trimmed() == "0")
        {
            painter->setPen(QPen(Qt::black,1,Qt::SolidLine));
            painter->setBrush(QBrush(Qt::white, Qt::SolidPattern));
            if(state_ref) painter->drawRect(rect_new);
            else painter->drawEllipse(x,y,2*rad,2*rad);
        }
        else
        {
            painter->setPen(QPen(QColor(245,245,245),1,Qt::SolidLine));
            painter->setBrush(QBrush(QColor(235,235,235), Qt::SolidPattern));
            painter->drawRect(rect);
        }
    }
    if(state_ref)
    {
        f_new.setBold(true);
        painter->setFont(f_new);
        painter->drawText(rect, Qt::AlignHCenter | Qt::AlignVCenter, "R");
        painter->setFont(f);

    }
    else
    {
        if(numeration)
        {
            painter->setFont(f_new);
            color.getRgb(&r,&g,&b);
            value = 0.3*(double)(r) + 0.59*(double)(g) + 0.11*(double)(b);
            if(value < 128) color = Qt::white;
            else color = Qt::black;
            if(text.trimmed() == "0") color = Qt::black;

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
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Legend::Legend(int id, QWidget *parent): QGroupBox(parent)
{
    index = id;
    setObjectName("Transparent");

    name_legend = new QLabel(QString("%1_%2").arg(tr("Cluster")).arg(index+1), this);
    color_legend = new QPushButton(this);
    color_legend->setFixedHeight(25);

    QHBoxLayout *layout = new QHBoxLayout;
    setLayout(layout);
    layout->setMargin(1);
    layout->setSpacing(20);

    layout->addWidget(name_legend);
    layout->addWidget(color_legend);
    layout->addStretch(1);

    connect(color_legend, SIGNAL(clicked(bool)), this, SLOT(Change_color()));

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Legend::~Legend()
{
    delete name_legend;
    delete color_legend;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Legend::Change_color()
{
    QString qss;

    QColor color = QColorDialog::getColor();
    if(color.isValid())
    {
        qss = QString("background-color: %1; border: 1px solid black;").arg(color.name());
        color_legend->setStyleSheet(qss);
        color_legend->update();
        emit sChange_color(index);
    }
}

//-----------------------------------------------------------------------------
//--- AddResult(vector<string>&, QString, QString)
//-----------------------------------------------------------------------------
void Analyser_HRM::AddResult(vector<string> &vec, QString result, QString ref)
{
    int i,id;
    QString text;
    bool replace = false;

    for(i=0; i<vec.size(); i++)
    {
        text = QString::fromStdString(vec.at(i));
        if(text.startsWith(ref))
        {
            replace = true;
            id = i;
            break;
        }
    }
    if(replace) vec.at(id) = result.toStdString();
    else vec.push_back(result.toStdString());

}
