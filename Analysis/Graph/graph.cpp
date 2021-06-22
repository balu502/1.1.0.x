#include "graph.h"


//-----------------------------------------------------------------------------
//--- GraphPlot()
//-----------------------------------------------------------------------------
GraphPlot::GraphPlot(QWidget *parent):
    QwtPlot(parent)
{    
    main_widget = parent;    

    //... Apply Common Settings ...
    QString dir_path = qApp->applicationDirPath();
    CommonSettings = new QSettings(dir_path + "/tools/ini/preference.ini", QSettings::IniFormat);
    readCommonSettings();
    setFont(qApp->font());

    canvas()->setCursor(Qt::PointingHandCursor);
    canvas()->installEventFilter(this);
    canvas()->setMouseTracking(true);
    pos_previous.setX(0);
    pos_previous.setY(0);

    grid = new QwtPlotGrid;
    grid->setMajorPen(QPen(Qt::gray, 0, Qt::DotLine));
    grid->attach(this);
    setAxisFont(QwtPlot::yLeft, qApp->font());
    setAxisFont(QwtPlot::xBottom, qApp->font());


    size_symbol = 0;
    thickness_line = 1;
    curve_edit = false;
    d_selectedCurve = NULL;
    d_selectedPoint = -1;

    label_curve = new QLabel("pp", canvas());
    label_curve->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    label_curve->setAutoFillBackground(true);
    label_curve->setAlignment(Qt::AlignCenter);
    label_curve->setFixedWidth(50);
    label_curve->setVisible(false);

    label_threshold = new QLabel("", canvas());
    label_threshold->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    //label_threshold->setFixedWidth(120);
    QFont font = label_threshold->font();
    font.setBold(false);
    //font.setPointSizeF(font.pointSize()-2);
    label_threshold->setFont(font);
    label_threshold->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    label_threshold->setAutoFillBackground(true);
    label_threshold->setVisible(false);

    label_melting = new QLabel("", canvas());
    label_melting->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    label_melting->setFont(font);
    label_melting->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    label_melting->setAutoFillBackground(true);
    label_melting->setVisible(false);

    /*label_XY = new QLabel("", canvas());
    label_XY->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    label_XY->setFont(font);
    label_XY->setFixedWidth(100);
    label_XY->setVisible(true);*/

    //... 1. Fluor
    Fluor_Box = new Fluor_ComboBox();
    Fluor_Delegate = new Fluor_ItemDelegate;
    Fluor_Box->setItemDelegate(Fluor_Delegate);
    Fluor_Box->setVisible(false);
    Fluor_Delegate->style = StyleApp;
    Fluor_Box->setMinimumWidth(100);
    Fluor_Box->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(Fluor_Box, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu_Fluor()));

    //... 1.1 Group_FluorButton
    Group_FluorButton = new QGroupBox();
    Group_FluorButton->setObjectName("Transparent");
    Group_FluorButton->setFixedSize(110,24);
    Group_FluorButton->setVisible(false);
    /*QHBoxLayout *graph_layout = new QHBoxLayout();
    graph_layout->setMargin(0);
    setLayout(graph_layout);
    graph_layout->addWidget(Group_FluorButton, 0, Qt::AlignRight | Qt::AlignTop);
    */
    QHBoxLayout *fluor_layout = new QHBoxLayout();
    Group_FluorButton->setLayout(fluor_layout);
    fluor_layout->setMargin(0);
    fluor_layout->setSpacing(2);
    fam_button = new QToolButton(this);
    hex_button = new QToolButton(this);
    rox_button = new QToolButton(this);
    cy5_button = new QToolButton(this);
    cy55_button = new QToolButton(this);

    //QPalette pp = fam_button->palette();
    //pp.setColor(QPalette::Button, QColor(255,0,0));
    //fam_button->setPalette(pp);
    fam_button->setStyleSheet("QToolButton::checked {background-color: #A5BCDE; border: 1px solid black}");
    hex_button->setStyleSheet("QToolButton::checked {background-color: #93C193; border: 1px solid black}");
    rox_button->setStyleSheet("QToolButton::checked {background-color: #FCD05F; border: 1px solid black}");
    cy5_button->setStyleSheet("QToolButton::checked {background-color: #FFC4FF; border: 1px solid black}");
    cy55_button->setStyleSheet("QToolButton::checked {background-color: #FF8080; border: 1px solid black}");

    fluor_layout->addWidget(fam_button);
    fluor_layout->addWidget(hex_button);
    fluor_layout->addWidget(rox_button);
    fluor_layout->addWidget(cy5_button);
    fluor_layout->addWidget(cy55_button);

    Fluor_Buttons = new QButtonGroup(this);
    Fluor_Buttons->setExclusive(false);
    Fluor_Buttons->addButton(fam_button,0);
    Fluor_Buttons->addButton(hex_button,1);
    Fluor_Buttons->addButton(rox_button,2);
    Fluor_Buttons->addButton(cy5_button,3);
    Fluor_Buttons->addButton(cy55_button,4);

    fam_button->setCheckable(true);
    hex_button->setCheckable(true);
    rox_button->setCheckable(true);
    cy5_button->setCheckable(true);
    cy55_button->setCheckable(true);

    connect(Fluor_Buttons, SIGNAL(buttonClicked(int)), this, SLOT(slot_FluorButton(int)));

    //... 1.1.1 Fluor_Single  GONCHAROVA !!!!!
    Group_FluorSingle = new QGroupBox();
    //Group_FluorSingle->setObjectName("Transparent");
    Group_FluorSingle->setStyleSheet("QGroupBox {border: 1px solid transparent; background: transparent;}");
    Group_FluorSingle->setFixedSize(110,24);
    //Group_FluorSingle->setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
    Group_FluorSingle->setVisible(false);

    QHBoxLayout *single_layout = new QHBoxLayout();
    Group_FluorSingle->setLayout(single_layout);
    single_layout->setMargin(0);
    single_layout->setSpacing(2);
    fam_single = new QToolButton(this);
    hex_single = new QToolButton(this);
    rox_single = new QToolButton(this);
    cy5_single = new QToolButton(this);
    cy55_single = new QToolButton(this);

    fam_single->setStyleSheet("QToolButton::checked {background-color: #A5BCDE; border: 1px solid black}"
                              "QToolButton {background-color: #A0A5BCDE; border: 1px solid #ddd}"
                              "QToolButton::disabled {background-color: transparent; border: 1px solid transparent}");
    hex_single->setStyleSheet("QToolButton::checked {background-color: #93C193; border: 1px solid black}"
                              "QToolButton {background-color: #A093C193; border: 1px solid #ddd}"
                              "QToolButton::disabled {background-color: transparent; border: 1px solid transparent}");
    rox_single->setStyleSheet("QToolButton::checked {background-color: #FCD05F; border: 1px solid black}"
                              "QToolButton {background-color: #A0FCD05F; border: 1px solid #ddd}"
                              "QToolButton::disabled {background-color: transparent; border: 1px solid transparent}");
    cy5_single->setStyleSheet("QToolButton::checked {background-color: #FFC4FF; border: 1px solid black}"
                              "QToolButton {background-color: #A0FFC4FF; border: 1px solid #ddd}"
                              "QToolButton::disabled {background-color: transparent; border: 1px solid transparent}");
    cy55_single->setStyleSheet("QToolButton::checked {background-color: #FF8080; border: 1px solid black}"
                               "QToolButton {background-color: #A0FF8080; border: 1px solid #ddd}"
                               "QToolButton::disabled {background-color: transparent; border: 1px solid transparent}");

    single_layout->addWidget(fam_single);
    single_layout->addWidget(hex_single);
    single_layout->addWidget(rox_single);
    single_layout->addWidget(cy5_single);
    single_layout->addWidget(cy55_single);

    Fluor_Single = new QButtonGroup(this);
    Fluor_Single->setExclusive(true);
    Fluor_Single->addButton(fam_single,0);
    Fluor_Single->addButton(hex_single,1);
    Fluor_Single->addButton(rox_single,2);
    Fluor_Single->addButton(cy5_single,3);
    Fluor_Single->addButton(cy55_single,4);

    fam_single->setCheckable(true);
    hex_single->setCheckable(true);
    rox_single->setCheckable(true);
    cy5_single->setCheckable(true);
    cy55_single->setCheckable(true);

    //... 1.2 Norm_1000
    Norm_1000 = new QToolButton(this);
    Norm_1000->setIcon(QIcon(":/images/Norm_1000.png"));
    Norm_1000->setCheckable(true);
    Norm_1000->setToolTip(tr("Normalization(1000) for all channels"));
    Norm_1000->setVisible(false);

    connect(Norm_1000, SIGNAL(clicked(bool)), this, SLOT(slot_Norm1000()));

    //... 2. Rejime
    RejimeData_Box = new QComboBox();
    Rejime_Delegate = new RejimeData_ItemDelegate();
    RejimeData_Box->setItemDelegate(Rejime_Delegate);
    RejimeData_Box->setVisible(false);

    //... 3. Type
    Type_Box = new QComboBox(this);
    Type_Delegate = new TypeData_ItemDelegate();
    Type_Box->setItemDelegate(Type_Delegate);
    Type_Box->addItem(tr("PCR"));
    Type_Box->addItem(tr("Melting Curve"));
    Type_Box->setVisible(false);


    //QVBoxLayout *plot_layout = new QVBoxLayout();
    //setLayout(plot_layout);
    //plot_layout->setMargin(0);
    //plot_layout->addWidget(Type_Box,0,Qt::AlignCenter);
    //plot_layout->addStretch();

    QHBoxLayout *graph_H_layout = new QHBoxLayout();
    QHBoxLayout *graph_H_layout_1 = new QHBoxLayout();
    QVBoxLayout *graph_V_layout = new QVBoxLayout();

    graph_H_layout->setMargin(0);
    graph_H_layout_1->setMargin(0);
    graph_V_layout->setMargin(2);

    graph_H_layout->addWidget(RejimeData_Box,0,Qt::AlignLeft);
    //graph_H_layout->addStretch(1);
    graph_H_layout->addWidget(Norm_1000,1,Qt::AlignRight);
    graph_H_layout->addSpacing(10);
    graph_H_layout->addWidget(Group_FluorButton,0,Qt::AlignRight);
    graph_H_layout->addWidget(Group_FluorSingle,1,Qt::AlignRight);
    graph_H_layout->addWidget(Fluor_Box,0,Qt::AlignRight);
    RejimeData_Box->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    RejimeData_Box->setVisible(false);

    graph_H_layout_1->addWidget(Type_Box,0,Qt::AlignLeft);

    canvas()->setLayout(graph_V_layout);
    graph_V_layout->addLayout(graph_H_layout,0);
    graph_V_layout->addLayout(graph_H_layout_1,0);
    graph_V_layout->addStretch();


    bg_color = canvasBackground();

    prot = NULL;
    id_wcurve = -1;
    value_MeltingMarker = 0;

    rejime_Ct = false;
    rejime_Lg = false;
    rejime_Normalization = false;
    rejime_Melting = false;

    connect(Fluor_Single, SIGNAL(buttonClicked(int)), SLOT(slot_ChangeFluor(int)));
    connect(Fluor_Box, SIGNAL(currentIndexChanged(int)), SLOT(change_Fluor(int)));
    connect(RejimeData_Box, SIGNAL(currentIndexChanged(int)), SLOT(change_RejimeData(int)));
    connect(Type_Box, SIGNAL(currentIndexChanged(int)), SLOT(change_TypeData(int)));
    connect(this, SIGNAL(rePaint_Threshold(double,bool)), this, SLOT(Repaint_Threshold(double,bool)));
    connect(this, SIGNAL(rePaint_Melting(double)), this, SLOT(Repaint_Melting(double)));
    //connect(this, SIGNAL(rePaint_XY()), this, SLOT(Repaint_XY()));

    //... Actions ...
    copy_to_clipboard = new QAction(QIcon(":/images/copy_clipboard.png"),tr("copy to ClipBoard"), this);
    copy_to_excelfile = new QAction(QIcon(":/images/copy_excel.png"),tr("copy to Excel"), this);
    save_as_PNG = new QAction(QIcon(":/images/images.png"),tr("save as PNG_Image"), this);
    copy_to_clipboard_as_image = new QAction(QIcon(":/images/image_to_clipboard_24.png"),tr("copy to ClipBoard as Image"), this);
    view_as_buttons = new QAction(tr("View as buttons"), this);
    view_as_buttons->setCheckable(true);

    connect(copy_to_clipboard, SIGNAL(triggered(bool)), this, SLOT(to_ClipBoard()));
    connect(copy_to_excelfile, SIGNAL(triggered(bool)), this, SLOT(to_Excel()));
    connect(save_as_PNG, SIGNAL(triggered(bool)), this, SLOT(to_Image()));
    connect(copy_to_clipboard_as_image, SIGNAL(triggered(bool)), this, SLOT(to_ImageClipBoard()));
    connect(view_as_buttons, SIGNAL(triggered(bool)), this, SLOT(slot_View_as_Buttons()));

    //... Menu ...
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu_Graph()));

    //
    PCR_1000 = new QVector<double>;
    MC_1000 = new QVector<double>;

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

}

