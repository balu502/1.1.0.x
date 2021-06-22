#include "graph_temperature.h"




//-----------------------------------------------------------------------------
//--- GraphTemperaturePlot()
//-----------------------------------------------------------------------------
GraphTemperaturePlot::GraphTemperaturePlot(QWidget *parent):
    QwtPlot(parent)
{        
    readCommonSettings();
    setFont(qApp->font());

    main_widget = parent;    

    canvas()->setCursor(Qt::PointingHandCursor);
    canvas()->installEventFilter(this);
    canvas()->setMouseTracking(true);

    //QColor color_bg = 0x99D9EA;
    //QColor color_bg = 0xEAF8FB;
    //setCanvasBackground(color_bg);
    set_Enable(false);

    grid = new QwtPlotGrid;
    grid->setMajorPen(QPen(Qt::gray, 0, Qt::DotLine));
    grid->attach(this);

    curve = new TemperatureCurve();
    curve->setStyle(QwtPlotCurve::Lines);
    curve->setPen(Qt::darkGreen,2);
    curve->attach(this);
    XY.reserve(HISTORY);
    curve->color_XY.reserve(HISTORY);
    base = QTime(0,0,0,0);
    x0 = 0;

    setAxisScale(QwtPlot::yLeft,0,100);
    setAxisScale(QwtPlot::xBottom,0,HISTORY);

    scaleDraw = new TimeScaleDraw(QTime(0,0,0,0));
    setAxisScaleDraw(QwtPlot::xBottom, scaleDraw);
    //setAxisMaxMajor(QwtPlot::xBottom, 6);

    QFont f = qApp->font();
    f.setBold(false);
    f.setPointSize(f.pointSize()-1);
    setAxisFont(QwtPlot::yLeft, f);
    setAxisFont(QwtPlot::xBottom, f);


    label_caption = new QLabel("");
    QFont font_label = qApp->font();
    font_label.setItalic(true);
    font_label.setBold(false);
    label_caption->setFont(font_label);
    label_caption->setAlignment(Qt::AlignLeft);
    //label_caption->setFrameStyle(QFrame::Panel | QFrame::Plain);
    //label_caption->setAlignment(Qt::AlignCenter);
    //label_caption->setStyleSheet("QLabel{background-color: rgb(220, 220, 220)}");
    label_caption->setStyleSheet("QLabel{background-color: QColor(0x99D9EA); color:darkblue}");

    label_measuring = new QLabel("");
    QPixmap image_measuring(":/images/foto.png");
    label_measuring->setPixmap(image_measuring);
    label_measuring->setVisible(false);

    label_SaveImageRejime = new QLabel("");
    QPixmap image_save(":/images/save_flat_16.png");
    label_SaveImageRejime->setPixmap(image_save);
    label_SaveImageRejime->setVisible(false);
    flag_SaveImageRejime = false;

    QHBoxLayout *graph_H_layout = new QHBoxLayout();
    graph_H_layout->setMargin(1);
    graph_H_layout->addWidget(label_caption,0,Qt::AlignLeft);
    graph_H_layout->addWidget(label_SaveImageRejime,1,Qt::AlignRight);
    graph_H_layout->addWidget(label_measuring,0,Qt::AlignRight);

    current_Temperature = new QLabel();
    font_label.setFamily("Times New Roman");
    font_label.setPointSize(18);
    font_label.setBold(true);
    font_label.setItalic(false);
    current_Temperature->setFont(font_label);
    current_Temperature->setAlignment(Qt::AlignLeft);    
    current_Temperature->setFrameShape(QFrame::StyledPanel);
    current_Temperature->setStyleSheet("QLabel {background-color: #88FAFAFA; color: black; border: 1px solid gray;}");
    QHBoxLayout *graph_Ht_layout = new QHBoxLayout();
    graph_Ht_layout->addWidget(current_Temperature,0,Qt::AlignLeft);

    warning_State = new QLabel();
    font_label.setFamily("Times New Roman");
    font_label.setPointSize(26);
    font_label.setBold(true);
    font_label.setItalic(false);
    warning_State->setFont(font_label);
    warning_State->setText("");
    warning_State->setVisible(false);
    warning_State->setStyleSheet("QLabel{color:darkblue}");
    //warning_State->setAlignment(Qt::AlignCenter);
    QHBoxLayout *graph_Hw_layout = new QHBoxLayout();
    graph_Hw_layout->addWidget(warning_State,0,Qt::AlignCenter);

    QVBoxLayout *graph_V_layout = new QVBoxLayout();
    canvas()->setLayout(graph_V_layout);
    graph_V_layout->addLayout(graph_H_layout);
    graph_V_layout->addStretch(1);
    graph_V_layout->addLayout(graph_Hw_layout);
    graph_V_layout->addStretch(1);
    graph_V_layout->addLayout(graph_Ht_layout);

    connect(&timer_warning, SIGNAL(timeout()), this, SLOT(slot_WarningRejime()));
    connect(&timer_measuring, SIGNAL(timeout()), this, SLOT(slot_MeasuringRejime()));

    replot();

}
//-----------------------------------------------------------------------------
//--- ~GraphTemperaturePlot()
//-----------------------------------------------------------------------------
GraphTemperaturePlot::~GraphTemperaturePlot()
{
    delete curve;
    delete label_caption;
    delete label_measuring;
    delete label_SaveImageRejime;
    delete current_Temperature;
    delete warning_State;
    delete grid;

}
//-----------------------------------------------------------------------------
//--- slotRefreshData()
//-----------------------------------------------------------------------------
void GraphTemperaturePlot::readCommonSettings()
{

    QString text;
    QString dir_path = qApp->applicationDirPath();
    QSettings *CommonSettings = new QSettings(dir_path + "/tools/ini/preference.ini", QSettings::IniFormat);

    CommonSettings->beginGroup("Common");
        //... Language ...
        text = CommonSettings->value("language","ru").toString();
        if(translator.load(":/translations/graph_temperature_" + text + ".qm"))
        {
            qApp->installTranslator(&translator);
        }

    CommonSettings->endGroup();
    delete CommonSettings;
}

