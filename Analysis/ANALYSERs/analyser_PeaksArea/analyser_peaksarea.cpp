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
    QHBoxLayout *layout_plot = new QHBoxLayout;
    //layout_plot->addStretch(1);
    layout_plot->addWidget(name_Tube, 0, Qt::AlignTop | Qt::AlignLeft);
    plot_Peaks->canvas()->setLayout(layout_plot);

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
        foreach(item, map_Area.values())
        {
            delete item;
        }
        map_Area.clear();

        delete name_Tube;
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
    QPointF P;
    QPoint p;
    int num = -1;
    QVector<double> Y,X,y;
    QVector<QPointF> vec_Peaks;
    QVector<int> vec_K;
    int min_k, max_k;
    QVector<QPointF> vec_Area;
    double dvalue, s;

    Prot = prot;

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

                        break;
                }

                // Calculate area:
                s = 0.;
                for(k=1; k<=(max_k-min_k); k++)
                {
                    s += 0.5 * (X.at(min_k+k) - X.at(min_k+k-1)) * (Y.at(min_k+k) + Y.at(min_k+k-1));
                }
                area->s.append(s);
            }

            //qDebug() << "j = " << j << area->s << area->p;
        }
    }

    Fluor_Box->setCurrentIndex(first_ch);
    Fluor_Box->blockSignals(false);

    Fill_TableResults();
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

    int row,col;
    Prot->Plate.PlateSize(Prot->count_Tubes,row,col);

    for(i=0; i<=Fluor_Box->currentIndex(); i++)
    {
        if(Prot->active_Channels & (0x0f<<(i*4))) ch++;
    }

    header << "S1" << "S2" << "S1/S2";

    Table_Results->clear();
    Table_Results->setColumnCount(3);
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
        s = map_Area.value(key)->s;

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

            case 2:     // s_0/s_1

                        if(s.size() >= 2 && s.at(1) != 0.)
                        {
                            text = QString("%1").arg(s.at(0)/s.at(1),0,'f',2);
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
    Table_Results->selectRow(0);


}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_PeaksArea::Change_Select(int cur_row, int cur_col, int prev_row, int prev_col)
{
    int i,j,id;
    int ch = -1;
    QString text;
    QVector<double> Y,X;
    int min_k, max_k;
    QVector<QPointF> vec;
    QPointF P,Pc;
    QSizeF size;
    QPen pen;
    int row,col;
    Prot->Plate.PlateSize(Prot->count_Tubes,row,col);
    int count = Prot->count_MC;
    int count_t = Prot->count_Tubes;

    if(cur_row == prev_row) return;

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


    item_first->setShape(ShapeFactory::path(ShapeFactory::Polygon, Pc, size, vec));
    item_second->setShape(ShapeFactory::path(ShapeFactory::Polygon, Pc, size, vec));

    QColor fillColor;

    for(i=0; i<area->p.size(); i++)
    {
        vec.clear();
        min_k = area->p.at(i).x();
        max_k = area->p.at(i).y();
        for(j=0; j<=(max_k-min_k); j++)
        {
            P.setX(X.at(min_k+j));
            P.setY(Y.at(min_k+j));
            vec.append(P);
        }
        if(area->p.size() == 1) fillColor = Qt::red;
        else
        {
            if(i == 0) fillColor = Qt::blue;
            if(i == 1) fillColor = Qt::green;
        }
        fillColor.setAlpha(30);

        pen.setColor(fillColor);
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


    curve->setData(NULL);
    curve->setSamples(X,Y);
    QPen pen_curve(Qt::black, 2);
    curve->setPen(pen_curve);
    plot_Peaks->setAxisAutoScale(QwtPlot::xBottom, true);
    plot_Peaks->setAxisAutoScale(QwtPlot::yLeft, true);


    name_Tube->setText(Table_Results->verticalHeaderItem(cur_row)->text());


    plot_Peaks->updateAxes();
    plot_Peaks->replot();

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_PeaksArea::Create_Curves()
{
    curve = new QwtPlotCurve();
    curve->setStyle(QwtPlotCurve::Lines);
    curve->attach(plot_Peaks);

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
    delete curve;
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