//-----------------------------------------------------------------------------
//--- readSettings()
//-----------------------------------------------------------------------------
void GraphPlot::readCommonSettings()
{
    QString text;

    CommonSettings->beginGroup("Common");

        //... Language ...
        text = CommonSettings->value("language","ru").toString();
        if(translator.load(":/translations/graph_" + text + ".qm"))
        {
            qApp->installTranslator(&translator);
        }
        //... Style ...
        text = CommonSettings->value("style","xp").toString();
        StyleApp = text;

    CommonSettings->endGroup();
}
//-----------------------------------------------------------------------------
//--- Graph_ToDigits
//-----------------------------------------------------------------------------
QString GraphPlot::Graph_ToDigits()
{
    int i,j;
    int value;
    QString text;


    QwtPlotCurve *plot_curve;
    int id_fluor = Fluor_Box->currentIndex();
    int id = -1;
    int count = prot->count_Tubes;
    int size;

    for(i=0; i<=id_fluor; i++)
    {
        if(prot->active_Channels & (0x0f << 4*i)) id++;
    }

    for(i=0; i<count; i++)
    {
        plot_curve = list_Curve.at(i+id*count);
        size = plot_curve->data()->size();
        if(!text.isEmpty()) text.append("\r\n");
        for(j=0; j<size; j++)
        {
            value = plot_curve->sample(j).y();
            text += QString::number(value) + "\t";
        }
    }

    return(text);
}

