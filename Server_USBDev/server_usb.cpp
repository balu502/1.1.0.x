#include "server_usb.h"




//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Server_USB::Server_USB(int nPort, QWidget *parent)
    : QMainWindow(parent), m_nNextBlockSize(0)
{
    int i;
    QString text;
    PORT = nPort;
    ip_addres = "localhost";
    ip_addres = "127.0.0.1";
    view_CloseMessage = true;
    exists_RT7 = false;

    readCommonSettings();
    message.setWindowIcon(QIcon(":/image/usb_32.png"));

    application_names << APPLICATION_NAMES;

    //qDebug() << "list: " << application_names.size() << application_names;

    /*foreach (const QHostAddress &address, QNetworkInterface::allAddresses())
    {
        if(address.protocol() == QAbstractSocket::IPv4Protocol &&
           address != QHostAddress(QHostAddress::LocalHost))
        {
             //qDebug() << address.toString();
             ip_addres = address.toString();
        }
    }*/

    setTrayIconActions();
    showTrayIcon();

    Qt::WindowFlags flags = Qt::Window | Qt::WindowSystemMenuHint |
                            Qt::WindowCloseButtonHint | Qt::WindowStaysOnTopHint;
    setWindowFlags(flags);

    Dev_Table = new QTableWidget();
    setCentralWidget(Dev_Table);
    Dev_Table->setFocusPolicy(Qt::NoFocus);
    Dev_Table->setSelectionMode(QAbstractItemView::NoSelection);
    Dev_Table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    Dev_Table->setContextMenuPolicy(Qt::CustomContextMenu);
    delegate = new ServerItemDelegate();
    Dev_Table->setItemDelegate(delegate);

    //event_button = new QToolButton(this);
    //connect(event_button, SIGNAL(clicked(bool)), this, SLOT(ReLoad_InfoUSBDevice()));

    setMinimumWidth(500);
    resize(500,150);
    move(2, 2);

    net_message = new QLabel(this);
    net_message->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    statusBar()->addWidget(net_message);

    //... NetWork ...
    m_ptcpServer = new QTcpServer(this);
    if(!m_ptcpServer->listen(QHostAddress::Any, nPort))
    {
        text = tr("Unable to start the server: ") + m_ptcpServer->errorString();
        if(text.contains("The bound address is already in use"))
        {
            text += "\r\n" + tr("It is possible that another user is running this server!");
        }

        message.setStandardButtons(QMessageBox::Ok);
        message.setIcon(QMessageBox::Critical);
        //message.button(QMessageBox::Ok)->animateClick(5000);
        message.setText(text);
        message.exec();
        //QMessageBox::critical(0,tr("Server Error"), text);

        m_ptcpServer->close();
        view_CloseMessage = false;
        QTimer::singleShot(1000, this, SLOT(slotCLOSE()));
        return;
    }
    QObject::connect(m_ptcpServer, SIGNAL(newConnection()), this, SLOT(slotNewConnection()));

    timer = new QTimer(this);
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(Check_CompatibilityVersion()));
    //timer->start(2000);

    //... Client NetWork ...
    //m_nNextBlockSize_client = 0;
    //m_pTcpSocket = new QTcpSocket(this);

    //connect(m_pTcpSocket, SIGNAL(connected()), SLOT(slotConnected()));
    //connect(m_pTcpSocket, SIGNAL(readyRead()), SLOT(slotReadyRead()));
    //connect(m_pTcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
    //            this,     SLOT(slotError(QAbstractSocket::SocketError)));
    timer_SLEEP = new QTimer(this);
    timer_SLEEP->setInterval(1000*60);
    QObject::connect(timer_SLEEP, SIGNAL(timeout()), this, SLOT(Check_SleepRejime()));
    //timer_SLEEP->start();


    SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED | ES_AWAYMODE_REQUIRED | ES_DISPLAY_REQUIRED);

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Server_USB::~Server_USB()
{
    HWND handle_win = NULL;
    QString str;
    LPCWSTR w_str;
    int WM_USERCLOSE = 0x7fff + 0x666;

    foreach(Server_Process *serv, list_server)
    {
        str = QString("%1_%2").arg(serv->SerName).arg(serv->port);
        w_str = (const wchar_t*) str.utf16();
        handle_win = FindWindow(NULL, w_str);

        if(handle_win)
        {
            SendMessage(handle_win, WM_USERCLOSE, 0, 0);
        }
        serv->waitForFinished(2000);
        //Sleep(100);
        serv->terminate();
        serv->waitForFinished(1000);
    }


    QString text = tr("Server_USBDev for DTmaster_9.x is CLOSED!");
    if(exists_RT7)
    {
        text += "\r\n" + tr("DTmaster and RealTime_PCR cannot work together with the device...");
    }



    if(FX2->IsOpen()) FX2->Close();
    delete FX2;

    qDeleteAll(list_fx2.begin(), list_fx2.end());
    list_fx2.clear();

    qDeleteAll(list_server.begin(), list_server.end());
    list_server.clear();

    //...
    if(view_CloseMessage || exists_RT7)
    {        
        message.setStandardButtons(QMessageBox::Ok);
        message.setIcon(QMessageBox::Warning);
        message.button(QMessageBox::Ok)->animateClick(5000);
        message.setText(text);
        message.exec();
    }
    //...

    delete trayIcon;

    m_ptcpServer->close();
    delete m_ptcpServer;

}
//-----------------------------------------------------------------------------
//--- readCommonSettings()
//-----------------------------------------------------------------------------
void Server_USB::readCommonSettings()
{
    QString text;
    QString dir_path = qApp->applicationDirPath();
    QSettings *CommonSettings = new QSettings(dir_path + "/tools/ini/preference.ini", QSettings::IniFormat);

    CommonSettings->beginGroup("Common");

        //... Language ...
        text = CommonSettings->value("language","ru").toString();
        if(translator.load(":/translations/serverUSBDev_" + text + ".qm"))
        {
            qApp->installTranslator(&translator);
        }

    CommonSettings->endGroup();
    delete CommonSettings;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Server_USB::slotCLOSE()
{
    trayIcon->setVisible(false);
    close();
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Server_USB::showTrayIcon()
{
    trayIcon = new QSystemTrayIcon(this);
    QIcon trayImage = QIcon(":/image/usb_32.png");
    trayIcon->setIcon(trayImage);
    trayIcon->setContextMenu(trayIconMenu);
    setWindowIcon(trayImage);

    QObject::connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this,     SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));

    trayIcon ->show();
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Server_USB::setTrayIconActions()
{
    //... Setting actions ...
    minimizeAction = new QAction("Hide", this);
    restoreAction = new QAction("Restore", this);
    quitAction = new QAction("Exit", this);

    //... Connecting actions to slots ...
    QObject::connect(minimizeAction, SIGNAL(triggered()), this, SLOT(hide()));
    QObject::connect(restoreAction, SIGNAL(triggered()), this, SLOT(showNormal()));
    QObject::connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));

    //... Setting system tray's icon menu ...
    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction (minimizeAction);
    trayIconMenu->addAction (restoreAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction (quitAction);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Server_USB::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::Trigger:
    case QSystemTrayIcon::DoubleClick:
                if(!isVisible()) this->showNormal();

    case QSystemTrayIcon::MiddleClick:
                //showMessage();
                break;
    default:    break;

    }
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Server_USB::closeEvent(QCloseEvent *event)
{
    if(trayIcon->isVisible())
    {
        hide();
        event->ignore();
    }
}
//-------------------------------------------------------------------------------------------------
//---
//-------------------------------------------------------------------------------------------------
bool Server_USB::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
    QString text;
    QStringList list;
    COPYDATASTRUCT* pCD;
    char buffer[32];
    int WM_USERCLOSE = 0x7fff + 0x666;

    MSG *msg = static_cast< MSG * >( message );    

    if(msg->message == WM_DEVICECHANGE)
    {
        switch(msg->wParam)
        {
        case DBT_DEVICEARRIVAL: qDebug() << "ARRIVAL+";
                                create_InfoUSBDevice();
                                Run_externalServer();

                                break;

        case DBT_DEVICEREMOVECOMPLETE:  qDebug() << "REMOVE-";
                                        create_InfoUSBDevice();
                                        Stop_externalServer();

                                        break;

        default:    break;
        }
    }

    if(msg->message == WM_USERCLOSE && msg->wParam == 666 && msg->lParam == 777)
    {
        view_CloseMessage = false;
        trayIcon->setVisible(false);
        close();
    }

    if(msg->message == WM_COPYDATA)
    {
        //qDebug() << "eventType: " << msg->message << msg->wParam << msg->lParam << result;

        pCD = (COPYDATASTRUCT*)msg->lParam;
        strncpy(buffer,(char*)pCD->lpData, pCD->cbData);

        QByteArray ba(&buffer[0]);
        text = QString::fromLatin1(ba.data());

        //qDebug() << "DATA: " << text;



        if(text.contains("Check all Devices"))          // Check all devices
        {
            QTimer::singleShot(1000, this, SLOT(ReLoad_InfoUSBDevice()));
        }
        if(text.contains("Kill"))                       // Kill device
        {
            list = text.split(QRegExp("\\s+"));
            if(list.size() == 2)
            {
                Kill_Server(list.at(1));
                create_InfoUSBDevice();
            }
        }
    }


    return(false);
}
//-------------------------------------------------------------------------------------------------
//---
//-------------------------------------------------------------------------------------------------
void Server_USB::create_FXCypress()
{
    //Check_CompatibilityVersion();
    //USB_Info* fx2;

    int i;


    HWND hWnd = (HWND)this->effectiveWinId();
    FX2 = new CyDev(hWnd);
    create_InfoUSBDevice();

    for(i=0; i<list_fx2.size(); i++)
    {
        if(list_fx2.at(i)->busy) continue;

        Run_externalSingleServer(list_fx2.at(i)->SerNum, i);
    }




    //Run_externalServer();
}
//-------------------------------------------------------------------------------------------------
//---
//-------------------------------------------------------------------------------------------------
void Server_USB::create_InfoUSBDevice()
{
    int i=0,k;
    int num;
    HANDLE      devHandle;
    USB_Info    *pfx2;    
    HWND handle_win;
    QString IP_port, text;
    LPCWSTR w_str;
    QString app_name;

    //timer_SLEEP->stop();

    qDeleteAll(list_fx2.begin(), list_fx2.end());
    list_fx2.clear();

    do
    {
        devHandle = FX2->Open(i);
        if(devHandle != INVALID_HANDLE_VALUE)
        {
            pfx2 = new USB_Info;
            pfx2->devHandle = FX2->DeviceHandle();
            pfx2->SerNum = QString::fromWCharArray(FX2->SerNum);
            pfx2->PID = FX2->ProductID;
            pfx2->VID = FX2->VendorID;
            pfx2->handle_win = NULL;
            pfx2->busy = false;
            num = pfx2->SerNum.mid(1,1).toInt();
            switch(num)
            {
            case 5:     pfx2->status = 96;  break;
            case 6:     pfx2->status = 384; break;
            case 7:     pfx2->status = 48; break;
            case 8:     pfx2->status = 192; break;
            default:    pfx2->status = 0;  break;
            }

            list_fx2.append(pfx2);

            //... handle_win
            foreach(app_name, application_names)
            {
                handle_win = NULL;
                text = app_name + " " + pfx2->SerNum;
                w_str = (const wchar_t*) text.utf16();
                handle_win = FindWindow(NULL, w_str);
                pfx2->handle_win = handle_win;

                if(handle_win) break;
            }


            //... IP_port
            IP_port = "";
            for(k=0; k<list_server.size(); k++)
            {
                if(list_server.contains(pfx2->SerNum))
                {
                    IP_port = list_server.value(pfx2->SerNum)->IP_port;
                    break;
                }
            }
            pfx2->IP_port = IP_port;

            //qDebug() << "pf2: " << pfx2->SerNum << pfx2->IP_port;

        }
        FX2->Close();
        i++;
    }
    while(devHandle != INVALID_HANDLE_VALUE);

    Check_DeviceBusy();
    fill_TableDevice();
}

