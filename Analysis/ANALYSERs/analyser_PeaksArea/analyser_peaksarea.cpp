#include "analyser_peaksarea.h"

extern "C" ANALYSER_PEAKSAREASHARED_EXPORT Analysis_Interface* __stdcall createAnalyser_plugin()
{
    return(new Analyser_PeaksArea());
}


//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Analyser_PeaksArea::Analyser_PeaksArea()
{
    MainBox = NULL;
    ru_Lang = false;
    readCommonSettings();

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void* Analyser_PeaksArea::Create_Win(void *pobj, void *main)
{
    int i;    
    QFont f = qApp->font();
    QFont f_big = qApp->font();
    f_big.setPointSize(f.pointSize()+3);
    f_big.setBold(true);

    QWidget *parent = (QWidget*)pobj;
    main_widget = (QWidget*)main;    

    MainBox = new QGroupBox(parent);
    MainBox->setFont(f);
    MainBox->setObjectName("Transparent");
    QVBoxLayout *main_layout = new QVBoxLayout;
    main_layout->setMargin(0);
    main_layout->setSpacing(4);
    MainBox->setLayout(main_layout);

    main_spl = new QSplitter(Qt::Horizontal, MainBox);
    main_spl->setHandleWidth(10);
    main_spl->setChildrenCollapsible(false);
    main_layout->addWidget(main_spl);

    plot_Peaks = new QwtPlot(MainBox);
    f.setPointSize(f.pointSize() - 1);
    plot_Peaks->setAxisFont(QwtPlot::yLeft, f);
    plot_Peaks->setAxisFont(QwtPlot::xBottom, f);
    name_Tube = new QLabel();
    name_Tube->setFont(f_big);
    name_Tube->setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
    Mode_Box = new QComboBox();
    Mode_Box->addItem(tr("Null Line"));
    Mode_Box->addItem(tr("Manual mode"));

    QHBoxLayout *layout_plot = new QHBoxLayout;
    //layout_plot->addStretch(1);
    layout_plot->addWidget(Mode_Box, 0, Qt::AlignTop | Qt::AlignLeft);
    layout_plot->addWidget(name_Tube, 0, Qt::AlignTop | Qt::AlignRight);
    plot_Peaks->canvas()->setLayout(layout_plot);
    plot_Peaks->canvas()->setCursor(Qt::PointingHandCursor);
    plot_Peaks->canvas()->installEventFilter(this);
    plot_Peaks->canvas()->setMouseTracking(true);

    QwtText text_x, text_y;
    text_x.setFont(f);
    text_y.setFont(f);
    text_x.setText(tr("Temperature,°C"));
    text_y.setText("dF/dT");
    plot_Peaks->setAxisTitle(QwtPlot::xBottom, text_x);
    plot_Peaks->setAxisTitle(QwtPlot::yLeft, text_y);

    Box_Control = new QGroupBox(MainBox);
    Box_Control->setObjectName("Transparent");
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(0);
    Box_Control->setLayout(layout);
    Fluor_Box = new QComboBox(MainBox);
    Fluor_Delegate = new Fluor_ItemDelegate;
    Fluor_Box->setItemDelegate(Fluor_Delegate);
    Table_Results = new QTableWidget(MainBox);
    Table_Results->setSelectionBehavior(QAbstractItemView::SelectRows);
    Table_Results->setSelectionMode(QAbstractItemView::SingleSelection);
    Table_Results->setEditTriggers(QAbstractItemView::NoEditTriggers);
    Delegate = new ResultsItemDelegate();
    Table_Results->setItemDelegate(Delegate);
    layout->addWidget(Fluor_Box, 0, Qt::AlignLeft);
    layout->addWidget(Table_Results, 1);
    main_spl->addWidget(plot_Peaks);
    main_spl->addWidget(Box_Control);

    Create_Curves();
    grid = new QwtPlotGrid;
    grid->setMajorPen(QPen(Qt::gray, 0, Qt::DotLine));
    grid->attach(plot_Peaks);
    plot_Peaks->setAxisFont(QwtPlot::yLeft, qApp->font());
    plot_Peaks->setAxisFont(QwtPlot::xBottom, qApp->font());

    connect(Table_Results, SIGNAL(currentCellChanged(int,int,int,int)), this, SLOT(Change_Select(int,int,int,int)));
    connect(Mode_Box, SIGNAL(currentIndexChanged(int)), this, SLOT(Mode_Changed()));

    return(MainBox);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_PeaksArea::Destroy()
{
    delete this;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_PeaksArea::Destroy_Win()
{

    if(MainBox != NULL)
    {
        Area    *item;
        QVector<QPointF> *vec;
        foreach(item, map_Area.values())
        {
            foreach(vec, item->Points)
            {
                vec->clear();
                delete vec;
            }
            delete item;
        }
        map_Area.clear();

        delete name_Tube;
        delete Mode_Box;
        Delete_Curves();
        Buf_X.clear();
        Buf_Y.clear();
        delete grid;

        delete Fluor_Delegate;
        delete Fluor_Box;
        delete Table_Results;
        delete Box_Control;
        delete main_spl;

        delete MainBox;
        MainBox = NULL;
    }
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_PeaksArea::Show()
{
    MainBox->show();
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_PeaksArea::GetInformation(QVector<QString> *info)
{
    info->append("0x0030");
    info->append(tr("PeaksArea"));
}
//-----------------------------------------------------------------------------
//--- readCommonSettings()
//-----------------------------------------------------------------------------
void Analyser_PeaksArea::readCommonSettings()
{
    QString text;
    QString dir_path = qApp->applicationDirPath();
    QSettings *CommonSettings = new QSettings(dir_path + "/tools/ini/preference.ini", QSettings::IniFormat);

    CommonSettings->beginGroup("Common");

        //... Language ...
        text = CommonSettings->value("language","ru").toString();

        if(translator.load(":/translations/analyser_peaksarea_" + text + ".qm"))
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
void Analyser_PeaksArea::Analyser(rt_Protocol *prot)
{
    int i,j,k;
    int id;
    QString text;
    QString fluor_name[COUNT_CH] = FLUOR_NAME;
    QIcon icon;
    int first_ch = -1;
    QPointF P,P_0,P_1;
    QPoint p;
    int num = -1;
    QVector<double> Y,X,y;
    QVector<QPointF> vec_Peaks;
    QVector<QPointF> *V;
    QVector<int> vec_K;
    int min_k, max_k;
    QVector<QPointF> vec_Area;
    double dvalue, s;

    Prot = prot;

    index_Curve = -1;
    current_area = NULL;

    int row, col;
    prot->Plate.PlateSize(prot->count_Tubes, row, col);


    int count = prot->count_MC;
    int count_t = prot->count_Tubes;

    if(prot->count_MC <= 0) return;

    Y.reserve(count);
    X.reserve(count);

    X = QVector<double>::fromStdVector(prot->X_MC);

    Fluor_Box->blockSignals(true);

    for(i=0; i<COUNT_CH; i++)
    {
        Fluor_Box->addItem(fluor_name[i]);
        switch(i)
        {
        case 0:  icon.addFile(":/images/fam_flat.png");   break;
        case 1:  icon.addFile(":/images/hex_flat.png");   break;
        case 2:  icon.addFile(":/images/rox_flat.png");   break;
        case 3:  icon.addFile(":/images/cy5_flat.png");   break;
        case 4:  icon.addFile(":/images/cy55_flat.png");   break;
        default: icon.addFile(NULL);   break;
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

        if(!(prot->active_Channels & (0x0f<<i*4))) continue;
        num++;

        for(j=0; j<count_t; j++)
        {
            if(prot->enable_tube.at(j) == 0) continue;
            Y = QVector<double>::fromStdVector(prot->MC_dF_dT).mid(count*count_t*num + j*count);

            vec_Peaks.clear();
            vec_K.clear();

            for(k=3; k<count-3; k++)
            {
                y.clear();
                y = Y.mid(k-3,7);

                if(*std::max_element(y.begin(), y.end()) < 10) {k += 7; continue;}

                if(std::distance(y.begin(), std::max_element(y.begin(), y.end())) == 3)
                {
                    if(y.at(2) > y.at(1) && y.at(1) > y.at(0) &&
                       y.at(4) > y.at(5) && y.at(5) > y.at(6) &&
                       y.at(3) > 20)
                    {
                        //qDebug() << "x,y:" << X.at(k) << Y.at(k);
                        P.setX(X.at(k));
                        P.setY(Y.at(k));
                        vec_Peaks.append(P);
                        vec_K.append(k);
                        k += 7;
                    }
                }
            }

            Area *area = new Area();

            text = QString("%1_%2").arg(num).arg(j);
            map_Area.insert(text, area);

            for(id=0; id<vec_K.size(); id++)
            {
                if(id > 1) break;

                switch(id)
                {
                case 0:         // First

                        k = vec_K.at(0);
                        min_k = 0;
                        while(k>=0)
                        {
                            if(Y.at(k) < 0) {min_k = k+1; break;}
                            k--;
                        }
                        dvalue = 0.;
                        if(vec_K.size() > 1)
                        {
                            dvalue = *std::min_element(Y.begin()+vec_K.at(0), Y.begin()+vec_K.at(1));
                            if(dvalue <= 0.) dvalue = 0.;
                        }

                        k = vec_K.at(0);
                        max_k = count-1;
                        while(k<count)
                        {
                            if(Y.at(k) <= dvalue) {max_k = k; if(dvalue <= 0.) max_k--; break;}
                            k++;
                        }
                        p.setX(min_k);
                        p.setY(max_k);
                        area->p.append(p);

                        P_0 = BorderPoint_ZeroCurve(&X,&Y,min_k,0);
                        P.setX(P_0.x());
                        P_1 = BorderPoint_ZeroCurve(&X,&Y,max_k,1);
                        P.setY(P_1.x());
                        area->p_manual.append(P);

                        V = new QVector<QPointF>();
                        V->append(P_0);
                        for(k=min_k; k<=max_k; k++)
                        {
                            P.setX(X.at(k));
                            P.setY(Y.at(k));
                            V->append(P);
                        }
                        V->append(P_1);
                        area->Points.append(V);


                        break;

                case 1:         // Second

                        k = vec_K.at(1);
                        min_k = max_k;
                        while(k>=min_k)
                        {
                            if(Y.at(k) <= dvalue) {min_k = k; if(dvalue <= 0.) min_k++; break;}
                            k--;
                        }
                        k = vec_K.at(1);
                        max_k = count-1;
                        while(k<count)
                        {
                            if(Y.at(k) <= 0) {max_k = k-1; break;}
                            k++;
                        }
                        p.setX(min_k);
                        p.setY(max_k);
                        area->p.append(p);

                        P_0 = BorderPoint_ZeroCurve(&X,&Y,min_k,0);
                        P.setX(P_0.x());
                        P_1 = BorderPoint_ZeroCurve(&X,&Y,max_k,1);
                        P.setY(P_1.x());
                        area->p_manual.append(P);

                        V = new QVector<QPointF>();
                        V->append(P_0);
                        for(k=min_k; k<=max_k; k++)
                        {
                            P.setX(X.at(k));
                            P.setY(Y.at(k));
                            V->append(P);
                        }
                        V->append(P_1);
                        area->Points.append(V);

                        break;
                }

                // Calculate area:
                s = 0.;
                for(k=1; k<=(max_k-min_k); k++)
                {
                    s += 0.5 * (X.at(min_k+k) - X.at(min_k+k-1)) * (Y.at(min_k+k) + Y.at(min_k+k-1));
                }
                area->s.append(s);

                // Calculate area for manual mode                
                area->s_manual.append(CalculateArea_ManualMode(area,id));
            }            


            //if(area->Points.size()) qDebug() << "j = " << j << *area->Points.at(0);
        }
    }

    Fluor_Box->setCurrentIndex(first_ch);
    Fluor_Box->blockSignals(false);

    Fill_TableResults();
    Table_Results->setFocus();
    Table_Results->selectRow(0);    
    Change_Select(0,0,-1,-1);
}


//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_PeaksArea::Fill_TableResults()
{
    int i,j,id;
    int ch = -1;
    int num;
    QStringList list, header;
    QString key, text, str;
    QTableWidgetItem *item;
    QVector<double> s;
    double dvalue;
    QMap<int,QString> map;

    Table_Results->blockSignals(true);

    int row,col;
    Prot->Plate.PlateSize(Prot->count_Tubes,row,col);

    for(i=0; i<=Fluor_Box->currentIndex(); i++)
    {
        if(Prot->active_Channels & (0x0f<<(i*4))) ch++;
    }

    header << "S1" << "S2" << "S1/(S1+S2)" << "S1/(S1+S2)";

    Table_Results->clear();

    Table_Results->setColumnCount(4);
    Table_Results->setHorizontalHeaderLabels(header);
    Table_Results->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    Table_Results->horizontalHeader()->setFont(qApp->font());
    Table_Results->verticalHeader()->setFont(qApp->font());

    header.clear();

    foreach(key, map_Area.keys())
    {
        list = key.split("_");
        if(list.at(0).toInt() != ch) continue;

        num = list.at(1).toInt();

        //qDebug() << "area: " << key << map_Area.value(key)->s << map_Area.value(key)->s_manual;

        switch(Mode_Box->currentIndex())
        {
        default:
        case 0:     s = map_Area.value(key)->s;         break;
        case 1:     s = map_Area.value(key)->s_manual;  break;
        }

        str = "";
        for(i=0; i<Table_Results->columnCount(); i++)
        {
            text = "-";
            switch(i)
            {
            case 0:     // S_0
                        if(s.size())
                        {
                            text = QString("%1").arg(s.at(0),0,'f',0);
                        }
                        break;

            case 1:     // s_1
                        if(s.size() > 1)
                        {
                            text = QString("%1").arg(s.at(1),0,'f',0);
                        }
                        break;

            case 2:     // s_0/(s_1+s_2)

                        if(s.size() >= 2 && (s.at(1)+s.at(0)) != 0.)
                        {
                            text = QString("%1").arg(s.at(0)/(s.at(1)+s.at(0))*100.,0,'f',1);
                        }
                        break;

            case 3:     // s_1/(s_1+s_2)

                        if(s.size() >= 2 && (s.at(1)+s.at(0)) != 0.)
                        {
                            text = QString("%1").arg(s.at(1)/(s.at(1)+s.at(0))*100.,0,'f',1);
                        }
                        break;
            }
            if(!str.isEmpty()) str += ";";
            str += text;
        }
        map.insert(num, str);
    }

    Table_Results->setRowCount(map.size());

    id = 0;
    foreach(i, map.keys())
    {
        header.append(Convert_IndexToName(i,col));
        list.clear();
        str = map.value(i);
        list = str.split(";");
        for(j=0; j<Table_Results->columnCount(); j++)
        {
            item = new QTableWidgetItem();
            item->setText(list.at(j));
            Table_Results->setItem(id,j,item);
        }

        id++;
    }

    map.clear();
    Table_Results->setVerticalHeaderLabels(header);

    Table_Results->setFocus();
    //Table_Results->selectRow(0);

    Table_Results->blockSignals(false);


}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_PeaksArea::Mode_Changed()
{
    //qDebug() << "mode_change: " << Table_Results->columnCount() << Table_Results->rowCount();
    if(Table_Results->columnCount() <= 0 || Table_Results->rowCount() <= 0) return;

    //int cur_row = Table_Results->currentRow();
    //int cur_col = Table_Results->currentColumn();

    //qDebug() << "Mode_Changed: " << Mode_Box->currentIndex() << cur_row << cur_col;

    Fill_TableResults();
    Table_Results->setCurrentCell(current_row, 0);
    //Change_Select(cur_row, cur_col, -1, -1);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
QPointF Analyser_PeaksArea::BorderPoint_ZeroCurve(QVector<double> *X, QVector<double> *Y, int k, int type)
{
    double dx, y1,y0,DX;
    QPointF P(X->at(k), 0.);

    switch(type)
    {
    case 0:             // Left border
            if(k == 0 || Y->at(k-1) > 0 || Y->at(k) < 0 || Y->at(k) < 0.001) break;
            y1 = fabs(Y->at(k-1));
            y0 = fabs(Y->at(k));
            DX = fabs(X->at(k) - X->at(k-1));
            dx = y0*DX/(y0+y1);
            P.setX(X->at(k) - dx);
            break;

    case 1:             // Right border
            if(k >= (X->size()-1) || Y->at(k+1) > 0 || Y->at(k) < 0 || Y->at(k) < 0.001) break;
            y1 = fabs(Y->at(k+1));
            y0 = fabs(Y->at(k));
            DX = fabs(X->at(k+1) - X->at(k));
            dx = y0*DX/(y0+y1);
            P.setX(X->at(k) + dx);
            break;

    default:    break;
    }

    return(P);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
QPointF Analyser_PeaksArea::BorderPoint_ManualMode(QVector<double> *X, QVector<double> *Y, int k, double value, int type)
{
    double dx,dy,DY,DX;
    QPointF P(X->at(k), Y->at(k));

    //if(fabs(X->at(k) - value) < 0.001) {qDebug() << "mini: ";  return(P);}


    switch(type)
    {
    case 0:             // Left border
            if(k == 0) break;
            dx = X->at(k) - value;
            DY = Y->at(k) - Y->at(k-1);
            DX = X->at(k) - X->at(k-1);
            dy = dx*DY/DX;
            P.setY(Y->at(k) - dy);

            break;

    case 1:             // Right border
            if(k >= (X->size()-1)) break;
            dx = value - X->at(k);
            DY = Y->at(k) - Y->at(k+1);
            DX = X->at(k+1) - X->at(k);
            dy = dx*DY/DX;
            P.setY(Y->at(k) - dy);

            break;

    default:    break;
    }

    return(P);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
double Analyser_PeaksArea::CalculateArea_ManualMode(Area *area, int id)
{
    int i,j;
    double s = 0.;
    double h,a,tg_a,dx,dy;
    QPointF Pi,Pj;

    if(area->Points.size() <= id) return(0.);

    QVector<QPointF> *vec = area->Points.at(id);

    Pi = vec->first();
    Pj = vec->last();
    dy = Pi.y() - Pj.y();
    dx = Pj.x() - Pi.x();
    if(dx == 0.) return(0.);

    tg_a = dy/dx;

    for(i=1; i<vec->size(); i++)
    {
        Pi = vec->at(i);
        Pj = vec->at(i-1);

        h = Pi.x() - Pj.x();
        if(h == 0.) continue;

        a = 0.;
        for(j=0; j<2; j++)
        {
            if(i==1 && j==0) continue;
            if(i==(vec->size()-1) && j == 1) continue;

            Pi = vec->at(i-j);

            a += Pi.y() - vec->last().y() - tg_a*(vec->last().x() - Pi.x());
        }

        s += 0.5 * h * a;
    }

    return(s);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_PeaksArea::RepaintArea(QVector<double> *X, QVector<double> *Y, Area *area)
{
    int i,j;
    int min_k, max_k;
    double min_val, max_val, value;
    QVector<QPointF> vec;
    QVector<QPointF> *V;
    QPointF P,Pc;
    QSizeF size;
    QPen pen;


    item_first->setShape(ShapeFactory::path(ShapeFactory::Polygon, Pc, size, vec));
    item_second->setShape(ShapeFactory::path(ShapeFactory::Polygon, Pc, size, vec));


    QColor fillColor;

    for(i=0; i<area->p.size(); i++)
    {
        vec.clear();

        switch(Mode_Box->currentIndex())
        {
        default:
        case 0:         // Zero line
            min_k = area->p.at(i).x();
            max_k = area->p.at(i).y();

            vec.append(BorderPoint_ZeroCurve(X,Y,min_k,0));
            for(j=0; j<=(max_k-min_k); j++)
            {
                P.setX(X->at(min_k+j));
                P.setY(Y->at(min_k+j));
                vec.append(P);
            }
            vec.append(BorderPoint_ZeroCurve(X,Y,max_k,1));
            break;

        case 1:         // Manual mode
            min_val = area->p_manual.at(i).x();
            max_val = area->p_manual.at(i).y();

            //qDebug() << "Manual mode:" << min_val << max_val;

            for(j=0; j<X->size(); j++)
            {
                value = X->at(j);
                if(min_val > value) continue;
                if(max_val < value) {j--; break;}
                if(vec.isEmpty())
                {
                    vec.append(BorderPoint_ManualMode(X,Y,j,min_val,0));
                }
                P.setX(value);
                P.setY(Y->at(j));
                vec.append(P);
            }
            if(j >= X->size()) j--;
            vec.append(BorderPoint_ManualMode(X,Y,j,max_val,1));

            area->Points.at(i)->clear();
            foreach(P, vec)
            {
                area->Points.at(i)->append(P);
            }

            break;
        }


        if(area->p.size() == 1) fillColor = Qt::red;
        else
        {
            if(i == 0) fillColor = Qt::blue;
            if(i == 1) fillColor = Qt::green;
        }
        fillColor.setAlpha(30);

        //pen.setColor(fillColor);
        pen.setColor(Qt::black);
        pen.setWidth(1);

        switch(i)
        {
        default:
        case 0:     item_first->setShape(ShapeFactory::path(ShapeFactory::Polygon, Pc, size, vec));
                    item_first->setPen(pen);
                    item_first->setBrush(fillColor);
                    break;

        case 1:     item_second->setShape(ShapeFactory::path(ShapeFactory::Polygon, Pc, size, vec));
                    item_second->setPen(pen);
                    item_second->setBrush(fillColor);
                    break;
        }
    }

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_PeaksArea::Change_Select(int cur_row, int cur_col, int prev_row, int prev_col)
{
    int i,j,id,k;
    int ch = -1;
    QString text;
    QVector<double> Y,X;    
    QVector<QPointF> vec;
    QPointF P;
    QPen pen;
    int row,col;
    double y_lower,y_upper,dy;
    Prot->Plate.PlateSize(Prot->count_Tubes,row,col);
    int count = Prot->count_MC;
    int count_t = Prot->count_Tubes;

    current_row = cur_row;
    //if(cur_row == prev_row) return;

    for(i=0; i<=Fluor_Box->currentIndex(); i++)
    {
        if(Prot->active_Channels & (0x0f<<(i*4))) ch++;
    }

    text = Table_Results->verticalHeaderItem(cur_row)->text();
    id = Convert_NameToIndex(text, col);

    X = QVector<double>::fromStdVector(Prot->X_MC);
    Y = QVector<double>::fromStdVector(Prot->MC_dF_dT).mid(count*count_t*ch + id*count);

    text = QString("%1_%2").arg(ch).arg(id);
    Area *area = map_Area.value(text);
    current_area = area;

    foreach(QwtPlotCurve *c, list_curve)
    {
        c->setVisible(false);
        c->setData(NULL);
    }

    // Repaint Area
    RepaintArea(&X, &Y, area);

    curve->setData(NULL);
    curve->setSamples(X,Y);
    QPen pen_curve(Qt::black, 2);
    curve->setPen(pen_curve);

    vec.clear();
    P.setX(X.first());
    P.setY(0.);
    vec.append(P);
    P.setX(X.last());
    vec.append(P);
    curve_zero->setData(NULL);
    curve_zero->setSamples(vec);
    QPen pen_zero(Qt::red, 1);
    curve_zero->setPen(pen_zero);

    plot_Peaks->setAxisAutoScale(QwtPlot::xBottom, true);
    plot_Peaks->setAxisAutoScale(QwtPlot::yLeft, true);

    name_Tube->setText(Table_Results->verticalHeaderItem(cur_row)->text());

    plot_Peaks->updateAxes();
    plot_Peaks->replot();

    plot_Peaks->setAxisAutoScale(QwtPlot::xBottom, false);

    // Manual mode
    if(Mode_Box->currentIndex() == 1)
    {
        //qDebug() << "Manual mode: " << area->p_manual.size();
        for(i=0; i<area->p_manual.size(); i++)
        {
            for(j=0; j<2; j++)
            {
                vec.clear();
                P.setX(area->p_manual.at(i).x());
                if(j) P.setX(area->p_manual.at(i).y());

                y_lower = plot_Peaks->axisScaleDiv(QwtPlot::yLeft).lowerBound();
                y_upper = plot_Peaks->axisScaleDiv(QwtPlot::yLeft).upperBound();
                dy = (y_upper -y_lower)/100.;
                for(k=0; k<100; k++)
                {
                   P.setY(y_lower + k*dy);
                   vec.append(P);
                }
                list_curve.at(i*2+j)->setSamples(vec);
                list_curve.at(i*2+j)->setVisible(true);
            }
        }
    }
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_PeaksArea::Create_Curves()
{
    curve = new QwtPlotCurve();
    curve->setStyle(QwtPlotCurve::Lines);
    curve->attach(plot_Peaks);

    curve_zero = new QwtPlotCurve();
    curve_zero->setStyle(QwtPlotCurve::Lines);
    curve_zero->attach(plot_Peaks);

    curve_1_1 = new QwtPlotCurve();
    curve_1_2 = new QwtPlotCurve();
    curve_2_1 = new QwtPlotCurve();

    curve_1_1->setStyle(QwtPlotCurve::Lines);
    curve_1_1->attach(plot_Peaks);
    curve_1_1->setVisible(false);
    curve_2_2 = new QwtPlotCurve();
    list_curve.append(curve_1_1);
    list_curve.append(curve_1_2);
    list_curve.append(curve_2_1);
    list_curve.append(curve_2_2);
    QPen pen_curve(Qt::black, 1);
    foreach(QwtPlotCurve *c, list_curve)
    {
        c->setStyle(QwtPlotCurve::Lines);
        c->setPen(pen_curve);
        c->attach(plot_Peaks);
        c->setVisible(false);
    }

    item_first = new QwtPlotShapeItem();
    item_first->attach(plot_Peaks);
    item_second = new QwtPlotShapeItem();
    item_second->attach(plot_Peaks);


    curve->setData(NULL);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_PeaksArea::Delete_Curves()
{
    qDebug() << "delete curves";
    curve->setData(NULL);
    curve_zero->setData(NULL);
    delete curve;
    delete curve_zero;

    list_curve.clear();
    delete curve_1_1;
    delete curve_1_2;
    delete curve_2_1;
    delete curve_2_2;

    delete item_first;
    delete item_second;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_PeaksArea::Enable_Tube(QVector<short> *e)
{

}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_PeaksArea::Select_Tube(int pos)
{

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_PeaksArea::Save_Results(char *fname)
{

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
bool Analyser_PeaksArea::eventFilter(QObject *object, QEvent *e)
{
    QPoint pos;
    int id;
    double d, min_dist;
    QVector<double> vec_distance;

    if(Mode_Box->currentIndex() != 1) return(false);
    if(!curve_1_1->isVisible()) return(false);
    if(object != (QObject *)plot_Peaks->canvas()) return(false);

    switch(e->type())
    {
    case QEvent::MouseButtonPress:

                //qDebug() << "click:";
                break;
    case QEvent::MouseButtonRelease:

                index_Curve = -1;
                plot_Peaks->canvas()->setCursor(Qt::PointingHandCursor);

                break;

    case QEvent::MouseMove:

                pos = ((QMouseEvent *)e)->pos();

                if(((QMouseEvent *)e)->buttons() == Qt::NoButton)       // NoButton
                {
                    vec_distance.clear();
                    foreach(QwtPlotCurve *c, list_curve)
                    {
                        id = c->closestPoint(pos, &d);
                        if(id < 0) d = 1000;
                        vec_distance.append(d);
                    }
                    //qDebug() << "vec_distance: " << vec_distance;

                    index_Curve = -1;
                    min_dist = *std::min_element(vec_distance.begin(),vec_distance.end());
                    if(min_dist < 10)
                    {
                        plot_Peaks->canvas()->setCursor(Qt::SplitHCursor);
                        index_Curve = std::distance(vec_distance.begin(), std::min_element(vec_distance.begin(), vec_distance.end()));
                        //qDebug() << "min_dist: " << min_dist << index_Curve;
                    }
                    else plot_Peaks->canvas()->setCursor(Qt::PointingHandCursor);

                    break;
                }

                if(((QMouseEvent *)e)->buttons() & Qt::LeftButton)
                {
                    //qDebug() << "pos: " << pos << plot_Peaks->invTransform(2, pos.x());
                    if(index_Curve >= 0 && list_curve.at(index_Curve)->isVisible())
                    {
                        RepaintCurve_And_RecalculateArea(plot_Peaks->invTransform(2, pos.x()));
                    }
                }

                break;
    //-------------------------------------------------------------------------
    default:    break;
    }

    return(false);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_PeaksArea::RepaintCurve_And_RecalculateArea(double value)
{
    int i,j;
    QVector<QPointF> vec;
    QPointF P;
    int id,k;
    bool sts = true;
    QVector<double> Y,X;
    int row,col,cur_row;
    double s;
    QTableWidgetItem *item;
    QString text;

    id = index_Curve;

    // check on valid
    if(index_Curve < 0 || !list_curve.at(index_Curve)->isVisible() || !current_area || current_area->p_manual.size() == 0) return;

    // check
    switch(id)
    {
    case 0:     if(value >= current_area->p_manual.at(0).y())   sts = false;
                break;

    case 1:     if(value <= current_area->p_manual.at(0).x())   sts = false;
                if(current_area->p_manual.size() > 1)
                {
                    if(value >= current_area->p_manual.at(1).x())   sts = false;
                }
                break;

    case 2:     if(value <= current_area->p_manual.at(0).y())   sts = false;
                if(value >= current_area->p_manual.at(1).y())   sts = false;
                break;

    case 3:     if(value <= current_area->p_manual.at(1).x())   sts = false;
                break;

    default:    break;
    }

    if(!sts) return;
    //---

    // check on axis X border
    double x_lower = plot_Peaks->axisScaleDiv(QwtPlot::xBottom).lowerBound();
    double x_upper = plot_Peaks->axisScaleDiv(QwtPlot::xBottom).upperBound();
    if(value < (x_lower + 0.1) || value > (x_upper - 0.1)) return;

    //---


    QwtPlotCurve *c = list_curve.at(id);
    c->setData(NULL);

    P.setX(value);
    double y_lower = plot_Peaks->axisScaleDiv(QwtPlot::yLeft).lowerBound();
    double y_upper = plot_Peaks->axisScaleDiv(QwtPlot::yLeft).upperBound();
    double dy = (y_upper -y_lower)/100.;
    for(k=0; k<100; k++)
    {
       P.setY(y_lower + k*dy);
       vec.append(P);
    }
    c->setSamples(vec);

    int num = div(id,2).quot;
    P = current_area->p_manual.at(num);
    if(div(id,2).rem == 0) P.setX(value);
    else P.setY(value);
    current_area->p_manual.replace(num,P);

    int count = Prot->count_MC;
    int count_t = Prot->count_Tubes;
    int ch = -1;
    for(i=0; i<=Fluor_Box->currentIndex(); i++)
    {
        if(Prot->active_Channels & (0x0f<<(i*4))) ch++;
    }

    Prot->Plate.PlateSize(Prot->count_Tubes,row,col);
    cur_row = Table_Results->currentRow();
    text = Table_Results->verticalHeaderItem(cur_row)->text();
    id = Convert_NameToIndex(text, col);

    X = QVector<double>::fromStdVector(Prot->X_MC);
    Y = QVector<double>::fromStdVector(Prot->MC_dF_dT).mid(count*count_t*ch + id*count);
    RepaintArea(&X, &Y, current_area);

    //...
    for(i=0; i<current_area->Points.size(); i++)
    {
        s = CalculateArea_ManualMode(current_area, i);
        current_area->s_manual.replace(i,s);
        item = Table_Results->item(cur_row, i);
        text = QString("%1").arg(s,0,'f',0);
        item->setText(text);
    }
    if(current_area->s_manual.size() >= 2)
    {
        for(j=0; j<2; j++)
        {
            text = "-";
            if((current_area->s_manual.at(0) + current_area->s_manual.at(1)) != 0.)
            {
                text = QString("%1").arg(current_area->s_manual.at(j)/(current_area->s_manual.at(0) + current_area->s_manual.at(1))*100., 0, 'f', 1);
            }
            item = Table_Results->item(cur_row, j+2);
            item->setText(text);
        }
    }
    //...
    Table_Results->repaint();
    plot_Peaks->replot();
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
    QPixmap pixmap(":/images/fam_flat.png");
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
    painter->setFont(font);

    if(option.state & QStyle::State_Enabled)
    {
            switch(row)
            {
            case 0:  pixmap.load(":/images/fam_flat.png");    break;
            case 1:  pixmap.load(":/images/hex_flat.png");    break;
            case 2:  pixmap.load(":/images/rox_flat.png");    break;
            case 3:  pixmap.load(":/images/cy5_flat.png");    break;
            case 4:  pixmap.load(":/images/cy55_flat.png");   break;
            default: pixmap.load(NULL);   break;
            }
    }
    else
    {
        painter->setPen(QPen(QColor(192,192,192) ,1,Qt::SolidLine));
        pixmap.load(":/images/flat/disable_flat.png");
    }

    painter->drawText(rect, Qt::AlignHCenter | Qt::AlignVCenter, text);

    /*if(!text.contains("---"))*/ painter->drawPixmap(0, rect.y()+2, pixmap);

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void ResultsItemDelegate::paint(QPainter *painter,
                               const QStyleOptionViewItem &option,
                               const QModelIndex &index) const
{
    int i,id;
    QRect rect;
    QString text,str;
    int flag;
    bool ok;
    QStringList list;
    QFont f = qApp->font();
    painter->setFont(f);

    QStyleOptionViewItem  viewOption(option);
    viewOption.state &= ~QStyle::State_HasFocus;    // disable state HasFocus
    QStyledItemDelegate::paint(painter, viewOption, index);

    rect = option.rect;
    text = index.data().toString();
    int row = index.row();
    int col = index.column();

    QColor bg_color = Qt::white;

    // Background

    if(option.state & QStyle::State_Selected)
    {
        painter->fillRect(option.rect, QColor(215,215,255));
    }
    else
    {
        if(div(row,2).rem)  painter->fillRect(option.rect, QColor(240,240,240));
        else painter->fillRect(option.rect, QColor(255,255,255));
    }
    //painter->fillRect(rect, bg_color);

    // Data
    painter->setPen(QPen(Qt::black,1,Qt::SolidLine));

    painter->drawText(option.rect, Qt::AlignHCenter | Qt::AlignVCenter, text);

}


