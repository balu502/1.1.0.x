#include "graph_fluor.h"


//-----------------------------------------------------------------------------
//--- GraphFluorPlot()
//-----------------------------------------------------------------------------
GraphFluorPlot::GraphFluorPlot(QWidget *parent):
    QwtPlot(parent)
{        
    int i;

    readCommonSettings();
    setFont(qApp->font());

    main_widget = parent;    
    prot = NULL;
    size_symbol = 2;

    canvas()->setCursor(Qt::PointingHandCursor);
    canvas()->installEventFilter(this);
    canvas()->setMouseTracking(true);

    grid = new QwtPlotGrid;
    grid->setMajorPen(QPen(Qt::gray, 0, Qt::DotLine));
    grid->attach(this);

    QFont f = qApp->font();
    f.setBold(false);
    f.setPointSize(f.pointSize()-1);
    setAxisFont(QwtPlot::yLeft, f);
    setAxisFont(QwtPlot::xBottom, f);

    //... flour select ...
    fluor_select = new Fluor_ComboBox();
    fluor_select->setVisible(false);    
    //fluor_select->setFont(font);
    fluor_delegate = new Fluor_ItemDelegate;
    fluor_select->setItemDelegate(fluor_delegate);
    fluor_delegate->style = StyleApp;
    //...

    label_curve = new QLabel("pp", canvas());
    label_curve->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    label_curve->setAutoFillBackground(true);
    label_curve->setAlignment(Qt::AlignCenter);
    label_curve->setFixedWidth(50);
    label_curve->setVisible(false);
    //...

    label_caption = new QLabel(tr("Fluorescence"));
    //label_caption->setFont(font);
    label_caption->setAlignment(Qt::AlignLeft);
    //...
    label_fn = new QLabel("");
    f.setFamily("Times New Roman");
    f.setPointSize(18);
    f.setBold(true);
    f.setItalic(false);
    label_fn->setFont(f);
    label_fn->setAlignment(Qt::AlignLeft);
    label_fn->setFrameShape(QFrame::StyledPanel);
    label_fn->setStyleSheet("QLabel {background-color: #88FAFAFA; color: black; border: 1px solid gray;}");
    QHBoxLayout *graph_Hf_layout = new QHBoxLayout();
    graph_Hf_layout->addWidget(label_fn,0,Qt::AlignLeft);


    QHBoxLayout *graph_H_layout = new QHBoxLayout();
    QVBoxLayout *graph_V_layout = new QVBoxLayout();
    graph_H_layout->setMargin(0);
    //graph_V_layout->setMargin(2);

    graph_H_layout->addWidget(label_caption,0,Qt::AlignLeft);
    graph_H_layout->addWidget(fluor_select,0,Qt::AlignRight);

    graph_V_layout->addLayout(graph_H_layout,0);
    graph_V_layout->addStretch(1);
    graph_V_layout->addLayout(graph_Hf_layout);

    canvas()->setLayout(graph_V_layout);    

    connect(fluor_select,SIGNAL(currentIndexChanged(int)), this,SLOT(change_Fluor(int)));

}
//-----------------------------------------------------------------------------
//--- ~GraphFluorPlot()
//-----------------------------------------------------------------------------
GraphFluorPlot::~GraphFluorPlot()
{
    delete label_curve;
    delete label_caption;
    delete label_fn;
    X.clear();
    Y.clear();
    Delete_Curve();
}
//-----------------------------------------------------------------------------
//--- readCommonSettings()
//-----------------------------------------------------------------------------
void GraphFluorPlot::readCommonSettings()
{
    QString text;
    QString dir_path = qApp->applicationDirPath();
    QSettings *CommonSettings = new QSettings(dir_path + "/tools/ini/preference.ini", QSettings::IniFormat);

    CommonSettings->beginGroup("Common");

        //... Language ...
        text = CommonSettings->value("language","ru").toString();
        if(translator.load(":/translations/graph_fluor_" + text + ".qm"))
        {
            qApp->installTranslator(&translator);
        }
        //... Style ...
        text = CommonSettings->value("style","xp").toString();
        StyleApp = text;

    CommonSettings->endGroup();
    delete CommonSettings;
}