//-------------------------------------------------------------------------------------------------
//---
//-------------------------------------------------------------------------------------------------
void Server_USB::fill_TableDevice()
{
    int i,j,k;
    QStringList header;
    QTableWidgetItem *newItem;
    QString text;
    USB_Info    *pfx2;
    int num;


    for(i = Dev_Table->rowCount(); i>0; i--) Dev_Table->removeRow(i-1);
    Dev_Table->clear();
    header << "SerialNumber" << "status" << "PID" << "VID" << "WinID" << "IP:port";
    Dev_Table->setColumnCount(6);
    Dev_Table->setHorizontalHeaderLabels(header);
    Dev_Table->setColumnWidth(0,100);
    Dev_Table->setColumnWidth(1,50);
    Dev_Table->setColumnWidth(2,50);
    Dev_Table->setColumnWidth(3,50);
    Dev_Table->setColumnWidth(4,80);
    Dev_Table->setColumnWidth(5,120);
    Dev_Table->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    Dev_Table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);

    Dev_Table->setRowCount(list_fx2.size());

    for(i=0; i<Dev_Table->rowCount(); i++)
    {
        pfx2 = list_fx2.at(i);
        for(j=0; j<Dev_Table->columnCount(); j++)
        {
            newItem = new QTableWidgetItem();
            switch(j)
            {
            case 0: //... serial number ...
                    text = pfx2->SerNum;
                    if(pfx2->busy) text += "*";
                    break;

            case 1: //... status ...
                    text = QString::number(pfx2->status);
                    break;

            case 2: //... PID ...
                    text = "0x" + QString::number(pfx2->PID,16);
                    break;

            case 3: //... VID ...
                    text = "0x" + QString::number(pfx2->VID,16);
                    break;

            case 4: //... WinID ...
                    num = (int)pfx2->handle_win;
                    text = "0x" + QString::number(num,16);                    
                    break;

            case 5: //... IP_port ...
                    text =pfx2->IP_port;
                    break;

            default:    text = "";
                        break;
            }
            newItem->setText(text);
            newItem->setFlags(newItem->flags() & ~Qt::ItemIsEditable);
            Dev_Table->setItem(i,j,newItem);
        }
    }


    //timer_SLEEP->start();
}
//-------------------------------------------------------------------------------------------------
//---
//-------------------------------------------------------------------------------------------------
void Server_USB::Run_externalSingleServer(QString ser_name, int id)
{
    int i,j,k;
    bool find = true;
    int port = PORT+1;

    QString file = "Server_Dev.exe";
    QStringList arguments;


    //qDebug() << list_fx2 << list_server;
    //qDebug() << "fx2:" << ser_name;

    //for(i=0; i<list_fx2.size(); i++)
    //{

            port += id;

            server_SYT = new Server_Process();

            QObject::connect(server_SYT, SIGNAL(start_proc(Server_Process*)), this, SLOT(Started_Process(Server_Process*)));
            QObject::connect(server_SYT, SIGNAL(finish_proc(Server_Process*)), this, SLOT(Finished_Process(Server_Process*)));


            //server_SYT->setReadChannel();
            //server_SYT->setReadChannelMode();
            server_SYT->SerName = ser_name;
            server_SYT->port = port;
            server_SYT->IP_port = ip_addres + ":" + QString::number(port);
            arguments << ser_name << QString::number(port);

            //qDebug() << "server_SYT: " << server_SYT->SerName << server_SYT->port << server_SYT->IP_port;

            server_SYT->start(file,arguments);
            server_SYT->waitForStarted();
            //Sleep(3000);

            //qDebug() << "server_SYT: started";

            //break;

    //}
}

