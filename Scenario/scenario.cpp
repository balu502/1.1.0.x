#include "scenario.h"

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Scenario::Scenario(QWidget *parent): QDialog(parent)
{
    QStringList header;
    active_scenario = false;
    current_point = -1;
    web_break = false;

    readCommonSettings();
    QFont f = qApp->font();
    f.setPointSize(f.pointSize()-2);
    setFont(f);

    timer_wait = new QTimer(this);
    timer_wait->setSingleShot(true);
    connect(timer_wait, SIGNAL(timeout()), this, SLOT(WaitProtocolfromWeb()));

    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);
    layout->setMargin(2);
    layout->setSpacing(2);

    Table_Scenario = new QTableWidget(0, 0, this);
    //Table_Scenario->setSelectionBehavior(QAbstractItemView::SelectRows);
    Table_Scenario->setSelectionMode(QAbstractItemView::NoSelection);
    Table_Scenario->setEditTriggers(QAbstractItemView::NoEditTriggers);
    Delegate = new Scenario_ItemDelegate;
    Delegate->active = &active_scenario;
    Delegate->current = &current_point;
    Table_Scenario->setItemDelegate(Delegate);
    Table_Scenario->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Minimum);
    Table_Scenario->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    /*
    header << tr("Name of point") << tr("Parameters of point") << tr("status");
    Table_Scenario->setHorizontalHeaderLabels(header);
    Table_Scenario->setColumnWidth(0, 150);
    Table_Scenario->setColumnWidth(1, 350);
    Table_Scenario->setColumnWidth(2, 50);
    */
    //Table_Scenario->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    //Table_Scenario->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

    Stop_Point = new Point_Action();
    Stop_Point->Unique_Name = "Stop";

    QHBoxLayout *control_layout = new QHBoxLayout;
    control_layout->setMargin(2);
    control_layout->setSpacing(1);
    Scenario_Start = new QPushButton(tr("Start"), this);
    Scenario_Stop = new QPushButton(tr("Stop"), this);
    Scenario_Open = new QPushButton(tr("Open"), this);
    Scenario_Clear = new QPushButton(tr("Clear"), this);
    active_label = new QLabel(this);
    active_label->setFixedSize(64,64);
    active_gif = new QMovie(":/images/active_1.gif");
    active_label->setMovie(active_gif);
    active_label->setVisible(false);
    label_status = new QLabel("", this);
    label_status->setFont(f);
    //control_layout->addWidget(active_label, 0, Qt::AlignLeft);
    control_layout->addWidget(label_status, 1, Qt::AlignLeft);
    control_layout->addWidget(Scenario_Start, 1, Qt::AlignRight);
    control_layout->addWidget(Scenario_Stop, 0, Qt::AlignRight);
    control_layout->addWidget(Scenario_Open, 0, Qt::AlignRight);
    control_layout->addWidget(Scenario_Clear, 0, Qt::AlignRight);

    layout->addWidget(Table_Scenario);
    //layout->addStretch(1);
    layout->addLayout(control_layout);

    connect(Scenario_Open, SIGNAL(clicked(bool)), this, SLOT(Open_Scenario()));
    connect(Scenario_Start, SIGNAL(clicked(bool)), this, SLOT(Start_Scenario()));
    connect(Scenario_Clear, SIGNAL(clicked(bool)), this, SLOT(Clear_Scenario()));
    connect(Scenario_Stop, SIGNAL(clicked(bool)), this, SLOT(slot_StopScenario()));

    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowTitle(tr("Scenario Actions"));    
    //setWindowIcon(QIcon(":/images/DTm.ico"));
    //setWindowIcon(QIcon(":/images/RT.ico"));
    resize(700,50);
    move(20,20);
    hide();

    Change_Status();
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Scenario::~Scenario()
{
    Clear_Scenario();
    Map_Receivers.clear();

    delete timer_wait;

    Table_Scenario->clear();
    delete Table_Scenario;

    delete Scenario_Start;
    delete Scenario_Stop;
    delete Scenario_Open;
    delete Scenario_Clear;

    delete Stop_Point;
}
//-----------------------------------------------------------------------------
//--- readCommonSettings()
//-----------------------------------------------------------------------------
void Scenario::readCommonSettings()
{
    QString text;
    QString dir_path = qApp->applicationDirPath();
    QSettings *CommonSettings = new QSettings(dir_path + "/tools/ini/preference.ini", QSettings::IniFormat);

    CommonSettings->beginGroup("Common");

        //... Language ...
        text = CommonSettings->value("language","ru").toString();
        if(translator.load(":/translations/scenario_" + text + ".qm"))
        {
            qApp->installTranslator(&translator);
        }

    CommonSettings->endGroup();
    delete CommonSettings;
}
//-----------------------------------------------------------------------------
//--- Clear_Scenario()
//-----------------------------------------------------------------------------
void Scenario::Clear_Scenario()
{
    Point_Action    *point;
    foreach(point, List_PointAction)
    {
        delete point;
    }
    List_PointAction.clear();
    active_scenario = false;
    name_Scenario = "";
    current_point = -1;
    label_status->setText("");

    Table_Scenario->clear();
    Table_Scenario->setColumnCount(0);
    Table_Scenario->setRowCount(0);

    Change_Status();
}
//-----------------------------------------------------------------------------
//--- slot_StopScenario()
//-----------------------------------------------------------------------------
void Scenario::slot_StopScenario()
{
    // 1.
    Point_Action *point = List_PointAction.at(current_point);
    Stop_Point->receiver = point->receiver;
    Stop_Point->sender = this;
    scenario_Event.point_action = Stop_Point;
    QApplication::sendEvent(Stop_Point->receiver, &scenario_Event);

    // 2.
    Stop_Scenario();
}