//-----------------------------------------------------------------------------
//--- slotRefreshData()
//-----------------------------------------------------------------------------
void GraphTemperaturePlot::slotRefreshData(QMap<QString,QString> *map)
{
    QString text = map->value(INFO_Temperature);
    double value = text.split(" ").at(0).toDouble();
    value /= 100.;
    QString str = QString::number(value,'f',1);

    if(value > 110 || value < 0) return;

    QPointF point;
    QByteArray ba("\xB0\x43");
    QColor color_point;
    int state_dev;
    int measuring;
    bool ok;

    text = QString::fromLatin1(ba);

    str = "t=" + str + text;
    current_Temperature->setText(str);


    x0 = (double)(base.elapsed())/1000.;
    point.setX(x0);
    point.setY(value);
    if(XY.size() >= HISTORY)
    {
        XY.removeFirst();
        setAxisScale(QwtPlot::xBottom,XY.at(0).x(), x0);
    }
    XY.append(point);

    text = map->value(INFO_status);
    sscanf(text.toStdString().c_str(),"%d", &state_dev);
    switch(state_dev)
    {
    default:
    case 0:     color_point = Qt::darkGreen;    break;      // Nothing
    case 2:     color_point = Qt::darkYellow;   break;      // Pause
    case 3:     color_point = Qt::blue;         break;      // Hold
    case 4:     color_point = Qt::darkGreen;    break;      // Warming

    case 1:                                                 // Run
                measuring = map->value(INFO_isMeasuring,"0").toInt(&ok);
                if(measuring == 0) color_point = Qt::darkGreen;
                else color_point = Qt::red;

                flag_SaveImageRejime = (bool)(map->value(INFO_fmode,"0").toInt(&ok));
                //qDebug() << "measuring: " << measuring << "flag_SaveImageRejime: " << flag_SaveImageRejime;

                if(color_point == Qt::red && !timer_measuring.isActive()) timer_measuring.start(500);
                if(color_point != Qt::red && timer_measuring.isActive()) timer_measuring.stop();
                if(!timer_measuring.isActive())
                {
                    label_measuring->setVisible(false);
                    label_SaveImageRejime->setVisible(false);
                }
                break;
    }
    if(state_dev != 1 && timer_measuring.isActive())
    {
        timer_measuring.stop();
        label_measuring->setVisible(false);
        label_SaveImageRejime->setVisible(false);
    }

    if(curve->color_XY.size() >= HISTORY) curve->color_XY.removeFirst();
    curve->color_XY.append(color_point);

    curve->setSamples(XY.toVector());
    replot();

}
//-----------------------------------------------------------------------------
//--- clearPlot()
//-----------------------------------------------------------------------------
void GraphTemperaturePlot::clearPlot()
{
    base.restart();
    XY.clear();
    curve->color_XY.clear();
    curve->setSamples(NULL);
    setAxisScale(QwtPlot::xBottom,0,HISTORY);
    replot();
}

//-----------------------------------------------------------------------------
//--- slot_ChangeRejime(int)
//-----------------------------------------------------------------------------
void GraphTemperaturePlot::slot_ChangeRejime(int rejime)
{
    switch(rejime)
    {
    default:
    case 0: label_caption->setText("");
            warning_State->setText("");
            timer_warning.stop();
            warning_State->setVisible(false);
            break;
    case 1: label_caption->setText(tr("Running..."));
            warning_State->setText("");
            timer_warning.stop();
            warning_State->setVisible(false);
            break;
    case 2: label_caption->setText(tr("Running..."));
            warning_State->setText("Pause");
            warning_State->setStyleSheet("QLabel{background-color: QColor(0x99D9EA); color:yellow}");            
            timer_warning.start(500);
            break;
    case 3: label_caption->setText(tr("Running..."));
            warning_State->setText(tr("Holding"));
            warning_State->setStyleSheet("QLabel{background-color: QColor(0x99D9EA); color:darkblue}");
            timer_warning.start(500);
            break;
    case 4: label_caption->setText(tr("Running..."));
            warning_State->setText(tr("Warming up"));
            warning_State->setStyleSheet("QLabel{background-color: QColor(0x99D9EA); color:yellow}");
            timer_warning.start(500);
            break;

    }

}
//-----------------------------------------------------------------------------
//--- slot_WarningRejime()
//-----------------------------------------------------------------------------
void GraphTemperaturePlot::slot_WarningRejime()
{
    warning_State->setVisible(!warning_State->isVisible());
}

//-----------------------------------------------------------------------------
//--- slot_MeasuringRejime()
//-----------------------------------------------------------------------------
void GraphTemperaturePlot::slot_MeasuringRejime()
{
    label_measuring->setVisible(!label_measuring->isVisible());
    if(flag_SaveImageRejime) label_SaveImageRejime->setVisible(label_measuring->isVisible());
    //label_SaveImageRejime->setVisible(!label_SaveImageRejime->isVisible());
}

//-----------------------------------------------------------------------------
//--- set_Enable(bool)
//-----------------------------------------------------------------------------
void GraphTemperaturePlot::set_Enable(bool status)
{
    QColor color_bg;


    if(!status)      // Disable
    {
        color_bg = 0xF7FCFD; //0xBFE8F2;
    }
    else            // Enable
    {
        color_bg = 0xEAF8FB; //0x99D9EA;
    }

    setCanvasBackground(color_bg);
    setEnabled(status);

    replot();
}