//-------------------------------------------------------------------------------------------------
//---
//-------------------------------------------------------------------------------------------------
void Server_USB::Run_externalServer()
{
    int i,j,k;
    bool find = true;
    int port = PORT+1;

    QString file = "Server_Dev.exe";
    QStringList arguments;


    qDebug() << "Run_externalServer: " << list_fx2 << list_server;


    for(i=0; i<list_fx2.size(); i++)
    {
        if(list_fx2.at(i)->busy) continue;

        if(!list_server.contains(list_fx2.at(i)->SerNum))
        {
            find = true;
            while(find)
            {
                find = false;
                for(k=0; k<list_server.size(); k++)
                {
                    if(list_server.values().at(k)->port == port)
                    {
                        find = true;
                        port++;
                        break;
                    }
                }
            }
            if(list_server.size() == 0) port += i;

            server_SYT = new Server_Process();

            QObject::connect(server_SYT, SIGNAL(start_proc(Server_Process*)), this, SLOT(Started_Process(Server_Process*)));
            QObject::connect(server_SYT, SIGNAL(finish_proc(Server_Process*)), this, SLOT(Finished_Process(Server_Process*)));


            //server_SYT->setReadChannel();
            //server_SYT->setReadChannelMode();
            server_SYT->SerName = list_fx2.at(i)->SerNum;
            server_SYT->port = port;
            server_SYT->IP_port = ip_addres + ":" + QString::number(port);
            arguments << list_fx2.at(i)->SerNum << QString::number(port);

            qDebug() << "server_SYT: " << server_SYT->SerName << server_SYT->port << server_SYT->IP_port;

            server_SYT->start(file,arguments);
            server_SYT->waitForStarted();
            //Sleep(3000);

            qDebug() << "server_SYT: started";

            //break;
        }
    }
}
//-------------------------------------------------------------------------------------------------
//---
//-------------------------------------------------------------------------------------------------
void Server_USB::Kill_Server(QString sernum)
{
    Server_Process *serv;
    USB_Info *p_fx2;

    HWND handle_win = NULL;
    QString str;
    LPCWSTR w_str;
    int WM_USERCLOSE = 0x7fff + 0x666;


    serv = list_server.value(sernum, NULL);
    if(serv)
    {
        str = QString("%1_%2").arg(serv->SerName).arg(serv->port);
        w_str = (const wchar_t*) str.utf16();
        handle_win = FindWindow(NULL, w_str);

        if(handle_win)
        {
            SendMessage(handle_win, WM_USERCLOSE, 0, 0);
        }
        serv->waitForFinished(2000);
        //Sleep(100);

        //serv->kill();
        serv->terminate();
        serv->waitForFinished(1000);

        delete serv;
        list_server.remove(sernum);
    }

    foreach(p_fx2, list_fx2)
    {
        if(p_fx2->SerNum == sernum)
        {
            p_fx2->IP_port = "";
            break;
        }
    }

}

