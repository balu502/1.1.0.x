#include "listdev.h"
#include "../Run/request_dev.h"

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
ListDev::ListDev(QWidget *parent, QString ip, int port): QDialog(parent)
{    
    p_obj = parent;
    p_main = NULL;
    //current_dev = 0;
    current_dev.clear();
    QString text;

    //application_name = "Calibr";

    readCommonSettings();
    setFont(qApp->font());

    //ip_addres = "localhost";
    ip_addres = "127.0.0.1";
    port_serverUSB = port;

    /*foreach (const QHostAddress &address, QNetworkInterface::allAddresses())
    {
        if(address.protocol() == QAbstractSocket::IPv4Protocol &&
           address != QHostAddress(QHostAddress::LocalHost))
        {
             qDebug() << address.toString();
             ip_addres = address.toString();
             text += "  :" + address.toString();
        }
    }*/

    setWindowTitle(tr("Devices List"));
    //setWindowIcon(QIcon(":/images/adjustment.ico"));
    setFixedSize(450,200);
    setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint);
    /*app_font = parent->font();
    app_font.setPointSize(app_font.pointSize()-2);
    app_font.setBold(false);
    this->setFont(app_font);*/

    QVBoxLayout *layout = new QVBoxLayout();
    this->setLayout(layout);
    QHBoxLayout *ctrl_layout = new QHBoxLayout();


    Dev_Table = new QTableWidget();    
    Dev_Table->setFont(qApp->font());
    Delegate = new ListDev_ItemDelegate;
    Dev_Table->setItemDelegate(Delegate);
    Dev_Table->setSelectionBehavior(QAbstractItemView::SelectRows);
    Dev_Table->setSelectionMode(QAbstractItemView::SingleSelection);
    Dev_Table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    connect_a5d411 = new QPushButton("...", this);
    connect_a5d411->setVisible(false);
    if(!Device_Debug.isEmpty() && !Port_Debug.isEmpty() && !IP_Debug.isEmpty()) connect_a5d411->setVisible(true);
    //connect_a5d411->setDisabled(true);
    connect_dev = new QPushButton(tr("connect"), this);
    disconnect_dev = new QPushButton(tr("disconnect"), this);
    cancel_dev = new QPushButton(tr("Cancel"), this);
    connect_dev->setEnabled(false);
    disconnect_dev->setEnabled(false);
    //ctrl_layout->addStretch(1);
    ctrl_layout->addWidget(connect_a5d411,1,Qt::AlignLeft);
    ctrl_layout->addWidget(connect_dev,1,Qt::AlignRight);
    ctrl_layout->addWidget(disconnect_dev,0,Qt::AlignRight);
    ctrl_layout->addWidget(cancel_dev,0,Qt::AlignRight);
    ctrl_layout->setSpacing(2);
    layout->setMargin(4);

    layout->addWidget(Dev_Table,1);
    layout->addLayout(ctrl_layout);

    connect(cancel_dev, SIGNAL(clicked(bool)), this, SLOT(close()));
    connect(connect_dev, SIGNAL(clicked(bool)), this, SLOT(slotConnectDev()));
    connect(connect_a5d411, SIGNAL(clicked(bool)), this, SLOT(slotConnectDev_a5d411()));
    connect(disconnect_dev, SIGNAL(clicked(bool)), this, SLOT(slotDisconnectDev()));

    connect(Dev_Table, SIGNAL(itemSelectionChanged()), this, SLOT(slotCheckSelection()));
    //connect(Dev_Table, SIGNAL(itemDoubleClicked(QTableWidgetItem*)), this,  SLOT(slotApply_ConnectAction(QTableWidgetItem*)));


    //... NetWork ...
    m_nNextBlockSize = 0;
    m_pTcpSocket = new QTcpSocket(this);

    connect(m_pTcpSocket, SIGNAL(connected()), SLOT(slotConnected()));
    connect(m_pTcpSocket, SIGNAL(readyRead()), SLOT(slotReadyRead()));
    connect(m_pTcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
                this,     SLOT(slotError(QAbstractSocket::SocketError)));
    connect(&dev_reconnected, SIGNAL(timeout()), this, SLOT(sReConnected()));


    s_nNextBlockSize = 0;
    s_pTcpSocket = new QTcpSocket(this);

    connect(s_pTcpSocket, SIGNAL(connected()), SLOT(slotConnected_status()));
    connect(s_pTcpSocket, SIGNAL(readyRead()), SLOT(slotReadyRead_status()));
    connect(s_pTcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
                this,     SLOT(slotError_status(QAbstractSocket::SocketError)));
    connect(this, SIGNAL(signal_connect(int)), this, SLOT(slotConnection_status(int)));

    PA_active = false;
    PA_DeviceName = "";
    connect(this, SIGNAL(sPA_listDevice()), this, SLOT(PA_listDevice()));
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
ListDev::~ListDev()
{        
    int i;    

    for(i = Dev_Table->rowCount(); i>0; i--) Dev_Table->removeRow(i-1);
    Dev_Table->clear();
    delete Dev_Table;    

    m_pTcpSocket->disconnect();
    m_pTcpSocket->close();

    s_pTcpSocket->disconnect();
    s_pTcpSocket->close();

}
//-----------------------------------------------------------------------------
//--- readCommonSettings()
//-----------------------------------------------------------------------------
void ListDev::readCommonSettings()
{
    QString text;
    QString dir_path = qApp->applicationDirPath();
    QSettings *CommonSettings = new QSettings(dir_path + "/tools/ini/preference.ini", QSettings::IniFormat);

    CommonSettings->beginGroup("Common");

        //... Language ...
        text = CommonSettings->value("language","ru").toString();
        if(translator.load(":/translations/listdev_" + text + ".qm"))
        {
            qApp->installTranslator(&translator);
        }

        //... Server_Debug ...
        //qDebug() << "Server_Debug - 0";
        Device_Debug = CommonSettings->value("Device_Debug","").toString().trimmed();
        Port_Debug = CommonSettings->value("Port_Debug","").toString().trimmed();
        IP_Debug = CommonSettings->value("IP_Debug","").toString().trimmed();
        //qDebug() << "Server_Debug:" <<  Device_Debug << Port_Debug;

    CommonSettings->endGroup();


    delete CommonSettings;
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void ListDev::showEvent(QShowEvent *event)
{
    int i;
    QStringList header;

    //qDebug() << "socket state (show) = " << m_pTcpSocket->state();

    if(m_pTcpSocket->state() == QAbstractSocket::UnconnectedState)
    {
        m_pTcpSocket->connectToHost("localhost", port_serverUSB);
        //m_pTcpSocket->connectToHost("192.168.0.55", 9001);

    }

    for(i = Dev_Table->rowCount(); i>0; i--) Dev_Table->removeRow(i-1);
    Dev_Table->clear();
    header << tr("SerialNumber") << tr("type") << tr("Status") << tr("IP:port");
    Dev_Table->setColumnCount(4);
    Dev_Table->setHorizontalHeaderLabels(header);
    Dev_Table->setColumnWidth(0,80);
    Dev_Table->setColumnWidth(1,70);
    Dev_Table->setColumnWidth(2,100);
    Dev_Table->setColumnWidth(3,120);
    Dev_Table->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    Dev_Table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    Dev_Table->horizontalHeader()->setFont(qApp->font());

    Client_Net.clear();
    Client_Net.append("list_devices");

    slotSendToServer();

    //... p_main ...
    foreach(QWidget *widget, QApplication::topLevelWidgets())
    {
        if(widget->windowTitle().startsWith(application_name))
        {
            p_main = widget;
            break;
        }
    }

    QWidget::showEvent(event);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void ListDev::closeEvent(QCloseEvent *event)
{
    PA_active = false;
    PA_DeviceName = "";    

    QWidget::closeEvent(event);
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void ListDev::slotConnectDev()
{
    int id;
    QString text, name, port, ip;


    if(Dev_Table->selectedItems().size())
    {
        id = Dev_Table->selectedItems().at(0)->row();
        name = Dev_Table->item(id,0)->text();
        port = Dev_Table->item(id,3)->text().split(":").at(1);
        ip = Dev_Table->item(id,3)->text().split(":").at(0);
        //ip = "192.168.0.55";

        Client_Net.clear();
        Client_Net.append("connect");
        Client_Net.append(name);
        Client_Net.append(port);

        connect_event.port = port.toInt();
        connect_event.ip = ip;
        slotSendToServer();

        //qDebug() << name << port << ip << Client_Net << application_name;

        if(p_main)
        {
            text = application_name + " " + name;
            p_main->setWindowTitle(text);
        }
        p_main->setProperty("Device_State", QVariant(name));
        /*foreach(QWidget *widget, QApplication::topLevelWidgets())
        {
            text = widget->windowTitle();
            //qDebug() << "title = " << text;
            if(text.startsWith(application_name))
            {
                text = application_name + " " + name;
                widget->setWindowTitle(text);
                break;
            }
        }*/

    }
    //close();
}
//-----------------------------------------------------------------------------
//--- slotConnectDev_a5d411
//-----------------------------------------------------------------------------
void ListDev::slotConnectDev_a5d411()
{
    int id;
    QString text, name, port, ip;


        name = Device_Debug;    //"A5H607";
        port = Port_Debug;      //"9012";
        ip = IP_Debug;          //ip = "127.0.0.1";

        Client_Net.clear();
        Client_Net.append("connect");
        Client_Net.append(name);
        Client_Net.append(port);

        connect_event.port = port.toInt();
        connect_event.ip = ip;
        slotSendToServer();

        if(p_main)
        {
            text = application_name + " " + name;
            p_main->setWindowTitle(text);
        }
        p_main->setProperty("Device_State", QVariant(name));
        /*foreach(QWidget *widget, QApplication::topLevelWidgets())
        {
            text = widget->windowTitle();
            //qDebug() << "title = " << text;
            if(text.startsWith(application_name))
            {
                text = application_name + " " + name;
                widget->setWindowTitle(text);
                break;
            }
        }*/        

        //qDebug() << "connect...(send)" << Client_Net;

    //close();
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void ListDev::slotDisconnectDev()
{
    int id;
    QString text, name;

    if(Dev_Table->selectedItems().size())
    {
        id = Dev_Table->selectedItems().at(0)->row();
        name = Dev_Table->item(id,0)->text();

        Client_Net.clear();
        Client_Net.append("disconnect");
        Client_Net.append(name);


        if(p_main)
        {
            text = application_name;
            p_main->setWindowTitle(text);
        }
        p_main->setProperty("Device_State", QVariant(""));
        /*foreach(QWidget *widget, QApplication::topLevelWidgets())
        {
            text = widget->windowTitle();
            //qDebug() << "title = " << text;
            if(text.startsWith(application_name))
            {
                text = application_name;
                widget->setWindowTitle(text);
                break;
            }
        }*/

        slotSendToServer();

        //qDebug() << "disconnect..." << Client_Net;
    }


}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void ListDev::slotRefresh()
{
    Client_Net.clear();
    Client_Net.append("refresh");
    slotSendToServer();
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void ListDev::slotCheckSelection()
{
    QTableWidgetItem *item;
    bool ok;

    HWND handle;
    QString name_win;
    LPCWSTR w_str;

    if(!p_main) return;

    //qDebug() << "slotCheckSelection";

    if(Dev_Table->selectedItems().isEmpty())
    {
        connect_dev->setEnabled(false);
        disconnect_dev->setEnabled(false);
        return;
    }


    int id = Dev_Table->selectedItems().at(0)->row();
    //qDebug() << id;

    item = Dev_Table->item(id,0);
    QString name_dev = item->text();
    item = Dev_Table->item(id,2);
    QString status_dev = item->text();
    //QString name_appl = p_main->windowTitle().split(" ").at(1);

    //qDebug() << name_dev << status_dev ;

    if(status_dev == "READY") {connect_dev->setEnabled(true); disconnect_dev->setEnabled(false);}
    //if(status_dev == "BUSY" && name_appl == name_dev) disconnect_dev->setEnabled(true);
    if(status_dev == "BUSY") {connect_dev->setEnabled(false); disconnect_dev->setEnabled(false);}
    if(status_dev == "CONNECTED") {connect_dev->setEnabled(false); disconnect_dev->setEnabled(true);}

    name_win = "RealTime_PCR_" + Dev_Table->item(id,0)->text().trimmed();
    w_str = (const wchar_t*)name_win.utf16();
    handle = FindWindow(NULL, w_str);

    if(handle) {connect_dev->setEnabled(false); disconnect_dev->setEnabled(false);}

}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void ListDev_ItemDelegate::paint(QPainter *painter,
                                 const QStyleOptionViewItem &option,
                                 const QModelIndex &index) const
{
    int i,id;
    QRect rect;
    QString text;
    QFont f = qApp->font();

    QPalette pal = option.palette;
    QStyleOptionViewItem  viewOption(option);
    viewOption.state &= ~QStyle::State_HasFocus;    // disable state HasFocus
    QStyledItemDelegate::paint(painter, viewOption, index);

    rect = option.rect;
    text = index.data().toString();

    //__1. Background
    int row = index.row();
    int col = index.column();
    if(option.state & QStyle::State_Selected)
    {
        painter->fillRect(option.rect, QColor(215,215,255));
    }
    else
    {
        painter->fillRect(rect, QColor(250,250,250));
    }
    painter->setFont(f);

    //__2. Data    
    painter->setPen(QPen(Qt::black,1,Qt::SolidLine));
    if(index.flags() == QStyle::State_None) painter->setPen(QPen(Qt::gray,1,Qt::SolidLine));

    if(col == 2 && text.contains("RT 7.9"))
    {
        painter->setPen(QPen(Qt::red,1,Qt::SolidLine));
    }

    painter->drawText(option.rect, Qt::AlignHCenter | Qt::AlignVCenter, text);
}

//======================== NETWORK ============================================
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void ListDev::slotConnected_status()
{
    //qDebug() << "connect:";
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void ListDev::slotConnection_status(int num)
{
    int count_dev = Dev_Table->rowCount();
    if(num >= count_dev) return;

    int port;
    QString address;
    QString text;
    QTableWidgetItem *item = Dev_Table->item(num,3);
    text = item->text();
    if(text.trimmed().isEmpty())
    {
        return;
        //item->setText("192.168.0.13:9002");
        item->setText(ip_addres +  ":9002");
        text = item->text();
    }

    //qDebug() << "Connection_status: " << text;

    address = text.split(":").at(0);
    text = text.split(":").at(1);
    port = text.toInt();

    if(s_pTcpSocket->state() == QAbstractSocket::UnconnectedState || s_pTcpSocket->waitForDisconnected(100))
    {
        s_pTcpSocket->connectToHost(address, port);
    }
    else
    {
        //qDebug() << "Unable connect with " << address << port;
    }
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void ListDev::slotReadyRead_status()
{
    int i,j;
    QString answer, text, name;
    short status;
    QMap<QString,QString> map;
    QTableWidgetItem *item;
    QStringList list = p_main->windowTitle().split(" ");
    QString name_dev;

    if(list.size() > 1) name_dev = list.at(1);


    QDataStream in(s_pTcpSocket);
    in.setVersion(QDataStream::Qt_4_7);

    for(;;)
    {
        if(!s_nNextBlockSize)
        {
            if(s_pTcpSocket->bytesAvailable() < sizeof(quint32)) break;
            in >> s_nNextBlockSize;
        }

        if(s_pTcpSocket->bytesAvailable() < s_nNextBlockSize) break;

        in >> answer >> status >> map;
        //qDebug() << "server_dev: " << answer << status << map;
        s_nNextBlockSize = 0;
     }

    s_pTcpSocket->disconnectFromHost();

    if(answer == CONNECT_REQUEST && status == 0 && !map.isEmpty())
    {
        item = Dev_Table->item(current_dev.at(0),0);
        name = item->text().trimmed();

        text = map.value(CONNECT_STATUS,"???");
        if(text == "BUSY" && name == name_dev) text = "CONNECTED";
        item = Dev_Table->item(current_dev.at(0),2);
        item->setText(text);

        current_dev.removeFirst();
        //current_dev++;

        /*for(i=current_dev+1; i<Dev_Table->rowCount(); i++)
        {
            text = Dev_Table->item(i,3)->text().trimmed();
            if(text.length()) {current_dev = i; break;}
            else current_dev++;
        }*/

        if(current_dev.size()) emit signal_connect(current_dev.at(0));
        else slotCheckStatusDevice();
    }
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void ListDev::slotCheckStatusDevice()
{
    int i,j;
    QTableWidgetItem *item;
    QString text;

    HWND handle;
    QString name_win;
    LPCWSTR w_str;

    for(i=0; i<Dev_Table->rowCount(); i++)
    {
        item = Dev_Table->item(i,2);
        text = item->text();
        name_win = "RealTime_PCR_" + Dev_Table->item(i,0)->text().trimmed();
        w_str = (const wchar_t*)name_win.utf16();
        handle = FindWindow(NULL, w_str);

        if(text == "BUSY" ||
          (text == "READY" && dev_state >= 0) ||
          handle)
        {
            for(j=0; j<Dev_Table->columnCount(); j++) Dev_Table->item(i,j)->setFlags(Qt::NoItemFlags);
        }

        if(handle &&
           Dev_Table->item(i,2)->text().trimmed() == "0x0" &&
           Dev_Table->item(i,3)->text().trimmed().isEmpty())
        {
            item->setText("RT 7.9");
        }
    }
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void ListDev::slotError_status(QAbstractSocket::SocketError err)
{
    QString strError =
        "Error: " + (err == QAbstractSocket::HostNotFoundError ?
                     "The host was not found." :
                     err == QAbstractSocket::RemoteHostClosedError ?
                     "The remote host is closed." :
                     err == QAbstractSocket::ConnectionRefusedError ?
                     "The connection was refused." :
                     QString(m_pTcpSocket->errorString())
                    );
}



//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void ListDev::slotConnected()
{
    //qDebug() << "Received the connected() signal (list_dev)";
    //qDebug() << m_pTcpSocket->state();
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void ListDev::slotReadyRead()
{
    int i,j,k;
    QString text;
    QVector<QString> answer;
    QStringList list;
    QTableWidgetItem *newItem;
    int win_id;
    bool ok;
    int row_count;


    QDataStream in(m_pTcpSocket);
    in.setVersion(QDataStream::Qt_4_7);

    for(;;)
    {
        if(!m_nNextBlockSize)
        {
            if(m_pTcpSocket->bytesAvailable() < sizeof(quint32)) break;
            in >> m_nNextBlockSize;
        }

        if(m_pTcpSocket->bytesAvailable() < m_nNextBlockSize) break;

        QTime   time;       
        in >> time >> answer;
        m_nNextBlockSize = 0;        

        //qDebug() << "get" << time.toString() << answer;
     }

    if(answer.isEmpty()) return;
    text = answer.at(0);

    // Server response:
    //
    // 1. list_devices
    if(text == "list_devices")
    {
        current_dev.clear();
        row_count = answer.size() - 1;
        Dev_Table->setRowCount(row_count);
        for(i=0; i<row_count; i++)
        {
            list = answer.at(i+1).split(QRegExp("\\s+"));

            if(list.size() >= 3 && QString(list.at(3)).trimmed().length())
            {
                current_dev.append(i);
            }

            //qDebug() << "list_all:" << list;
            for(j=0; j<Dev_Table->columnCount(); j++)
            {
                text = list.at(j);
                //qDebug() << "list: " << j << text;

                newItem = new QTableWidgetItem();
                //if(i == 0) newItem->setFlags(Qt::NoItemFlags);
                if(j == 2)  // win_id
                {
                    win_id = text.toUInt(&ok,10);
                    text = "0x" + QString::number(win_id,16);
                }

                newItem->setText(text);
                newItem->setFlags(newItem->flags() & ~Qt::ItemIsEditable);
                Dev_Table->setItem(i,j,newItem);
            }
        }
        //qDebug() << "current_dev: " << current_dev;
        if(current_dev.size()) emit signal_connect(current_dev.at(0));
        else slotCheckStatusDevice();
        //if(Dev_Table->rowCount()) {current_dev = 0; emit signal_connect(0);}
        //QTimer::singleShot(1000, this, SLOT(slotCheckStatusDevice()));


        if(PA_active) QTimer::singleShot(1000, this, SLOT(PA_listDevice())); //emit sPA_listDevice();

        return;
    }

    // 2. connect
    if(text == "connect")
    {
        //if(!PA_active) close();
        //else QTimer::singleShot(1000, this, SLOT(close()));

        close();

        connect_event.connect = 1;
        QApplication::sendEvent(p_obj, &connect_event);        
    }

    // 3. disconnect
    if(text == "disconnect")
    {

        close();

        connect_event.connect = 0;
        QApplication::sendEvent(p_obj, &connect_event);        
    }

    // 4. reconnect
    if(text == "reconnect")
    {
        if(answer.size() == 1) QTimer::singleShot(1000, this, SLOT(sReConnected()));
        else
        {
            if(answer.at(1) == name_reconnect && answer.size() >= 3 && !answer.at(2).isEmpty() && !answer.at(1).isEmpty())
            {
                name_reconnect = "";
                connect_event.connect = 2;
                connect_event.ip = answer.at(2).split(":").at(0);
                text = answer.at(2).split(":").at(1);
                connect_event.port = text.toInt();
                QApplication::sendEvent(p_obj, &connect_event);
            }
            else QTimer::singleShot(1000, this, SLOT(sReConnected()));
        }
    }

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void ListDev::slotSendToServer()
{
    QByteArray  arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_7);
    out << quint32(0) << QTime::currentTime() << Client_Net;

    out.device()->seek(0);
    out << quint32(arrBlock.size() - sizeof(quint32));

    m_pTcpSocket->write(arrBlock);    

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void ListDev::slotError(QAbstractSocket::SocketError err)
{
    QString strError =
        "Error: " + (err == QAbstractSocket::HostNotFoundError ?
                     "The host was not found." :
                     err == QAbstractSocket::RemoteHostClosedError ?
                     "The remote host is closed." :
                     err == QAbstractSocket::ConnectionRefusedError ?
                     "The connection was refused." :
                     QString(m_pTcpSocket->errorString())
                    );
    //net_message->setText(strError);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void ListDev::sReConnected()
{
    //qDebug() << QDateTime::currentDateTime().toString(" d MMMM, h:mm:ss ") << name_reconnect;

    Client_Net.clear();
    Client_Net.append("reconnect");
    Client_Net.append(name_reconnect);

    slotSendToServer();

    //QTimer::singleShot(1000, this, SLOT(sReConnected()));
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void ListDev::slotApply_ConnectAction(QTableWidgetItem *item)
{
    if(item->isSelected())
    {
        if(connect_dev->isEnabled()) {slotConnectDev(); return;}
        if(disconnect_dev->isEnabled()) {slotDisconnectDev(); return;}
    }
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void ListDev::PA_listDevice()
{
    int i;
    QTableWidgetItem *item;
    QString key,value;
    QMap<QString,QString> map_device;
    int id_disconnect = -1;
    int id_connect = -1;

    for(i=0; i<Dev_Table->rowCount(); i++)
    {
        key = Dev_Table->item(i,0)->text();     // name device
        value = Dev_Table->item(i,2)->text();   // status (ready,busy,connected,???)
        map_device.insert(key,value);

        if(key == PA_DeviceName) id_connect = i;
        if(value == "CONNECTED" && key != PA_DeviceName) id_disconnect = i;
    }


    // 1. disconnect
    if(id_disconnect >= 0)
    {
        item = Dev_Table->item(id_disconnect,0);
        Dev_Table->setCurrentItem(item);
        slotDisconnectDev();
        //QTimer::singleShot(1000, this, SLOT(close()));
        return;
    }

    // 2. connect
    if(id_connect >= 0 && value == "READY")
    {
        item = Dev_Table->item(id_connect,0);
        Dev_Table->setCurrentItem(item);
        slotConnectDev();
        //QTimer::singleShot(500, this, SLOT(close()));
        return;
    }

    // 3. nothing
    close();
}