//-----------------------------------------------------------------------------
//--- Stop_Scenario()
//-----------------------------------------------------------------------------
void Scenario::Stop_Scenario()
{
    QDomDocument doc;
    QDomElement  request;
    QDomElement  element;
    QDomElement  child;
    QString message;
    QString state = "finished";
    int code = 2;

    if(!name_Scenario.isEmpty())
    {
        doc.clear();
        QDomNode xmlNode = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\"");
        doc.insertBefore(xmlNode, doc.firstChild());
        QDomElement  root = doc.createElement("answer");
        doc.appendChild(root);
        root.setAttribute("time", QString("%1").arg(QDateTime::currentMSecsSinceEpoch()));
        root.appendChild(MakeElement(doc,"name", name_Scenario));
        root.appendChild(MakeElement(doc, "status", "0"));

        if(web_break) {state = "break"; code = 4;}

        child = MakeElement(doc, "execute_status", state);
        child.setAttribute("code", QString::number(code));
        root.appendChild(child);

        message = "...;" + doc.toString();
        doc.clear();
        emit sStatus_ExecutePoint(message);
    }
    active_scenario = false;    
    Change_Status();
}
//-----------------------------------------------------------------------------
//--- Start_Scenario()
//-----------------------------------------------------------------------------
void Scenario::Start_Scenario()
{
    int i;
    active_scenario = true;
    label_status->setText("");
    current_point = -1;
    QTableWidgetItem *item;

    for(i=0; i<Table_Scenario->rowCount(); i++)
    {
        item = Table_Scenario->item(i,2);
        item->setText("");
    }

    if(List_PointAction.size() > 0)
    {
        RunNextPoint();
    }
    else Clear_Scenario();

    Change_Status();
}
//-----------------------------------------------------------------------------
//--- RunNextPoint()
//-----------------------------------------------------------------------------
void Scenario::RunNextPoint(bool next)
{
    Point_Action    *point, *p_pa;
    QString text;
    QString barcode = "";
    QString name = "";
    QString error_msg = "";
    QFile file;
    int h,w;
    h = this->height();
    w = this->width();

    if(!active_scenario) return;

    if(next) current_point++;
    point = List_PointAction.at(current_point);
    point->sender = this;
    scenario_Event.point_action = point;



    //label_table = new QLabel("pp", this);
    //Table_Scenario->setCellWidget(current_point, 2, label_table);

    //qDebug() << "Start active Point: " << point->Unique_Name;

    // check on RUN_PROTOCOL
    if(point->Unique_Name == RUN_PROTOCOL)
    {
        Sleep(1000);
    }


    // check on Analysis_Protocol
    if(point->Unique_Name == ANALYSIS_PROTOCOL && point->Param.value("FileName", "").isEmpty())
    {
        foreach(p_pa, List_PointAction)
        {
            if(p_pa->Unique_Name == RUN_PROTOCOL)
            {
                text = p_pa->Param.value("FileName_Measurements", "");
                if(!text.isEmpty())
                {
                    file.setFileName(text);
                    if(file.exists())
                    {
                        point->Param.insert("FileName", text);
                    }
                }
                break;
            }
        }        
    }
    //...
    // check on OpenProtocolBarcode
    if(point->Unique_Name == OPEN_PROTOCOL_BARCODE && point->Param.value("FileName","").isEmpty())
    {
        foreach(p_pa, List_PointAction)
        {
            if(p_pa->Unique_Name == GET_BARCODE)
            {
                barcode = p_pa->Param.value("barcode", "");
                if(!barcode.isEmpty())
                {
                    name = List_Protocols->value(barcode,"");
                }
                break;
            }
        }
        if(name.isEmpty()) error_msg = "don't find protocol";
        else
        {
            name = "";
            emit sGetProtocolFromList(name, barcode);
            timer_wait->start(5000);
            return;
        }
    }
    //...
    // check on OpenProtocolID
    if(point->Unique_Name == OPEN_PROTOCOL_ID && point->Param.value("FileName","").isEmpty())
    {
        text = point->Param.value("ID_Protocol","");
        if(text.isEmpty()) error_msg = "don't find ID_protocol";
        else
        {
            qDebug() << "List: " <<  *List_Protocols;
            emit sGetProtocolFromList("", text);
            timer_wait->start(5000);
            return;
        }

    }


    text = "   " + point->Unique_Name + ": ";
    label_status->setText(text);

    // ...

    if(!error_msg.isEmpty())
    {
        point->Param.insert("status", "0");
        point->Param.insert("error", error_msg);
        QApplication::sendEvent(this, &scenario_Event);
        return;
    }

    emit sSignalToReceiver(point->receiver);
    QApplication::sendEvent(point->receiver, &scenario_Event);

    repaint();
    qApp->processEvents();
}
//-----------------------------------------------------------------------------
//--- Open_Scenario()
//-----------------------------------------------------------------------------
void Scenario::Run_Scenario(QString fn, QMap<QString,QString> map_param)
{
    QString name;
    QString text;
    Point_Action *pa;
    QStringList list_param;
    QStringList list_value;

    if(fn.isEmpty()) return;
    if(active_scenario) return;

    QFile file(fn);
    if(!file.exists()) return;

    if(isHidden()) show();

    Clear_Scenario();
    if(open_xmlScenario(fn))
    {
        if(map_param.size())
        {
            foreach(name, map_param.keys())
            {
                foreach(pa, List_PointAction)
                {
                    if(pa->Unique_Name == name)
                    {
                        list_param = map_param.value(name).split(";");
                        foreach(text, list_param)
                        {
                            list_value = text.split("=");
                            if(list_value.size() > 1)
                            {
                                pa->Param.insert(list_value.at(0), list_value.at(1));
                            }
                        }

                        break;
                    }
                }
            }
        }


        if(List_PointAction.size())
        {
            load_TableScenario();
        }

        name_Scenario = map_param.value("Name_Request", "");
        web_break = false;

        Start_Scenario();
    }
}