//-------------------------------------------------------------------------------------------------
//---
//-------------------------------------------------------------------------------------------------
void Server_USB::Stop_externalServer()
{
    int i,j;
    Server_Process *serv;
    QString text;
    bool find;
    int port;

    //QObject *obj;
    //QEvent *e = new QEvent((QEvent::Type)3302);

    HWND handle_win = NULL;
    QString str;
    LPCWSTR w_str;
    int WM_USERCLOSE = 0x7fff + 0x666;


    for(i=0; i<list_server.keys().size(); i++)
    {
        text = list_server.keys().at(i);
        find = false;
        for(j=0; j<list_fx2.size(); j++)
        {
            if(text == list_fx2.at(j)->SerNum) {find = true; break;}
        }
        if(!find)
        {

            serv = list_server.value(text);
            //serv->kill();

            //obj = reinterpret_cast<QObject*>(serv);
            //QApplication::sendEvent(obj, e);

            handle_win = NULL;
            str = QString("%1_%2").arg(serv->SerName).arg(serv->port);
            w_str = (const wchar_t*) str.utf16();
            handle_win = FindWindow(NULL, w_str);            

            if(handle_win)
            {
                SendMessage(handle_win, WM_USERCLOSE, 0, 0);
                //qDebug() << "SendMessage(H, WM_USERCLOSE, 0, 0);" << str;
            }

            serv->waitForFinished(2000);
            //Sleep(100);

            serv->terminate();
            serv->waitForFinished(1000);

            delete serv;
            list_server.remove(text);
            //port = serv->port;


/*
           m_pTcpSocket->connectToHost(ip_addres, port);
            if(m_pTcpSocket->waitForConnected(3000))
            {

               slotSendToServer("Quit");
               //m_pTcpSocket->disconnectFromHost();
            }
*/
            //serv->terminate();
            //qDebug() << "exit:" << serv->waitForFinished();
            //qDebug() << "status:" << serv->exitStatus();
            //qDebug() << "code:" << serv->exitCode();

            //serv->write("Balu\n");
            //serv->waitForBytesWritten();
            //serv->closeWriteChannel();
        }
    }
}