//-----------------------------------------------------------------------------
//---  Change_SizeMarker()
//-----------------------------------------------------------------------------
void GraphFluorPlot::Change_SizeMarker()
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
//--- Create_Curve
//-----------------------------------------------------------------------------
void GraphFluorPlot::Create_Curve(rt_Protocol *P)
{
    int i,j;
    QIcon icon;
    QString fluor_name[COUNT_CH] = FLUOR_NAME;
    int first_ch = -1;

    prot = P;

    Delete_Curve();

    // 1. curves
    /*QwtSymbol *symbol = new QwtSymbol(QwtSymbol::Ellipse,
                                      QBrush(Qt::black),
                                      QPen(Qt::black, 1),
                                      QSize( 1, 1));*/
    for(i=0; i<prot->count_Tubes; i++)
    {
        curve = new QwtPlotCurve();
        curve->setStyle(QwtPlotCurve::Lines);
        //curve->setSymbol(symbol);
        curve->setSymbol( new QwtSymbol(QwtSymbol::Ellipse,
                                        QBrush(Qt::white),
                                        QPen(Qt::black, 1),
                                        QSize(size_symbol,size_symbol)));
        curve->attach(this);
        list_Curve.append(curve);
    }


    // 2. fluor_select
    fluor_select->blockSignals(true);
    for(i=0; i<COUNT_CH; i++)
    {
       fluor_select->addItem(fluor_name[i]);
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

       fluor_select->setItemIcon(i,icon);

       if(!(prot->active_Channels & (0x0f<<i*4)))
       {
          fluor_select->setItemData(i,0,Qt::UserRole - 1);
       }
       else
       {
           if(first_ch < 0) first_ch = i;
       }

    }
    fluor_select->blockSignals(false);
    fluor_select->setVisible(true);
    fluor_select->setCurrentIndex(first_ch);

    //...
    Draw_Chart(prot,first_ch);
    //...

}
//-----------------------------------------------------------------------------
//--- Delete_Curve
//-----------------------------------------------------------------------------
void GraphFluorPlot::Delete_Curve()
{
    qDeleteAll(list_Curve.begin(), list_Curve.end());
    list_Curve.clear();

    fluor_select->blockSignals(true);
    fluor_select->clear();
    fluor_select->blockSignals(false);

    replot();
}
//-----------------------------------------------------------------------------
//--- Clear_Graph
//-----------------------------------------------------------------------------
void GraphFluorPlot::Clear_Graph()
{
    int i;

    for(i=0; i<list_Curve.count(); i++)
    {
        curve = list_Curve.at(i);
        curve->setData(NULL);
    }

    replot();
}
//-----------------------------------------------------------------------------
//--- slot_NewProtocol
//-----------------------------------------------------------------------------
void GraphFluorPlot::slot_NewProtocol(rt_Protocol *prot)
{
    int i;
    int count_ch = 0;
    int all_measure = prot->count_PCR + prot->count_MC;

    for(i=0; i<COUNT_CH; i++) if(prot->active_Channels & (0x0f<<(i*4))) count_ch++;
    current_fn = 0;
    X.clear();
    X.reserve(all_measure);
    for(i=0; i<all_measure; i++) X.push_back((double)(i+1));
    Y.clear();
    Y.reserve(all_measure*count_ch*prot->count_Tubes);
    Y.fill(0.,all_measure*count_ch*prot->count_Tubes);

    Create_Curve(prot);

    replot();
}

//-----------------------------------------------------------------------------
//--- change_Fluor
//-----------------------------------------------------------------------------
void GraphFluorPlot::change_Fluor(int ch)
{
    if(fluor_select->hasFocus()) fluor_select->clearFocus();
    if(ch >= 0) Draw_Chart(prot, ch);
    canvas()->setCursor(Qt::PointingHandCursor);
}