//-----------------------------------------------------------------------------
//--- Open_Scenario()
//-----------------------------------------------------------------------------
void Scenario::Open_Scenario(QString fn)
{
    QString fileName;
    QString selectedFilter;
    QString dir = qApp->applicationDirPath() + "/scenario";

    if(fn.isEmpty())
    {
        fileName = QFileDialog::getOpenFileName(this,
                                                tr("Open Scenario"),
                                                dir,
                                                tr("Scenarioo File (*.sce)"),
                                                &selectedFilter);
        fn = fileName;
    }


    if(fn.isEmpty()) return;

    Clear_Scenario();
    if(open_xmlScenario(fn))
    {
        if(List_PointAction.size())
        {
            load_TableScenario();
        }
    }
}
//-----------------------------------------------------------------------------
//--- Open_Scenario()
//-----------------------------------------------------------------------------
void Scenario::Open_CalibrationScenario()
{
    QString fn = qApp->applicationDirPath() + "/scenario/Validation_96.sce";

    //qDebug() << "fn: " << fn;

    //if(isHidden()) show();
    //Open_Scenario(fn);
}

//-----------------------------------------------------------------------------
//--- Open_Scenario()
//-----------------------------------------------------------------------------
bool Scenario::open_xmlScenario(QString fn)
{
    int i,j,k;
    bool sts = false;
    Point_Action    *p_action;

    QFile file(fn);
    if(!file.exists()) return(sts);

    QDomDocument    doc;
    QDomElement     root;
    QDomElement     Points;
    QDomNode        node;
    QDomNode        item;
    QDomNode        item_ch;
    QDomElement     name;
    QDomElement     value;

    if(file.exists() && file.open(QIODevice::ReadOnly))
    {
        if(!doc.setContent(&file))
        {
            file.close();
            return(sts);
        }
        file.close();

        root = doc.documentElement();

        //root.firstChildElement("name").text();
        Points = root.firstChildElement("Points");
        for(i=0; i<Points.childNodes().size(); i++)
        {
            item = Points.childNodes().at(i);
            p_action = new Point_Action();
            for(j=0; j<item.childNodes().size(); j++)
            {
                node = item.childNodes().at(j);

                if(node.nodeName() == "name")                               // name
                {
                    p_action->Unique_Name = node.toElement().text();
                }
                if(node.nodeName() == "receiver")                           // receiver
                {
                    p_action->receiver = Map_Receivers.value(node.toElement().text(), NULL);
                }
                if(node.nodeName() == "Parameters")                         // Parameters
                {
                    for(k=0; k<node.childNodes().size(); k++)
                    {
                        item_ch = node.childNodes().at(k);
                        name = item_ch.firstChildElement("name");
                        value = item_ch.firstChildElement("value");
                        if(name.isElement() && value.isElement())
                        {
                            p_action->Param.insert(name.text(), value.text());
                        }
                    }
                }
            }
            //qDebug() << "p_action: " << p_action->Unique_Name << p_action->Param;

            List_PointAction.append(p_action);
        }


        sts = true;
    }

    return(sts);
}
//-----------------------------------------------------------------------------
//--- load_TableScenario()
//-----------------------------------------------------------------------------
void Scenario::load_TableScenario()
{
    int i,j,k;
    QString text;
    Point_Action    *p_action;
    QTableWidgetItem *newItem;
    QStringList header;
    int h,num;

    Table_Scenario->setColumnCount(3);
    header << tr("Name of point") << tr("Parameters of point") << tr("status");
    Table_Scenario->setHorizontalHeaderLabels(header);
    Table_Scenario->setColumnWidth(0, 300);
    Table_Scenario->setColumnWidth(1, 350);
    Table_Scenario->setColumnWidth(2, 80);
    Table_Scenario->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    Table_Scenario->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    Table_Scenario->setRowCount(List_PointAction.size());

    for(i=0; i<List_PointAction.size(); i++)
    {
        p_action = List_PointAction.at(i);
        h = Table_Scenario->rowHeight(i);
        num = 0;

        for(j=0; j<Table_Scenario->columnCount(); j++)
        {
            newItem = new QTableWidgetItem();
            switch(j)
            {
            case 0:     text = p_action->Unique_Name;
                        if(text == OPEN_PROTOCOL) {text = tr("OpenProtocol"); break;}
                        if(text == OPEN_DEVICE) {text = tr("OpenDevice"); break;}
                        if(text == RUN_PROTOCOL) {text = tr("RunProtocol"); break;}
                        if(text == RUN_STOP) {text = tr("Stop"); break;}
                        if(text == GET_BARCODE) {text = tr("GetBarcode"); break;}
                        if(text == OPEN_PROTOCOL_BARCODE) {text = tr("OpenProtocolBarcode"); break;}
                        if(text == SCANCOEFFICIENTS) {text = tr("ScanCoefficients"); break;}
                        if(text == SET_PLATE) {text = tr("SetPlate"); break;}
                        if(text == OPEN_PROTOCOL_ID) {text = tr("OpenProtocolID"); break;}
                        if(text == CHANGE_EXPOSURE) {text = tr("ChangeExposure"); break;}
                        if(text == CHANGE_HEIGHT_TUBES) {text = tr("ChangeHeightTubes"); break;}
                        if(text == BACKUP_EXPOSURE) {text = tr("BackupExposure"); break;}
                        if(text == BACKUP_HEIGHT_TUBES) {text = tr("BackupHeightTubes"); break;}
                        if(text == ANALYSIS_PROTOCOL) {text = tr("AnalysisProtocol"); break;}
                        if(text == OPENBLOCK) {text = tr("OpenBlock"); break;}
                        if(text == CLOSEBLOCK) {text = tr("CloseBlock"); break;}

                        break;
            case 1:
                        text = "";
                        for(k=0; k<p_action->Param.size(); k++)
                        {
                            if(k) text += "\r\n";
                            text += p_action->Param.keys().at(k) + ":  " + p_action->Param.values().at(k);
                            num++;
                        }
                        break;

            case 2:     text = "";  break;
            default:    text = "";  break;
            }
            newItem->setText(text);
            Table_Scenario->setItem(i, j, newItem);
        }
        if(num > 1) Table_Scenario->setRowHeight(i, h*num*0.7);
    }

}
//-----------------------------------------------------------------------------
//--- Change_Status()
//-----------------------------------------------------------------------------
void Scenario::Change_Status()
{
    bool sts = active_scenario;
    int h,w;
    h = this->height();
    w = this->width();

    Scenario_Open->setDisabled(sts);
    Scenario_Start->setDisabled(sts);
    Scenario_Stop->setDisabled(!sts);
    Scenario_Clear->setDisabled(sts);    

    active_label->setVisible(sts);
    if(sts) active_gif->start();
    else active_gif->stop();

    h++;
    resize(w,h);
    h--;
    resize(w,h);
}
//-----------------------------------------------------------------------------
//--- GetRunButton()
//-----------------------------------------------------------------------------
void Scenario::GetRunButton()
{
    QString dir = qApp->applicationDirPath();
    if(active_scenario)
    {

        return;
    }

    // 1. clear and open *.sce
    QFile fn(dir + "/scenario/barcode_scenario.sce");
    if(!fn.exists())
    {
        qDebug() << "not file";
        return;
    }
    Open_Scenario(fn.fileName());

    // 2. Run scenario
    Start_Scenario();

}
//-----------------------------------------------------------------------------
//--- GetProtocolFromWeb()
//-----------------------------------------------------------------------------
void Scenario::GetProtocolFromWeb(QString fn)
{
    Point_Action *point;
    //qDebug() << "protocol from web: " << fn;
    QFile file(fn);

    timer_wait->stop();

    if(file.exists())
    {
        if(active_scenario && current_point >= 0)
        {
            point = List_PointAction.at(current_point);
            if(point->Unique_Name == OPEN_PROTOCOL_BARCODE || point->Unique_Name == OPEN_PROTOCOL_ID)
            {
                point->Param.insert("FileName", fn);
                RunNextPoint(false);
            }
        }
    }
}
//-----------------------------------------------------------------------------
//--- WaitProtocolfromWeb()
//-----------------------------------------------------------------------------
void Scenario::WaitProtocolfromWeb()
{
    Point_Action *point;
    if(active_scenario && current_point >= 0)
    {
        point = List_PointAction.at(current_point);
        point->Param.insert("status", "0");
        point->Param.insert("error", "don't get protocol from web");
        QApplication::sendEvent(this, &scenario_Event);
    }

}
//-------------------------------------------------------------------------------
//--- CreateInfoExecutePoint
//-------------------------------------------------------------------------------
void Scenario::CreateInfoExecutePoint(Action_Event *p)
{
    QString message;
    bool ok;
    bool state;
    QDomDocument doc;

    QDomNode xmlNode = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\"");
    doc.insertBefore(xmlNode, doc.firstChild());

    QDomElement  root = doc.createElement("notice");
    doc.appendChild(root);

    root.setAttribute("time", QString("%1").arg(QDateTime::currentMSecsSinceEpoch()));

    state = p->point_action->Param.value("status","").toInt(&ok);
    if(!ok) state = false;

    root.appendChild(MakeElement(doc,"status",QString("%1").arg((short)(!state))));
    root.appendChild(MakeElement(doc,"name",p->point_action->Unique_Name));

    message = p->point_action->Unique_Name;
    if(state) message += ": ok";
    else message += ": " + p->point_action->Param.value("error","");
    root.appendChild(MakeElement(doc,"execute_status",message));

    message = "...;" + doc.toString();      // id_protocol == ... (unknown)

    doc.clear();
    emit sStatus_ExecutePoint(message);
}