//-------------------------------------------------------------------------------------------------
//---
//-------------------------------------------------------------------------------------------------
void Server_USB::Started_Process(Server_Process *serv)
{
    int i;
    qDebug() << "start_process = " << serv->SerName << serv->IP_port;
    for(i=0; i<list_fx2.size(); i++)
    {
        if(list_fx2.at(i)->SerNum == serv->SerName)
        {
            list_fx2.at(i)->IP_port = serv->IP_port;
            list_server.insert(serv->SerName, serv);

            break;
        }
    }
    create_InfoUSBDevice();

    qDebug() << "started... " << list_fx2 << list_server;
}
//-------------------------------------------------------------------------------------------------
//---
//-------------------------------------------------------------------------------------------------
void Server_USB::Finished_Process(Server_Process *serv)
{
    int i;
    QString text;
    qDebug() << "finish_process = " << serv->SerName << serv->IP_port;


    for(i=0; i<list_server.size(); i++)
    {
        if(serv->SerName == list_server.keys().at(i))
        {
            list_server.remove(serv->SerName);
            break;
        }
    }

    create_InfoUSBDevice();
}

//-------------------------------------------------------------------------------------------------
//---
//-------------------------------------------------------------------------------------------------
Server_Process::Server_Process(QWidget *parent) : QProcess(parent)
{
    SerName = "";
    IP_port = "";

    QObject::connect(this,SIGNAL(started()),SLOT(Started()));
    QObject::connect(this,SIGNAL(finished(int)),SLOT(Finished()));
}
Server_Process::~Server_Process()
{

}