//-----------------------------------------------------------------------------
//--- change_Fluor
//-----------------------------------------------------------------------------
void GraphFluorPlot::Draw_Chart(rt_Protocol *p, int channel)
{
    int i,j;
    QColor color;
    //int r,g,b;
    QColor bg_color[] ={0xD8E2F1,0xD8E9D8,0xFEF0CB,0xFFE6FF,0xFFE6FF};
    QColor curve_color[] = {Qt::blue, Qt::green, Qt::magenta, Qt::red, Qt::darkRed};
    const double *x;
    const double *y;
    int ch = -1;

    int all_measure = p->count_PCR + p->count_MC;
    int count_tubes = p->count_Tubes;

    //qDebug() << "all_meas: " << all_measure << p->count_PCR << p->count_MC;

    //--- active channel ---
    for(i=0; i<=channel; i++)
    {
        if(!(p->active_Channels & (0x0f<<4*i))) continue;
        ch++;
    }
    if(ch < 0) return;

    Clear_Graph();

    setCanvasBackground(bg_color[channel]);
    setAxisAutoScale(QwtPlot::yLeft, true);
    setAxisScale(QwtPlot::xBottom,0,all_measure+1);

    if(current_fn)
    {
        x = &X.at(0);
        y = &Y.at(ch*count_tubes*all_measure);

        for(i=0; i<count_tubes; i++)
        {
            curve = list_Curve.at(i);
            color = p->color_tube.at(i);
            curve->setPen(color, 1 );
            curve->setSamples(x, y+all_measure*i, current_fn);

            if(p->enable_tube.at(i)) curve->setVisible(true);
            else curve->setVisible(false);
        }
        label_fn->setText(QString("fn=%1").arg(current_fn));
    }
    else label_fn->setText("");


    //...
    updateAxes();
    show();
    replot();
}
//-----------------------------------------------------------------------------
//--- slot_RefreshFluor()
//-----------------------------------------------------------------------------
void GraphFluorPlot::slot_RefreshFluor()
{
    Draw_Chart(prot,fluor_select->currentIndex());
}
//-----------------------------------------------------------------------------
//--- slot_RefreshFluor()
//-----------------------------------------------------------------------------
void GraphFluorPlot::Change_Enable(rt_Protocol *p, QVector<short> *buf)
{
    int i,j, id;
    int count = buf->size();

    for(i=0; i<count; i++)
    {
        id = buf->at(i);
        curve = list_Curve.at(id);
        if(p->enable_tube.at(id)) curve->setVisible(true);
        else                      curve->setVisible(false);
    }
    replot();
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

    painter->setFont(font);

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
    //painter->setFont(QFont("Comic Sans MS", 10, QFont::Bold));

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


    painter->drawPixmap(0, rect.y()+2, pixmap);

    //qDebug() << "row = " << index.row();
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
bool GraphFluorPlot::eventFilter(QObject *object, QEvent *e)
{

    if(object != (QObject *)canvas()) return(false);

    switch(e->type())
    {
    case    QEvent::MouseButtonPress:
            if(((QMouseEvent *)e)->buttons() & Qt::LeftButton)
            {
                if(!prot) break;

                select_GraphCurve(((QMouseEvent *)e)->pos());
                QTimer::singleShot(1000, this, SLOT(Hide_LabelCurve()));
            }
            break;

    default:    break;
    }

    return(false);
}
//-----------------------------------------------------------------------------
//--- select_GraphCurve
//-----------------------------------------------------------------------------
void GraphFluorPlot::select_GraphCurve(const QPoint &pos)
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


    for(i=0; i<prot->count_Tubes; i++)
    {
        if(!prot->enable_tube.at(i)) continue;

        curve = list_Curve.at(i);
        size = curve->dataSize();
        id = curve->closestPoint(pos, &d);

        pen = curve->pen();
        pen.setWidth(1);
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


    if(index >= 0 && dist < 10)     // 10 pixels tolerance
    {
        curve = list_Curve.at(index);
        pen = curve->pen();
        pen.setWidth(3);
        curve->setPen(pen);

        prot->Plate.PlateSize(prot->count_Tubes, row, col);
        text = Convert_IndexToName(index, col);

        label_curve->setText(text);
        pos_before = pos;
        pos_before.setY(pos.y()-label_curve->height()-2);
        label_curve->move(pos_before);
        label_curve->setVisible(true);

    }
    replot();

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
double GraphFluorPlot::Calculate_closestDistance(QPoint pos, QPoint pos_before, QPoint pos_after)
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