//-------------------------------------------------------------------------------
//--- User events
//-------------------------------------------------------------------------------
bool Scenario::event(QEvent *e)
{
    bool sts = false;
    bool ok;
    QTableWidgetItem *item;
    QString text;


    // 1 Point_Action Event
    if(e->type() == 2002 && active_scenario)
    {        
        Action_Event *p_event = static_cast<Action_Event *>(e);
        if(p_event)
        {
            //qDebug() << "Scenario p_event: " << p_event->point_action->Unique_Name << p_event->point_action->Param.value("status");
            //... signal to WEB ...
            CreateInfoExecutePoint(p_event);
            //...

            sts = p_event->point_action->Param.value("status").toInt(&ok);
            item = Table_Scenario->item(current_point,2);
            item->setText(QString::number((int)sts));

            if(sts && current_point < (List_PointAction.size()-1)) RunNextPoint();
            else
            {                
                Stop_Scenario();
                if(sts && p_event->point_action->Unique_Name == ANALYSIS_PROTOCOL) hide();
            }

            text = "   " + p_event->point_action->Unique_Name + ": " + p_event->point_action->Param.value("error","");
            label_status->setText(text);



        }

        Change_Status();
        repaint();
        return(true);
    }

    return QWidget::event(e);
}
//-------------------------------------------------------------------------------
//--- resizeEvent
//-------------------------------------------------------------------------------
void Scenario::resizeEvent(QResizeEvent *e)
{
    int w = width();
    int h = height();

    active_label->move(w/2 - active_label->width()/2, h/2 - active_label->height()/2);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Scenario_ItemDelegate::paint(QPainter *painter,
                                 const QStyleOptionViewItem &option,
                                 const QModelIndex &index) const
{
    int i,id;
    QRect rect;
    QString text,str;
    int flag;
    bool ok;
    QStringList list;

    QStyleOptionViewItem  viewOption(option);
    viewOption.state &= ~QStyle::State_HasFocus;    // disable state HasFocus
    QStyledItemDelegate::paint(painter, viewOption, index);

    rect = option.rect;
    text = index.data().toString();
    int row = index.row();
    int col = index.column();

    QPixmap pixmap(":/images/check.png");

    if(*active) painter->setPen(Qt::black);
    else        painter->setPen(Qt::gray);

    //__1. Background

    if(row == *current && *active)
    {
        painter->fillRect(option.rect, QColor(215,215,255));
    }
    else
    {
        painter->fillRect(rect, QColor(250,250,250));
    }
    //painter->fillRect(rect, QColor(250,250,250));

    //__2. Data
    switch(col)
    {
    case 0: flag = flag = Qt::AlignHCenter | Qt::AlignVCenter;
            painter->drawText(option.rect, flag, text);
            break;

    case 1: flag = Qt::AlignLeft | Qt::AlignVCenter;
            str = "";
            list = text.split("\r\n");
            for(i=0; i<list.size(); i++)
            {
                if(i) str += "\r\n";
                str += "    " + list.at(i);
            }
            text = str;
            painter->drawText(option.rect, flag, text);
            break;

    case 2:
            if(!text.trimmed().isEmpty())
            {
                id = text.toInt(&ok);
                if(ok)
                {
                    switch(id)
                    {
                    case 0:     pixmap.load(":/images/error.png");
                                break;

                    default:    break;
                    }
                    painter->drawPixmap(option.rect.x() + (option.rect.width()-16)/2,
                                        option.rect.y() + (option.rect.height()-16)/2,
                                        pixmap);
                }
            }
    }
}