//================= NETWORK ===================================================

//-------------------------------------------------------------------------------------------------
//---
//-------------------------------------------------------------------------------------------------
void Server_USB::slotNewConnection()
{
    QTcpSocket* pClientSocket = m_ptcpServer->nextPendingConnection();
    QObject::connect(pClientSocket, SIGNAL(disconnected()),
            pClientSocket, SLOT(deleteLater()));
    QObject::connect(pClientSocket, SIGNAL(readyRead()),
            this,          SLOT(slotReadClient()));

    //...
    m_pClientSocketList.push_back(pClientSocket);
    QObject::connect(pClientSocket,SIGNAL(disconnected()), this, SLOT(slotClientDisconnected()));
    //...

    create_InfoUSBDevice();

    Server_Net.clear();
    Server_Net.append("Connected");
    sendToClient(pClientSocket);

    //qDebug() << "new connection";
}
//-------------------------------------------------------------------------------------------------
//---
//-------------------------------------------------------------------------------------------------
void Server_USB::slotReadClient()
{
    int i;
    QTime   time;
    QString str, name;
    QTcpSocket* pClientSocket = (QTcpSocket*)sender();
    QDataStream in(pClientSocket);
    in.setVersion(QDataStream::Qt_4_7);
    USB_Info *p_dev;
    QVector<QString> request;

    for(;;)
    {
        if(!m_nNextBlockSize)
        {
            if(pClientSocket->bytesAvailable() < sizeof(quint32)) break;
            in >> m_nNextBlockSize;
        }

        if(pClientSocket->bytesAvailable() < m_nNextBlockSize) break;

        in >> time >> request;
        QString strMessage = time.toString() + " " + "Client has sended - " + request.at(0);
        net_message->setText(strMessage);

        //... Client Commands ...
        Server_Net.clear();
        str = request.at(0);
        Server_Net.append(str);

        //qDebug() << request;

        // 1. create list of devices
        if(str == "list_devices")
        {            
            create_InfoUSBDevice();

            for(i=0; i<list_fx2.size(); i++)
            {
                p_dev = list_fx2.at(i);
                str = p_dev->SerNum + " " +
                      QString::number(p_dev->status) + " " +
                      QString::number((uint)p_dev->handle_win) + " " + p_dev->IP_port;

                Server_Net.append(str);
            }
        }

        // 2. connect with device
        if(str == "connect")
        {
            Server_Net.append(request.at(2)); // name
            create_InfoUSBDevice();
        }

        // 3. disconnect from device
        if(str == "disconnect")
        {            
            create_InfoUSBDevice();
        }

        // 4. refresh
        if(str == "refresh")
        {
            create_InfoUSBDevice();
        }

        // 5. reconnect with device
        if(str == "reconnect")
        {
            create_InfoUSBDevice();
            name = request.at(1);

            for(i=0; i<list_fx2.size(); i++)
            {
                p_dev = list_fx2.at(i);
                if(p_dev->SerNum == name)
                {
                    Server_Net.append(name);
                    Server_Net.append(p_dev->IP_port);
                    break;
                }
            }
        }


        m_nNextBlockSize = 0;

        sendToClient(pClientSocket);
    }
}
//-------------------------------------------------------------------------------------------------
//---
//-------------------------------------------------------------------------------------------------
void Server_USB::sendToClient(QTcpSocket *pSocket)
{
    QByteArray  arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_7);

    out << quint32(0) << QTime::currentTime() << Server_Net;

    out.device()->seek(0);
    out << quint32(arrBlock.size() - sizeof(quint32));

    pSocket->write(arrBlock);
}
//-------------------------------------------------------------------------------------------------
//--- slotClientDisconnected
//-------------------------------------------------------------------------------------------------
void Server_USB::slotClientDisconnected()
{
    QTcpSocket* pClient = static_cast<QTcpSocket*>(QObject::sender());
    m_pClientSocketList.removeOne(pClient);
    create_InfoUSBDevice();
}

