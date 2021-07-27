#include "protocol_information.h"

//-----------------------------------------------------------------------------
extern "C" PROTOCOL_INFORMATIONSHARED_EXPORT HWND __stdcall cr_Scheme(HWND handle)
{
    int argc = 0;
    char **argv = 0;
    if(QApplication::instance() == 0)
    {
        new QApplication(argc, argv);
    }
    RECT rect;
    GetClientRect(handle, &rect);
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    Scheme_ProgramAmpl* p = new Scheme_ProgramAmpl(Qt::red);
    p->setWindowFlags(Qt::ToolTip);
    HWND handle_scheme = (HWND)p->winId();

    ((Scheme_ProgramAmpl*)p)->show();


    ::SetParent(handle_scheme, handle);
    ::SetWindowPos(handle_scheme, HWND_TOP, 0,0, width, height, SWP_SHOWWINDOW);

    return(handle_scheme);
}
//-----------------------------------------------------------------------------
extern "C" PROTOCOL_INFORMATIONSHARED_EXPORT void __stdcall resize_Scheme(HWND handle)
{
    HWND handle_parent;
    RECT rect;
    int width, height;

    QWidget *p = QWidget::find((WId)handle);
    if(p)
    {
        handle_parent = ::GetAncestor(handle, GA_PARENT);

        if(handle_parent)
        {
            GetClientRect(handle_parent, &rect);
            width = rect.right - rect.left;
            height = rect.bottom - rect.top;
            ::SetWindowPos(handle, HWND_TOP, 0,0, width, height, SWP_SHOWWINDOW);
        }
    }
}
//-----------------------------------------------------------------------------
extern "C" PROTOCOL_INFORMATIONSHARED_EXPORT void __stdcall draw_Scheme(HWND handle, char* Pro)
{
    QString text(Pro), str;

    //qDebug() << "text: " << text;
    text = text.replace(";", "\r\n");
    //qDebug() << "text_new: " << text;

    QStringList list = text.split("\r\n");
    QVector<std::string> Vec;

    foreach(str, list)
    {
        if(str.trimmed().isEmpty()) continue;
        Vec.append(str.toStdString());        
    }

    //qDebug() << "list program: " << list;

    Scheme_ProgramAmpl* p = (Scheme_ProgramAmpl*)QWidget::find((WId)handle);
    p->draw_Program(&Vec);
}
//-----------------------------------------------------------------------------
extern "C" PROTOCOL_INFORMATIONSHARED_EXPORT void __stdcall delete_Scheme(HWND handle)
{
    QWidget *p = QWidget::find((WId)handle);
    delete (Scheme_ProgramAmpl*)p;
    return;
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Plot_ProgramAmpl::Plot_ProgramAmpl(QWidget *parent):
    QwtPlot(parent)
{
    QwtText title_Y;

    readCommonSettings();
    setFont(qApp->font());

    canvas()->setCursor(Qt::ArrowCursor);
    //canvas()->installEventFilter(this);
    //canvas()->setMouseTracking(true);

    //curve = new QwtPlotCurve();
    curve = new ColorCurve();
    curve->setStyle(QwtPlotCurve::Lines);
    curve->attach(this);

    QColor color_bg = 0x99D9EA;
    if(style == "fusion") color_bg = 0xEAF8FB;
    setCanvasBackground(color_bg);

    QFont axisFont("Arial", 8, QFont::Normal);
    setAxisFont(QwtPlot::yLeft, axisFont);

    setAxisScale(QwtPlot::yLeft,0.,100.);
    enableAxis(QwtPlot::xBottom,false);
    enableAxis(QwtPlot::yLeft,true);

    grid = new QwtPlotGrid;
    grid->setMajorPen(QPen(Qt::gray, 0, Qt::DotLine));
    grid->attach(this);


    name_program = new QLabel();
    QHBoxLayout *plot_H1_layout = new QHBoxLayout();
    plot_H1_layout->addStretch(1);
    plot_H1_layout->addWidget(name_program);
    param_program = new QLabel();
    volume_program = new QLabel();
    QHBoxLayout *plot_H2_layout = new QHBoxLayout();
    plot_H2_layout->addWidget(param_program, 0, Qt::AlignLeft);
    plot_H2_layout->addWidget(volume_program, 1, Qt::AlignRight);
    QVBoxLayout *plot_V_layout = new QVBoxLayout();
    plot_V_layout->addLayout(plot_H1_layout);
    plot_V_layout->addStretch(1);
    plot_V_layout->addLayout(plot_H2_layout);
    canvas()->setLayout(plot_V_layout);

    QFont f = parent->font();
    f.setBold(true);
    f.setPointSize(12);
    name_program->setFont(f);
    f.setPointSize(8);
    f.setBold(false);
    param_program->setFont(f);
    //f.setBold(true);
    volume_program->setFont(f);

    //title_Y.setText("Temperature,°C");
    //setAxisTitle(QwtPlot::yLeft, title_Y);

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Plot_ProgramAmpl::~Plot_ProgramAmpl()
{
    delete curve;
    delete name_program;
    delete param_program;
    delete volume_program;
}

//-----------------------------------------------------------------------------
//--- readCommonSettings()
//-----------------------------------------------------------------------------
void Plot_ProgramAmpl::readCommonSettings()
{
    QString text;
    QString dir_path = qApp->applicationDirPath();
    QSettings *CommonSettings = new QSettings(dir_path + "/tools/ini/preference.ini", QSettings::IniFormat);

    CommonSettings->beginGroup("Common");

        //... Language ...
        text = CommonSettings->value("language","ru").toString();
        if(translator.load(":/translations/ProtInfo_" + text + ".qm"))
        {
            qApp->installTranslator(&translator);
        }

        //... Style ...
        text = CommonSettings->value("style","xp").toString();
        style = text.trimmed();

    CommonSettings->endGroup();
    delete CommonSettings;
}

//-----------------------------------------------------------------------------
//--- clear_Program()
//-----------------------------------------------------------------------------
void Plot_ProgramAmpl::clear_Program()
{
    name_program->setText("");
    param_program->setText("");
    volume_program->setText("");
    curve->setData(NULL);
    replot();
}
//-----------------------------------------------------------------------------
//--- draw_Program()
//-----------------------------------------------------------------------------
void Plot_ProgramAmpl::draw_Program(rt_Protocol *prot)
{
    int i;
    double value_x = 0;    
    double value_y;
    QVector<double> x,y;
    QColor color;
    int count = prot->PrAmpl_value.size();
    QString text;
    QwtText title_Y;
    QFont f = qApp->font();


    curve->color_lev.clear();    

    for(i=0; i<count; i++)
    {
        value_x += 5;
        x.push_back(value_x);
        value_x += prot->PrAmpl_time.at(i);
        x.push_back(value_x);

        value_y = prot->PrAmpl_value.at(i);
        y.push_back(value_y);
        y.push_back(value_y);        

        switch(prot->PrAmpl_color.at(i))
        {
            default:
            case 0: color = Qt::darkGreen;  break;
            case 1: color = Qt::red;        break;
            case 2: color = Qt::red;        break;
            case 3: color = Qt::blue;   break;
        }
        curve->color_lev.push_back(color);
    }

    curve->setSamples(x,y);

    //... name ...
    QVector<string> program = QVector<string>::fromStdVector(prot->program);
    for(i=0; i<program.size(); i++)
    {
        text = QString::fromStdString(program.at(i));
        if(text.startsWith("XSAV"))
        {
           text = text.mid(5);
           break;
        }
        else text = "";
    }
    f.setBold(true);
    name_program->setFont(f);
    name_program->setText(text);

    //... param ...
    if(program.size())
    {
        text = QString("PCR = %1; MC = %2;").arg(prot->count_PCR).arg(prot->count_MC);
        param_program->setText(text);
        if(prot->volume) volume_program->setText(QString("%1 = %2 %3").arg(tr("Vol")).arg(prot->volume).arg(tr("ul")));
        else volume_program->setText("");
    }
    else
    {
        param_program->setText("");
        volume_program->setText("");
    }


    f.setBold(true);
    //f.setPointSize(12);
    title_Y.setText(tr("Temperature,°C"));
    title_Y.usedFont(f);

    setAxisTitle(QwtPlot::yLeft, title_Y);

    updateAxes();
    replot();
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Plate_Protocol::Plate_Protocol(QWidget *parent):
    QTableWidget(parent)
{
    clear();
    setRowCount(0);
    setColumnCount(0);


}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Plate_Protocol::~Plate_Protocol()
{

}
//-----------------------------------------------------------------------------
//--- clear_Plate()
//-----------------------------------------------------------------------------
void Plate_Protocol::clear_Plate()
{

}
//-----------------------------------------------------------------------------
//--- draw_Plate()
//-----------------------------------------------------------------------------
void Plate_Protocol::draw_Plate(rt_Protocol *prot)
{
    int i;
    int row,col;
    QStringList header;
    int tubes = prot->count_Tubes;

    prot->Plate.PlateSize(tubes,row,col);    

    setRowCount(row);
    setColumnCount(col);
    setSelectionMode(QAbstractItemView::NoSelection);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setFocusPolicy(Qt::NoFocus);

    horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    header.clear();
    for(i=0; i<col; i++) header.append(QString::number(i+1));   // Column Header
    setHorizontalHeaderLabels(header);
    header.clear();
    for(i=0; i<row; i++) header.append(QChar(0x41 + i));        // Row Header
    setVerticalHeaderLabels(header);
    //setFont(QFont("Comic Sans MS", 5, QFont::Normal));
    horizontalHeader()->setFont(QFont("Comic Sans MS", 4, QFont::Normal));
    verticalHeader()->setFont(QFont("Comic Sans MS", 4, QFont::Normal));
    setFont(QFont("Comic Sans MS", 4, QFont::Normal));

    ItemDelegate_info *Delegate = new ItemDelegate_info();
    Delegate->get_prot(prot);
    setItemDelegate(Delegate);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void ItemDelegate_info::paint(QPainter *painter,
                         const QStyleOptionViewItem &option,
                         const QModelIndex &index) const
{
    QColor color,rgb;
    //int r,g,b;
    int rad;
    int x,y;
    double X,Y,R;
    int pos;
    int min_value = qMin(option.rect.width(), option.rect.height());

    R = (double)min_value/2. * 0.80;
    X = (double)option.rect.x() + (double)option.rect.width()/2. - R;
    Y = (double)option.rect.y() + (double)option.rect.height()/2. - R;

    rad = (int)(R+0.5);
    x = (int)(X+0.5);
    y = (int)(Y+0.5);

    QPalette pal = option.palette;
    QStyleOptionViewItem  viewOption(option);

    viewOption.state &= ~QStyle::State_HasFocus;    // disable state HasFocus

    QStyledItemDelegate::paint(painter, viewOption, index);

    //__1. Background
    painter->fillRect(option.rect, QColor(255,255,225));

    //__2. Enable Tube
    pos = index.column() + index.row() * index.model()->columnCount();

    //qDebug() << pos << index.column() << index.row() << index.model()->columnCount();

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
    }
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Info_Protocol::Info_Protocol(QWidget *parent):
    QTreeWidget(parent)
{
    readCommonSettings();
    setFont(qApp->font());

    Delegate_Info = new ItemDelegate_Information();
    setItemDelegate(Delegate_Info);
    Delegate_Info->style = style;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Info_Protocol::~Info_Protocol()
{
    clear();
    delete Delegate_Info;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Info_Protocol::readCommonSettings()
{
    QString text;
    QString dir_path = qApp->applicationDirPath();
    QSettings *CommonSettings = new QSettings(dir_path + "/tools/ini/preference.ini", QSettings::IniFormat);

    CommonSettings->beginGroup("Common");

        //... Style ...
        text = CommonSettings->value("style","xp").toString();
        style = text.trimmed();

    CommonSettings->endGroup();
    delete CommonSettings;
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Info_Protocol::clear_Info()
{
    clear();
    setColumnCount(2);

    QStringList header;
    header << tr("Parameter") << tr("Value");
    setHeaderLabels(header);
    this->header()->setSectionResizeMode(0,QHeaderView::ResizeToContents);
    this->header()->setSectionResizeMode(1,QHeaderView::ResizeToContents);
    this->header()->setFont(qApp->font());
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Info_Protocol::fill_Info(rt_Protocol *p_prot)
{
    int i;
    rt_Preference *preference;

    if(p_prot == NULL) return;

    QTreeWidgetItem *item;
    QString text;
    int num;


    // 1. Name of Protocol
    item = new QTreeWidgetItem(this);
    item->setText(0, tr("Name of Protocol"));
    //p_prot->name = Name_Protocol->text().trimmed().toStdString();
    text = QString::fromStdString(p_prot->name);
    item->setText(1, text);

    // 2. Name of Program
    item = new QTreeWidgetItem(this);
    item->setText(0, tr("Name of Program"));
    text = QString::fromStdString(p_prot->PrAmpl_name);
    item->setText(1, text);

    // 3. ID Prpotocol
    item = new QTreeWidgetItem(this);
    item->setText(0, tr("ID of Protocol"));
    text = QString::fromStdString(p_prot->regNumber);

        // Find Web_Protocol property
        for(i=0; i<p_prot->preference_Pro.size(); i++)
        {
            preference = p_prot->preference_Pro.at(i);
            if(preference->name == "Web_Protocol" && text == QString::fromStdString(preference->value))
            {
                text += "  (Web)";
                break;
            }
        }
        //
    item->setText(1, text);

    // 4. Operator
    item = new QTreeWidgetItem(this);
    item->setText(0, tr("Operator"));
    //prot->owned_by = Operator.toStdString();
    text = QString::fromStdString(p_prot->owned_by);
    item->setText(1, text);

    // 5. BarCode
    item = new QTreeWidgetItem(this);
    item->setText(0, tr("BarCode"));
    text = QString::fromStdString(p_prot->barcode);
    item->setText(1, text);

    // 6. Type_Plate
    item = new QTreeWidgetItem(this);
    item->setText(0, tr("Type of Plate"));
    num = p_prot->count_Tubes;
    text = QString::number(num);
    item->setText(1, text);

    // 7. ActiveChannels
    item = new QTreeWidgetItem(this);
    item->setText(0, tr("AciveChannels"));
    num = p_prot->active_Channels;
    text = QString("0x%1").arg(num,5,16);
    item->setText(1, text);

    // Volume
    item = new QTreeWidgetItem(this);
    item->setText(0, tr("Volume of reaction mixture (ul)"));
    text = QString("%1").arg(p_prot->volume);
    item->setText(1, text);

    // 8. File of Protocol
    item = new QTreeWidgetItem(this);
    item->setText(0, tr("File of protocol"));
    text = QString::fromStdString(p_prot->xml_filename);
    item->setText(1, text);

    // 9. Device
    item = new QTreeWidgetItem(this);
    item->setText(0, tr("Device RT"));
    text = QString::fromStdString(p_prot->SerialName);
    if(text.isEmpty()) text = "...";
    text += "  (Exp: ";
    for(i=0; i<p_prot->Exposure.size(); i++)
    {
        if(i) text += ",";
        text += QString("%1").arg(p_prot->Exposure.at(i));
    }
    text += ")";
    if(p_prot->Exposure.size() == 0) text = "";

    item->setText(1, text);
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void ItemDelegate_Information::paint(QPainter *painter,
                                     const QStyleOptionViewItem &option,
                                     const QModelIndex &index) const
{
    QString text;
    int active_ch;
    bool ok;
    int i;

    painter->setFont(qApp->font());
    QPixmap pixmap(":/images/fam.png");

    QPalette pal = option.palette;
    QStyleOptionViewItem  viewOption(option);
    viewOption.state &= ~QStyle::State_HasFocus;    // disable state HasFocus

    QStyledItemDelegate::paint(painter, viewOption, index);


    text = index.data().toString();
    //qDebug() << "tree: " << index.row() << index.column() << text;
    if(index.row() == 6 && index.column() == 1)
    {
        text = text.replace(" ","").trimmed();
        active_ch = text.toInt(&ok, 16);

        for(i=0; i<COUNT_CH; i++)
        {
            if(active_ch & (0xf<<4*i))
            {
                if(style == "fusion")
                {
                    switch(i)
                    {
                        case 0: pixmap.load(":/images/fam_pro.png");    break;
                        case 1: pixmap.load(":/images/hex_pro.png");    break;
                        case 2: pixmap.load(":/images/rox_pro.png");    break;
                        case 3: pixmap.load(":/images/cy5_pro.png");    break;
                        case 4: pixmap.load(":/images/cy55_pro.png");   break;
                        default: pixmap.load(":/images/disable_pro.png");   break;
                    }
                }
                else
                {
                    switch(i)
                    {
                        case 0: pixmap.load(":/images/fam.png");    break;
                        case 1: pixmap.load(":/images/hex.png");    break;
                        case 2: pixmap.load(":/images/rox.png");    break;
                        case 3: pixmap.load(":/images/cy5.png");    break;
                        case 4: pixmap.load(":/images/cy55.png");   break;
                        default: pixmap.load(":/images/disable.png");   break;
                    }
                }

            }
            else
            {
                if(style == "fusion") pixmap.load(":/images/disable_pro.png");
                else pixmap.load(":/images/disable.png");
            }

            painter->drawPixmap(option.rect.x() + 13*i + 100, option.rect.y(), pixmap);
        }
    }
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Scheme_ProgramAmpl::Scheme_ProgramAmpl(QColor color, QWidget *parent):
    QScrollArea(parent)
{
    readCommonSettings();

    setWidget(&label_Program);
    setWidgetResizable(true);
    //setFixedWidth(260);

    if(color != Qt::transparent)
    {
        if(style == "fusion") setStyleSheet("background-color: #EAF8FB;"); //rgb(215, 240, 247);");
        else setStyleSheet("background-color:rgb(153, 217, 234);");
    }
    //setStyleSheet("background-color:transparent;");
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Scheme_ProgramAmpl::readCommonSettings()
{
    QString text;
    QString dir_path = qApp->applicationDirPath();
    QSettings *CommonSettings = new QSettings(dir_path + "/tools/ini/preference.ini", QSettings::IniFormat);

    CommonSettings->beginGroup("Common");

        //... Style ...
        text = CommonSettings->value("style","xp").toString();
        style = text.trimmed();

    CommonSettings->endGroup();
    delete CommonSettings;
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Scheme_ProgramAmpl::draw_Program(QVector<string> *pr, int DRAW_Level)
{
    int i,j;
    QString text,temp,time,meas,tmp;
    QString pro_name = "";
    bool new_block = true;
    QVector<QString> str;
    QStringList s;
    double dvalue;
    short val;
    QByteArray ba("\xB0\x43");
    int height_pm = 0;
    QPixmap pixmap_foto(":/images/foto.png");
    QPixmap pixmap_fotoMC(":/images/foto_mc.png");
    QPixmap pixmap_incr;
    QFont f;
    bool grad_step = false;
    bool gradient = false;

    if(pr->size() < 2) {clear_scheme(); return;}

    QVector<Block_Pro*> Blocks;
    Block_Pro   *block;

    int y_0,y_1;
    QPolygon polygon;
    int offset;

    int draw_block = -1;
    int draw_level = -1;
    bool draw_status = false;
    if(DRAW_Level >= 0)
    {
        draw_status = (bool)(DRAW_Level & 0x000001);
        draw_level = (DRAW_Level & 0x00ff00) >> 8;
        draw_block = (DRAW_Level & 0xff0000) >> 16;

        if(draw_block == 0)
        {
            draw_block = -1;
            draw_level = -1;
            draw_status = false;
        }
    }


    for(i=0; i<pr->size(); i++)
    {
        text = QString::fromStdString(pr->at(i));
        if(text.indexOf("XPRG") == 0)
        {
            s = text.split(QRegExp("\\s+"));
            tmp = s.at(3);
            val = tmp.toShort();
            if(val >= 4) grad_step = true;
            continue;
        }
        if(text.indexOf("XTCH") == 0)
        {
            gradient = true;
            continue;
        }
        if(text.indexOf("XLEV") == 0)
        {
            if(new_block)
            {
                block = new Block_Pro();
                Blocks.append(block);
                block->Num_Cycle = 1;
            }
            new_block = false;

            s = text.split(QRegExp("\\s+"));
            //temperature
            temp = s.at(1);
            dvalue = temp.toDouble();
            dvalue /= 100.;
            temp = QString::number(dvalue,'f',1) + " " + QString::fromLatin1(ba);
            //time
            time = s.at(2);            
            dvalue = time.toDouble();            
            time = QDateTime::fromTime_t(dvalue).toUTC().toString("h:mm:ss");
            //measure
            meas = s.at(6);
            val = meas.toShort();
            block->Measure.append(val);
            if(val == 2) block->Type_Block = bMelt;

            //gradient
            tmp = s.at(5);
            val = tmp.toShort();
            if(val != 0 && !gradient) val = 0;
            block->Gradient.append(val);
            gradient = false;
            //incr_time
            tmp = s.at(4);
            val = tmp.toShort();
            block->Incr_Time.append(val);
            //incr_temp
            tmp = s.at(3);
            val = tmp.toShort();
            block->Incr_Temp.append(val);

            if(i+1 < pr->size())
            {
                //i++;
                text = QString::fromStdString(pr->at(i+1));
                if(text.indexOf("XLEV") != 0 && text.indexOf("XTCH") != 0) new_block = true;
                if(text.indexOf("XPAU") == 0) {time = tr("Pause"); block->Type_Block = bPause;}
                if(text.indexOf("XHLD") == 0) {time = tr("Hold"); block->Type_Block = bHold;}
                if(text.indexOf("XCYC") == 0)
                {
                    s = text.split(QRegExp("\\s+"));
                    tmp = s.at(1);
                    block->Num_Cycle = tmp.toInt();
                }
            }

            block->Level.append(temp + " - " + time);
            str.append(temp + " - " + time);
        }
        if(text.indexOf("XSAV") == 0)
        {
            pro_name = text.mid(5);
            continue;
        }
    }

    // Height Pixmap:
    for(i=0; i<Blocks.size(); i++)
    {
        block = Blocks.at(i);
        height_pm += 16 + block->Level.size()*16;
    }
    height_pm += 30;

    QPixmap pixmap(260,height_pm);
    pixmap.fill(QColor("transparent"));
    //pixmap.fill();

    QPainter painter(&pixmap);
    QPen myPen(Qt::black, 1, Qt::SolidLine);
    painter.setPen(myPen);

    f = QApplication::font();
    //f.setPointSize(8);
    painter.setFont(f);
    QFontMetrics fm(f);

    height_pm = 0;

    //
    painter.drawText(5, 16, pro_name);
    height_pm += 24;
    //


    for(i=0; i<Blocks.size(); i++)
    {
        block = Blocks.at(i);
        if(block->Type_Block == bHold) myPen.setColor(Qt::blue);
        else myPen.setColor(Qt::black);
        painter.setPen(myPen);

        for(j=0; j<block->Level.size(); j++)
        {
            //... draw current(On_Line) level
            if(draw_block == i+1 && draw_level == j && block->Type_Block != bHold)
            {
                myPen.setColor(Qt::red);
                if(draw_status) f.setBold(true);
                painter.setFont(f);
                painter.setPen(myPen);
            }

            //...

            if(!j) y_0 = height_pm;
            height_pm += 16;
            if(!j) painter.drawText(5, height_pm, QString("%1.").arg(i+1));
            painter.drawText(25, height_pm, block->Level.at(j));


            //offset = 125;
            offset = fm.width("1. 80.0 C - 0:00:00 ");
            offset += 20;


            if(block->Measure.at(j) == 1) {painter.drawPixmap(offset, height_pm-14, pixmap_foto); offset += 20;}
            if(block->Measure.at(j) == 2) {painter.drawPixmap(offset, height_pm-14, pixmap_fotoMC); offset += 20;}

            if(block->Incr_Temp.at(j) != 0)
            {
                pixmap_incr.load(":/images/incr_temp.png");
                painter.drawPixmap(offset, height_pm-14, pixmap_incr);
                offset += 20;
            }
            if(block->Incr_Time.at(j) != 0)
            {
                pixmap_incr.load(":/images/incr_time.png");
                painter.drawPixmap(offset, height_pm-14, pixmap_incr);
                offset += 20;
            }
            if(block->Gradient.at(j) != 0)
            {
                pixmap_incr.load(":/images/grad.png");
                if(grad_step) pixmap_incr.load(":/images/grad_step.png");
                painter.drawPixmap(offset, height_pm-14, pixmap_incr);
                offset += 20;
            }

            if(block->Type_Block != bHold)
            {
                myPen.setColor(Qt::black);
                f.setBold(false);
                painter.setFont(f);
                painter.setPen(myPen);
            }

        }
        y_1 = height_pm;
        height_pm += 16;


        if(block->Num_Cycle > 1)
        {
            y_0 += 2;
            y_1 += 2;
            polygon.clear();
            polygon << QPoint(200,y_0) << QPoint(205,y_0) << QPoint(205,y_1) << QPoint(200,y_1);
            painter.drawPolyline(polygon);
            text = QString(" x %1").arg(block->Num_Cycle);
            painter.drawText(205, y_0 + (y_1-y_0)/2 + 6, text);
        }
    }


    label_Program.setPixmap(pixmap);



    for(i=0; i<Blocks.size(); i++) delete Blocks.at(i);
    Blocks.clear();
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Scheme_ProgramAmpl::clear_scheme()
{
    QPixmap pixmap_NULL;
    label_Program.setPixmap(pixmap_NULL);
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Scheme_ProgramAmpl::draw_Program(rt_Protocol *p, int DRAW_Level)
{
    /*int i,j;
    QString text,temp,time,meas,tmp;
    bool new_block = true;
    QVector<QString> str;
    QStringList s;
    double dvalue;
    short val;
    QByteArray ba("\xB0\x43");
    int height_pm = 0;
    QPixmap pixmap_foto(":/images/foto.png");
    QPixmap pixmap_fotoMC(":/images/foto_mc.png");
    QPixmap pixmap_incr;
    QFont f;
    bool grad_step = false;
    bool gradient = false;

    QVector<Block_Pro*> Blocks;
    Block_Pro   *block;

    int y_0,y_1;
    QPolygon polygon;
    int offset;

    int draw_block = -1;
    int draw_level = -1;
    bool draw_status = false;
    if(DRAW_Level >= 0)
    {
        draw_status = (bool)(DRAW_Level & 0x000001);
        draw_level = (DRAW_Level & 0x00ff00) >> 8;
        draw_block = (DRAW_Level & 0xff0000) >> 16;

        if(draw_block == 0)
        {
            draw_block = -1;
            draw_level = -1;
            draw_status = false;
        }
    }

    QVector<string> pr = QVector<string>::fromStdVector(p->program);
    for(i=0; i<pr.size(); i++)
    {
        text = QString::fromStdString(pr.at(i));
        if(text.indexOf("XPRG") == 0)
        {
            s = text.split(QRegExp("\\s+"));
            tmp = s.at(3);
            val = tmp.toShort();
            if(val >= 4) grad_step = true;
            continue;
        }
        if(text.indexOf("XTCH") == 0)
        {
            gradient = true;
            continue;
        }
        if(text.indexOf("XLEV") == 0)
        {
            if(new_block)
            {
                block = new Block_Pro();
                Blocks.append(block);
                block->Num_Cycle = 1;
            }
            new_block = false;

            s = text.split(QRegExp("\\s+"));
            //temperature
            temp = s.at(1);
            dvalue = temp.toDouble();
            dvalue /= 100.;
            temp = QString::number(dvalue,'f',1) + " " + QString::fromLatin1(ba);
            //time
            time = s.at(2);
            dvalue = time.toDouble();
            time = QDateTime::fromTime_t(dvalue).toString("mm:ss");
            //measure
            meas = s.at(6);
            val = meas.toShort();
            block->Measure.append(val);
            if(val == 2) block->Type_Block = bMelt;

            //gradient
            tmp = s.at(5);
            val = tmp.toShort();
            if(val != 0 && !gradient) val = 0;
            block->Gradient.append(val);
            gradient = false;
            //incr_time
            tmp = s.at(4);
            val = tmp.toShort();
            block->Incr_Time.append(val);
            //incr_temp
            tmp = s.at(3);
            val = tmp.toShort();
            block->Incr_Temp.append(val);

            if(i+1 < pr.size())
            {
                //i++;
                text = QString::fromStdString(pr.at(i+1));
                if(text.indexOf("XLEV") != 0 && text.indexOf("XTCH") != 0) new_block = true;
                if(text.indexOf("XPAU") == 0) {time = "Pause"; block->Type_Block = bPause;}
                if(text.indexOf("XHLD") == 0) {time = "Hold"; block->Type_Block = bHold;}
                if(text.indexOf("XCYC") == 0)
                {
                    s = text.split(QRegExp("\\s+"));
                    tmp = s.at(1);
                    block->Num_Cycle = tmp.toInt();
                }
            }

            block->Level.append(temp + " - " + time);
            str.append(temp + " - " + time);
        }        
    }

    // Height Pixmap:
    for(i=0; i<Blocks.size(); i++)
    {
        block = Blocks.at(i);
        height_pm += 16 + block->Level.size()*16;
    }

    QPixmap pixmap(240,height_pm);
    pixmap.fill(QColor("transparent"));
    //pixmap.fill();

    QPainter painter(&pixmap);
    QPen myPen(Qt::black, 1, Qt::SolidLine);
    painter.setPen(myPen);

    f = QApplication::font();
    f.setPointSize(8);
    painter.setFont(f);

    height_pm = 0;
    for(i=0; i<Blocks.size(); i++)
    {
        block = Blocks.at(i);
        if(block->Type_Block == bHold) myPen.setColor(Qt::blue);
        else myPen.setColor(Qt::black);
        painter.setPen(myPen);

        for(j=0; j<block->Level.size(); j++)
        {
            //... draw current(On_Line) level
            if(draw_block == i+1 && draw_level == j && block->Type_Block != bHold)
            {
                myPen.setColor(Qt::red);
                if(draw_status) f.setBold(true);
                painter.setFont(f);
                painter.setPen(myPen);
            }

            //...

            if(!j) y_0 = height_pm;
            height_pm += 16;
            if(!j) painter.drawText(5, height_pm, QString("%1.").arg(i+1));
            painter.drawText(20, height_pm, block->Level.at(j));

            offset = 115;
            if(block->Measure.at(j) == 1) {painter.drawPixmap(offset, height_pm-14, pixmap_foto); offset += 20;}
            if(block->Measure.at(j) == 2) {painter.drawPixmap(offset, height_pm-14, pixmap_fotoMC); offset += 20;}

            if(block->Incr_Temp.at(j) != 0)
            {
                pixmap_incr.load(":/images/incr_temp.png");
                painter.drawPixmap(offset, height_pm-14, pixmap_incr);
                offset += 20;
            }
            if(block->Incr_Time.at(j) != 0)
            {
                pixmap_incr.load(":/images/incr_time.png");
                painter.drawPixmap(offset, height_pm-14, pixmap_incr);
                offset += 20;
            }
            if(block->Gradient.at(j) != 0)
            {
                pixmap_incr.load(":/images/grad.png");
                if(grad_step) pixmap_incr.load(":/images/grad_step.png");
                painter.drawPixmap(offset, height_pm-14, pixmap_incr);
                offset += 20;
            }

            if(block->Type_Block != bHold)
            {
                myPen.setColor(Qt::black);
                f.setBold(false);
                painter.setFont(f);
                painter.setPen(myPen);
            }

        }
        y_1 = height_pm;
        height_pm += 16;


        if(block->Num_Cycle > 1)
        {
            y_0 += 2;
            y_1 += 2;
            polygon.clear();
            polygon << QPoint(200,y_0) << QPoint(205,y_0) << QPoint(205,y_1) << QPoint(200,y_1);
            painter.drawPolyline(polygon);
            text = QString(" x %1").arg(block->Num_Cycle);
            painter.drawText(205, y_0 + (y_1-y_0)/2 + 6, text);
        }
    }


    label_Program.setPixmap(pixmap);


    for(i=0; i<Blocks.size(); i++) delete Blocks.at(i);
    Blocks.clear();*/

    QVector<string> pr = QVector<string>::fromStdVector(p->program);
    if(pr.size() < 2) {clear_scheme(); return;}
    draw_Program(&pr, DRAW_Level);
}