//-----------------------------------------------------------------------------
//--- to_ClipBoard
//-----------------------------------------------------------------------------
void GraphPlot::to_ClipBoard()
{
    QString text = Graph_ToDigits();

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(text);

}
//-----------------------------------------------------------------------------
//--- to_Excel
//-----------------------------------------------------------------------------
void GraphPlot::to_Excel()
{
    QString filename = "";

    QString text = Graph_ToDigits();
    text.replace(QString("\t"), QString(";"));

    filename = QFileDialog::getSaveFileName(this, tr("Save to Excel(*.csv)..."),
                                            "ToExcel.csv", tr("Excel (*.csv)"));

    QFile file(filename);

    if(!filename.isEmpty())
    {
        if(file.open(QIODevice::WriteOnly))
        {
            QTextStream(&file) << text;
            file.close();
        }
    }

}
//-----------------------------------------------------------------------------
//--- to_Image
//-----------------------------------------------------------------------------
void GraphPlot::to_Image()
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
void GraphPlot::to_ImageClipBoard()
{
    int i;
    QColor color;
    int count;
    QPointF P;
    QVector<QPointF> vec;
    QFont font_plot = qApp->font();
    font_plot.setBold(false);

    QClipboard *clipboard = QApplication::clipboard();

    foreach(curve, list_Curve)
    {
        if(!curve->isVisible()) continue;

        curve_image = new QwtPlotCurve();
        curve_image->setStyle(QwtPlotCurve::Lines);
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

    plot_image->setTitle(this->title());
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

    //setCanvasBackground(QColor(255,255,255));
    //clipboard->setPixmap(canvas()->grab());
}
//-----------------------------------------------------------------------------
//--- slot_ChangeFluor
//-----------------------------------------------------------------------------
void GraphPlot::slot_ChangeFluor(int ch)
{
    //qDebug() << "slot_ChangeFluor: Fluor_Single-> " << ch;
    Fluor_Box->setCurrentIndex(ch);
}

//-----------------------------------------------------------------------------
//--- slot_View_as_Buttons()
//-----------------------------------------------------------------------------
void GraphPlot::slot_View_as_Buttons()
{
    qDebug() << "slot_View_as_Buttons(): ";
    if(prot && Fluor_Box->currentIndex() < COUNT_CH) Draw_Chart(prot, current_rejime, current_ch, current_type);
}

//-----------------------------------------------------------------------------
//--- contextMenu
//-----------------------------------------------------------------------------
void GraphPlot::contextMenu_Fluor()
{
    QMenu menu;
    menu.setStyleSheet("QMenu::item:selected {background-color: #d7d7ff; color: black;}");
    menu.setFont(qApp->font());

    menu.addAction(view_as_buttons);

    if(!prot) menu.setEnabled(false);
    menu.exec(QCursor::pos());
    menu.clear();
}

//-----------------------------------------------------------------------------
//--- contextMenu
//-----------------------------------------------------------------------------
void GraphPlot::contextMenu_Graph()
{
    QMenu menu;
    menu.setStyleSheet("QMenu::item:selected {background-color: #d7d7ff; color: black;}");
    menu.setFont(qApp->font());
    bool state = true;

    if(Fluor_Box->currentIndex() >= COUNT_CH) state = false;
    copy_to_clipboard->setEnabled(state);
    copy_to_excelfile->setEnabled(state);
    //copy_to_clipboard_as_image->setEnabled(state);

    menu.addAction(copy_to_clipboard);
    menu.addAction(copy_to_excelfile);
    menu.addSeparator();
    menu.addAction(copy_to_clipboard_as_image);
    menu.addAction(save_as_PNG);
    menu.addSeparator();

    if(!prot) menu.setEnabled(false);

    menu.exec(QCursor::pos());

    menu.clear();
}
//-----------------------------------------------------------------------------
//--- slot_FluorButton()
//-----------------------------------------------------------------------------
void GraphPlot::slot_FluorButton(int id)
{
    int rejime_data = RejimeData_Box->currentIndex();

    Draw_Chart(prot, 0x01 + rejime_data, COUNT_CH, current_type);
    canvas()->setCursor(Qt::PointingHandCursor);
}
//-----------------------------------------------------------------------------
//--- slot_Norm1000()
//-----------------------------------------------------------------------------
void GraphPlot::slot_Norm1000()
{
    int rejime_data = RejimeData_Box->currentIndex();

    if(Norm_1000->isChecked()) Norm_1000->setIcon(QIcon(":/images/Norm_1000_Checked.png"));
    else Norm_1000->setIcon(QIcon(":/images/Norm_1000.png"));

    Draw_Chart(prot, 0x01 + rejime_data, COUNT_CH, current_type);
    canvas()->setCursor(Qt::PointingHandCursor);
}

//-----------------------------------------------------------------------------
//--- Draw_Chart
//-----------------------------------------------------------------------------
void GraphPlot::Draw_Chart(rt_Protocol *prot, int rejime, int channel, int type)
{
    int i,j;
    QColor color;
    QColor bg_color[] ={0xD8E2F1,0xD8E9D8,0xFEF0CB,0xFFE6FF,0xFFE6FF};
    QColor curve_color[] = {Qt::blue, Qt::green, 0xFF9933, Qt::magenta, Qt::red};
    double *x;
    double *y;
    int count_meas;
    int count_tubes = prot->count_Tubes;
    int ch = -1;
    int id_ch;
    double value;
    QString text;
    QFont font_plot = qApp->font();
    QFont font_title = qApp->font(); //font_plot;
    font_title.setBold(false);
    //font_title.setPointSize(font_title.pointSize()+2);

    bool sts_1000 = false;
    bool sts_all = false;
    if(channel >= COUNT_CH) sts_all = true;
    if(sts_all && Norm_1000->isChecked() && rejime == 0x03 && !rejime_Normalization && !rejime_Lg) sts_1000 = true;

    bool active_1000 = false;
    if(sts_all && !rejime_Normalization && !rejime_Lg && rejime == 0x03) active_1000 = true;
    Norm_1000->setEnabled(active_1000);

    //qDebug() << "sts_1000: " << sts_1000;

    Group_FluorSingle->setVisible(!sts_all & view_as_buttons->isChecked());
    Group_FluorButton->setVisible(sts_all);
    Norm_1000->setVisible(sts_all);


    QwtText text_x, text_y;
    QwtText title;

    title.setFont(font_title);
    title.usedFont(font_title);

    QVector<QPointF> threshold;
    QPointF p_th;
    QPoint  p_label;

    //--- active channel ---
    for(i=0; i<=channel; i++)
    {        
        if(!(prot->active_Channels & (0x0f<<4*i))) continue;
        ch++;
    }    
    if(ch < 0) return;

    current_ch = ch;
    if(sts_all) current_ch = COUNT_CH + 1;
    current_rejime = rejime;
    current_type = type;

    //qDebug() << "Draw: " << ch << rejime << type << sts_all << channel;

    //----------------------

    Clear_Graph();

    switch(type)
    {
    case 1:     x = &prot->X_PCR.at(0);
                count_meas = prot->count_PCR;
                text = tr("PCR");
                text_x.setText(tr("Cycle number"));
                text_y.setText(tr("Fluorescence"));
                break;

    case 2:     x = &prot->X_MC.at(0);
                count_meas = prot->count_MC;
                text = tr("Melting Curve");
                text_x.setText(tr("Temperature,°C"));
                if(rejime == 0x03) text_y.setText("dF/dT");
                else text_y.setText(tr("Fluorescence"));                
                break;

    default:    text = "";   break;
    }

    /*if(rejime_Lg && type == 1)
    {
        y = &prot->PCR_Bace.at(0);
        curve = list_Curve.at(0);

        for(i=0; i<count_meas; i++)
        {
            y[i] = 100.;
            qDebug() << "y = " << x[i] << y[i];
        }

        curve->setSamples(x, y, count_meas);
        curve->setVisible(true);


        setAxisAutoScale(QwtPlot::yLeft, false);
        setAxisScale(QwtPlot::yLeft, 1, 20000);
        setAxisScaleEngine(QwtPlot::yLeft, new QwtLogScaleEngine());
        updateAxes();
        show();
        replot();

        Sleep(3000);


        Clear_Graph();
    }*/

    //qDebug() << prot->count_MC << prot->count_PCR << rejime << type << count_meas << prot->X_MC.size() << RejimeData_Box->currentIndex()+1 << sts_all;
    //qDebug() << "text: " << text << x[0]-1 << x[count_meas-1]+1;

    title.setText(text);
    //setTitle(title);

    //if(sts_all) setCanvasBackground(QColor(255,255,225));
    if(sts_all) setCanvasBackground(QColor(250,250,250));
    else setCanvasBackground(bg_color[channel]);

    setAxisAutoScale(QwtPlot::xBottom, false);   
    setAxisScale(QwtPlot::xBottom,x[0]-1,x[count_meas-1]+1);    

    if(rejime_Lg)
    {
        setAxisAutoScale(QwtPlot::yLeft, false);

        setAxisScale(QwtPlot::yLeft, 1, 20000);
        updateAxes();
        setAxisScaleEngine(QwtPlot::yLeft, new QwtLogScaleEngine());
        //setAxisAutoScale(QwtPlot::yLeft, true);
    }
    else
    {
        setAxisScaleEngine(QwtPlot::yLeft, new QwtLinearScaleEngine());
        setAxisAutoScale(QwtPlot::yLeft, true);
    }

    text_x.usedFont(font_plot);
    text_y.usedFont(font_plot);
    text_y.setFont(font_plot);
    text_x.setFont(font_plot);

    setAxisTitle(QwtPlot::xBottom, text_x);
    setAxisTitle(QwtPlot::yLeft, text_y);

    setAxisFont(QwtPlot::yLeft, font_plot);
    setAxisFont(QwtPlot::xBottom, font_plot);

    j = count_meas*count_tubes*ch;    

    if(sts_all) j = 0;        

    switch(type)
    {
    default:
    case 1:
        switch(rejime)
        {
        default:    y = NULL;                       break;
        case 0x01:  y = &prot->PCR_RawData.at(j);   break;
        case 0x02:  y = &prot->PCR_Filtered.at(j);  break;
        case 0x03:  y = &prot->PCR_Bace.at(j);
                    if(rejime_Normalization)
                    {
                        if(rejime_Ct) y = &prot->NormCt_Value.at(j);
                        else y = &prot->NormCp_Value.at(j);
                    }
                    if(sts_1000) y = PCR_1000->data();
                    break;
        }
        break;

    case 2:
        switch(rejime)
        {
        default:    y = NULL;                       break;
        case 0x01:  y = &prot->MC_RawData.at(j);   break;
        case 0x02:  y = &prot->MC_Filtered.at(j);  break;
        case 0x03:  y = &prot->MC_dF_dT.at(j);      break;
        }
        break;
    }

    if(y == NULL) return;

    if(rejime != RejimeData_Box->currentIndex()+1)
    {        
        RejimeData_Box->blockSignals(true);
        RejimeData_Box->setCurrentIndex(rejime-1);
        RejimeData_Box->blockSignals(false);
    }


    if(sts_all)
    {
        id_ch = 0;

        //qDebug() << "list_curve = " << list_Curve.size();

        for(j=0; j<COUNT_CH; j++)
        {
            if(!(prot->active_Channels & (0x0f<<4*j))) continue;

            QAbstractButton *button = Fluor_Buttons->button(j);

            if(button->isChecked())
            {
                for(i=0; i<count_tubes; i++)
                {
                    curve = list_Curve.at(i+count_tubes*id_ch);
                    color = curve_color[j];

                    if(rejime == 0x03 && type == 2)
                    {
                        curve->setCurveAttribute(QwtPlotCurve::Fitted, true);   //true
                    }
                    else
                    {
                        curve->setCurveAttribute(QwtPlotCurve::Fitted, false);
                    }

                    curve->setPen(color, thickness_line);
                    curve->setSamples(x, y+count_meas*i+count_meas*count_tubes*id_ch, count_meas);


                    if(prot->enable_tube.at(i)) curve->setVisible(true);
                    else curve->setVisible(false);
                }
            }

            id_ch++;
        }
    }
    else
    {
        for(i=0; i<count_tubes; i++)
        {
            curve = list_Curve.at(i+count_tubes*ch);
            color = prot->color_tube.at(i);

            if(rejime == 0x03 && type == 2)
            {
                curve->setCurveAttribute(QwtPlotCurve::Fitted, true);   // true
            }
            else
            {
                curve->setCurveAttribute(QwtPlotCurve::Fitted, false);
            }

            curve->setPen(color, thickness_line);
            curve->setSamples(x, y+count_meas*i, count_meas);

            if(prot->enable_tube.at(i)) curve->setVisible(true);
            else curve->setVisible(false);
        }
    }


    updateAxes();


    //... XY ...
    //Repaint_XY();

    //... Melting ...
    if(rejime == 0x03 && type == 2 && rejime_Melting)
    {
        curve_melting.setPen(Qt::black,1);
        Repaint_Melting(value_MeltingMarker);
        curve_melting.setVisible(true);
        label_melting->setVisible(true);
    }
    else {curve_melting.setVisible(false); label_melting->setVisible(false);}

    //... Threshold ...
    if(rejime == 0x03 && type == 1 && rejime_Ct && !sts_all)
    {
        curve_threshold.setPen(Qt::black,1);
        value = prot->Threshold_Ct.at(ch);

        Repaint_Threshold(value,true);

        /*for(i=0; i<count_meas; i++)
        {
            p_th.setX(i+1);            
            p_th.setY(value);
            threshold.push_back(p_th);
        }*/

        //curve_threshold.setSamples(threshold);
        curve_threshold.setVisible(true);

        /*p_label.setY(transform(0,value));
        p_label.setX(10);
        //qDebug() << p_label << value;
        label_threshold->setText("Threshold = " + QString::number(value,'f',5));
        label_threshold->move(p_label);*/
        label_threshold->setVisible(true);

    }
    else {curve_threshold.setVisible(false); label_threshold->setVisible(false);}
    //.................

    setTitle(title);

    //QPoint pos = Group_FluorButton->pos();
    //Group_FluorButton->move(pos.x()-2,pos.y()-3);

    updateAxes();
    show();
    replot();    
}
//-----------------------------------------------------------------------------
//--- slot Repaint_XY
//-----------------------------------------------------------------------------
/*void GraphPlot::Repaint_XY()
{
    QPoint point;

    point.setX(canvas()->width()-100);
    point.setY(canvas()->height()-25);

    label_XY->move(point);
    replot();
}*/

//-----------------------------------------------------------------------------
//--- slot Repaint_Melting
//-----------------------------------------------------------------------------
void GraphPlot::Repaint_Melting(double value)
{
    int i;
    QVector<QPointF> x;
    QPointF p;
    QString text;
    QPoint point;
    QFontMetrics fm(qApp->font());
    int pixelsWide;
    double lower = axisScaleDiv(QwtPlot::yLeft).lowerBound();
    double upper = axisScaleDiv(QwtPlot::yLeft).upperBound();
    double dy = (upper - lower)/100.;

    curve_melting.setSamples(NULL);
    p.setX(value);

    for(i=1; i<99; i++)
    {
        p.setY(lower + i*dy);
        x.push_back(p);
    }

    curve_melting.setSamples(x);

    point.setX(5);
    point.setY(canvas()->height()-35);
    text = "t° Marker = " + QString::number(value,'f',1);
    pixelsWide = fm.width(text);
    label_melting->setFixedWidth(pixelsWide + 10);

    label_melting->setText(text);
    label_melting->move(point);

    replot();
}

//-----------------------------------------------------------------------------
//--- slot Repaint_Threshold
//-----------------------------------------------------------------------------
void GraphPlot::Repaint_Threshold(double value, bool ct_analysis)
{
    int i;
    QVector<QPointF> y;
    QPointF p;
    QPoint point;
    QFontMetrics fm(qApp->font());
    QString text;
    int pixelsWide;

    int count_meas = prot->count_PCR;

    if(ct_analysis)
    {
        curve_threshold.setSamples(NULL);
        prot->Threshold_Ct.at(current_ch) = value;
        p.setY(value);
        for(i=0; i<count_meas; i++)
        {
            p.setX(i+1);
            y.push_back(p);
        }
        curve_threshold.setSamples(y);
    }

    point.setX(5);
    //point.setY(transform(0,value)-20);
    point.setY(canvas()->height()-35);

    text = "Threshold = " + QString::number(value,'f',1);
    pixelsWide = fm.width(text);
    label_threshold->setFixedWidth(pixelsWide + 10);

    //qDebug() << label_threshold->width() << pixelsWide;

    label_threshold->setText(text);
    label_threshold->move(point);

    replot();

    //... calculate Ct for current channel ...
    if(ct_analysis)
    {
        Calculate_Ct(prot, current_ch);
        reanalysis.param = 0x0f;
        QApplication::sendEvent(main_widget, &reanalysis);
    }
}
//-----------------------------------------------------------------------------
//---  Change_SizeMarker()
//-----------------------------------------------------------------------------
void GraphPlot::Change_ThicknessLine()
{
    QPen pen;

    if(!prot) return;

    foreach(curve, list_Curve)
    {
        pen = curve->pen();
        pen.setWidth(thickness_line);
        curve->setPen(pen);
    }

    replot();
}

//-----------------------------------------------------------------------------
//---  Change_SizeMarker()
//-----------------------------------------------------------------------------
void GraphPlot::Change_SizeMarker()
{
    int i;
    QwtSymbol* symbol;

    if(!prot) return;

    for(i=0; i<list_Curve.size(); i++)
    {
        const QwtSymbol* sym = list_Curve.at(i)->symbol();

        symbol = const_cast<QwtSymbol*>(sym);
        symbol->setSize(size_symbol,size_symbol);

        //((QwtSymbol*)sym)->setSize(size_symbol,size_symbol);
    }


    replot();
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void GraphPlot::Create_Curve(rt_Protocol *P)
{
    int i,j,k;
    QString fluor_name[COUNT_CH] = FLUOR_NAME;
    QIcon icon;
    int first_ch = -1;
    int first_type = 1;
    QVector<double> vec;
    double coef_1000;

    prot = P;   // protocol

    rejime_Ct = false;
    rejime_Lg = false;
    rejime_Normalization = false;
    rejime_Melting = false;

    for(i=0; i<COUNT_CH; i++)
    {
        if(!(P->active_Channels & (0x0f<<4*i))) continue;

        for(j=0; j<P->count_Tubes; j++)
        {
            curve = new QwtPlotCurve();
            //curve->setSymbol(sym);
            curve->setStyle(QwtPlotCurve::Lines);
            curve->setSymbol( new QwtSymbol(QwtSymbol::Ellipse,
                                            QBrush(Qt::white),
                                            QPen(Qt::black, 1),
                                            QSize(size_symbol,size_symbol)));
            //curve->setCurveFitter(fitter);
            //curve->setCurveAttribute(QwtPlotCurve::Fitted);
            //curve->setCurveFitter();
            curve->attach(this);
            list_Curve.append(curve);
        }
    }
    curve_threshold.attach(this);
    curve_melting.attach(this);

    //... fill Fluor_Buttons ...
    Fluor_Buttons->blockSignals(true);
    for(i=0; i<COUNT_CH; i++)
    {
        QAbstractButton *button = Fluor_Buttons->button(i);
        if((prot->active_Channels & (0x0f<<i*4)))
        {
           button->setDisabled(false);
           button->setChecked(true);
        }
        else
        {
            button->setDisabled(true);
            button->setChecked(false);
        }
    }
    Fluor_Buttons->blockSignals(false);

    //... fill Fluor_Single ...
    Fluor_Single->blockSignals(true);
    for(i=COUNT_CH-1; i>=0; i--)
    {
        QAbstractButton *button = Fluor_Single->button(i);
        if((prot->active_Channels & (0x0f<<i*4)))
        {
           button->setDisabled(false);
           button->setChecked(true);
           //button->setVisible(true);
        }
        else
        {
            button->setDisabled(true);
            button->setChecked(false);
            //button->setVisible(false);
        }
    }

    Fluor_Single->blockSignals(false);

    //... fill Fluor_Box ...
    Fluor_Box->blockSignals(true);
    Fluor_Box->setVisible(true);    
    for(i=0; i<COUNT_CH; i++)
    {
       Fluor_Box->addItem(fluor_name[i]);

       if(StyleApp == "fusion")
       {
           switch(i)
           {
           case 0:  icon.addFile(":/images/flat/fam_flat.png");   break;
           case 1:  icon.addFile(":/images/flat/hex_flat.png");   break;
           case 2:  icon.addFile(":/images/flat/rox_flat.png");   break;
           case 3:  icon.addFile(":/images/flat/cy5_flat.png");   break;
           case 4:  icon.addFile(":/images/flat/cy55_flat.png");   break;
           default: icon.addFile(NULL);   break;
           }
       }
       else
       {
           switch(i)
           {
           case 0:  icon.addFile(":/images/fam.png");   break;
           case 1:  icon.addFile(":/images/hex.png");   break;
           case 2:  icon.addFile(":/images/rox.png");   break;
           case 3:  icon.addFile(":/images/cy5.png");   break;
           case 4:  icon.addFile(":/images/cy55.png");   break;
           default: icon.addFile(NULL);   break;
           }
       }

       Fluor_Box->setItemIcon(i,icon);       

       if(!(prot->active_Channels & (0x0f<<i*4)))
       {
          Fluor_Box->setItemData(i,0,Qt::UserRole - 1);
       }
       if(first_ch < 0 && (prot->active_Channels & (0x0f<<i*4)))
       {
           first_ch = i;
       }
    }
    Fluor_Box->addItem("-------");
    Fluor_Box->setItemData(COUNT_CH,0,Qt::UserRole - 1);

    Fluor_Box->addItem(tr("All channels"));
    //icon.addFile(":/images/all_channels.png");
    //Fluor_Box->setItemIcon(COUNT_CH+1,icon);

    Fluor_Box->blockSignals(false);

    //... fill RejimeData_Box ...
    QStringList list;
    list << tr("Raw Data") << tr("Filtered raw data") << tr("Analysis data");

    RejimeData_Box->blockSignals(true);
    RejimeData_Box->addItems(list);
    RejimeData_Box->setVisible(true);
    RejimeData_Box->blockSignals(false);

    //... fill Type Box ...
    list.clear();
    list << tr("PCR") << tr("Melting Curve");
    Type_Box->blockSignals(true);
    Type_Box->addItems(list);
    Type_Box->setVisible(true);
    if(prot->count_PCR <= 0)
    {
        Type_Box->setItemData(0,0,Qt::UserRole - 1);
        Type_Box->setCurrentIndex(1);
        first_type = 2;
    }
    if(prot->count_MC <= 0)  Type_Box->setItemData(1,0,Qt::UserRole - 1);

    if(prot->count_MC > 0) // if MC exists - MC main
    {
        first_type = 2;
        Type_Box->setCurrentIndex(1);
    }

    Type_Box->blockSignals(false);

    //... fill norm_1000 ...
    PCR_1000->clear();
    MC_1000->clear();
    //PCR_1000 = QVector<double>::fromStdVector(prot->PCR_Bace);
    //MC_1000 = QVector<double>::fromStdVector(prot->MC_dF_dT);

    int tubes = prot->count_Tubes;
    int pcr = prot->count_PCR;
    int mc = prot->count_MC;
    double max_val;

    if(pcr)
    {
        k = div(prot->PCR_Bace.size(), tubes*pcr).quot;
        for(i=0; i<k; i++)
        {
            vec.clear();
            vec = QVector<double>::fromStdVector(prot->PCR_Bace).mid(i*tubes*pcr, tubes*pcr);
            max_val = *std::max_element(vec.begin(),vec.end());
            if(max_val > 1) coef_1000 = 1000./max_val;
            else coef_1000 = 1.;
            for(j=0; j<vec.size(); j++)
            {
                PCR_1000->append(vec.at(j)*coef_1000);
            }
        }
    }


    //---
    emit clear_rejimes(first_type-1);
    Draw_Chart(prot, 0x03, first_ch, first_type);
    //---
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void GraphPlot::Delete_Curve()
{
    int i;
    //if(list_Curve.size()) list_Curve.at(0)->setCurveFitter(NULL);
    //if(fitter) {delete fitter; fitter = NULL;}

    qDeleteAll(list_Curve.begin(), list_Curve.end());
    list_Curve.clear();

    setCanvasBackground(bg_color);

    Fluor_Box->blockSignals(true);
    Fluor_Box->clear();
    Fluor_Box->setVisible(false);
    Fluor_Box->blockSignals(false);

    Group_FluorButton->setVisible(false);
    Norm_1000->setVisible(false);
    Group_FluorSingle->setVisible(false);

    RejimeData_Box->blockSignals(true);
    RejimeData_Box->clear();
    RejimeData_Box->setVisible(false);
    RejimeData_Box->blockSignals(false);

    Type_Box->blockSignals(true);
    Type_Box->clear();
    Type_Box->setVisible(false);
    Type_Box->blockSignals(false);

    setTitle("");
    setAxisTitle(QwtPlot::xBottom, "");
    setAxisTitle(QwtPlot::yLeft, "");
    setAxisScale(QwtPlot::xBottom,0,1000);
    setAxisScale(QwtPlot::yLeft,0,1000);

    label_threshold->setVisible(false);

    //CoeffNorm_PCR.clear();
    //CoeffNorm_MC.clear();

    replot();

    prot = NULL;
    id_wcurve = -1;    
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
GraphPlot::~GraphPlot()
{
    Delete_Curve();

    delete CommonSettings;

    delete fam_single;
    delete hex_single;
    delete rox_single;
    delete cy5_single;
    delete cy55_single;
    delete Group_FluorSingle;

    delete fam_button;
    delete hex_button;
    delete rox_button;
    delete cy5_button;
    delete cy55_button;
    delete Group_FluorButton;

    delete grid;    
    delete label_curve;
    delete label_threshold;
    delete label_melting;
    //delete label_XY;
    delete Fluor_Box;
    delete RejimeData_Box;
    delete Type_Box;

    PCR_1000->clear();
    delete PCR_1000;
    MC_1000->clear();
    delete MC_1000;    

    qDeleteAll(listCurve_image.begin(), listCurve_image.end());
    listCurve_image.clear();
    delete grid_image;
    delete plot_image;
    delete box_image;

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void GraphPlot::Clear_Graph()
{
    int i;

    for(i=0; i<list_Curve.count(); i++)
    {
        curve = list_Curve.at(i);        
        curve->setData(NULL);
    }

    curve_threshold.setData(NULL);
    curve_melting.setData(NULL);

    replot();
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void GraphPlot::Change_Enable(rt_Protocol *P, QVector<short> *buf)
{
    int i,j, id;
    int count_tubes = P->count_Tubes;
    int count = buf->size();
    int num = 0;
    QColor color;
    bool sts_all = false;
    int id_ch = 0;
    QColor curve_color[] = {Qt::blue, Qt::green, 0xFF9933, Qt::magenta, Qt::red};


    j = Fluor_Box->currentIndex();
    if(j >= COUNT_CH) sts_all = true;
    //qDebug() << "count = " << j << count << P->active_Channels << count_tubes << *buf;

    if(sts_all)
    {
        for(j=0; j<COUNT_CH; j++)
        {
             if(!(P->active_Channels & (0x0f<<4*j))) continue;
             for(i=0; i<count_tubes; i++)
             {
                 curve = list_Curve.at(i+count_tubes*id_ch);
                 color = curve_color[j];
                 curve->setPen(color, thickness_line);
                 if(prot->enable_tube.at(i)) curve->setVisible(true);
                 else curve->setVisible(false);
             }
             id_ch++;
        }
    }
    else
    {
        for(i=0; i<=j; i++)
        {
            if(P->active_Channels & (0x0f << 4*i)) num++;
        }
        num--;

        //qDebug() << "num = " << num;

        for(i=0; i<count; i++)
        {
            id = buf->at(i);
            curve = list_Curve.at(id+num*count_tubes);
            color = P->color_tube.at(id);
            curve->setPen(color,thickness_line);
            if(P->enable_tube.at(id)) curve->setVisible(true);
            else                      curve->setVisible(false);
            //qDebug() << "id = " << id << " " << P->enable_tube.at(id);
        }
    }

    replot();
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
/*void GraphPlot::Calculate_NormCoefficients(rt_Protocol *P)
{
    int i;
    int num = 0;
    QVector<double> vec;
    int leng;
    double max;

    CoeffNorm_PCR.clear();
    CoeffNorm_MC.clear();

    for(i=0; i<COUNT_CH; i++)
    {
        if(P->active_Channels & (0x0f << 4*i)) num++;
    }

    if(P->count_PCR)
    {
        leng = P->count_PCR*P->count_Tubes;
        for(i=0; i<num; i++)
        {
            vec = QVector<double>::fromStdVector(P->PCR_Bace).mid(i*leng,leng);
            max = *std::max_element(vec.begin(),vec.end());
            CoeffNorm_PCR.append(max);
        }
    }
    if(P->count_MC)
    {
        leng = P->count_MC*P->count_Tubes;
        for(i=0; i<num; i++)
        {
            vec = QVector<double>::fromStdVector(P->MC_dF_dT).mid(i*leng,leng);
            max = *std::max_element(vec.begin(),vec.end());
            CoeffNorm_MC.append(max);
        }
    }

    qDebug() << CoeffNorm_PCR << CoeffNorm_MC;

}*/

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
bool GraphPlot::eventFilter(QObject *object, QEvent *e)
{
    int numDegrees;
    int numSteps;
    double delta, value;
    double dx, dy;
    double dy_lower,dy_upper;
    QPoint pos;
    int id;
    double d;
    double x_min,x_max;
    int count_tubes;
    int ch = -1;
    int i;
    int channel;

    if(object != (QObject *)canvas()) return(false);

    switch(e->type())
    {
    case QEvent::Resize:

        //qDebug() << ((QResizeEvent *)e)->size().height() << ((QResizeEvent *)e)->size().width();

        if(prot && curve_threshold.isVisible())
        {
            value = prot->Threshold_Ct.at(current_ch);
            emit rePaint_Threshold(value,false);
        }

        if(prot && curve_melting.isVisible())
        {
            value = value_MeltingMarker;
            emit rePaint_Melting(value);
        }

        //pos = Group_FluorButton->pos();
        //Group_FluorButton->move(pos.x()-2,pos.y()-3);

        //emit rePaint_XY();

        break;
    //-------------------------------------------------------------------------
    case QEvent::MouseButtonPress:
        if(((QMouseEvent *)e)->buttons() & Qt::LeftButton)
        {
            if(!prot) break;
            if(curve_edit)
            {
                select_GraphMarker(((QMouseEvent *)e)->pos());
            }
            else
            {
                select_GraphCurve(((QMouseEvent *)e)->pos());
                QTimer::singleShot(1000, this, SLOT(Hide_LabelCurve()));
            }
        }
        break;
    //-------------------------------------------------------------------------
    case QEvent::MouseButtonDblClick:
        if(((QMouseEvent *)e)->buttons() & Qt::LeftButton)
        {
            if(!prot) break;

            if(prot) count_tubes = prot->count_Tubes;

            //--- active channel ---
            channel = Fluor_Box->currentIndex();
            for(i=0; i<=channel; i++)
            {
                if(!(prot->active_Channels & (0x0f<<4*i))) continue;
                ch++;
            }
            if(ch < 0) break;

            //SetAutoScale();
            curve = list_Curve.at(0 + count_tubes*ch);
            x_min = curve->minXValue() - 1;
            x_max = curve->maxXValue() + 1;

            setAxisAutoScale(QwtPlot::yLeft, true);
            setAxisAutoScale(QwtPlot::xBottom, false);
            setAxisScale(QwtPlot::xBottom,x_min,x_max);

            replot();
        }
        break;
    //-------------------------------------------------------------------------
    case QEvent::MouseButtonRelease:

        if(!prot) break;

        canvas()->setCursor(Qt::PointingHandCursor);
        pos_previous.setX(0);
        pos_previous.setY(0);

        if(curve_threshold.isVisible())
        {
            value = prot->Threshold_Ct.at(current_ch);
            emit rePaint_Threshold(value,false);
        }

        if(curve_melting.isVisible())
        {
            value = value_MeltingMarker;
            emit rePaint_Melting(value);
        }

        //emit rePaint_XY();

        break;
    //-------------------------------------------------------------------------
    case QEvent::MouseMove:

        if(!prot) break;

        pos = ((QMouseEvent *)e)->pos();

        //... XY label ...
        //dx = invTransform(QwtPlot::xBottom, pos.x());
        //dy = invTransform(QwtPlot::yLeft, pos.y());

        //label_XY->setText(QString("xy(%1,%2)").arg(dx, 0, 'f',1).arg(dy, 0, 'f', 1));
        //emit rePaint_XY();
        //...

        if(((QMouseEvent *)e)->buttons() == Qt::NoButton)       // NoButton
        {

            //qDebug() << "mouse pos: xy -  " << invTransform(QwtPlot::xBottom, pos.x()) << invTransform(QwtPlot::yLeft, pos.y());

            /*if(Threshold_value >= 0)
            {
                dy = canvas()->height();
                dx = fabs(((QMouseEvent *)e)->pos().y() - transform(0,Threshold_value));
                if(dx/dy < 0.01) canvas()->setCursor(Qt::SplitVCursor);
                else canvas()->setCursor(Qt::PointingHandCursor);
            }*/
            if(curve_threshold.isVisible())
            {
                id = curve_threshold.closestPoint(pos, &d);
                if(id >= 0 && d < 10) canvas()->setCursor(Qt::SplitVCursor);
                else canvas()->setCursor(Qt::PointingHandCursor);
            }
            if(curve_melting.isVisible())
            {
                id = curve_melting.closestPoint(pos, &d);
                if(id >= 0 && d < 10) canvas()->setCursor(Qt::SplitHCursor);
                else canvas()->setCursor(Qt::PointingHandCursor);
            }
        }

        if(((QMouseEvent *)e)->buttons() & Qt::LeftButton)      // LeftButton
        {            
            //pos = ((QMouseEvent *)e)->pos();

            //... CURVE_EDIT Mode ....
            if(curve_edit)
            {
                if(!d_selectedCurve) break;

                QVector<double> xData(d_selectedCurve->dataSize());
                QVector<double> yData(d_selectedCurve->dataSize());
                int count = static_cast<int>(d_selectedCurve->dataSize());

                for(i = 0; i < count; i++)
                {
                    if(i == d_selectedPoint)
                    {
                        xData[i] = invTransform(d_selectedCurve->xAxis(), pos.x());
                        yData[i] = invTransform(d_selectedCurve->yAxis(), pos.y());
                    }
                    else
                    {
                        const QPointF sample = d_selectedCurve->sample(i);
                        xData[i] = sample.x();
                        yData[i] = sample.y();
                    }
                }
                d_selectedCurve->setSamples(xData, yData);

                /*
                   Enable QwtPlotCanvas::ImmediatePaint, so that the canvas has been
                   updated before we paint the cursor on it.
                 */
                QwtPlotCanvas *plotCanvas = qobject_cast<QwtPlotCanvas*>(canvas());

                plotCanvas->setPaintAttribute(QwtPlotCanvas::ImmediatePaint, true);
                replot();
                plotCanvas->setPaintAttribute(QwtPlotCanvas::ImmediatePaint, false);

                select_Marker(true);

                break;
            }
            //........................


            if(canvas()->cursor().shape() == Qt::SplitVCursor && rejime_Ct ||
               canvas()->cursor().shape() == Qt::SplitHCursor && rejime_Melting)
            {
                if(rejime_Ct)
                {
                    value = invTransform(0, pos.y());
                    if(value > 1){emit rePaint_Threshold(value,true);}
                }
                if(rejime_Melting)
                {
                    value_MeltingMarker = invTransform(QwtPlot::xBottom, pos.x());
                    emit rePaint_Melting(value_MeltingMarker);
                }
            }
            else
            {
                canvas()->setCursor(Qt::ClosedHandCursor);
                if(!pos_previous.isNull())
                {
                    dx = invTransform(2, pos.x()) - invTransform(2, pos_previous.x());
                    dy = invTransform(0, pos.y()) - invTransform(0, pos_previous.y());



                    if(rejime_Lg)
                    {
                        dy_lower = axisScaleDiv(0).lowerBound() * 0.1;
                        dy_upper = axisScaleDiv(0).upperBound() * 0.1;

                        if(dy < 0) {dy_lower = - dy_lower; dy_upper = - dy_upper;}

                        //qDebug() << "dy: " << dy << dy_lower << dy_upper;

                        if(dy != 0. &&
                          (axisScaleDiv(0).lowerBound() - dy_lower) > 0 &&
                          (axisScaleDiv(0).upperBound() - dy_upper) > 0 &&
                          (axisScaleDiv(0).upperBound() - dy_upper) > (axisScaleDiv(0).lowerBound() - dy_lower))
                        {
                            setAxisScale(0,
                                        axisScaleDiv(0).lowerBound() - dy_lower,
                                        axisScaleDiv(0).upperBound() - dy_upper,
                                        0);
                        }

                    }
                    else
                    {

                        setAxisScale(0,
                                    axisScaleDiv(0).lowerBound() - dy,
                                    axisScaleDiv(0).upperBound() - dy,
                                    0);
                    }

                    setAxisScale(2,
                                axisScaleDiv(2).lowerBound() - dx,
                                axisScaleDiv(2).upperBound() - dx,
                                0);
                }
                pos_previous = pos;

                if(curve_threshold.isVisible())
                {
                    value = prot->Threshold_Ct.at(current_ch);
                    emit rePaint_Threshold(value,false);
                }
                if(curve_melting.isVisible())
                {
                    value = value_MeltingMarker;
                    emit rePaint_Melting(value);
                }

                //qDebug() << "Move: " << pos;

                replot();
            }
        }

        break;
    //-------------------------------------------------------------------------
    case QEvent::Wheel:

        if(!prot) break;


        if(rejime_Lg) break;    // Lg rejime

        numDegrees = ((QWheelEvent *)e)->delta()/8;
        numSteps = numDegrees/15;

        delta = axisScaleDiv(0).range()/10.;    // 10 percent Y axis
        delta *= numSteps;
        setAxisScale(0,
                     axisScaleDiv(0).lowerBound() + delta/2,
                     axisScaleDiv(0).upperBound() - delta/2,
                     0);
        delta = axisScaleDiv(2).range()/10.;    // 10 percent X axis
        delta *= numSteps;
        setAxisScale(2,
                     axisScaleDiv(2).lowerBound() + delta/2,
                     axisScaleDiv(2).upperBound() - delta/2,
                     0);

        if(curve_threshold.isVisible())
        {
            value = prot->Threshold_Ct.at(current_ch);
            emit rePaint_Threshold(value,false);
        }
        if(curve_melting.isVisible())
        {
            value = value_MeltingMarker;
            emit rePaint_Melting(value);
        }

        replot();

        break;

    //-------------------------------------------------------------------------
    default:    break;
    }

    return(false);
}

//-----------------------------------------------------------------------------
//--- select_Marker
//-----------------------------------------------------------------------------
void GraphPlot::select_Marker(bool showIt)
{
    if(!d_selectedCurve) return;

    QwtSymbol *symbol = const_cast<QwtSymbol *>(d_selectedCurve->symbol());
    const QBrush brush = symbol->brush();

    //if(showIt) symbol->setBrush(symbol->brush().color().dark(180));
    if(showIt) symbol->setBrush(QColor(255,0,0,255));   // red

    QwtPlotDirectPainter directPainter;
    directPainter.drawSeries(d_selectedCurve, d_selectedPoint, d_selectedPoint);

    if(showIt) symbol->setBrush(brush); // reset brush
}

//-----------------------------------------------------------------------------
//--- select_GraphMarker
//-----------------------------------------------------------------------------
void GraphPlot::select_GraphMarker(const QPoint &pos)
{
    int i,j,k;
    double dist = 10e10;
    double d;
    int id;
    QwtPlotCurve *c = NULL;
    int index = -1;

    if(prot == NULL) return;

    j = 0;
    for(k=0; k<=Fluor_Box->currentIndex(); k++)
    {
        if(prot->active_Channels & (0x0f << k*4)) j++;
    }
    j--;

    for(i=0; i<prot->count_Tubes; i++)
    {
        if(!prot->enable_tube.at(i)) continue;

        curve = list_Curve.at(i+j*prot->count_Tubes);
        id = curve->closestPoint(pos,&d);

        if(d < dist)
        {
            c = curve;
            index = id;
            dist = d;
        }
    }

    select_Marker(false);
    d_selectedCurve = NULL;
    d_selectedPoint = -1;

    if(c && dist < 10 ) // 10 pixels tolerance
    {
        d_selectedCurve = c;
        d_selectedPoint = index;
        select_Marker(true);
    }
}

//-----------------------------------------------------------------------------
//--- select_GraphCurve
//-----------------------------------------------------------------------------
void GraphPlot::select_GraphCurve(const QPoint &pos)
{
    int i,j,k;
    int id, id_before, id_after;
    int index = -1;
    double d;
    int size;
    double dist = 10e10;
    QPen pen;
    QPoint pos_before, pos_after;
    QString text;
    int col, row;

    if(prot == NULL) return;

    if(curve_threshold.isVisible())
    {
        id = curve_threshold.closestPoint(pos, &d);
        if(id >= 0 && d < 10)
        {
            canvas()->setCursor(Qt::SplitVCursor);
            return;
        }
    }
    if(curve_melting.isVisible())
    {
        id = curve_melting.closestPoint(pos, &d);
        if(id >= 0 && d < 10)
        {
            canvas()->setCursor(Qt::SplitHCursor);
            return;
        }
    }

    //qDebug() << "active_channel" << active_ch;
/*
    for(j=0; j<count_channels; j++)
    {
        if((active_ch & 0x100) == 0 && j != active_ch) continue;
        if((active_ch & 0x100) && (active_ch & (0x01<<j)) == 0) continue;
*/

        if(Fluor_Box->currentIndex() >= COUNT_CH) return;   // All_channel


        j = 0;
        for(k=0; k<=Fluor_Box->currentIndex(); k++)
        {
            if(prot->active_Channels & (0x0f << k*4)) j++;
        }
        j--;

        for(i=0; i<prot->count_Tubes; i++)
        {            
            if(!prot->enable_tube.at(i)) continue;

            curve = list_Curve.at(i+j*prot->count_Tubes);
            size = curve->dataSize();
            id = curve->closestPoint(pos, &d);            

            pen = curve->pen();
            pen.setWidth(thickness_line);
            curve->setPen(pen);

            if(id < 0) continue;

            if(pos.x() > transform(2,curve->minXValue()) && pos.x() < transform(2,curve->maxXValue()))
            {
                if(pos.x() > transform(2, curve->sample(id).x()))
                {
                    id_before = id;
                    id_after = id + 1;
                }
                else
                {
                    id_before = id - 1;
                    id_after = id;
                }

                pos_before.setX(transform(2, curve->sample(id_before).x()));
                pos_before.setY(transform(0, curve->sample(id_before).y()));
                pos_after.setX(transform(2, curve->sample(id_after).x()));
                pos_after.setY(transform(0, curve->sample(id_after).y()));

                d = Calculate_closestDistance(pos, pos_before, pos_after);
            }

            if(d < dist)
            {
                dist = d;
                index = i+j*prot->count_Tubes;
            }
        }
        id_wcurve = -1;
    //}

    if(index >= 0 && dist < 10)     // 10 pixels tolerance
    {
        curve = list_Curve.at(index);
        pen = curve->pen();
        pen.setWidth(thickness_line+2);
        curve->setPen(pen);
        index = div(index,prot->count_Tubes).rem;
        id_wcurve = index;

        prot->Plate.PlateSize(prot->count_Tubes, row, col);
        text = Convert_IndexToName(index, col);

        label_curve->setText(text);
        pos_before = pos;
        pos_before.setY(pos.y()-label_curve->height()-2);
        label_curve->move(pos_before);
        label_curve->setVisible(true);

        //qDebug() << "pos = " << pos;

        select_tube.pos_tube = index;
        select_tube.cell_grid = text;
        QApplication::sendEvent(main_widget, &select_tube);
    }
    replot();

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
double GraphPlot::Calculate_closestDistance(QPoint pos, QPoint pos_before, QPoint pos_after)
{
    double a,b,c;
    double d;
    double alfa;
    double res;

    a = qSqrt(qPow(qAbs(pos.x() - pos_before.x()), 2) + qPow(qAbs(pos.y() - pos_before.y()), 2));
    b = qSqrt(qPow(qAbs(pos.x() - pos_after.x()), 2) + qPow(qAbs(pos.y() - pos_after.y()), 2));
    c = qSqrt(qPow(qAbs(pos_after.x() - pos_before.x()), 2) + qPow(qAbs(pos_after.y() - pos_before.y()), 2));

    if(b < 10e-10 || c < 10e-10) return(0.);

    d = (pow(b,2) + pow(c,2) - pow(a,2))/(2*b*c);
    alfa = qAcos(d);
    res = b * qSin(alfa);

    return(res);

}
//-----------------------------------------------------------------------------
//--- 
//-----------------------------------------------------------------------------
void GraphPlot::Select_Curve(int pos)
{
    QwtPlotCurve *curve;
    QPen pen;
    int i,j=0;
    int count = prot->count_Tubes;
    int id, width;

    if(Fluor_Box->currentIndex() < COUNT_CH)
    {
        for(i=0; i<=Fluor_Box->currentIndex(); i++)
        {
            if(prot->active_Channels & (0x0f << i*4)) j++;
        }
        j--;

        //qDebug() << "select_curve" << pos << id_wcurve << j;

        if(prot && prot->count_Tubes > pos && pos >= 0 && j>=0)
        {
            if(id_wcurve >= 0 && id_wcurve < prot->count_Tubes)
            {
                curve = (QwtPlotCurve*)list_Curve.at(id_wcurve+j*count);
                pen = curve->pen();
                pen.setWidth(thickness_line);
                curve->setPen(pen);
            }

            curve = (QwtPlotCurve*)list_Curve.at(pos+j*count);
            pen = curve->pen();
            pen.setWidth(thickness_line+2);
            curve->setPen(pen);
            id_wcurve = pos;

            replot();
        }
    }
    else
    {
        id = 0;
        for(i=0; i<=COUNT_CH; i++)
        {
            if(!(prot->active_Channels & (0x0f << i*4))) continue;

            for(j=0; j<count; j++)
            {
                width = thickness_line;
                if(pos == j) width = thickness_line + 2;

                curve = (QwtPlotCurve*)list_Curve.at(j + id*count);
                pen = curve->pen();
                pen.setWidth(width);
                curve->setPen(pen);
            }

            id++;
        }
        replot();
    }


}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void GraphPlot::Increase_Scale()
{
    double delta,min,max;
    QwtScaleDiv d;
    int i;

    setAxisAutoScale(QwtPlot::xBottom, false);
    setAxisAutoScale(QwtPlot::yLeft, false);

    d = axisScaleDiv(QwtPlot::xBottom);
    delta = d.range()*0.1;
    min = d.lowerBound() + delta;
    max = d.upperBound() - delta;
    if(!rejime_Lg) setAxisScale(QwtPlot::xBottom, min, max);

    d = axisScaleDiv(QwtPlot::yLeft);
    delta = d.range()*0.1;
    min = d.lowerBound() + delta;
    max = d.upperBound() - delta;
    if(rejime_Lg)
    {
        i = 5;
        while(i)
        {
            min = d.lowerBound()*i;
            max = d.upperBound()/i;
            i--;

            if(min < max) break;
            if(i == 1)
            {
                min = d.lowerBound();
                max = d.upperBound();
                break;
            }
        }
    }

    setAxisScale(QwtPlot::yLeft, min, max);

    replot();

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void GraphPlot::Decrease_Scale()
{
    double delta,min,max;
    QwtScaleDiv d;

    setAxisAutoScale(QwtPlot::xBottom, false);
    setAxisAutoScale(QwtPlot::yLeft, false);

    d = axisScaleDiv(QwtPlot::xBottom);
    delta = d.range()*0.1;
    min = d.lowerBound() - delta;
    max = d.upperBound() + delta;
    if(!rejime_Lg) setAxisScale(QwtPlot::xBottom, min, max);

    d = axisScaleDiv(QwtPlot::yLeft);
    delta = d.range()*0.1;
    min = d.lowerBound() - delta;
    max = d.upperBound() + delta;
    if(rejime_Lg)
    {
        min = d.lowerBound()/5.;
        max = d.upperBound()*5.;
    }
    setAxisScale(QwtPlot::yLeft, min, max);

    replot();
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void GraphPlot::Auto_Scale()
{
    setAxisAutoScale(QwtPlot::xBottom, true);
    setAxisAutoScale(QwtPlot::yLeft, true);

    replot();
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void GraphPlot::change_Fluor(int ch)
{

        Fluor_Single->blockSignals(true);
        if(ch < COUNT_CH)
        {
        QAbstractButton *button = Fluor_Single->button(ch);
        button->setChecked(true);
        }
        Fluor_Single->blockSignals(false);


    int rejime_data = RejimeData_Box->currentIndex();
    int type = Type_Box->currentIndex();
    if(Fluor_Box->hasFocus()) Fluor_Box->clearFocus();

    //qDebug() << "ch,rejime_data,current_type: " << ch << rejime_data << current_type;

    if(ch >= 0) Draw_Chart(prot, 0x01 + rejime_data, ch, 0x01 + type);
    canvas()->setCursor(Qt::PointingHandCursor);
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void GraphPlot::change_RejimeData(int rejime)
{    
    int type_fluor = Fluor_Box->currentIndex();
    if(RejimeData_Box->hasFocus()) RejimeData_Box->clearFocus();
    if(rejime >= 0) Draw_Chart(prot, 0x01 + rejime, type_fluor, current_type);
    canvas()->setCursor(Qt::PointingHandCursor);
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void GraphPlot::change_TypeData(int type)
{    
    int type_fluor = Fluor_Box->currentIndex();
    int rejime_data = RejimeData_Box->currentIndex();
    if(Type_Box->hasFocus()) Type_Box->clearFocus();
    //... clear rejimes ...
    rejime_Ct = false;
    rejime_Lg = false;
    rejime_Normalization = false;
    rejime_Melting = false;
    emit clear_rejimes(type);
    //...    
    if(type >= 0) Draw_Chart(prot, 0x01 + rejime_data, type_fluor, type + 1);
    canvas()->setCursor(Qt::PointingHandCursor);
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void GraphPlot::helpChart()
{
    QMenu menu;

    menu.setStyleSheet("QMenu::item:selected {background-color: #d7d7ff; color: black;}");

    /*menu.setStyleSheet("QMenu {border: 1px 1px 1px 1px solid transparent;}"
                       "QMenu::item {height: 22px; text-align: left;"
                                    "padding-left: 10px; border-top:1px solid transparent;"
                                    "border-bottom: 1px solid #D4D0C8; border-right: 1px solid transparent;"
                                    "border-left: 1px solid transparent;}");*/

    menu.setTitle(tr("Help Chart"));
    menu.addAction(tr("Wheel+ -> zoom IN"));
    menu.addAction(tr("Wheel- -> zoom OUT"));
    menu.addAction(tr("Left button double click -> autoscale"));
    menu.addAction(tr("Left button press & move -> move Chart"));
    menu.addAction(tr("Left button press on curve -> select curve"));
    menu.addAction(tr("Right button press -> popup menu"));

    menu.exec(QCursor::pos());
    menu.clear();

}
//-----------------------------------------------------------------------------
//--- Melt_Chart()
//-----------------------------------------------------------------------------
bool GraphPlot::Melt_Chart(bool sts)
{
    if(!prot || prot->count_MC == 0) return(false);

    rejime_Melting = sts;
    Draw_Chart(prot, 0x03, Fluor_Box->currentIndex(), 2);
    return(true);
}
//-----------------------------------------------------------------------------
//--- Ct_Chart()
//-----------------------------------------------------------------------------
bool GraphPlot::Ct_Chart(bool sts)
{
    if(!prot || prot->count_PCR == 0) return(false);

    rejime_Ct = sts;
    Draw_Chart(prot, 0x03, Fluor_Box->currentIndex());
    return(true);
}
//-----------------------------------------------------------------------------
//--- Norm_Chart()
//-----------------------------------------------------------------------------
bool GraphPlot::Norm_Chart(bool sts)
{
    if(!prot || prot->count_PCR == 0) return(false);

    rejime_Normalization = sts;    
    Draw_Chart(prot, 0x03, Fluor_Box->currentIndex());
    return(true);
}
//-----------------------------------------------------------------------------
//--- Lg_Chart()
//-----------------------------------------------------------------------------
bool GraphPlot::Lg_Chart(bool sts)
{
    if(!prot || prot->count_PCR == 0) return(false);

    rejime_Lg = sts;
    Draw_Chart(prot, 0x03, Fluor_Box->currentIndex());
    return(true);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
GraphPlot* Create_Graph(QWidget *p_mainwidget)
{
    GraphPlot* graph = new GraphPlot();
    graph->main_widget = p_mainwidget;

    /*graph->font_plot = p_mainwidget->font();

    graph->font_plot.setBold(false);

    graph->font_plot.setPointSize(8);
    graph->setAxisFont(QwtPlot::yLeft, graph->font_plot);
    graph->setAxisFont(QwtPlot::xBottom, graph->font_plot);*/
    return(graph);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Delete_Graph(GraphPlot* graph)
{
    delete graph;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Fluor_ItemDelegate::paint(QPainter *painter,
                               const QStyleOptionViewItem &option,
                               const QModelIndex &index) const
{
    QRect rect = option.rect;
    QString text = index.data().toString();
    QFont font = qApp->font();
    QPixmap pixmap(":/images/fam.png");
    int row = index.row();

    QPalette pal = option.palette;
    QStyleOptionViewItem  viewOption(option);
    viewOption.state &= ~QStyle::State_HasFocus;    // disable state HasFocus
    QStyledItemDelegate::paint(painter, viewOption, index);

    //__1. Background

    if(option.state & QStyle::State_Selected)
    {
        painter->fillRect(option.rect, QColor(215,215,255));
    }
    else
    {
        painter->fillRect(rect, QColor(255,255,255)); //QColor(255,255,225)
    }

    //__2. Data
    painter->setPen(QPen(Qt::black,1,Qt::SolidLine));
    //font = GraphPlot::font_plot;
    //font = painter->font();
    //font.setBold(true);
    painter->setFont(font); //QFont("Comic Sans MS", 10, QFont::Bold));

    if(option.state & QStyle::State_Enabled)
    {
        if(style == "fusion")
        {
            switch(row)
            {
            case 0:  pixmap.load(":/images/flat/fam_flat.png");    break;
            case 1:  pixmap.load(":/images/flat/hex_flat.png");    break;
            case 2:  pixmap.load(":/images/flat/rox_flat.png");    break;
            case 3:  pixmap.load(":/images/flat/cy5_flat.png");    break;
            case 4:  pixmap.load(":/images/flat/cy55_flat.png");   break;
            default: pixmap.load(NULL);   break;
            }
        }
        else
        {
            switch(row)
            {
            case 0:  pixmap.load(":/images/fam.png");    break;
            case 1:  pixmap.load(":/images/hex.png");    break;
            case 2:  pixmap.load(":/images/rox.png");    break;
            case 3:  pixmap.load(":/images/cy5.png");    break;
            case 4:  pixmap.load(":/images/cy55.png");   break;
            //default: pixmap.load(":/images/all_channels.png");   break;
            default: pixmap.load(NULL);   break;
            }
        }
    }
    else
    {
        painter->setPen(QPen(QColor(192,192,192) ,1,Qt::SolidLine));
        if(style == "fusion") pixmap.load(":/images/flat/disable_flat.png");
        else pixmap.load(":/images/disable.png");
    }

    painter->drawText(rect, Qt::AlignHCenter | Qt::AlignVCenter, text);


    if(!text.contains("---")) painter->drawPixmap(0, rect.y()+2, pixmap);

    //qDebug() << "row = " << index.row();
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void RejimeData_ItemDelegate::paint(QPainter *painter,
                              const QStyleOptionViewItem &option,
                              const QModelIndex &index) const
{
    QRect rect = option.rect;
    QString text = index.data().toString();
    QFont font = qApp->font();
    int row = index.row();

    QPalette pal = option.palette;
    QStyleOptionViewItem  viewOption(option);
    viewOption.state &= ~QStyle::State_HasFocus;    // disable state HasFocus
    QStyledItemDelegate::paint(painter, viewOption, index);

    //__1. Background

    if(option.state & QStyle::State_Selected)
    {
        painter->fillRect(option.rect, QColor(215,215,255));
    }
    else
    {
        painter->fillRect(rect, QColor(255,255,255)); //QColor(255,255,225)
    }

    //__2. Data
    painter->setPen(QPen(Qt::black,1,Qt::SolidLine));
    //font = GraphPlot::font_plot;
    //font = painter->font();
    //font.setBold(true);
    painter->setFont(font); //QFont("Comic Sans MS", 10, QFont::Bold));

    if(option.state & QStyle::State_Enabled)
    {
    }
    else
    {
        painter->setPen(QPen(QColor(192,192,192) ,1,Qt::SolidLine));
    }

    painter->drawText(rect, Qt::AlignVCenter | Qt::AlignLeft, text);

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void TypeData_ItemDelegate::paint(QPainter *painter,
                              const QStyleOptionViewItem &option,
                              const QModelIndex &index) const
{
    QRect rect = option.rect;
    QFont font = qApp->font();
    QString text = index.data().toString();    
    int row = index.row();

    QPalette pal = option.palette;
    QStyleOptionViewItem  viewOption(option);
    viewOption.state &= ~QStyle::State_HasFocus;    // disable state HasFocus
    QStyledItemDelegate::paint(painter, viewOption, index);

    //__1. Background

    if(option.state & QStyle::State_Selected)
    {
        painter->fillRect(option.rect, QColor(215,215,255));
    }
    else
    {
        painter->fillRect(rect, QColor(255,255,255)); //QColor(255,255,225)
    }

    //__2. Data
    painter->setPen(QPen(Qt::black,1,Qt::SolidLine));
    //font = GraphPlot::font_plot;
    //font = painter->font();
    //font.setBold(true);
    painter->setFont(font);

    if(option.state & QStyle::State_Enabled)
    {
    }
    else
    {
        painter->setPen(QPen(QColor(192,192,192) ,1,Qt::SolidLine));
    }

    painter->drawText(rect, Qt::AlignVCenter | Qt::AlignLeft, text);

}