//================= CLIENT NETWORK ===================================================

//-------------------------------------------------------------------------------------------------
//---
//-------------------------------------------------------------------------------------------------
void Server_USB::slotConnected()
{
    //qDebug() << "Received the connected() signal from SytServer...";
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Server_USB::slotReadyRead()
{
    /*int i;
    QString text, str;

    QDataStream in(m_pTcpSocket);
    in.setVersion(QDataStream::Qt_4_7);

    for(;;)
    {
        if(!m_nNextBlockSize_client)
        {
            if(m_pTcpSocket->bytesAvailable() < sizeof(quint16)) break;
            in >> m_nNextBlockSize_client;
        }

        if(m_pTcpSocket->bytesAvailable() < m_nNextBlockSize_client) break;

        QString answer;
        in >> answer;

        m_nNextBlockSize_client = 0;        
     }*/
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Server_USB::slotSendToServer(QString request)
{
    /*QByteArray  arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_7);
    out << quint16(0) << request;

    out.device()->seek(0);
    out << quint16(arrBlock.size() - sizeof(quint16));

    m_pTcpSocket->write(arrBlock);
    */

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Server_USB::slotError(QAbstractSocket::SocketError err)
{
    /*QString strError =
        "Error: " + (err == QAbstractSocket::HostNotFoundError ?
                     "The host was not found." :
                     err == QAbstractSocket::RemoteHostClosedError ?
                     "The remote host is closed." :
                     err == QAbstractSocket::ConnectionRefusedError ?
                     "The connection was refused." :
                     QString(m_pTcpSocket->errorString())
                    );
    qDebug() << strError;
    */
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Server_USB::Check_SleepRejime()
{
    QString app_name, text;
    LPCWSTR w_str;
    HWND hWnd;
    USB_Info    *pfx2;
    bool is_exists = false;

    QVector<USB_Info*> list = list_fx2;

    list.append(NULL);


    foreach(pfx2, list)
    {
        foreach(app_name, application_names)
        {
            hWnd = NULL;
            if(pfx2 == NULL) text = app_name;
            else text = app_name + " " + pfx2->SerNum;
            w_str = (const wchar_t*) text.utf16();
            hWnd = FindWindow(NULL, w_str);

            if(hWnd) {is_exists = true; break;}
        }
        if(is_exists) break;
    }

    if(!is_exists)  QCoreApplication::exit();
}



//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Server_USB::Check_CompatibilityVersion()
{
    HWND handle_win = NULL;
    QString text = "Server_RTDevice";
    LPCWSTR w_str;

    w_str = (const wchar_t*)text.utf16();
    handle_win = FindWindow(NULL, w_str);   // Find server USB for version 7.x

    if(handle_win) {exists_RT7 = true; trayIcon->setVisible(false); close();} //qApp->quit();
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Server_USB::Check_DeviceBusy()
{
    QString text = "RealTime_PCR";
    QString name;
    LPCWSTR w_str;
    HWND handle_win = NULL;
    USB_Info    *pfx2;

    foreach(pfx2, list_fx2)
    {
        name = text + "_" + pfx2->SerNum;
        w_str = (const wchar_t*)name.utf16();
        handle_win = FindWindow(NULL, w_str);

        if(handle_win) pfx2->busy = true;
        else  pfx2->busy = false;
    }
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Server_USB::ReLoad_InfoUSBDevice()
{    
    USB_Info    *pfx2;
    Server_Process *server_SYT;
    QString     sernum;    
    QVector<int> vec_port;
    int id;

    foreach(server_SYT, list_server)
    {
        vec_port.append(server_SYT->port - PORT - 1);
    }

    create_InfoUSBDevice();

    foreach(pfx2, list_fx2)
    {
        sernum = pfx2->SerNum;        
        server_SYT = list_server.value(sernum, NULL);

        // 1. Run server
        if(!pfx2->busy && !server_SYT)
        {
            id = 0;
            while(id<100)
            {
                if(!vec_port.contains(id)) {vec_port.append(id); break;}
                id++;
            }

            Run_externalSingleServer(sernum, id);
        }

        // 2. Stop server
        //if(pfx2->busy && server_SYT) Kill_Server(sernum);

    }
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void ServerItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QColor color,rgb;
    //int r,g,b;
    int rad;
    int x,y;
    double X,Y,R;
    int pos;
    int width,height;
    QString text = index.data().toString();


    QPalette pal = option.palette;
    QStyleOptionViewItem  viewOption(option);
    viewOption.state &= ~QStyle::State_HasFocus;    // disable state HasFocus


    QStyledItemDelegate::paint(painter, viewOption, index);



    //__1. Background
    painter->fillRect(option.rect, QColor(255,255,225));


    //__2. Data
    painter->setPen(QPen(Qt::black,1,Qt::SolidLine));

    if(index.column() == 0)
    {
        if(text.contains("*"))
        {
            painter->setPen(QPen(Qt::red,1,Qt::SolidLine));
        }
    }

    painter->drawText(option.rect,Qt::AlignCenter,text);



}
