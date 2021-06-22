#include "pcranalysis.h"

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
PCRAnalysis::PCRAnalysis(QWidget *parent):
    QGroupBox(parent)
{
    QVBoxLayout *main_layout = new QVBoxLayout;
    main_layout->setMargin(5);
    main_layout->setSpacing(4);
    setLayout(main_layout);

    QFont f = font();

    main_spl = new QSplitter(Qt::Horizontal,this);
    main_spl->setHandleWidth(4);
    main_spl->setChildrenCollapsible(false);
    main_layout->addWidget(main_spl);

    ChartBox = new QGroupBox(this);
    GridBox = new QGroupBox(this);
    main_spl->addWidget(ChartBox);
    main_spl->addWidget(GridBox);

    //... CHARTS ...
    QVBoxLayout *chart_layout = new QVBoxLayout;
    ChartBox->setLayout(chart_layout);
    ChartBox->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Ignored);

    //... single plot ...
    Single_plot = new SinglePCRPlot(this);

    //... sigmoid plot ...
    Sigmoid_plot = new SigmoidPlot(this);


    chart_layout->addWidget(Single_plot);
    chart_layout->addWidget(Sigmoid_plot);
    //chart_layout->addStretch();

    //... GRIDS ...
    QVBoxLayout *grid_layout = new QVBoxLayout;
    GridBox->setLayout(grid_layout);
    GridBox->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Ignored);
    QHBoxLayout *grid_H_layout = new QHBoxLayout;
    grid_H_layout->setMargin(0);

    //... flour select ...
    fluor_select = new Fluor_ComboBox();
    //fluor_select->setFont(font);
    fluor_delegate = new Fluor_ItemDelegate;
    fluor_select->setItemDelegate(fluor_delegate);
    connect(fluor_select, SIGNAL(currentIndexChanged(int)), this, SLOT(Select_Fluor(int)));
    //...

    index_tube = new QLabel("A1");
    index_tube->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    index_tube->setLineWidth(2);
    index_tube->setMidLineWidth(2);

    //... Select_Grid ...    
    Select_Grid = new TableWidget(0,0);
    f.setPointSize(f.pointSize()-4);
    f.setBold(false);
    Select_Grid->setFont(f);
    //Select_Grid->setSelectionMode(QAbstractItemView::NoSelection);
    Select_Grid->setSelectionMode(QAbstractItemView::SingleSelection);
    Select_Grid->setEditTriggers(QAbstractItemView::NoEditTriggers);
    Delegate = new ItemDelegate();
    Select_Grid->setItemDelegate(Delegate);
    connect(Select_Grid, SIGNAL(cellClicked(int,int)), this, SLOT(Select_Tube(int,int)));

    //... TakeOff Points Table ...
    Table_Points = new QTableWidget(0,0);
    Table_Points->setSelectionBehavior(QAbstractItemView::SelectRows);
    Table_Points->setSelectionMode(QAbstractItemView::SingleSelection);
    Table_Points->setEditTriggers(QAbstractItemView::NoEditTriggers);
    toff_Delegate = new TableTOFF_ItemDelegate();
    Table_Points->setItemDelegate(toff_Delegate);
    connect(Table_Points, SIGNAL(itemSelectionChanged()), this, SLOT(Select_PointsTOFF()));

    grid_H_layout->addWidget(fluor_select,0,Qt::AlignLeft);
    grid_H_layout->addWidget(index_tube,0,Qt::AlignRight);

    grid_layout->addLayout(grid_H_layout);
    grid_layout->addWidget(Select_Grid);
    grid_layout->addWidget(Table_Points);

    //grid_layout->addStretch();

    prot = NULL;
    sigmoid = new Sigmoid();    
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
PCRAnalysis::~PCRAnalysis()
{
    delete fluor_select;
    delete index_tube;
    delete Select_Grid;
    delete Table_Points;

    delete Single_plot;
    delete Sigmoid_plot;

    delete sigmoid;

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void PCRAnalysis::Draw_SigmoidPlot(int pos, int channel, int num_ptoff)
{
    int i,j;
    int count_meas;
    int count_tubes;
    double *x, *y;
    double value;
    int ch = -1;
    POINT_TAKEOFF *pp_OFF;

    QVector<double> X,Y,S;
    QVector<QPointF> P;
    QPointF point_Cp;

    if(!prot) return;
    if(!prot->count_PCR) return;

    //--- active channel ---
    for(i=0; i<=channel; i++)
    {
        if(!(prot->active_Channels & (0x0f<<4*i))) continue;
        ch++;
    }
    if(ch < 0) return;

    // clear curve
    Sigmoid_plot->curveXY_Cp->setData(NULL);
    Sigmoid_plot->curve_real->setData(NULL);
    Sigmoid_plot->curve_spline->setData(NULL);
    Sigmoid_plot->curve_sigmoid->setData(NULL);
    Sigmoid_plot->curve_sigmoid->bounds_Sigmoid = 0;

    // load data
    count_meas = prot->count_PCR;
    count_tubes = prot->count_Tubes;

    j = pos*count_meas + count_meas*count_tubes*ch;

    x = &prot->X_PCR.at(0);
    y = &prot->PCR_Filtered.at(j);

    X = QVector<double>::fromStdVector(prot->X_PCR);

    // ...
    if(num_ptoff < 0) Y.fill(0,count_meas);
    else
    {
        Y = QVector<double>::fromStdVector(prot->PCR_Filtered).mid(j,count_meas);
        pp_OFF = sigmoid->list_pointOFF.at(num_ptoff);
        for(i=0; i<count_meas; i++)
        {
            if(i < pp_OFF->ct_Sigmoid) value = 0.;
            else value = Y.at(i) - (pp_OFF->c0 + pp_OFF->c1*i);
            Y.replace(i,value);
            S.append(pp_OFF->A_Sigmoid/(1. + exp((pp_OFF->x0_Sigmoid - i)/pp_OFF->b_Sigmoid)));
        }
        point_Cp.setX(pp_OFF->cp_Sigmoid);
        point_Cp.setY(pp_OFF->Fluor_Cp);
        P.append(point_Cp);
        Sigmoid_plot->curveXY_Cp->setSamples(P);
        //Sigmoid_plot->curve_sigmoid->bounds_Sigmoid = pp_OFF->ct_Sigmoid + sigmoid->preference.

    }
    Sigmoid_plot->curve_spline->setSamples(X,Y);
    Sigmoid_plot->curve_real->setSamples(X,Y);
    Sigmoid_plot->curve_sigmoid->setSamples(X,S);
    //...

    Sigmoid_plot->setAxisAutoScale(QwtPlot::yLeft, true);
    Sigmoid_plot->setAxisAutoScale(QwtPlot::xBottom, false);
    Sigmoid_plot->setAxisScale(QwtPlot::xBottom,x[0]-1,x[count_meas-1]+1);


    Sigmoid_plot->updateAxes();
    Sigmoid_plot->show();
    Sigmoid_plot->replot();
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void PCRAnalysis::Draw_SinglePlot(int pos, int channel)
{
    int i,j;
    int id;
    double *x;
    double *y;
    int count_meas;
    int count_tubes;
    int ch = -1;
    POINT_TAKEOFF *pp_OFF;

    QVector<double> X,Y,S;

    if(!prot) return;
    if(!prot->count_PCR) return;

    //
    Criterion_Test *criterion;
    rt_Test *ptest;    

    //--- active channel ---
    for(i=0; i<=channel; i++)
    {
        if(!(prot->active_Channels & (0x0f<<4*i))) continue;
        ch++;
    }
    if(ch < 0) return;

    // clear curve
    Single_plot->curve->setData(NULL);

    // load data
    count_meas = prot->count_PCR;
    count_tubes = prot->count_Tubes;

    j = pos*count_meas + count_meas*count_tubes*ch;

    x = &prot->X_PCR.at(0);
    y = &prot->PCR_Filtered.at(j);
    //Single_plot->curve->setSamples()(x, y, count_meas);

    Y = QVector<double>::fromStdVector(prot->PCR_Filtered).mid(j,count_meas);
    X = QVector<double>::fromStdVector(prot->X_PCR);
    Single_plot->curve->setSamples(X,Y);

    Single_plot->setAxisAutoScale(QwtPlot::yLeft, true);
    Single_plot->setAxisAutoScale(QwtPlot::xBottom, false);
    Single_plot->setAxisScale(QwtPlot::xBottom,x[0]-1,x[count_meas-1]+1);

    // Point Take_Off
    ptest = prot->Plate_Tests.at(div(i,prot->count_Tubes).rem);
    for(i=0; i<prot->Criterion.size(); i++)
    {
        criterion = prot->Criterion.at(i);
        if(criterion->ptest == ptest) break;
    }

    Single_plot->curve->points_Toff.fill(0,count_meas);
    Analysis_PCR_Ex(Y,S,sigmoid,criterion->criterion_PosResult);
    for(i=0; i<sigmoid->list_pointOFF.size(); i++)
    {
        pp_OFF = sigmoid->list_pointOFF.at(i);
        id = pp_OFF->ct_Sigmoid;
        Single_plot->curve->points_Toff.replace(id,1);
    }
    id = sigmoid->best_pointOFF;

    if(id >= 0)
    {
        id = sigmoid->list_pointOFF.at(id)->ct_Sigmoid;
        Single_plot->curve->points_Toff.replace(id,2);        
    }

    // Clear Criterion:
    /*foreach (criterion, Criterion)
    {
        delete criterion;
    }
    Criterion.clear();
    PCR_Tests.clear();*/

    Single_plot->updateAxes();
    Single_plot->show();
    Single_plot->replot();
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void PCRAnalysis::Fill_PCRAnalysis(rt_Protocol *p)
{
    int i,j;
    QIcon icon;
    QString fluor_name[COUNT_CH] = FLUOR_NAME;
    int first_ch = -1;    

    prot = p;
    if(!p) return;

    // 1. fluor_select ...
    fluor_select->blockSignals(true);
    fluor_select->clear();

    for(i=0; i<COUNT_CH; i++)
    {
       fluor_select->addItem(fluor_name[i]);
       switch(i)
       {
       case 0:  icon.addFile(":/images/flat/fam_flat.png");   break;
       case 1:  icon.addFile(":/images/flat/hex_flat.png");   break;
       case 2:  icon.addFile(":/images/flat/rox_flat.png");   break;
       case 3:  icon.addFile(":/images/flat/cy5_flat.png");   break;
       case 4:  icon.addFile(":/images/flat/cy55_flat.png");   break;
       default: icon.addFile(NULL);   break;
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

    //qDebug() << "Fill_PCRAnalysis: " << first_ch;

    fluor_select->blockSignals(false);
    fluor_select->setCurrentIndex(first_ch);

    //qDebug() << "fluor_select->setCurrentIndex(first_ch): ";

    // 2. Select_Grid ...
    Fill_GridResults(p);
    //qDebug() << "Fill_GridResults: ";

    // 3. Draw Charts (Single_Plot) ...
    Draw_SinglePlot(0,first_ch);
    //qDebug() << "Draw_SinglePlot: ";

    // 4. Draw Charts (Sigmoid_Plot) ...
    Draw_SigmoidPlot(0,first_ch,sigmoid->best_pointOFF);
    //qDebug() << "Draw_SigmoidPlot: ";

    // 5. Table_Points ...
    Fill_TablePOFF();
    //qDebug() << "Fill_TablePOFF: ";

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void PCRAnalysis::Fill_GridResults(rt_Protocol *p, int pos)
{
    int i,j;
    int col,row,id;
    QStringList header;
    QString text;
    QTableWidgetItem *newItem;
    int count_tubes = p->count_Tubes;
    p->Plate.PlateSize(count_tubes,row,col);

    //--- active channel ---
    int ch = -1;
    for(i=0; i<=fluor_select->currentIndex(); i++)
    {
        if(!(p->active_Channels & (0x0f<<4*i))) continue;
        ch++;
    }
    if(ch < 0) return;

    //qDebug() << "Fill_GridResults channel:" << ch;
    //Delegate->get_prot(p);

    Select_Grid->clear();
    Select_Grid->setColumnCount(col);
    Select_Grid->setRowCount(row);
    for(i=0; i<col; i++) header.append(QString::number(i+1));   // Column Header header.append("");
    //Select_Grid->setHorizontalHeaderLabels(header);
    header.clear();
    for(i=0; i<row; i++) header.append(QChar(0x41 + i));        // Row Header header.append("");
    Select_Grid->setVerticalHeaderLabels(header);
    header.clear();
    for(i=0; i<col; i++) Select_Grid->horizontalHeader()->setSectionResizeMode(i,QHeaderView::Stretch);
    for(i=0; i<row; i++) Select_Grid->verticalHeader()->setSectionResizeMode(i,QHeaderView::Stretch);
    //Select_Grid->horizontalHeader()->setSectionResizeMode(0,QHeaderView::Fixed);
    //Select_Grid->setColumnWidth(0,20);

    //qDebug() << "list_PointsOff: " << p->list_PointsOff.size();

    for(i=0; i<row; i++)
    {
        for(j=0; j<col; j++)
        {
            newItem = new QTableWidgetItem();
            id = j + i*col;
            POINT_TAKEOFF *pptof = p->list_PointsOff.at(id+count_tubes*ch);
            if(pptof->valid) text = "1";
            else text = "";
            //text = "1";
            newItem->setText(text);
            newItem->setFlags(newItem->flags() & ~Qt::ItemIsEditable);
            Select_Grid->setItem(i,j,newItem);
        }
    }

    i = div(pos,col).quot;
    j = div(pos,col).rem;
    Select_Grid->setCurrentCell(i,j);


    //Select_Grid->horizontalHeader()->hide();
    //Select_Grid->verticalHeader()->hide();
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void PCRAnalysis::Fill_TablePOFF()
{
    int i,j;
    int col,row;
    QStringList header;
    QTableWidgetItem *newItem;
    POINT_TAKEOFF *pp_OFF;
    QString text;
    double value;

    //...
    int index = Convert_NameToIndex(index_tube->text(), Select_Grid->columnCount());
    int ch = -1;
    for(i=0; i<=fluor_select->currentIndex(); i++)
    {
        if(!(prot->active_Channels & (0x0f<<4*i))) continue;
        ch++;
    }
    if(ch < 0) return;
    int pos = prot->count_Tubes*ch + index;
    SPLINE_CP *P_Spline = prot->list_SplineCp.at(pos);
    //...

    Table_Points->blockSignals(true);

    toff_Delegate->get_sigmoid(sigmoid);

    Table_Points->clear();
    header.clear();
    header << "Cp" << "F(Cp)" << "chi^2" << "sigma" << "eff" << "Min" ;
    row = sigmoid->list_pointOFF.size();
    col = header.size();
    Table_Points->setColumnCount(col);
    Table_Points->setRowCount(row);

    Table_Points->setHorizontalHeaderLabels(header);
    Table_Points->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    Table_Points->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    for(i=0; i<row; i++)
    {
        pp_OFF = sigmoid->list_pointOFF.at(i);

        for(j=0; j<col; j++)
        {
            switch(j)
            {
            default:    text = "";  break;

            case 0:     // Cp
                        value = pp_OFF->cp_Sigmoid;
                        if(value > 0)
                        {
                            text = QString::number(value,'f',1);
                            if(i == sigmoid->best_pointOFF) text += QString(" (%1)").arg(P_Spline->Cp,0,'f',1);
                        }
                        else text = "-";                        
                        break;

            case 1:     // F(Cp)
                        value = pp_OFF->Fluor_Cp;
                        if(value > 0)
                        {
                            text = QString::number(value,'f',0);
                            if(i == sigmoid->best_pointOFF) text += QString(" (%1)").arg(P_Spline->F_Cp,0,'f',0);
                        }
                        else text = "-";

                        break;

            case 2:     // chi^2
                        value = pp_OFF->chi_Sigmoid;
                        text = QString::number(value,'f',1);
                        break;

            case 3:     // sigma
                        value = pp_OFF->sigma;
                        text = QString::number(value,'f',2);
                        break;

            case 4:     // eff
                        value = pp_OFF->e0_Sigmoid;
                        text = QString::number(value,'f',2);
                        break;

            case 5:     // Min
                        value = pp_OFF->Criteria;
                        text = QString::number(value,'f',2);
                        break;

            }

            newItem = new QTableWidgetItem();
            newItem->setText(text);
            newItem->setFlags(newItem->flags() & ~Qt::ItemIsEditable);
            Table_Points->setItem(i,j,newItem);
        }
    }
    i = sigmoid->best_pointOFF;
    if(i >= 0 && i<row) Table_Points->setCurrentCell(i,0);
    Table_Points->blockSignals(false);

    //Table_Points->resizeColumnsToContents();
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void PCRAnalysis::Select_Tube(int row, int col)
{
    int count_col = Select_Grid->columnCount();
    int pos = row*count_col + col;
    int ch = fluor_select->currentIndex();

    if(row < 0 || col < 0) pos = 0;

    QString text = Convert_IndexToName(pos, count_col);
    index_tube->setText(text);

    //qDebug() << "Select_Tube: " << row << col << text << ch;

    // 1. Draw Charts (Single_Plot) ...
    Draw_SinglePlot(pos,ch);

    // 2. Draw Charts (Sigmoid_Plot) ...    
    Draw_SigmoidPlot(pos,ch,sigmoid->best_pointOFF);

    // 3. Table_Points ...
    Fill_TablePOFF();
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void PCRAnalysis::Select_Fluor(int ch)
{
    int row = Select_Grid->currentRow();
    int col = Select_Grid->currentColumn();
    int pos = col + row*Select_Grid->columnCount();

    // 1. Select_Grid
    Fill_GridResults(prot, pos);

    // 2. Select tube
    Select_Tube(row, col);

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void PCRAnalysis::Select_PointsTOFF()
{
    int i;
    int id;
    short value;

    int row = Table_Points->currentRow();
    int col = Table_Points->currentColumn();

    int ch = fluor_select->currentIndex();
    int pos_col = Select_Grid->currentColumn();
    int pos_row = Select_Grid->currentRow();
    int pos = pos_col + pos_row*Select_Grid->columnCount();

    int count_meas = prot->count_PCR;

    //qDebug() << row << col;

    // 1. Replot SinglePCRCurve
    SinglePCRCurve *curve = Single_plot->curve;
    for(i=0; i<curve->points_Toff.size(); i++)
    {
        value = curve->points_Toff.at(i);
        if(value) curve->points_Toff.replace(i,1);
    }
    id = sigmoid->list_pointOFF.at(row)->ct_Sigmoid;
    curve->points_Toff.replace(id,2);
    Single_plot->replot();

    // 2. Draw_SigmoidPlot
    Draw_SigmoidPlot(pos, ch, row);
}

//-----------------------------------------------------------------------------
//--- Fluor_ItemDelegate::paint
//-----------------------------------------------------------------------------
void Fluor_ItemDelegate::paint(QPainter *painter,
                               const QStyleOptionViewItem &option,
                               const QModelIndex &index) const
{
    QRect rect = option.rect;
    QString text = index.data().toString();
    //QFont font;
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
    //painter->setFont(QFont("Comic Sans MS", 10, QFont::Bold));

    if(option.state & QStyle::State_Enabled)
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
        painter->setPen(QPen(QColor(192,192,192) ,1,Qt::SolidLine));
        pixmap.load(":/images/flat/disable_flat.png");
    }

    painter->drawText(rect, Qt::AlignHCenter | Qt::AlignVCenter, text);
    painter->drawPixmap(0, rect.y()+2, pixmap);
}
//-----------------------------------------------------------------------------
//--- TableTOFF_ItemDelegate::paint (Select_Grid)
//-----------------------------------------------------------------------------
void TableTOFF_ItemDelegate::paint(QPainter *painter,
                         const QStyleOptionViewItem &option,
                         const QModelIndex &index) const
{
    int i,id;
    QRect rect;
    QString text;
    QFont font;

    QPalette pal = option.palette;
    QStyleOptionViewItem  viewOption(option);
    viewOption.state &= ~QStyle::State_HasFocus;    // disable state HasFocus
    QStyledItemDelegate::paint(painter, viewOption, index);

    rect = option.rect;

    text = index.data().toString();

    //----------------------

    //__1. Background
    if(option.state & QStyle::State_Selected)
    {
        painter->fillRect(option.rect, QColor(215,215,255));
    }
    else
    {
        painter->fillRect(rect, QColor(255,255,255));
    }
    painter->setPen(QPen(Qt::black,1,Qt::SolidLine));

    font = painter->font();
    font.setBold(false);
    painter->setFont(font);
    if(index.row() == sigmoid->best_pointOFF)
    {
        painter->fillRect(option.rect, QColor(255,128,128));
        painter->setPen(QPen(Qt::white,1,Qt::SolidLine));
        //font.setBold(true);
        painter->setFont(font);
    }

    //__2. Data



    painter->drawText(rect, Qt::AlignCenter | Qt::AlignVCenter, text);


}
//-----------------------------------------------------------------------------
//--- ItemDelegate::paint (Select_Grid)
//-----------------------------------------------------------------------------
void ItemDelegate::paint(QPainter *painter,
                         const QStyleOptionViewItem &option,
                         const QModelIndex &index) const
{
    QColor color,rgb;
    int pos;
    int r,g,b;
    QString text;
    QColor color_plus = QColor(255,128,128);
    QColor color_minus = QColor(128,255,128);


    QPalette pal = option.palette;
    QStyleOptionViewItem  viewOption(option);
    viewOption.state &= ~QStyle::State_HasFocus;    // disable state HasFocus
    QStyledItemDelegate::paint(painter, viewOption, index);

    //__1. Background
    painter->fillRect(option.rect, QColor(255,255,255));

    //__2. Enable Tube
    pos = index.column() + index.row() * index.model()->columnCount();
    text = index.data().toString();

    if(text == "1") color = color_plus; //Qt::red;
    else color = color_minus; //Qt::green;
    //painter->setBrush(QBrush(color, Qt::SolidPattern));
    painter->fillRect(option.rect,color);
    if(viewOption.state & QStyle::State_Selected)
    {
        //painter->setPen(QPen(Qt::black,4,Qt::SolidLine));
        //painter->drawRect(option.rect);
        //color.getRgb(&r,&g,&b);
        //color = QColor(r,g,b,150);
        if(color == color_minus) color = Qt::darkGreen;
        if(color == color_plus) color = Qt::darkRed;
        painter->fillRect(option.rect,color);
    }
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
SinglePCRPlot::SinglePCRPlot(QWidget *parent):
    QwtPlot(parent)
{
    canvas()->setCursor(Qt::PointingHandCursor);

    curve = new SinglePCRCurve();
    curve->setStyle(QwtPlotCurve::Lines);
    curve->setSymbol(new QwtSymbol(QwtSymbol::Ellipse,
                                   QBrush(Qt::white),
                                   QPen(Qt::black, 1),
                                   QSize(2,2)));
    curve->setPen(Qt::red,1);
    curve->setCurveAttribute(QwtPlotCurve::Fitted, false);
    curve->attach(this);


    QFont f = qApp->font();
    f.setBold(false);
    f.setPointSize(f.pointSize()-1);
    setAxisFont(QwtPlot::yLeft, f);
    setAxisFont(QwtPlot::xBottom, f);

    grid = new QwtPlotGrid;
    grid->setMajorPen(QPen(Qt::gray, 0, Qt::DotLine));
    grid->attach(this);

    label_caption = new QLabel(tr("TakeOFF points"), this);
    label_caption->setAlignment(Qt::AlignLeft);
    QHBoxLayout *graph_H_layout = new QHBoxLayout();
    QVBoxLayout *graph_V_layout = new QVBoxLayout();
    graph_H_layout->setMargin(0);
    graph_V_layout->setMargin(2);
    graph_H_layout->addWidget(label_caption,0,Qt::AlignLeft);
    graph_V_layout->addLayout(graph_H_layout,0);
    graph_V_layout->addStretch();
    canvas()->setLayout(graph_V_layout);
    f = qApp->font();
    //f.setBold(true);
    label_caption->setFont(f);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
SinglePCRPlot::~SinglePCRPlot()
{
    delete grid;
    delete curve;
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
SigmoidPlot::SigmoidPlot(QWidget *parent):
    QwtPlot(parent)
{
    canvas()->setCursor(Qt::PointingHandCursor);

    curve_real = new QwtPlotCurve();
    curve_real->setStyle(QwtPlotCurve::Dots);
    curve_real->setSymbol(new QwtSymbol(QwtSymbol::Ellipse,
                                   QBrush(Qt::white),
                                   QPen(Qt::black, 1),
                                   QSize(4,4)));
    //curve_real->setPen(Qt::red,1);
    curve_real->setCurveAttribute(QwtPlotCurve::Fitted, false);
    curve_real->attach(this);

    curve_spline = new QwtPlotCurve();
    curve_spline->setStyle(QwtPlotCurve::Lines);
    curve_spline->setPen(Qt::green,1);
    curve_spline->setCurveAttribute(QwtPlotCurve::Fitted, true);
    curve_spline->attach(this);

    curve_sigmoid = new SigmoidPCRCurve();
    curve_sigmoid->setStyle(QwtPlotCurve::Lines);
    curve_sigmoid->bounds_Sigmoid = 0;
    //curve_sigmoid->setPen(Qt::red,2);
    curve_sigmoid->setCurveAttribute(QwtPlotCurve::Fitted, true);
    curve_sigmoid->attach(this);

    curveXY_Cp = new QwtPlotCurve();
    curveXY_Cp->setStyle(QwtPlotCurve::Dots);
    curveXY_Cp->setSymbol(new QwtSymbol(QwtSymbol::Cross,
                                   QBrush(Qt::white),
                                   QPen(Qt::red, 1),
                                   QSize(22,22)));
    curveXY_Cp->attach(this);

    QFont f = qApp->font();
    f.setBold(false);
    f.setPointSize(f.pointSize()-1);
    setAxisFont(QwtPlot::yLeft, f);
    setAxisFont(QwtPlot::xBottom, f);

    grid = new QwtPlotGrid;
    grid->setMajorPen(QPen(Qt::gray, 0, Qt::DotLine));
    grid->attach(this);

    label_caption = new QLabel(tr("Sigmoid fitting"));
    label_caption->setAlignment(Qt::AlignLeft);
    QHBoxLayout *graph_H_layout = new QHBoxLayout();
    QVBoxLayout *graph_V_layout = new QVBoxLayout();
    graph_H_layout->setMargin(0);
    graph_V_layout->setMargin(2);
    graph_H_layout->addWidget(label_caption,0,Qt::AlignLeft);
    graph_V_layout->addLayout(graph_H_layout,0);
    graph_V_layout->addStretch();
    canvas()->setLayout(graph_V_layout);
    f = qApp->font();
    //f.setBold(true);
    canvas()->setFont(f);

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
SigmoidPlot::~SigmoidPlot()
{
    delete grid;
    delete curve_real;
    delete curve_spline;
    delete curve_sigmoid;
    delete curveXY_Cp;
    delete label_caption;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
/*void TableWidget::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
}*/
