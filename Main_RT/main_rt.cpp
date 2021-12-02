#include "main_rt.h"

#include <openssl/applink.c>

//-----------------------------------------------------------------------------
//--- Constructor
//-----------------------------------------------------------------------------
Main_RT::Main_RT(QWidget *parent)
    : QMainWindow(parent)
{
    setAccessibleDescription("Main_RT");   

    //... Apply Settings ...
    QString dir_path = qApp->applicationDirPath();
    ApplSettings = new QSettings(dir_path + "/tools/ini/preference.ini", QSettings::IniFormat);

    readSettings();

    // Style:
    if(StyleApp == "xp")
    {
        setStyleSheet(
            "QSplitter::handle:vertical   {height: 6px; image: url(:/images/v_splitter_pro.png);}"
            "QSplitter::handle:horizontal {width:  6px; image: url(:/images/h_splitter_pro.png);}"
            "QLineEdit {selection-background-color: #d7d7ff; selection-color: black;}"
            "QSpinBox {selection-background-color: #d7d7ff; selection-color: black;}"
            "QComboBox {selection-background-color: #d7d7ff; selection-color: black;}"
            );
    }
    if(StyleApp == "fusion")
    {
        qApp->setStyle(QStyleFactory::create("Fusion"));
        setStyleSheet(
            "QSplitter::handle:vertical   {height: 6px; image: url(:/images/v_splitter_pro.png);}"
            "QSplitter::handle:horizontal {width:  6px; image: url(:/images/h_splitter_pro.png);}"
            "QToolBar {border: 1px solid #ddd; background-color: #FAFAFA;}"
            "QToolButton::hover {background: white; border: 1px solid #ddd;}"
            "QToolButton::disabled {background-color: #ccc;}"
            "QToolButton::checked {background-color: #FFFFFF; border: 1px solid black}"
            //"QGroupBox {padding-top:15px; margin-top:-15px; background: #FAFAFA; border: 1px solid #ddd;}"
            "QGroupBox {margin-top: 1ex; background: #FAFAFA; border: 1px solid #ddd;}"
            "QGroupBox#Transparent {border: 1px solid transparent;}"
            //"QGroupBox#Border {padding-top:15px; margin-top:-15px; background-color: #eee; border: 1px solid #aaa;}"
            //"QPushButton {border: 1px solid #bbb;}"
            "QTextEdit {background-color: #ffffff; selection-background-color: #d7d7ff; selection-color: black;}"
            "QDialog {background-color: #FAFAFA; border: 1px solid #ddd;}"
            "QPushButton::hover {background-color: #fff;}"
            "QPushButton::disabled {background-color: #ccc;}"
            "QPushButton::enabled {background-color: #FAFAFA;}"
            "QPushButton::pressed {background-color: #ccc;}"
            "QListWidget {background-color: #ffffff; selection-background-color: #d7d7ff; selection-color: black;}"
            //"QPushButton::focus {background-color: #EAEAEA;}"
            "QPushButton {text-align: center;}"
            "QPushButton {min-width: 4em; margin:1; padding:5;}"
            "QPushButton {border: 1px solid #aaa; border-radius: 0px;}"
            "QPushButton#Transparent {border: 0px solid #aaa; min-width: 1em; margin:0; padding: 2;}"
            "QLineEdit {selection-background-color: #d7d7ff; selection-color: black;}"
            "QSpinBox {selection-background-color: #d7d7ff; selection-color: black;}"
            "QDoubleSpinBox {selection-background-color: #d7d7ff; selection-color: black;}"
            "QTreeWidget {selection-background-color: #d7d7ff; selection-color: black;}"
            //"QHeaderView::section {background-color: #FAFAFA;}"
            "QComboBox {background-color: #ffffff; selection-background-color: #d7d7ff; selection-color: black;}"
            "QMenu::item:selected {background-color: #d7d7ff; color: black;}"
            "QMenuBar::item:selected {background-color: #d7d7ff; color: black;}"
            "QStatusBar {background: #FAFAFA; border: 1px solid #ddd;}"
            //"QTabBar::tab:!selected {background: #FAFAFA; border: 1px solid #aaa; min-width: 12ex;}"
            "QToolTip {background: #FFFFFF; border: 1px solid #ddd;}"
            "QProgressBar:horizontal {border: 1px solid gray; background: #FAFAFA; padding: 2px;}"
            "QProgressBar::chunk:horizontal {background: qlineargradient(x1: 0, y1: 0.1, x2: 1, y2: 0.1, stop: 0 #B5D8E6, stop: 1 #409BBD);}"
            //"QProgressBar::chunk:horizontal {background: qlineargradient(x1: 0, y1: 0.1, x2: 1, y2: 0.1, stop: 0 #CCCCFF, stop: 1 #6464FF);}"
            "QHeaderView::section {background-color: #FFFFFF; border: 1px solid gray; border-left: 0px; border-top: 0px; padding:3; margin:0;}"
            //"QHeaderView {background-color: #FFFFFF;}"
            );
    }

    MainGroupBox = new QGroupBox();
    MainGroupBox->setObjectName("Transparent");
    setCentralWidget(MainGroupBox);
    main_layout = new QHBoxLayout;
    main_layout->setMargin(5);
    main_layout->setSpacing(4);
    MainGroupBox->setLayout(main_layout);

    create_StatusBar();    
    create_ListWidget();    
    create_PagesWidget();    
    create_Menu();    
    create_WebSocket();

    QApplication::processEvents();    

    //... Text Codec ...
    QTextCodec *russianCodec = QTextCodec::codecForName("UTF-8");
    QTextCodec::setCodecForLocale(russianCodec);

    //... Timer_Rita ...    
    timer_Rita = new QTimer(this);
    connect(timer_Rita, SIGNAL(timeout()), this, SLOT(update_Rita()));


    //... Data&Time ...    
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update_DateTime()));
    update_DateTime();
    timer->start(1000);

    //... Timer Web ...    
    timer_Web = new QTimer(this);
    connect(timer_Web, SIGNAL(timeout()), this, SLOT(check_WebConnection()));
#ifndef CALIBRATION
    timer_Web->start(10000);
#endif

    //pagesWidget->setCurrentWidget(p_saver);
    contentsWidget->setCurrentRow(-1);
    connect(contentsWidget, SIGNAL(currentRowChanged(int)), this, SLOT(slot_DisableMenu(int)));

    connect(pagesWidget, SIGNAL(sJump_SetupToRun(QString)), this, SLOT(slot_SetupToRun(QString)));
    connect(pagesWidget, SIGNAL(sJump_RunToAnalysis(QString)), this, SLOT(slot_RunToAnalysis(QString)));
    connect(pagesWidget, SIGNAL(sJump_AnalysisToSetup(QString)), this, SLOT(slot_AnalysisToSetup(QString)));
    connect(pagesWidget, SIGNAL(sJump_SetupToAnalysis(QString)), this, SLOT(slot_SetupToAnalysis(QString)));

    //connect(this, SIGNAL())

    //qApp->setStyle(QStyleFactory::create("Fusion"));
    //qApp->setStyleSheet(
    //    "QSplitter::handle:vertical   {height: 6px; image: url(:/images/v_splitter_pro.png);}"
    //    "QSplitter::handle:horizontal {width:  6px; image: url(:/images/h_splitter_pro.png);}"
        //"QGroupBox {padding-top:15px; margin-top:-15px;"
        //"QPushButton {background-color: white;"
        //"border:1px solid #555;}"
        //"min-width: 70px;}"
        //"QPushButton::disabled {background-color: #aaa;}"
        //"QPushButton::hover {background-color: #eee;}"
        //);
    /*qApp->setStyleSheet(
        "QSplitter::handle:vertical   {height: 32px; image: url(images/vert_splitter.png);}"
        //"QSplitter::handle:horizontal {width:  6px; image: url(images/horis_splitter.png);}"
        );*/

    setWindowTitle(APP_NAME);
    setWindowIcon(QIcon(":/images/DTm.ico"));
#ifdef CALIBRATION
    setWindowIcon(QIcon(":/images/check.ico"));
#endif
    //setWindowIcon(QIcon(":/images/RT.ico"));

    //setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint );
    //setWindowIcon(QIcon(":/images/dna_2.ico"));
    //#ifdef CALIBRATION
    //setWindowIcon(QIcon(":/images/dna_2_red.ico"));
    //#endif


    //...Scenario...    
    scenario = new Scenario(this);
    scenario->Map_Receivers.insert("setup", p_setup);
    scenario->Map_Receivers.insert("run", p_run);
    scenario->Map_Receivers.insert("analysis", p_analysis);
    connect(scenario, SIGNAL(sSignalToReceiver(QWidget*)), this, SLOT(changePage(QWidget*)));
    connect(pagesWidget, SIGNAL(sSendRunButton()), scenario, SLOT(GetRunButton()));


    //... HttpProcess ...    
    //http = new HttpProcess(this);

    //... Email ...
    //email = new Email(this);


    //...WEB...    
    Web = new Web_Interface(this);
    Web_additional = new Web_Interface(this);
    qmlRegisterType<WrapperAx>("QmlDNA", 1,0, "WrapperAx"); // Anton Gusev
    Web->scenario = scenario;

    //... Server ...
    ApplSettings->beginGroup("Server");
    Web->url_Server = ApplSettings->value("address","").toString();
    Web_additional->url_Server = Web->url_Server;
    ApplSettings->endGroup();


    scenario->List_Protocols = &list_WebProtocolsOrder; // &Web->list_Protocols;

    //...QML...    
    Web_Dialog = new WebDialog(this);
    connect(Web_Dialog->connect_button, SIGNAL(clicked(bool)), this, SLOT(WebConnect()));
    connect(Web, SIGNAL(message_FromServer(QString)), Web_Dialog, SLOT(get_MessageFromServer(QString)));
    connect(Web, SIGNAL(message_ToServer(QString)), Web_Dialog, SLOT(send_MessageToServer(QString)));

    pweb = new QQuickWidget(Web_Dialog);
    pweb->rootContext()->setContextProperty("Web", Web);

    pweb_add = new QQuickWidget();
    pweb_add->rootContext()->setContextProperty("Web", Web_additional);

    web_Protocols = NULL;

#ifndef CALIBRATION
    //pweb->setSource(QUrl::fromLocalFile("qml/balu_dir.qml"));
    //pweb->setSource(QUrl::fromLocalFile("qml/integrator.qml"));

    pweb->setSource(QUrl::fromLocalFile(dir_path + "/qml/rtapp.qml"));
    pweb_add->setSource(QUrl::fromLocalFile(dir_path + "/qml/consumer.qml"));
#endif


    connect(Web, SIGNAL(connection_Changed(bool)), this, SLOT(WebConnection_Changed(bool)));
    connect(Web, SIGNAL(listProtocols_Changed()), this, SLOT(WebListProtocolsOrder_Changed()));
    connect(Web, SIGNAL(send_ProtocolToSetup(QString)), this, SLOT(WebProtocolToSetup(QString)));
    connect(Web, SIGNAL(get_List(QString,QString)), this, SLOT(WebList_Get(QString,QString)));
    connect(Web, SIGNAL(get_protocol(QString)), this, SLOT(WebProtocolOrder_Get(QString)));
    connect(Web, SIGNAL(confirmation_FromServer(QString,QString)), this, SLOT(WebConfirmationFromSetup(QString,QString)));

    connect(Web_additional, SIGNAL(listProtocols_Changed()), this, SLOT(WebListProtocols_Changed()));
    connect(Web_additional, SIGNAL(connection_Changed(bool)), this, SLOT(WebConnection_add_Changed(bool)));
    connect(Web_additional, SIGNAL(get_List(QString,QString)), this, SLOT(WebList_add_Get(QString,QString)));
    connect(Web_additional, SIGNAL(get_protocol(QString)), this, SLOT(WebProtocol_Get(QString)));
    connect(Web_additional, SIGNAL(send_ProtocolToAnalysis(QString)), this, SLOT(WebProtocolToAnalysis(QString)));

    connect(Web, SIGNAL(sAnswerToServer(QString)), this, SLOT(ChangeStateProtocol_ToWeb(QString)));
    connect(pagesWidget, SIGNAL(sChange_StateProtocolToWeb(QString)), this, SLOT(ChangeStateProtocol_ToWeb(QString)));
    connect(pagesWidget, SIGNAL(sSend_ResultProtocolToWeb(QString)), this, SLOT(slot_ProtocolToServer(QString)));
    connect(pagesWidget, SIGNAL(sSend_OrderProtocolToWeb(QString)), this, SLOT(slot_ProtocolOrderToServer(QString)));


    connect(scenario, SIGNAL(sStatus_ExecutePoint(QString)), this, SLOT(ChangeStateProtocol_ToWeb(QString)));
    connect(scenario->active_gif, SIGNAL(stateChanged(QMovie::MovieState)), this, SLOT(slot_ActivePoint(QMovie::MovieState)));
    connect(scenario, SIGNAL(sGetProtocolFromList(QString,QString)), this, SLOT(slot_ProtocolFromWeb(QString,QString)));
    connect(Web, SIGNAL(send_ProtocolToScenario(QString)), scenario, SLOT(GetProtocolFromWeb(QString)));
    connect(Web, SIGNAL(sRun_Scenario(QString,QMap<QString,QString>)), scenario, SLOT(Run_Scenario(QString,QMap<QString,QString>)));

    if(Web->url_Server == "LocalDir")
    {
        emit Web->connection_Changed(true);
        emit Web->listProtocols_Changed();
    }


    //...
    /*
    bool state_connection = true;
    QVariant returned_Value = NULL;
    QVariant state = state_connection;
    QObject *obj = (QObject*)(pweb->rootObject());
    QMetaObject::invokeMethod(obj, "set_StateSocket",
                              Q_RETURN_ARG(QVariant, returned_Value),
                              Q_ARG(QVariant, state));
    */

    SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED | ES_AWAYMODE_REQUIRED | ES_DISPLAY_REQUIRED);

    //... TrayIcon ...
    setTrayIconActions();
    showTrayIcon();

    //... Splash ...
    splash = NULL;

    /*QStringList locations = QStandardPaths::standardLocations(QStandardPaths::AppConfigLocation);
    foreach(QString str, locations)
    {
        if(str.contains("ProgramData"))
        {
            qDebug() << "AppConfigLocation: " << str;
        }
    }*/

    msgBox.setWindowIcon(QIcon(":/images/DTm.ico"));
#ifdef CALIBRATION
    msgBox.setWindowIcon(QIcon(":/images/check.ico"));
#endif
    //msgBox.setWindowIcon(QIcon(":/images/RT.ico"));
}
//-----------------------------------------------------------------------------
//--- setTrayIconActions()
//-----------------------------------------------------------------------------
void Main_RT::setTrayIconActions()
{
    //... Setting actions ...
    minimizeAction = new QAction("Hide", this);
    restoreAction = new QAction("Restore", this);
    quitAction = new QAction("Exit", this);

    //... Connecting actions to slots ...
    connect(minimizeAction, SIGNAL(triggered()), this, SLOT(hide()));
    connect(restoreAction, SIGNAL(triggered()), this, SLOT(showNormal()));
    connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));

    //... Setting system tray's icon menu ...
    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction (minimizeAction);
    trayIconMenu->addAction (restoreAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction (quitAction);
}
//-----------------------------------------------------------------------------
//--- showTrayIcon()
//-----------------------------------------------------------------------------
void Main_RT::showTrayIcon()
{
    trayIcon = new QSystemTrayIcon(this);
    QIcon trayImage = QIcon(":/images/DTm.ico");
    //QIcon trayImage = QIcon(":/images/RT.ico");
    trayIcon->setIcon(trayImage);
    //trayIcon->setIcon(windowIcon());
    trayIcon->setContextMenu(trayIconMenu);

    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this,     SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));

    //trayIcon->show();
    //trayIcon->setVisible(false);
    set_ActiveTray(false);
}
//-----------------------------------------------------------------------------
//--- iconActivated
//-----------------------------------------------------------------------------
void Main_RT::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::Trigger:
    case QSystemTrayIcon::DoubleClick:
                if(!isVisible()) this->showNormal();
                else close();

    case QSystemTrayIcon::MiddleClick:
                //showMessage();
                break;
    default:    break;

    }
}
//-----------------------------------------------------------------------------
//--- set_ActiveTray(bool state)
//-----------------------------------------------------------------------------
void Main_RT::set_ActiveTray(bool state)
{
    trayIcon->setVisible(state);
    QApplication::setQuitOnLastWindowClosed(!state);
}

//-----------------------------------------------------------------------------
//--- set_ConnectionWeb()
//-----------------------------------------------------------------------------
void Main_RT::set_ConnectionWeb()
{
    //qDebug() << "set_ConnectionWeb(): ";

    QVariant url = Web->url_Server; //"ws://localhost:9902";
    bool state_connection = true;
    QVariant returned_Value = NULL;
    QVariant state = state_connection;
    QObject *obj = (QObject*)(pweb->rootObject());

    QMetaObject::invokeMethod(obj, "set_SocketUrl",
                              Q_RETURN_ARG(QVariant, returned_Value),
                              Q_ARG(QVariant, url));
}
//-----------------------------------------------------------------------------
//--- set_ConnectionWeb_add()
//-----------------------------------------------------------------------------
void Main_RT::set_ConnectionWeb_add()
{
    //qDebug() << "set_ConnectionWeb(): ";

    QVariant url = Web_additional->url_Server; //"ws://localhost:9902";
    bool state_connection = true;
    QVariant returned_Value = NULL;
    QVariant state = state_connection;
    QObject *obj = (QObject*)(pweb_add->rootObject());

    QMetaObject::invokeMethod(obj, "set_SocketUrl",
                              Q_RETURN_ARG(QVariant, returned_Value),
                              Q_ARG(QVariant, url));

}
//-----------------------------------------------------------------------------
//--- Destructor
//-----------------------------------------------------------------------------
Main_RT::~Main_RT()
{
    int i;
    QWidget *w_obj;

    qDebug() << "Destructor:";

    if(WS_Client->state() == QAbstractSocket::ConnectedState) WS_Client->close();
    //if(!http->isHidden()) http->hide();
    if(!scenario->isHidden()) scenario->hide();

    //delete widgetgraph;
    last_page = contentsWidget->currentRow();

    writeSettings();    

    delete Web;
    delete pweb;
    delete Web_Dialog;
    delete scenario;
    //delete http;

    delete Web_additional;
    delete pweb_add;
    //delete email;

    //delete pagesWidget;
    for(i = pagesWidget->count(); i >= 0; i--)
    {
        w_obj = pagesWidget->widget(i);
        pagesWidget->removeWidget(w_obj);
        delete w_obj;
    }
    delete pagesWidget;


    if(dll_analysis) ::FreeLibrary(dll_analysis);    
    if(dll_saver) ::FreeLibrary(dll_saver);    
    if(dll_run) ::FreeLibrary(dll_run);    
    if(dll_setup) ::FreeLibrary(dll_setup);

    CoUninitialize();

    if(trayIcon) delete trayIcon;
    delete ApplSettings;
}

//-----------------------------------------------------------------------------
//--- SetConnectToServer()
//-----------------------------------------------------------------------------
void Main_RT::SetConnectToServer()
{
    Web_Dialog->edit_url->setText(Web->url_Server);
    int i=0;
    while(i<1)
    {
        i++;
        set_ConnectionWeb();
        Sleep(100);
        set_ConnectionWeb_add();
    }
}

//-----------------------------------------------------------------------------
//--- showEvent
//-----------------------------------------------------------------------------
/*void Main_RT::keyPressEvent(QKeyEvent *e)
{
    qDebug() << "KeyEvevnt: " << e->key();

    return QMainWindow::keyPressEvent(e);
}*/
//-----------------------------------------------------------------------------
//--- showEvent
//-----------------------------------------------------------------------------
bool Main_RT::event(QEvent *e)
{
    if(e->type() == QEvent::WindowStateChange)
    {
        if(this->windowState() == Qt::WindowMinimized)
        {
            last_page = contentsWidget->currentRow();
            //qDebug() << "Qt::WindowMinimized:" << last_page;
        }
    }

    /*if(e->type() == QKeyEvent::KeyPress)
    {
        QKeyEvent *p_event = static_cast<QKeyEvent *>(e);
        qDebug() << "Evevnt: " << p_event->key();
    }*/

    return QWidget::event(e);
}

//-----------------------------------------------------------------------------
//--- showEvent
//-----------------------------------------------------------------------------
void Main_RT::showEvent(QShowEvent *e)
{
    if(contentsWidget->currentRow() >= 0) last_page = contentsWidget->currentRow();

    if(!(contentsWidget->item(0)->flags() & Qt::ItemIsEnabled)) last_page = 2;
    if(!(contentsWidget->item(1)->flags() & Qt::ItemIsEnabled)) last_page = 2;

#ifdef CALIBRATION
    last_page = 1;      // Run

    contentsWidget->item(0)->setFlags(contentsWidget->item(0)->flags() & ~Qt::ItemIsEnabled);
    setup_menu->setDisabled(true);
    network_menu->setDisabled(true);
    preference_menu->setDisabled(true);

#endif

    contentsWidget->setCurrentRow(last_page);
    //SetConnectToServer();

   e->accept();

   //::SetForegroundWindow((HWND)winId());
   //::SetActiveWindow((HWND)winId());

}

//-----------------------------------------------------------------------------
//--- closeEvent
//-----------------------------------------------------------------------------
void Main_RT::closeEvent(QCloseEvent *event)
{
    bool sts_exit = true;

    //qDebug() << "closeEvent:" ;

    //... TrayIcon ...
    if(trayIcon && trayIcon->isVisible())
    {
        //qDebug() << "tray:";
        hide();
        event->ignore();
        QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::MessageIcon(QSystemTrayIcon::Information);
        trayIcon->showMessage("TrayIcon", tr("The DTmaster is minimized to the tray"), icon, 2000);

        return;
    }

    QApplication::setQuitOnLastWindowClosed(true);

    //... WebDialog ...
    if(!Web_Dialog->isHidden()) Web_Dialog->close();


    //... Run ............................................................
    if(dll_run && p_run)
    {
        IsExit run_func = reinterpret_cast<IsExit>(::GetProcAddress(dll_run,"is_exit@4"));
        if(run_func)
        {
            if(!run_func(p_run)) sts_exit = false;
            else Sleep(500);
        }
    }

    //... Analysis .......................................................
    if(dll_analysis && p_analysis)
    {
        IsExit analysis_func = reinterpret_cast<IsExit>(::GetProcAddress(dll_analysis,"is_exit@4"));
        if(analysis_func)
        {
            if(!analysis_func(p_analysis)) sts_exit = false;
        }
    }



    if(sts_exit) event->accept();
    else event->ignore();
}

//-----------------------------------------------------------------------------
//--- Load_POINTERDBASE
//-----------------------------------------------------------------------------
void Main_RT::Load_POINTERDBASE(QAxObject *axRita_resource)
{
    // ... Setup ...
    if(p_setup && dll_setup)
    {
        Load_PointTestDBase load_dbase = reinterpret_cast<Load_PointTestDBase>(
                                                ::GetProcAddress(dll_setup,"load_PointerDBase@8"));
        if(load_dbase)
        {
            load_dbase(p_setup, axRita_resource);
        }
    }
}

//-----------------------------------------------------------------------------
//--- Load_TESTVERIFICATION
//-----------------------------------------------------------------------------
void Main_RT::Load_TESTVERIFICATION(QAxObject *axgp_resource)
{
    //qDebug() << "Load_TESTVERIFICATION: START";
    // ... Setup ...
    if(p_setup && dll_setup)
    {
        Load_TestVerification load_verification = reinterpret_cast<Load_TestVerification>(
                                                ::GetProcAddress(dll_setup,"load_TestVerification@8"));
        if(load_verification)
        {            
            load_verification(p_setup, axgp_resource);
        }
    }    

    // ... Analysis ...
    if(p_analysis && dll_analysis)
    {
        Load_TestVerification_Analysis load_verification_analysis = reinterpret_cast<Load_TestVerification_Analysis>(
                                                ::GetProcAddress(dll_analysis,"load_TestVerification@8"));
        if(load_verification_analysis)
        {
            load_verification_analysis(p_analysis, axgp_resource);
        }
    }

    // ... Run ...
    if(p_run && dll_run)
    {
        Load_TestVerification_Run load_verification_run = reinterpret_cast<Load_TestVerification_Run>(
                                                ::GetProcAddress(dll_run,"load_TestVerification@8"));
        if(load_verification_run)
        {
            load_verification_run(p_run, axgp_resource);
        }
    }   
}

//-----------------------------------------------------------------------------
//--- Load_MENU
//-----------------------------------------------------------------------------
void Main_RT::Load_MENU()
{
    QStringList list_pActions;
    QStringList list;
    QString text;
    bool enable;

    // ... Setup ...
    if(p_setup && dll_setup)
    {
        Load_Menu load_menu = reinterpret_cast<Load_Menu>(
                    ::GetProcAddress(dll_setup,"load_Menu@8"));
        if(load_menu)
        {
            load_menu(p_setup, &list_pActions);

            setup_menu->clear();
            test_menu->clear();

            setup_menu->disconnect();
            test_menu->disconnect();

            foreach(text, list_pActions)
            {
                //qDebug() << "menu: " << text;
                list = text.split("\t");
                if(list.at(0) == "...")
                {
                    setup_menu->addSeparator();
                }
                else
                {
                    subMenu = new QAction(list.at(0), this);
                    subMenu->setWhatsThis(list.at(1));
                    enable = QString(list.at(2)).toInt();
                    subMenu->setEnabled(enable);

                    // ... Icons ...
                    if(subMenu->whatsThis() == "open") subMenu->setIcon(QIcon(":/images/flat/open_flat.png"));
                    if(subMenu->whatsThis() == "open_xml_7ver") subMenu->setIcon(QIcon(":/images/flat/open_xml_flat.png"));
                    if(subMenu->whatsThis() == "save") subMenu->setIcon(QIcon(":/images/flat/save_flat.png"));
                    if(subMenu->whatsThis() == "clear") subMenu->setIcon(QIcon(":/images/flat/clear_flat.png"));
                    if(subMenu->whatsThis() == "to_run") subMenu->setIcon(QIcon(":/images/flat/To_Run.png"));
                    if(subMenu->whatsThis() == "to_analysis") subMenu->setIcon(QIcon(":/images/flat/To_Analysis.png"));
                    if(subMenu->whatsThis() == "copy_block_tests") subMenu->setIcon(QIcon(":/images/flat/copyBlockTests_flat.png"));
                    if(subMenu->whatsThis() == "edit_test") subMenu->setIcon(QIcon(":/images/flat/edit_Test_flat.png"));
                    if(subMenu->whatsThis() == "edit_protocol") subMenu->setIcon(QIcon(":/images/flat/edit_Protocol_flat.png"));
                    // ...

                    if(subMenu->whatsThis() == "copy_block_tests") {test_menu->addAction(subMenu); continue;}
                    if(subMenu->whatsThis() == "edit_test") {test_menu->addAction(subMenu); continue;}

                    setup_menu->addAction(subMenu);
                }
            }
            connect(setup_menu, SIGNAL(triggered(QAction*)), this, SLOT(slot_SetupMenu(QAction*)));
            connect(setup_menu, SIGNAL(aboutToShow()), this, SLOT(slot_SetupShow()));
            connect(test_menu, SIGNAL(triggered(QAction*)), this, SLOT(slot_SetupMenu(QAction*)));
            connect(test_menu, SIGNAL(aboutToShow()), this, SLOT(slot_SetupShow()));
        }

    }
    list_pActions.clear();

    // ... Run ...
    if(p_run && dll_run)
    {
        Load_Menu load_menu = reinterpret_cast<Load_Menu>(
                    ::GetProcAddress(dll_run,"load_Menu@8"));
        if(load_menu)
        {
            load_menu(p_run, &list_pActions);

            run_menu->clear();
            run_menu->disconnect();

            foreach(text, list_pActions)
            {
                list = text.split("\t");
                if(list.at(0) == "...")
                {
                    run_menu->addSeparator();
                }
                else
                {
                    subMenu = new QAction(list.at(0), this);
                    subMenu->setWhatsThis(list.at(1));
                    enable = QString(list.at(2)).toInt();
                    subMenu->setEnabled(enable);
                    run_menu->addAction(subMenu);
                    // ... Icons ...
                    if(subMenu->whatsThis() == "select_protocol") subMenu->setIcon(QIcon(":/images/flat/open_flat.png"));
                    if(subMenu->whatsThis() == "select_device") subMenu->setIcon(QIcon(":/images/flat/open_flat.png"));
                    if(subMenu->whatsThis() == "contract") subMenu->setIcon(QIcon(":/images/flat/law_16.png"));
                    // ...
                }
            }
            connect(run_menu, SIGNAL(triggered(QAction*)), this, SLOT(slot_RunMenu(QAction*)));
            connect(run_menu, SIGNAL(aboutToShow()), this, SLOT(slot_RunShow()));
        }

    }
    list_pActions.clear();

    // ... Analysis ...
    if(p_analysis && dll_analysis)
    {
        Load_Menu load_menu = reinterpret_cast<Load_Menu>(
                    ::GetProcAddress(dll_analysis,"load_Menu@8"));
        if(load_menu)
        {
            load_menu(p_analysis, &list_pActions);

            analysis_menu->clear();
            analysis_menu->disconnect();

            foreach(text, list_pActions)
            {
                list = text.split("\t");

                //qDebug() << "menu: " << list;

                if(list.at(0) == "...")
                {
                    analysis_menu->addSeparator();
                }
                else
                {
                    subMenu = new QAction(list.at(0), this);
                    subMenu->setWhatsThis(list.at(1));
                    enable = QString(list.at(2)).toInt();
                    subMenu->setEnabled(enable);
                    // ... Icons ...
                    if(subMenu->whatsThis() == "open") subMenu->setIcon(QIcon(":/images/flat/open_flat.png"));
                    if(subMenu->whatsThis() == "save") subMenu->setIcon(QIcon(":/images/flat/save_flat.png"));
                    if(subMenu->whatsThis() == "info") subMenu->setIcon(QIcon(":/images/flat/info_flat.png"));
                    if(subMenu->whatsThis() == "report_param") subMenu->setIcon(QIcon(":/images/flat/report_preanalysis.png"));                    
                    if(subMenu->whatsThis() == "report") subMenu->setIcon(QIcon(":/images/flat/report_flat.png"));
                    if(subMenu->whatsThis() == "export") subMenu->setIcon(QIcon(":/images/flat/export_Results.png"));
                    if(subMenu->whatsThis() == "email") subMenu->setIcon(QIcon(":/images/flat/email.png"));
                    if(subMenu->whatsThis() == "to_setup") subMenu->setIcon(QIcon(":/images/flat/To_Protocol.png"));
                    if(subMenu->whatsThis() == "reopen") subMenu->setIcon(QIcon(":/images/flat/reopen_flat.png"));
                    if(subMenu->whatsThis() == "edit_test") subMenu->setIcon(QIcon(":/images/flat/edit_Test_flat.png"));
                    if(subMenu->whatsThis() == "edit_protocol") subMenu->setIcon(QIcon(":/images/flat/edit_Protocol_flat.png"));
                    if(subMenu->whatsThis() == "rdml_import") subMenu->setIcon(QIcon(":/images/flat/rdml.png"));
                    // ...

                    analysis_menu->addAction(subMenu);
                }
            }
            connect(analysis_menu, SIGNAL(triggered(QAction*)), this, SLOT(slot_AnalysisMenu(QAction*)));
            connect(analysis_menu, SIGNAL(aboutToShow()), this, SLOT(slot_AnalysisShow()));
        }

    }
    list_pActions.clear();
}
//-----------------------------------------------------------------------------
//--- Load_USERTESTs
//-----------------------------------------------------------------------------
void Main_RT::Load_USERTESTs(QAxObject *user, QTemporaryFile *f)
{
    int i;
    int id_user;
    QStringList	cache;
    QString item;
    QString text;
    QVector<int> vec;

    QDomDocument doc;
    QDomDocument doc_item;

    if(user)
    {
        nameUser_label->setText(user->property("userName").toString());
        if(f->open())
        {
            QDomNode xmlNode = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\"");
            doc.insertBefore(xmlNode, doc.firstChild());
            QDomElement  root = doc.createElement("TESTs");
            doc.appendChild(root);

            root.setAttribute("operator", user->property("userName").toString());

            cache = user->dynamicCall("getCache(QString)", "test").toStringList();

            foreach(item, cache) vec.append(item.toInt());
            qSort(vec);
            text = "";
            id_user = user->property("userId").toInt();

            for(i=0; i<vec.size(); i++)
            {
                QAxObject*  axItemCache = user->querySubObject("getCacheEntry(int)", vec.at(i));
                if(id_user == axItemCache->property("idOwner").toInt())
                {
                    text = axItemCache->property("dataEntry").toString();
                    doc_item.clear();
                    doc_item.setContent(text);

                    root.appendChild(doc_item);
                }
                delete axItemCache;
            }
            QTextStream(f) << doc.toString();
            f->close();
        }
    }
    else
    {
        qDebug() << "axUser is absent!";
        //QMessageBox::warning(0,"Warning!", "You are logged in as a disabled user...");
        As_DisabledUser();
    }
}
//-----------------------------------------------------------------------------
//--- load_user for Main
//-----------------------------------------------------------------------------
void Main_RT::load_user(QAxObject *user)
{
    flag_SaveLocationWin = false;
    flag_ApplicationSettings = false;
    flag_EnableSetup = false;
    flag_EnableRun = false;

    if(user)
    {
        flag_SaveLocationWin = user->dynamicCall("getPriv(QString)", "SAVE_LOCATION_WIN").toBool();
        flag_ApplicationSettings = user->dynamicCall("getPriv(QString)", "CHANGE_APP_PREF").toBool();
        flag_EnableSetup = user->dynamicCall("getPriv(QString)", "ENABLE_PAGE_SETUP").toBool();
        flag_EnableRun = user->dynamicCall("getPriv(QString)", "ENABLE_PAGE_RUN").toBool();
    }

    general->setEnabled(flag_ApplicationSettings);
    preference_export->setEnabled(flag_ApplicationSettings);

    edit_Programm->setEnabled(flag_EnableSetup);


#ifndef CALIBRATION

    setup_Page->setDisabled(!flag_EnableSetup);
    setup_menu->setDisabled(!flag_EnableSetup);
    test_menu->setDisabled(!flag_EnableSetup);
    if(!flag_EnableSetup)
    {
       contentsWidget->setCurrentRow(2);   // Analysis
       contentsWidget->item(0)->setFlags(contentsWidget->item(0)->flags() & ~Qt::ItemIsEnabled);
    }
    else contentsWidget->item(0)->setFlags(contentsWidget->item(0)->flags() | Qt::ItemIsEnabled);

    run_Page->setDisabled(!flag_EnableRun);
    run_menu->setDisabled(!flag_EnableRun);
    if(!flag_EnableRun)
    {
       contentsWidget->setCurrentRow(2);   // Analysis
       contentsWidget->item(1)->setFlags(contentsWidget->item(1)->flags() & ~Qt::ItemIsEnabled);
    }
    else contentsWidget->item(1)->setFlags(contentsWidget->item(1)->flags() | Qt::ItemIsEnabled);


#endif

}
//-----------------------------------------------------------------------------
//--- Load_USER
//-----------------------------------------------------------------------------
void Main_RT::Load_USER(QAxObject* user)
{
    user_authorization->setDisabled(false);    

    // ... Setup ...
    if(p_setup && dll_setup)
    {
        Load_User load_user = reinterpret_cast<Load_User>(
                       ::GetProcAddress(dll_setup,"load_User@8"));
        if(load_user)
        {
            load_user(p_setup, user);
        }

    }
    // ... Run ...
    if(p_run && dll_run)
    {
        Load_User load_user = reinterpret_cast<Load_User>(
                       ::GetProcAddress(dll_run,"load_User@8"));
        if(load_user)
        {
            load_user(p_run, user);
        }
    }
    // ... Analysis ...
    if(p_analysis && dll_analysis)
    {
        Load_User load_user = reinterpret_cast<Load_User>(
                       ::GetProcAddress(dll_analysis,"load_User@8"));
        if(load_user)
        {
            load_user(p_analysis, user);
        }
    }

    // ... Main ...
    load_user(user);
}
//-----------------------------------------------------------------------------
//--- APP_STATUS()
//-----------------------------------------------------------------------------
void Main_RT::APP_STATUS()
{
    QString app_name = APP_NAME;
    bool calibr_status = false;

#ifdef CALIBRATION
    calibr_status = true;
#endif


    // ... Run ...
    if(p_run && dll_run)
    {
        App_Status app_status = reinterpret_cast<App_Status>(
                    ::GetProcAddress(dll_run,"app_Status@12"));

        if(app_status)
        {
            app_status(p_run, app_name, calibr_status);
        }
    }
}

//-----------------------------------------------------------------------------
//--- Load_TESTs
//-----------------------------------------------------------------------------
void Main_RT::Load_TESTs(QString fn)
{
    if(p_setup && dll_setup)
    {
        Load_UserTests load_tests = reinterpret_cast<Load_UserTests>(
                       ::GetProcAddress(dll_setup,"load_UserTESTs@28"));
        if(load_tests)
        {            
            load_tests(p_setup, fn.toStdString());
        }        
    }
}
//-----------------------------------------------------------------------------
//--- Load_Protocol
//-----------------------------------------------------------------------------
void Main_RT::Load_PROTOCOL(QString fn)
{
    QFileInfo fi(fn);

    if(p_analysis && dll_analysis)
    {
        if(fi.exists() && fi.suffix() == "rt")
        {
            Load_Protocol load_protocol = reinterpret_cast<Load_Protocol>(
                           ::GetProcAddress(dll_analysis,"load_Protocol@8"));
            if(load_protocol)
            {
                contentsWidget->setCurrentRow(2);   // Analysis
                contentsWidget->item(0)->setFlags(contentsWidget->item(0)->flags() & ~Qt::ItemIsEnabled);
                contentsWidget->item(1)->setFlags(contentsWidget->item(1)->flags() & ~Qt::ItemIsEnabled);

                setup_menu->setDisabled(true);
                test_menu->setDisabled(true);

                run_menu->setDisabled(true);
                setup_Page->setDisabled(true);
                run_Page->setDisabled(true);

                load_protocol(p_analysis, (char*)(fn.toStdString().c_str()));
            }
        }
    }
}

//-----------------------------------------------------------------------------
//--- load Pages
//-----------------------------------------------------------------------------
void Main_RT::load_Pages()
{
    dll_analysis = NULL;
    dll_setup = NULL;
    dll_run = NULL;
    dll_saver = NULL;
    p_analysis = NULL;
    p_setup = NULL;
    p_run = NULL;
    p_saver = NULL;

    void *p_prot = NULL;

    //QString text;
    //LPCSTR lp = (LPCSTR)text.utf16();    

    //... Setup ............................................................    
#ifdef QT_DEBUG
    dll_setup = ::LoadLibraryW(L"setup_rtd.dll");
#else
    dll_setup = ::LoadLibraryW(L"setup_rt.dll");
#endif

    if(dll_setup)
    {
        Interface_Page setup_func = reinterpret_cast<Interface_Page>(
                       ::GetProcAddress(dll_setup,"create_page@0"));

        if(setup_func)
        {
            void *obj = setup_func();

            p_setup = reinterpret_cast<QWidget*>(obj);
        }
    }
    QApplication::processEvents();


    //... Analysis ............................................................    
#ifdef QT_DEBUG
    dll_analysis = ::LoadLibraryW(L"analysis_rtd.dll");
#else
    dll_analysis = ::LoadLibraryW(L"analysis_rt.dll");
#endif    
    if(dll_analysis)
    {
        Interface_Page analysis_func = reinterpret_cast<Interface_Page>(
                       ::GetProcAddress(dll_analysis,"create_page@0"));        
        if(analysis_func)
        {
            void *obj = analysis_func();            
            p_analysis = reinterpret_cast<QWidget*>(obj);
        }
    }
    QApplication::processEvents();    

    //... Saver ............................................................    
#ifdef QT_DEBUG
    dll_saver = ::LoadLibraryW(L"pages\\saver_rtd.dll");
#else
    dll_saver = ::LoadLibraryW(L"pages\\saver_rt.dll");
#endif

    if(dll_saver)
    {
        Interface_Page saver_func = reinterpret_cast<Interface_Page>(
                       ::GetProcAddress(dll_saver,"create_page@0"));
        if(saver_func)
        {
            void *obj = saver_func();
            p_saver = reinterpret_cast<QWidget*>(obj);
        }
    }
    QApplication::processEvents();    

    //... Run ............................................................    
#ifdef QT_DEBUG
    dll_run = ::LoadLibraryW(L"run_rtd.dll");
#else
    dll_run = ::LoadLibraryW(L"run_rt.dll");
#endif    

    if(dll_run)
    {
        Interface_Page run_func = reinterpret_cast<Interface_Page>(
                       ::GetProcAddress(dll_run,"create_page@0"));
        if(run_func)
        {
            void *obj = run_func();            
            p_run = reinterpret_cast<QWidget*>(obj);
            if(p_run)
            {
                /*Load_3Dobject load_3D = reinterpret_cast<Load_3Dobject>(::GetProcAddress(dll_run,"load_3Dbars@8"));
                if(load_3D)
                {
                    load_3D(p_run, widgetgraph);
                }*/
            }
        }
    }
    QApplication::processEvents();    

/*
    HINSTANCE dll = NULL;
    dll = ::LoadLibraryW(L"Plugin.dll");
    if(dll)
    {
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setText("Plugin - loaded!");
        msgBox.exec();
        ::FreeLibrary(dll);
    }
*/
    if(dll_run && dll_setup)
    {
        // 1. Get
        Get_PointerProtocol get_pointer = reinterpret_cast<Get_PointerProtocol>(
                    ::GetProcAddress(dll_setup,"Get_pPro@4"));
        if(get_pointer)
        {
            p_prot = get_pointer(p_setup);
            //qDebug() << "Get_pProtocol"   << get_pointer << p_prot;
        }

        // 2. Send
        Send_PointerProtocol send_pointer = reinterpret_cast<Send_PointerProtocol>(
                    ::GetProcAddress(dll_run,"Send_pPro@8"));
        if(send_pointer)
        {
            send_pointer(p_run, p_prot);
        }
    }

}

//-----------------------------------------------------------------------------
//--- create PagesWidget
//-----------------------------------------------------------------------------
void Main_RT::create_PagesWidget()
{    
    load_Pages();

    pagesWidget = new StackedWidget;
    pagesWidget->main_Window = this;

    if(p_saver) pagesWidget->addWidget(p_saver);
    if(p_setup) pagesWidget->addWidget(p_setup);
    if(p_run) pagesWidget->addWidget(p_run);
    if(p_analysis) pagesWidget->addWidget(p_analysis);    

    main_layout->addWidget(pagesWidget,1);
}
//-----------------------------------------------------------------------------
//--- change Page
//-----------------------------------------------------------------------------
void Main_RT::changePage(QWidget *p_widget)
{
    int id = -1;
    /*QEvent *e = new QEvent((QEvent::Type)9001);
    QApplication::sendEvent(p_run, e);
    QApplication::sendEvent(p_analysis, e);

    if(p_widget) pagesWidget->setCurrentWidget(p_widget);*/

    if(p_widget == p_setup) id = 0;
    if(p_widget == p_run) id = 1;
    if(p_widget == p_analysis) id = 2;

    if(id >= 0) contentsWidget->setCurrentRow(id);
}
//-----------------------------------------------------------------------------
//--- change Setup Page
//-----------------------------------------------------------------------------
void Main_RT::menu_SetupPage()
{
    contentsWidget->setCurrentRow(0);
}
//-----------------------------------------------------------------------------
//--- change Run Page
//-----------------------------------------------------------------------------
void Main_RT::menu_RunPage()
{
    contentsWidget->setCurrentRow(1);
}
//-----------------------------------------------------------------------------
//--- change Analysis Page
//-----------------------------------------------------------------------------
void Main_RT::menu_AnalysisPage()
{
    contentsWidget->setCurrentRow(2);
}
//-----------------------------------------------------------------------------
//--- menu_EditProgramm
//-----------------------------------------------------------------------------
void Main_RT::menu_EditProgramm()
{
    QEvent *e = new QEvent((QEvent::Type)4004);
    QApplication::sendEvent(p_setup, e);
}
//-----------------------------------------------------------------------------
//--- menu_ViewVideoArchive
//-----------------------------------------------------------------------------
void Main_RT::menu_ViewVideoArchive()
{
    QEvent *e = new QEvent((QEvent::Type)4005);
    QApplication::sendEvent(p_analysis, e);
}

//-----------------------------------------------------------------------------
//--- change Page
//-----------------------------------------------------------------------------
void Main_RT::changePage(int index)
{
    QWidget *w_obj = NULL;


    QEvent *e = new QEvent((QEvent::Type)9001);
    QApplication::sendEvent(p_run, e);
    QApplication::sendEvent(p_analysis, e);


    //setup_menu->setDisabled(true);
    //run_menu->setDisabled(true);
    //analysis_menu->setDisabled(true);

    //w_obj = p_saver;


    switch(index)
    {
    case 0:     if(p_setup) w_obj = p_setup;
                //setup_menu->setDisabled(false);
                break;
    //case 0:     if(p_saver) w_obj = p_saver;        break;
    case 1:     if(p_run) w_obj = p_run;
                //run_menu->setDisabled(false);
                break;
    case 2:     if(p_analysis) w_obj = p_analysis;
                //analysis_menu->setDisabled(false);
                break;
    default:    break;
    }    
    if(w_obj) pagesWidget->setCurrentWidget(w_obj);
    if(w_obj)
    {
        helpWidget->setCurrentRow(-1);
        helpWidget->item(0)->setIcon(QIcon(":/images/help_tmp.png"));
    }

    if(StyleApp == "fusion")
    {
        if(index == 0) contentsWidget->item(0)->setIcon(QIcon(":/images/flat/Protocol_enable.png"));
        else     {contentsWidget->item(0)->setIcon(QIcon(":/images/flat/Protocol_disable.png")); contentsWidget->item(0)->setTextColor(QColor(192,192,192));}
        //if(index == 1) contentsWidget->item(1)->setIcon(QIcon(":/images/flat/Run_enable.png"));
        //else     {contentsWidget->item(1)->setIcon(QIcon(":/images/flat/Run_disable.png")); contentsWidget->item(1)->setTextColor(QColor(192,192,192));}
        if(index == 1) contentsWidget->item(1)->setIcon(QIcon(":/images/flat/Device_enable.png"));
        else     {contentsWidget->item(1)->setIcon(QIcon(":/images/flat/Device_disable.png")); contentsWidget->item(1)->setTextColor(QColor(192,192,192));}
        if(index == 2) contentsWidget->item(2)->setIcon(QIcon(":/images//flat/Analysis_enable_pro.png"));
        else     {contentsWidget->item(2)->setIcon(QIcon(":/images/flat/Analysis_disable.png")); contentsWidget->item(2)->setTextColor(QColor(192,192,192));}
    }
    else
    {
        if(index == 0) contentsWidget->item(0)->setIcon(QIcon(":/images/setup_tmp1.png"));
        else     {contentsWidget->item(0)->setIcon(QIcon(":/images/setup_tmp.png")); contentsWidget->item(0)->setTextColor(QColor(192,192,192));}
        if(index == 1) contentsWidget->item(1)->setIcon(QIcon(":/images/run_tmp1.png"));
        else     {contentsWidget->item(1)->setIcon(QIcon(":/images/run_tmp.png")); contentsWidget->item(1)->setTextColor(QColor(192,192,192));}
        if(index == 2) contentsWidget->item(2)->setIcon(QIcon(":/images/analysis_tmp1.png"));
        else     {contentsWidget->item(2)->setIcon(QIcon(":/images/analysis_tmp.png")); contentsWidget->item(2)->setTextColor(QColor(192,192,192));}
    }

}

//-----------------------------------------------------------------------------
//--- change Page
//-----------------------------------------------------------------------------
void Main_RT::changeHelpPage()
{
    if(p_saver)
    {
        pagesWidget->setCurrentWidget(p_saver);
        helpWidget->item(0)->setIcon(QIcon(":/images/help_tmp1.png"));
    }
    contentsWidget->setCurrentRow(-1);
}

//-----------------------------------------------------------------------------
//--- create ListWidget
//-----------------------------------------------------------------------------
void Main_RT::create_ListWidget()
{
    contentsWidget = new QListWidget;
    contentsWidget->setFont(qApp->font());
    contentsWidget->setViewMode(QListView::IconMode);
    contentsWidget->setIconSize(QSize(64, 64));
    contentsWidget->setMovement(QListView::Static);
    contentsWidget->setMaximumWidth(74);
    contentsWidget->setSpacing(0);
    contentsWidget->setMinimumHeight(330);
    contentsWidget->setStyleSheet(
                                      "QListWidget {background: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 0,"
                                      "stop: 0 #111, stop: 1 #aaa);}"
                                      "QListWidget::item {background-color: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 0,"
                                      "stop: 0 #111, stop: 1 #aaa);}"
                                      "QListWidget::item:hover {background-color: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 0,"
                                      "stop: 0 #333, stop: 1 #ccc);}"
                                      "QListWidget::item:disabled {background-color: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 0,"
                                      "stop: 0 #444, stop: 1 #ddd);}"
                                      "QListWidget::item:selected {color: white;}"
                                      "QListWidget::item:selected {background-color: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 0,"
                                      "stop: 0 #000, stop: 1 #555);}");


    contentsWidget->setFocusPolicy(Qt::NoFocus);
    contentsWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    //contentsWidget->setMouseTracking(true);
    //contentsWidget->setStyleSheet( "QListWidget:item:selected:active {background: blue;}");
    //contentsWidget->setStyleSheet( "QListWidget::item:selected:active {background: q;lineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #111111, stop: 1 #dddddd);}");

    /*contentsWidget->setStyleSheet(
                //"QListWidget::item {"
                //     "border-style: solid;"
                //     "border-width:1px;"
                //     "border-color:black;"
                //     "background-color: green;"
                //  "}"
                //"QListWidget::item:!selected {"
                //   "background-color: gray;"
                //"}"
                  "QListWidget::item:selected {"
                     "background-color: black;" //#ffffe1;"
                  "}"
                  "QListWidget{background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
                  "stop: 0 #aaaaaa, stop: 1 #ffffff);}"
                  "QListWidget::item:hover {"
                  "background-color: black;;" //#ffffa1;"
                  "}"
                );*/


    helpWidget = new QListWidget;
    helpWidget->setViewMode(QListView::IconMode);
    helpWidget->setIconSize(QSize(64, 64));
    helpWidget->setMovement(QListView::Static);
    helpWidget->setMaximumWidth(74);
    helpWidget->setSpacing(0);
    helpWidget->setMaximumHeight(74);
    helpWidget->setMinimumHeight(74);
    helpWidget->setFocusPolicy(Qt::NoFocus);
    helpWidget->setStyleSheet(
                                  "QListWidget {background: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 0,"
                                  "stop: 0 #111, stop: 1 #aaa);}"
                                  "QListWidget::item {background-color: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 0,"
                                  "stop: 0 #111, stop: 1 #aaa);}"
                                  "QListWidget::item:hover {background-color: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 0,"
                                  "stop: 0 #333, stop: 1 #ccc);}"
                                  "QListWidget::item:disabled {background-color: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 0,"
                                  "stop: 0 #111, stop: 1 #aaa);}"
                                  "QListWidget::item:selected {color: white;}"
                                  "QListWidget::item:selected {background-color: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 0,"
                                  "stop: 0 #000, stop: 1 #888);}");

    helpWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    helpWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);


    create_Icons();    

    QVBoxLayout *layout = new QVBoxLayout;

    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(contentsWidget);
    layout->addWidget(helpWidget);
    main_layout->addLayout(layout);
}

//-----------------------------------------------------------------------------
//--- create Icons for ListWidget
//-----------------------------------------------------------------------------
void Main_RT::create_Icons()
{
    QListWidgetItem *config_Button = new QListWidgetItem(contentsWidget);
    config_Button->setIcon(QIcon(":/images/flat/Protocol_enable.png"));
    config_Button->setText(tr("Setup"));
    config_Button->setTextAlignment(Qt::AlignHCenter);
    config_Button->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    config_Button->setBackground(QColor(96,96,96)); //Qt::gray);
    config_Button->setTextColor(Qt::white);

    QListWidgetItem *run_Button = new QListWidgetItem(contentsWidget);
    run_Button->setIcon(QIcon(":/images/flat/Run_enable.png"));
    if(StyleApp == "fusion") run_Button->setIcon(QIcon(":/images/flat/Run_enable.png"));
    run_Button->setText(tr("Run"));
    run_Button->setTextAlignment(Qt::AlignHCenter);
    run_Button->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    run_Button->setBackground(QColor(96,96,96));
    run_Button->setTextColor(Qt::white);

    QListWidgetItem *analysis_Button = new QListWidgetItem(contentsWidget);
    analysis_Button->setIcon(QIcon(":/images/analysis_tmp1.png"));
    if(StyleApp == "fusion") analysis_Button->setIcon(QIcon(":/images/flat/Analysis_enable.png"));
    analysis_Button->setText(tr("Analysis"));
    analysis_Button->setTextAlignment(Qt::AlignHCenter);
    analysis_Button->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    analysis_Button->setBackground(QColor(96,96,96));
    analysis_Button->setTextColor(Qt::white);

    QListWidgetItem *help_Button = new QListWidgetItem(helpWidget);
    help_Button->setIcon(QIcon(":/images/help_tmp.png"));    
    //help_Button->setText(tr("Help"));
    //help_Button->setTextAlignment(Qt::AlignHCenter);
    help_Button->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    help_Button->setBackground(QColor(96,96,96));
    helpWidget->setDisabled(true);

    //connect(contentsWidget, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
    //        this,           SLOT(changePage(QListWidgetItem*,QListWidgetItem*)));

    connect(contentsWidget, SIGNAL(currentRowChanged(int)),
            this,           SLOT(changePage(int)));
    connect(helpWidget, SIGNAL(clicked(QModelIndex)),
            this,           SLOT(changeHelpPage()));
}

//-----------------------------------------------------------------------------
//--- Read Settings
//-----------------------------------------------------------------------------
void Main_RT::readSettings()
{
    QString text,font_family;
    int font_size;
    int font_weight;
    QStringList list;
    QSize size_default;    
    QString dir_path = qApp->applicationDirPath();
    QString dir_SysTranslate;

    /*QDesktopWidget desktop;
    QRect rect = desktop.availableGeometry(this);
    QPoint center = rect.center();*/


    //... MainWindow ...
    ApplSettings->beginGroup("MainWindow");
    size_default.setWidth(qApp->desktop()->availableGeometry(this).width() * 0.8);
    size_default.setHeight(qApp->desktop()->availableGeometry(this).height() * 0.6);
    QRect rect_default(0,0, size_default.width(), size_default.height());
    QSize size = ApplSettings->value("size",size_default).toSize();
    QPoint pos_default = qApp->desktop()->availableGeometry(this).center()-rect_default.center();
    QPoint pos = ApplSettings->value("pos", pos_default).toPoint();


    // validate pos/size
    if(pos.x() < 0 || pos.y() < 0 || size.width() <= 0 || size.height() <= 0)
    {
        size = size_default;
        pos = pos_default;
    }

#ifdef CALIBRATION
    size = size_default;
    pos = pos_default;
#endif

    resize(size);
    move(pos);
    if(ApplSettings->value("fullscreen").toBool())
    {
        size.setHeight(size.height()/1.5);
        size.setWidth(size.width()/1.5);
        resize(size);
        pos.setX((qApp->desktop()->width() - size.width())/2);
        pos.setY((qApp->desktop()->height() - size.height())/2);
        move(pos);
        setWindowState(Qt::WindowMaximized);
    }
    last_page = ApplSettings->value("last_page",0).toInt();
    if(last_page < 0) last_page = 0;

    ApplSettings->endGroup();

    //... Common ...
    ApplSettings->beginGroup("Common");

        ID_LANG = ID_LANG_RU;
        //... Language ...
        text = ApplSettings->value("language","ru").toString();
        //text = "ch";
        if(text.trimmed() == "en") ID_LANG = ID_LANG_EN;
        LanguageApp = text;
        dir_SysTranslate = dir_path + "/tools/translations";
        //dir_SysTranslate = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
        if(translator.load(":/translations/" + text + ".qm"))
        {
            if(qt_translator.load(("qt_" + text, dir_SysTranslate)))
            {
               qApp->installTranslator(&qt_translator);
               qDebug() << "Ok Main System Language";
            }

            qApp->installTranslator(&translator);
        }

        //... Font ...
        text = ApplSettings->value("font","").toString();
        list = text.split("\t");
        if(list.size() >= 3)
        {
            font_family = list.at(0);
            text = list.at(1);
            font_size = text.toInt();
            text = list.at(2);
            font_weight = text.toInt();

            app_font = QFont(font_family, font_size, font_weight);
            QApplication::setFont(app_font);
        }

        //... Style ...
        text = ApplSettings->value("style","xp").toString();
        StyleApp = text;

    ApplSettings->endGroup();


    //.........................................................................    
    setMinimumHeight(450);
    setMinimumWidth(600);    
}

//-----------------------------------------------------------------------------
//--- Write Settings
//-----------------------------------------------------------------------------
void Main_RT::writeSettings()
{    
    if(!flag_SaveLocationWin) return;

    bool fullscreen = false;
    if(windowState() == Qt::WindowMaximized) fullscreen = true;

#ifdef CALIBRATION
    return;
#endif
    //--- MainWindow ---
    ApplSettings->beginGroup("MainWindow");

    ApplSettings->setValue("fullscreen",fullscreen);
    ApplSettings->setValue("size",size());
    ApplSettings->setValue("pos",pos());
    ApplSettings->setValue("last_page",last_page);

    ApplSettings->endGroup();

 }
//-----------------------------------------------------------------------------
//--- Slot update_Rita()
//-----------------------------------------------------------------------------
void Main_RT::update_Rita()
{
    axRita->dynamicCall("triggerHeartBeat()");
}

//-----------------------------------------------------------------------------
//--- Slot update_DateTime()
//-----------------------------------------------------------------------------
void Main_RT::update_DateTime()
{
    QString tim = QDateTime::currentDateTime().toString(" dd.MM.yyyy, h:mm:ss ");
    mode_label->setText(tim);    
}
//-----------------------------------------------------------------------------
//--- check_WebConnection()
//-----------------------------------------------------------------------------
void Main_RT::check_WebConnection()
{
    //qDebug() << "Web: " << Web->connection_Server << Web->url_Server;

    if(!Web->connection_Server)
    {
        WebConnect();
    }
}

//-----------------------------------------------------------------------------
//--- create StatusBar
//-----------------------------------------------------------------------------
void Main_RT::create_StatusBar()
{
    mode_label = new QLabel(this);

    mode_label->setFixedWidth(220);
    mode_label->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);

    /*
    QLabel *version_label = new QLabel(this);
    version_label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    //version_label->setText("ver. " + QCoreApplication::applicationVersion() + " ");
    version_label->setText("");
    */
    statusBar_widget = new QWidget(this);
    /*QHBoxLayout* statusBar_layout = new QHBoxLayout;
    statusBar_layout->setMargin(0);
    statusBar_layout->setSpacing(2);
    statusBar_widget->setLayout(statusBar_layout);
    */

    active_label = new QLabel(this);
    active_label->setFixedWidth(24);
    //active_label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ActivePoint_gif = new QMovie(":/images/active_point.gif");
    active_label->setMovie(ActivePoint_gif);
    active_label->setVisible(false);

    connect_toServer = new QLabel(this);
    connect_toServer->setFixedWidth(80);
    connect_toServer->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
    obj_gif = new QMovie(":/images/connect.gif");
    connect_toServer->setMovie(obj_gif);
    connect_toServer->setVisible(false);

    client_label = new QLabel(tr("DTmaster"), this);
    client_label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    client_label->setFixedWidth(75);
    client_label->setVisible(false);
    server_label = new QLabel(this);
    server_label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    server_label->setFixedWidth(250);
    server_label->setVisible(false);

    account_label = new QLabel(this);
    account_label->setPixmap(QPixmap(":/images/flat/account_24.png"));
    nameUser_label = new QLabel("???", this);

    statusBar()->addPermanentWidget(mode_label);
    statusBar()->addPermanentWidget(client_label);
    statusBar()->addPermanentWidget(connect_toServer);
    statusBar()->addPermanentWidget(server_label);
    statusBar()->addPermanentWidget(statusBar_widget, 1);
    statusBar()->addPermanentWidget(active_label);
    statusBar()->addPermanentWidget(account_label);
    statusBar()->addPermanentWidget(nameUser_label);


    /*statusBar_layout->addWidget(mode_label);
    statusBar_layout->addWidget(client_label);
    statusBar_layout->addWidget(connect_toServer);
    statusBar_layout->addWidget(server_label, 1, Qt::AlignLeft);
    statusBar_layout->addWidget(active_label, 1, Qt::AlignRight);

    statusBar()->addWidget(statusBar_widget);
    */

    /*statusBar()->addWidget(mode_label);
    statusBar()->addWidget(client_label);
    statusBar()->addWidget(connect_toServer);
    statusBar()->addWidget(server_label);    
    statusBar()->addWidget(active_label);
    */
    //statusBar()->addWidget(version_label,1);    

    QFont font = app_font;
    font.setBold(false);
    //font.setPointSizeF(8);
    statusBar()->setFont(font);
}
//-----------------------------------------------------------------------------
//--- slot_ActivePoint
//-----------------------------------------------------------------------------
void Main_RT::slot_ActivePoint(QMovie::MovieState state)
{
    if(state == QMovie::NotRunning) {active_label->setVisible(false); ActivePoint_gif->stop();}
    if(state == QMovie::Running) {active_label->setVisible(true); ActivePoint_gif->start();}
}
//-----------------------------------------------------------------------------
//--- load_CalibrationScenario()
//-----------------------------------------------------------------------------
void Main_RT::load_CalibrationScenario()
{
    scenario->Open_CalibrationScenario();
}
//-----------------------------------------------------------------------------
//--- slot_DisableMenu
//-----------------------------------------------------------------------------
void Main_RT::slot_DisableMenu(int id)
{
    setup_menu->setDisabled(true);
    test_menu->setDisabled(true);

    run_menu->setDisabled(true);
    analysis_menu->setDisabled(true);

    switch(id)
    {
    case 0: if(flag_EnableSetup) {setup_menu->setDisabled(false); test_menu->setDisabled(false);}
            break;
    case 1: if(flag_EnableRun) run_menu->setDisabled(false);
            break;
    case 2: analysis_menu->setDisabled(false);
            break;

    default:    break;
    }
}

//-----------------------------------------------------------------------------
//--- create Menu
//-----------------------------------------------------------------------------
void Main_RT::create_Menu()
{
    QFont font = app_font;
    font.setBold(false);    
    menuBar()->setFont(font);

    file_menu = menuBar()->addMenu(tr("&File"));
    setup_Page = new QAction(QIcon(":/images/flat/protocol_24.png"), tr("setup_Page"), this);
    run_Page = new QAction(QIcon(":/images/flat/DT.png"), tr("run_Page"), this);
    analysis_Page = new QAction(QIcon(":/images/flat/analysis_24.png"), tr("analysis_Page"), this);
    edit_Programm = new QAction(QIcon(":/images/flat/editProgramm_24.png"), tr("edit Programm"), this);
    view_VideoArchive = new QAction(QIcon(":/images/flat/video_24.png"), tr("view VideoArchive"), this);
    exit = new QAction(QIcon(":/images/flat/exit_24.png"), tr("Exit"), this);

    file_menu->addAction(setup_Page);
    file_menu->addAction(run_Page);
    file_menu->addAction(analysis_Page);
    file_menu->addSeparator();
    file_menu->addAction(edit_Programm);
    file_menu->addAction(view_VideoArchive);
    file_menu->addSeparator();
    file_menu->addAction(exit);

    connect(setup_Page,SIGNAL(triggered(bool)), this, SLOT(menu_SetupPage()));
    connect(run_Page,SIGNAL(triggered(bool)), this, SLOT(menu_RunPage()));
    connect(analysis_Page,SIGNAL(triggered(bool)), this, SLOT(menu_AnalysisPage()));
    connect(edit_Programm,SIGNAL(triggered(bool)), this, SLOT(menu_EditProgramm()));
    connect(view_VideoArchive,SIGNAL(triggered(bool)), this, SLOT(menu_ViewVideoArchive()));
    connect(exit,SIGNAL(triggered(bool)), this, SLOT(close()));
    file_menu->setFont(app_font);

    test_menu = menuBar()->addMenu(tr("&Test"));
    test_menu->setFont(app_font);



    setup_menu = menuBar()->addMenu(tr("&Setup"));
    setup_menu->setFont(app_font);
    run_menu = menuBar()->addMenu(tr("&Run"));
    run_menu->setFont(app_font);
    analysis_menu = menuBar()->addMenu(tr("&Analysis"));
    analysis_menu->setFont(app_font);

    tools_menu = menuBar()->addMenu(tr("&Tools"));
    tools_menu->setFont(app_font);

    network_menu = menuBar()->addMenu(tr("&Network"));
    network_menu->setFont(app_font);
    preference_menu = menuBar()->addMenu(tr("&Preference"));
    preference_menu->setFont(app_font);
    help_menu = menuBar()->addMenu(tr("&Help"));
    help_menu->setFont(app_font);

    general = new QAction(tr("General"), this);    
    connect(general, SIGNAL(triggered(bool)), this, SLOT(slot_General()));
    general_menu = NULL;
    user_authorization = new QAction(tr("User authorization"), this);
    connect(user_authorization, SIGNAL(triggered(bool)), this, SLOT(slot_UserAuthorization()));
    user_authorization->setDisabled(true);
    user_replace_authorization = new QAction(tr("Replace users authorization"), this);
    connect(user_replace_authorization, SIGNAL(triggered(bool)), this, SLOT(slot_ReplaceUserAuthorization()));
    preference_export = new QAction(tr("Preference Export"), this);
    connect(preference_export, SIGNAL(triggered(bool)), this, SLOT(slot_PreferenceExport()));


    QString text = APP_NAME;
    RT_help = new QAction(text + " Help", this);
    RT_help->setDisabled(true);
    connect(RT_help, SIGNAL(triggered(bool)), this, SLOT(slot_Help()));
    about = new QAction(tr("About"), this);
    connect(about, SIGNAL(triggered(bool)), this, SLOT(slot_About()));
    about_moduls = new QAction(tr("About moduls"), this);
    connect(about_moduls, SIGNAL(triggered(bool)), this, SLOT(slot_AboutModuls()));
    aboutmoduls_menu = NULL;

    preference_menu->addAction(general);
    authorization_menu = preference_menu->addMenu(tr("authorization"));
    authorization_menu->addAction(user_authorization);
    authorization_menu->addAction(user_replace_authorization);
    authorization_menu->setFont(qApp->font());
    preference_menu->addAction(preference_export);

    help_menu->addAction(RT_help);
    help_menu->addSeparator();
    help_menu->addAction(about);
    help_menu->addAction(about_moduls);

    connect_WSServer = new QAction(tr("Connect to server..."), this);
    connect(connect_WSServer, SIGNAL(triggered(bool)), this, SLOT(slot_ConnectToServer()));
    qtScript = new QAction(tr("QTScript..."), this);
    connect(qtScript, SIGNAL(triggered(bool)), this, SLOT(slot_QtScript()));
    qtScript->setDisabled(true);
    list_ProtocolsOrder = new QAction(tr("List of orders Web_Protocols"), this);
    connect(list_ProtocolsOrder, SIGNAL(triggered(bool)), this, SLOT(slot_ListWebProtocolsOrder()));
    list_ProtocolsOrder->setDisabled(true);
    protocol_to_Server = new QAction(tr("Protocol to Web Server"), this);
    connect(protocol_to_Server, SIGNAL(triggered(bool)), this, SLOT(slot_ProtocolToServer()));
    protocol_to_Server->setDisabled(true);
    list_Protocols = new QAction(tr("List of Web_Protocols"), this);
    connect(list_Protocols, SIGNAL(triggered(bool)), this, SLOT(slot_ListWebProtocols()));
    list_Protocols->setDisabled(true);
    protocolOrder_to_Server = new QAction(tr("Protocol_Order to Web Server"), this);
    connect(protocolOrder_to_Server, SIGNAL(triggered(bool)), this, SLOT(slot_ProtocolOrderToServer()));
    protocolOrder_to_Server->setDisabled(true);
    httpprocess = new QAction(tr("Http Process"), this);
    connect(httpprocess, SIGNAL(triggered(bool)), this, SLOT(slot_HttpProcess()));


    //network_menu->addAction(qtScript);
    network_menu->addAction(list_ProtocolsOrder);
    network_menu->addAction(list_Protocols);
    network_menu->addAction(protocol_to_Server);
    network_menu->addSeparator();
    network_menu->addAction(httpprocess);


    scenario_action = new QAction(tr("Scenario..."), this);
    connect(scenario_action, SIGNAL(triggered(bool)), this, SLOT(slot_ScenarioAction()));
    //email_action = new QAction(QIcon(":/images/flat/email.png"), tr("Send message via Email"), this);
    //connect(email_action, SIGNAL(triggered(bool)), this, SLOT(slot_EmailAction()));

    tools_menu->addAction(scenario_action);
    //tools_menu->addAction(email_action);


}
//-----------------------------------------------------------------------------
//--- create_WebSocket()
//-----------------------------------------------------------------------------
void Main_RT::create_WebSocket()
{
    WS_Client = new QWebSocket("WS_Client from DTmaster", QWebSocketProtocol::VersionLatest, this);
    connect(WS_Client, SIGNAL(connected()), this, SLOT(WS_Connected()));
    connect(WS_Client, SIGNAL(disconnected()), this, SLOT(WS_Disconnected()));
    connect(WS_Client, SIGNAL(textMessageReceived(QString)), this, SLOT(WS_Received()));
}

//-----------------------------------------------------------------------------
//--- WS_Connected()
//-----------------------------------------------------------------------------
void Main_RT::WS_Connected()
{
    connect_WSServer->setText(tr("Disconnect from server..."));
}
//-----------------------------------------------------------------------------
//--- WS_Disconnected()
//-----------------------------------------------------------------------------
void Main_RT::WS_Disconnected()
{
    connect_WSServer->setText(tr("Connect to server..."));
}
//-----------------------------------------------------------------------------
//--- WS_Received()
//-----------------------------------------------------------------------------
void Main_RT::WS_Received()
{
    qDebug() << "WS received";
}
//-----------------------------------------------------------------------------
//--- slot_PreferenceExport()
//-----------------------------------------------------------------------------
void Main_RT::slot_PreferenceExport()
{
    //qDebug() << "slot_PreferenceExport() ";

    QString text = qApp->applicationDirPath() + "/forms";
    QString version = "9.0";

    HINSTANCE ext_dll_handle;
    ext_dll_handle = ::LoadLibraryW(L"DTReport2.dll");
    //qDebug() << "ext_dll_handle: " << ext_dll_handle;
    if(ext_dll_handle)
    {
        initialize = (Init)(::GetProcAddress(ext_dll_handle,"Initialize"));
        show_export_options = (ShowExportOptions)(::GetProcAddress(ext_dll_handle,"ShowExportOptions"));

        //qDebug() << "show_export_options: " << show_export_options;

        if(initialize) initialize(NULL, text.toUtf8().data(), version.toLatin1().data(), ID_LANG, NULL,NULL,NULL,NULL,NULL,NULL);
        if(show_export_options) show_export_options();

        //qDebug() << "Stop show_export_options ";

        ::FreeLibrary(ext_dll_handle);
    }
}

//-----------------------------------------------------------------------------
//--- slot_General()
//-----------------------------------------------------------------------------
void Main_RT::slot_General()
{
    if(general_menu) delete general_menu;

    general_menu = new General(this);
    general_menu->exec();

    delete general_menu;
    general_menu = NULL;
}
//-----------------------------------------------------------------------------
//--- slot_ReplaceUserAuthorization()
//-----------------------------------------------------------------------------
void Main_RT::slot_ReplaceUserAuthorization()
{
    QString text;    

    last_page = contentsWidget->currentRow();
    QString New_UserName = "NULL";
    QString UserName = "NULL";
    if(axUser) UserName = axUser->property("userName").toString();

    setWindowState(Qt::WindowMinimized);    
    axRita->dynamicCall("logOut()");    
    axUser = axRita->querySubObject("currentLogin");    

    Load_USER(axUser);
    Load_MENU();

    if(axUser) New_UserName = axUser->property("userName").toString();
    if(New_UserName != UserName && axUser)
    {
        QTemporaryFile file_TESTs;
        Load_USERTESTs(axUser, &file_TESTs);
        if(file_TESTs.exists()) text = file_TESTs.fileName();
        else text = "";
        Load_TESTs(text);
    }

    setWindowState(Qt::WindowNoState);

    if(!axUser)
    {
        nameUser_label->setText("???");
    }
}

//-----------------------------------------------------------------------------
//--- slot_UserAuthorization()
//-----------------------------------------------------------------------------
void Main_RT::slot_UserAuthorization()
{
    last_page = contentsWidget->currentRow();

    setWindowState(Qt::WindowMinimized);

    qDebug() << "axRita: " << axRita << axRita->control();
    if(!axRita->control().isNull())
    {
        //axRita->dynamicCall("execute(bool)", false);
        axRita->dynamicCall("setupUser()");
        axUser = axRita->querySubObject("currentLogin");
        Load_USER(axUser);
    }
    setWindowState(Qt::WindowNoState);



    //rita.axRita = axRita;
    //rita.start();
}
//-----------------------------------------------------------------------------
//--- slot_SetupToRun()
//-----------------------------------------------------------------------------
void Main_RT::slot_SetupToRun(QString fn)
{
    contentsWidget->setCurrentRow(1);   // To Run

    //qDebug() << "slot_SetupToRun()" << fn;


    QFileInfo f_info(fn);

    if(f_info.exists())
    {
        QEvent *e = new QEvent((QEvent::Type)3002);
        QApplication::sendEvent(p_run, e);
    }
    else
    {

    }
}

//-----------------------------------------------------------------------------
//--- slot_RunToAnalysis()
//-----------------------------------------------------------------------------
void Main_RT::slot_RunToAnalysis(QString fn)
{
    contentsWidget->setCurrentRow(2);   // To Analysis

    QFileInfo f_info(fn);

    if(f_info.exists())
    {
        QEvent *e = new QEvent((QEvent::Type)3004);
        QApplication::sendEvent(p_analysis, e);
    }
    else
    {

    }
}
//-----------------------------------------------------------------------------
//--- slot_AnalysisToSetup()
//-----------------------------------------------------------------------------
void Main_RT::slot_AnalysisToSetup(QString fn)
{
    contentsWidget->setCurrentRow(0);   // To Setup

    QFileInfo f_info(fn);

    if(f_info.exists())
    {
        QEvent *e = new QEvent((QEvent::Type)3006);
        QApplication::sendEvent(p_setup, e);
    }
    else
    {

    }
}

//-----------------------------------------------------------------------------
//--- slot_AnalysisToSetup()
//-----------------------------------------------------------------------------
void Main_RT::slot_SetupToAnalysis(QString fn)
{
    contentsWidget->setCurrentRow(2);   // To Analysis

    QFileInfo f_info(fn);

    if(f_info.exists())
    {
        QEvent *e = new QEvent((QEvent::Type)3008);
        QApplication::sendEvent(p_analysis, e);
    }
    else
    {

    }
}
//-----------------------------------------------------------------------------
//--- slot_Help()
//-----------------------------------------------------------------------------
void Main_RT::slot_Help()
{
    changeHelpPage();
    if(p_saver)
    {
        QEvent *e = new QEvent((QEvent::Type)3010);
        QApplication::sendEvent(p_saver, e);
    }
}

//-----------------------------------------------------------------------------
//--- slot_About()
//-----------------------------------------------------------------------------
void Main_RT::slot_About()
{
    //splash = new SplashScreen_RT(qApp, this, 1);
    if(!splash) splash = new SplashScreen(qApp, this, 1);
    splash->show();

    //splash->exec();

    //delete splash;
}
//-----------------------------------------------------------------------------
//--- slot_AboutModuls()
//-----------------------------------------------------------------------------
void Main_RT::slot_AboutModuls()
{
    if(aboutmoduls_menu) delete aboutmoduls_menu;

    aboutmoduls_menu = new AboutModuls(this);
    aboutmoduls_menu->exec();

    delete aboutmoduls_menu;
    aboutmoduls_menu = NULL;
}

//-----------------------------------------------------------------------------
//--- slot_QtScript()
//-----------------------------------------------------------------------------
void Main_RT::slot_QtScript()
{
    if(Web_Dialog->isHidden()) Web_Dialog->show();
}
//-----------------------------------------------------------------------------
//--- slot_ProtocolRequestToServer
//-----------------------------------------------------------------------------
void Main_RT::slot_ProtocolOrderToServer(QString message)
{
    QString fileName = "";
    QString dirName = qApp->applicationDirPath();
    QString selectedFilter;
    QString text;

    QVariant id, data, name;
    QVariant returned_Value = "";

    QDomDocument    doc;
    QDomElement     root;
    QString         reg_number;
    QFile file;

    if(message.isEmpty())
    {
        fileName = QFileDialog::getOpenFileName(this,
                                                tr("Open Protocol"),
                                                dirName,
                                                tr("Protocols File (*.rt *.xml)"),
                                                &selectedFilter);
        if(fileName.isEmpty()) return;
        file.setFileName(fileName);
        if(file.exists() && file.open(QIODevice::ReadOnly))
        {
            if(!doc.setContent(&file))
            {
                file.close();
                return;
            }

            file.close();

            //... find reg_number ...
            root = doc.documentElement();
            reg_number = root.firstChildElement("regNumber").text();
            doc.clear();

            if(reg_number.trimmed().isEmpty()) reg_number = GetRandomString();
            id = reg_number;
        }

    }
    else
    {
        QStringList list = message.split(";");
        id = list.at(0);
        if(list.size() > 1) fileName = list.at(1);
        if(fileName.isEmpty()) return;
    }



    file.setFileName(fileName);

    if(file.open(QIODevice::ReadOnly))
    {
        QTextStream in(&file);
        text = in.readAll();
        file.close();


        data = text;
        name = fileName;
        qDebug() << "send ProtocolOrder_To_Web: " << id << name << text.size();


        QObject *obj = (QObject*)(pweb_add->rootObject());
        QMetaObject::invokeMethod(obj, "send_ProtocolToWeb",
                                  Q_RETURN_ARG(QVariant, returned_Value),
                                  Q_ARG(QVariant, id),
                                  Q_ARG(QVariant, data),
                                  Q_ARG(QVariant, name));
    }
}

//-----------------------------------------------------------------------------
//--- slot_ProtocolToServer()
//-----------------------------------------------------------------------------
void Main_RT::slot_ProtocolToServer(QString message)
{
    QString fileName = "";
    QString dirName = qApp->applicationDirPath();
    QString selectedFilter;
    QString text;

    QVariant id, data, name;
    QVariant returned_Value = "";

    QDomDocument    doc;
    QDomElement     root;
    QString         reg_number;
    QFile file;

    if(message.isEmpty())
    {
        fileName = QFileDialog::getOpenFileName(this,
                                                tr("Open Protocol"),
                                                dirName,
                                                tr("Protocols File (*.rt *.xml)"),
                                                &selectedFilter);
        if(fileName.isEmpty()) return;
        file.setFileName(fileName);
        if(file.exists() && file.open(QIODevice::ReadOnly))
        {
            if(!doc.setContent(&file))
            {
                file.close();
                return;
            }

            file.close();

            //... find reg_number ...
            root = doc.documentElement();
            reg_number = root.firstChildElement("regNumber").text();
            doc.clear();

            if(reg_number.trimmed().isEmpty()) reg_number = GetRandomString();
            id = reg_number;
        }

    }
    else
    {
        QStringList list = message.split(";");
        id = list.at(0);
        if(list.size() > 1) fileName = list.at(1);
        if(fileName.isEmpty()) return;
    }



    file.setFileName(fileName);

    if(file.open(QIODevice::ReadOnly))
    {
        QTextStream in(&file);
        text = in.readAll();
        file.close();


        data = text;
        name = fileName;
        //qDebug() << "send_ProtocolToWeb: " << id << name;


        QObject *obj = (QObject*)(pweb->rootObject());
        QMetaObject::invokeMethod(obj, "send_ProtocolToWeb",
                                  Q_RETURN_ARG(QVariant, returned_Value),
                                  Q_ARG(QVariant, id),                                  
                                  Q_ARG(QVariant, data),
                                  Q_ARG(QVariant, name));
    }
}
//-----------------------------------------------------------------------------
//--- slot_ListWebProtocols()
//-----------------------------------------------------------------------------
void Main_RT::slot_ListWebProtocols()
{
    QVariant returned_Value;
    QVariant id_prot;
    web_Protocols = new List_WebProtocols(this);
    web_Protocols->type = rtapp;
    connect(web_Protocols, SIGNAL(reload_ListProtocols()), this, SLOT(slot_ReloadListWebProtocols()));
    web_Protocols->Load_Protocols(&list_WebProtocols);
    if(web_Protocols->exec())
    {
        id_prot = web_Protocols->Key_result;        
        QObject *obj = (QObject*)(pweb_add->rootObject());
        QMetaObject::invokeMethod(obj, "getWebProtocol",
                                  Q_RETURN_ARG(QVariant, returned_Value),
                                  Q_ARG(QVariant, id_prot));

        contentsWidget->setCurrentRow(2);   // to Analysis
        changePage(2);  // analysis page
    }

    delete web_Protocols;
    web_Protocols = NULL;
}
//-----------------------------------------------------------------------------
//--- slot_ReloadListWebProtocols()
//-----------------------------------------------------------------------------
void Main_RT::slot_ReloadListWebProtocols()
{
    QVariant returned_Value;
    QVariant id_prot;

    id_prot = "";
    QObject *obj = (QObject*)(pweb_add->rootObject());
    QMetaObject::invokeMethod(obj, "getWebProtocol",
                              Q_RETURN_ARG(QVariant, returned_Value),
                              Q_ARG(QVariant, id_prot));
}
//-----------------------------------------------------------------------------
//--- slot_ReloadListWebProtocolsOrder()
//-----------------------------------------------------------------------------
void Main_RT::slot_ReloadListWebProtocolsOrder()
{
    QVariant returned_Value;
    QVariant id_prot;

    id_prot = "";
    QObject *obj = (QObject*)(pweb->rootObject());
    QMetaObject::invokeMethod(obj, "getWebProtocol",
                              Q_RETURN_ARG(QVariant, returned_Value),
                              Q_ARG(QVariant, id_prot));
}
//-----------------------------------------------------------------------------
//--- slot_ListWebProtocolsOrder()
//-----------------------------------------------------------------------------
void Main_RT::slot_ListWebProtocolsOrder()
{
    QVariant returned_Value;
    QVariant id_prot;
    web_Protocols = new List_WebProtocols(this);
    web_Protocols->type = consumer;
    connect(web_Protocols, SIGNAL(reload_ListProtocols()), this, SLOT(slot_ReloadListWebProtocolsOrder()));
    web_Protocols->Load_Protocols(&list_WebProtocolsOrder); //&(Web->list_Protocols));
    if(web_Protocols->exec())
    {
        id_prot = web_Protocols->Key_result;
        qDebug() << "Key: " << web_Protocols->Key_result << id_prot;
        //emit Web->get_WebProtocol();
        QObject *obj = (QObject*)(pweb->rootObject());
        QMetaObject::invokeMethod(obj, "getWebProtocol",
                                  Q_RETURN_ARG(QVariant, returned_Value),
                                  Q_ARG(QVariant, id_prot));

        contentsWidget->setCurrentRow(0);   // to Setup
        changePage(0);  // setup page
    }

    delete web_Protocols;
    web_Protocols = NULL;
}
//-----------------------------------------------------------------------------
//--- slot_ProtocolFromWeb()
//-----------------------------------------------------------------------------
void Main_RT::slot_ProtocolFromWeb(QString name, QString barcode)
{
    QVariant id_prot = barcode;
    QVariant returned_Value;

    QObject *obj = (QObject*)(pweb->rootObject());
    QMetaObject::invokeMethod(obj, "getWebProtocol",
                              Q_RETURN_ARG(QVariant, returned_Value),
                              Q_ARG(QVariant, id_prot));
}
//-----------------------------------------------------------------------------
//--- slot_HttpProcess()
//-----------------------------------------------------------------------------
void Main_RT::slot_HttpProcess()
{
    /*if(http->isHidden())
    {
        http->show();
    }*/


    http = new HttpProcess(this);
    http->ax_user = axUser;

    http->exec();

    delete http;
}

//-----------------------------------------------------------------------------
//--- slot_ScenarioAction()
//-----------------------------------------------------------------------------
void Main_RT::slot_ScenarioAction()
{
    if(scenario->isHidden()) scenario->show();
}
//-----------------------------------------------------------------------------
//--- slot_EmailAction()
//-----------------------------------------------------------------------------
/*void Main_RT::slot_EmailAction()
{
    if(email->isHidden()) email->show();
}*/
//-----------------------------------------------------------------------------
//--- slot_ConnectToServer()
//-----------------------------------------------------------------------------
void Main_RT::slot_ConnectToServer()
{
    if(WS_Client)
    {
        //if(WS_Client->state() != QAbstractSocket::ConnectedState) WS_Client->open(QUrl("ws://localhost:9002"));
        //if(WS_Client->state() == QAbstractSocket::ConnectedState) WS_Client->close();
    }
}

//-----------------------------------------------------------------------------
//--- slot_SetupMenu(QAction *)
//-----------------------------------------------------------------------------
void Main_RT::slot_SetupMenu(QAction *action)
{
    Execution_Menu execution_menu = reinterpret_cast<Execution_Menu>(
                    ::GetProcAddress(dll_setup,"action_Menu@8"));
    if(execution_menu)
    {
        execution_menu(p_setup, action->whatsThis());
    }
}
//-----------------------------------------------------------------------------
//--- slot_RunMenu(QAction *)
//-----------------------------------------------------------------------------
void Main_RT::slot_RunMenu(QAction *action)
{
    Execution_Menu execution_menu = reinterpret_cast<Execution_Menu>(
                    ::GetProcAddress(dll_run,"action_Menu@8"));
    if(execution_menu)
    {
        execution_menu(p_run, action->whatsThis());
    }
}
//-----------------------------------------------------------------------------
//--- slot_AnalysisMenu(QAction *)
//-----------------------------------------------------------------------------
void Main_RT::slot_AnalysisMenu(QAction *action)
{
    Execution_Menu execution_menu = reinterpret_cast<Execution_Menu>(
                    ::GetProcAddress(dll_analysis,"action_Menu@8"));
    if(execution_menu)
    {
        execution_menu(p_analysis, action->whatsThis());
    }
}
//-----------------------------------------------------------------------------
//--- slot_SetupShow()
//-----------------------------------------------------------------------------
void Main_RT::slot_SetupShow()
{
    QMap<QString, int>  map_enable;
    QList<QAction*> list = setup_menu->actions();
    QList<QAction*> list_test = test_menu->actions();
    QAction *action;
    QString text;
    bool sts;

    //contentsWidget->setCurrentRow(0);   // to Setup

    Enable_Menu enable_menu = reinterpret_cast<Enable_Menu>(
                    ::GetProcAddress(dll_setup,"enable_Menu@8"));
    if(enable_menu)
    {
        enable_menu(p_setup, &map_enable);

        //qDebug() << "map_enable: " << map_enable;

        foreach(action, list)
        {
            text = action->whatsThis();
            //qDebug() << "slot_SetupShow(): " << text;

            if(map_enable.keys().contains(text))
            {
                sts = map_enable.value(text);
                action->setEnabled(sts);
            }
        }

        foreach(action, list_test)
        {
            text = action->whatsThis();
            //qDebug() << "slot_SetupShow(): " << text;

            if(map_enable.keys().contains(text))
            {
                sts = map_enable.value(text);
                action->setEnabled(sts);
            }
        }

    }

    map_enable.clear();
}
//-----------------------------------------------------------------------------
//--- slot_RunShow()
//-----------------------------------------------------------------------------
void Main_RT::slot_RunShow()
{
    QMap<QString, int>  map_enable;
    QList<QAction*> list = run_menu->actions();
    QAction *action;
    QString text;
    bool sts;

    //contentsWidget->setCurrentRow(1);   // to Run

    Enable_Menu enable_menu = reinterpret_cast<Enable_Menu>(
                    ::GetProcAddress(dll_run,"enable_Menu@8"));
    if(enable_menu)
    {
        enable_menu(p_run, &map_enable);

        foreach(action, list)
        {
            text = action->whatsThis();
            if(map_enable.keys().contains(text))
            {
                sts = map_enable.value(text);
                action->setEnabled(sts);
            }
        }
    }

    map_enable.clear();
}
//-----------------------------------------------------------------------------
//--- slot_AnalysisShow()
//-----------------------------------------------------------------------------
void Main_RT::slot_AnalysisShow()
{
    QMap<QString, int>  map_enable;
    QList<QAction*> list = analysis_menu->actions();
    QAction *action;
    QString text;
    bool sts;

    //contentsWidget->setCurrentRow(2);   // to Analysis

    Enable_Menu enable_menu = reinterpret_cast<Enable_Menu>(
                    ::GetProcAddress(dll_analysis,"enable_Menu@8"));
    if(enable_menu)
    {
        enable_menu(p_analysis, &map_enable);

        foreach(action, list)
        {
            text = action->whatsThis();
            if(map_enable.keys().contains(text))
            {
                sts = map_enable.value(text);
                action->setEnabled(sts);
            }
        }
    }

    map_enable.clear();
}

//-----------------------------------------------------------------------------
//--- Sleep
//-----------------------------------------------------------------------------
void Main_RT::Sleep(int ms)
{
    QEventLoop loop;
    QTimer::singleShot(ms, &loop, SLOT(quit()));
    loop.exec();
}

//-------------------------------------------------------------------------------
//--- User events
//-------------------------------------------------------------------------------
bool StackedWidget::event(QEvent *e)
{
    QString text = "";


    // 1. Setup -> Run
    if(e->type() == 3001)
    {
        text = property("pcrdocument").toString();
        //qDebug() << text;
        if(!text.isEmpty()) emit sJump_SetupToRun(text);
        return(true);
    }

    // 2. Run -> Analysis
    if(e->type() == 3003)
    {
        text = property("measurements_file").toString();
        //qDebug() << text;
        if(!text.isEmpty()) emit sJump_RunToAnalysis(text);
        return(true);
    }

    // 3. Analysis -> Setup
    if(e->type() == 3005)
    {
        text = property("analysis_to_setup").toString();
        //qDebug() << text;
        if(!text.isEmpty()) emit sJump_AnalysisToSetup(text);
        return(true);
    }

    // 4. Setup -> Analysis
    if(e->type() == 3007)
    {
        text = property("setup_to_analysis").toString();
        //qDebug() << text;
        if(!text.isEmpty()) emit sJump_SetupToAnalysis(text);
        return(true);
    }

    // 5. Run,Analysis -> Web
    if(e->type() == 3013)
    {
        text = property("ProtocolInfo_ToWeb").toString();
        if(!text.isEmpty())
        {
            //qDebug() << text;
            emit sChange_StateProtocolToWeb(text);
        }
        return(true);
    }

    // 6. Analysis results -> Web
    if(e->type() == 3014)
    {
        text = property("ProtocolResult_ToWeb").toString();
        if(!text.isEmpty())
        {
            qDebug() << "ProtocolResult_ToWeb: " << text;
            emit sSend_ResultProtocolToWeb(text);
        }
        return(true);
    }

    // 7. BarCode event from Run
    if(e->type() == 3016)
    {
        emit sSendRunButton();
        return(true);
    }

    // 8. protocol_order from setup to webserver
    if(e->type() == 3020)
    {
        text = property("order_to_web").toString();
        if(!text.isEmpty())
        {
            //qDebug() << "order_to_web: " << text;
            emit sSend_OrderProtocolToWeb(text);
        }
        return(true);
    }

    // 9. Change WinTitle:
    if(e->type() == 3009)
    {
        text = main_Window->property("Device_State").toString().trimmed();
        //qDebug() << "Device_State: " << text;
        if(text.isEmpty())
        {
            text = property("change_WinTitle").toString();
            //qDebug() << "MainRT->change_WinTitle: " << text;
            //if(!text.isEmpty())
            //{
                text = QString("%1 %2").arg(APP_NAME).arg(text).trimmed();
                main_Window->setWindowTitle(text);
            //}
        }

        return(true);
    }

    return QWidget::event(e);
}
//-------------------------------------------------------------------------------
//--- Web_Interface()
//-------------------------------------------------------------------------------
Web_Interface::Web_Interface(QWidget *parent)
    :QObject(parent)
{    
    connection_Server = false;

    connect(this, SIGNAL(connection_Changed(bool)), this, SLOT(change_connection(bool)));
}
//-------------------------------------------------------------------------------
//--- ~Web_Interface()
//-------------------------------------------------------------------------------
Web_Interface::~Web_Interface()
{

}
//-------------------------------------------------------------------------------
//--- get_connection()
//-------------------------------------------------------------------------------
bool Web_Interface::get_connection() const
{
    //qDebug() << "get_connection: " << connection_Server;
    return(connection_Server);
}
//-------------------------------------------------------------------------------
//--- set_connection(const bool&)
//-------------------------------------------------------------------------------
void Web_Interface::set_connection(const bool& state)
{
    //qDebug() << "set_connection: " << state;
    connection_Server = state;

    emit connection_Changed(state);
}
//-------------------------------------------------------------------------------
//--- set_url(const QString&)
//-------------------------------------------------------------------------------
void Web_Interface::set_url(const QString& url)
{
    url_Server = url;
}

//-------------------------------------------------------------------------------
//--- set_connection(const bool&)
//-------------------------------------------------------------------------------
void Web_Interface::change_connection(bool state)
{
    //qDebug() << "change connection: " << state;
    if(!state)
    {
        get_ListProtocols("","");
    }
}
//-------------------------------------------------------------------------------
//--- get_ListProtocols(QString,QString)
//-------------------------------------------------------------------------------
void Web_Interface::get_ListProtocols(QString str, QString separator)
{
    emit get_List(str, separator);
}
//-------------------------------------------------------------------------------
//--- confirmationNotice(QString,QString)
//-------------------------------------------------------------------------------
void Web_Interface::confirmationNotice(QString type, QString str)
{
    emit confirmation_FromServer(type, str);
}
//-------------------------------------------------------------------------------
//--- get_Message(QString message)
//-------------------------------------------------------------------------------
void Web_Interface::get_Message(QString message)
{    
    emit message_FromServer(message);

}
//-------------------------------------------------------------------------------
//--- exec_Command(QString message)
//-------------------------------------------------------------------------------
void Web_Interface::exec_Command(QString message)
{
    QDomDocument doc;
    QDomDocument doc_temp;
    QDomElement  root;
    QDomElement  request;
    QDomElement  element;
    QDomElement  item;
    QDomElement  child;
    bool busy = false;
    bool stop_request = false;
    QMap<QString,QString> map;
    QByteArray buf_msg;
    bool state = false;

    QString fn, param;
    QString answer = "";

    param = "";

    /*QString text = QString("%1 ->   %2").arg(QDateTime::currentDateTime().toString("h:mm:ss ")).arg(message);
    mb.setText(text);
    mb.exec();*/

    //qDebug() << "Exec_Command: message " << message;
    //message = ";answer from DTmaster:  " + message;
    //emit sAnswerToServer(message);

    if(doc.setContent(message))
    {
        request = doc.documentElement();

        //if(request.nodeName() != "request")

        element = request.firstChildElement("name");
        answer = element.text();
        map.insert("Name_Request", answer);

        //qDebug() << "Exec_Command: " << doc.toString();

        // RunProtocol
        if(!element.isNull() && element.text() == "RunProtocol")
        {
            fn = qApp->applicationDirPath() + "/scenario/Exec_Commands/Template_RunProtocol.sce";

            item = request.firstChildElement("id_protocol");
            if(!item.isNull())
            {
                param = QString("ID_Protocol=%1;").arg(item.text());
                map.insert("OpenProtocolID", param);
            }

            item = request.firstChildElement("id_device");
            if(!item.isNull())
            {
                param = QString("DeviceName=%1;").arg(item.text());
                map.insert("OpenDevice", param);
            }

            item = request.firstChildElement("file_template");
            if(!item.isNull())
            {

                fn = qApp->applicationDirPath() + "/scenario/" + item.text();
            }
        }

        // StopRun
        if(!element.isNull() && element.text() == "StopRun")
        {
            stop_request = true;
        }

        // OpenDevice
        if(!element.isNull() && element.text() == "OpenDevice")
        {
            fn = qApp->applicationDirPath() + "/scenario/Exec_Commands/Template_OpenDevice.sce";

            item = request.firstChildElement("id_device");
            if(!item.isNull())
            {
                param = QString("DeviceName=%1;").arg(item.text());
                map.insert("OpenDevice", param);
            }
        }

        // OpenBlock
        if(!element.isNull() && element.text() == "OpenBlock")
        {
            fn = qApp->applicationDirPath() + "/scenario/Exec_Commands/Template_OpenBlock.sce";

            item = request.firstChildElement("id_device");
            if(!item.isNull())
            {
                param = QString("DeviceName=%1;").arg(item.text());
                map.insert("OpenBlock", param);
            }
        }

        // CloseBlock
        if(!element.isNull() && element.text() == "CloseBlock")
        {
            fn = qApp->applicationDirPath() + "/scenario/Exec_Commands/Template_CloseBlock.sce";

            item = request.firstChildElement("id_device");
            if(!item.isNull())
            {
                param = QString("DeviceName=%1;").arg(item.text());
                map.insert("CloseBlock", param);
            }
        }

        // SaveResultsCallback
        if(!element.isNull() && element.text() == "SaveResultsCallback")
        {
            item = request.firstChildElement("msg");
            param = tr("Everything went wrong!");
            if(!item.isNull())
            {
                param = item.text();
                buf_msg = QByteArray::fromBase64(param.toStdString().c_str());
                param = QString(buf_msg);
                //qDebug() << "buf_msg: " << param;
                if(doc_temp.setContent(param))
                {
                    root = doc_temp.documentElement();
                    //qDebug() << "root name: " << root.nodeName();
                    if(root.nodeName() == "OK")
                    {
                        state = true;
                        param = tr("Operation completed successfully!");
                    }
                    else
                    {
                        if(root.nodeName() == "package")
                        {
                            child = root.firstChildElement("Error");
                            //qDebug() << "Error: " << child.isNull();
                            if(!child.isNull() && child.hasAttribute("Message"))
                            {
                                //qDebug() << "has attribute: " << child.hasAttribute("Message");
                                param = child.attribute("Message","");
                            }
                        }
                    }
                }
            }

            mb.setStandardButtons(QMessageBox::Ok);
            if(state) mb.setIcon(QMessageBox::Information);
            else mb.setIcon(QMessageBox::Warning);
            mb.setText(param);
            mb.exec();

            doc.clear();
            doc_temp.clear();
            return;
        }

        // Another request...



        if((!answer.isEmpty() && !fn.isEmpty()) || stop_request)
        {
            // 1. answer for request...
            doc.clear();
            QDomNode xmlNode = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\"");
            doc.insertBefore(xmlNode, doc.firstChild());
            QDomElement  root = doc.createElement("answer");
            doc.appendChild(root);
            root.setAttribute("time", QString("%1").arg(QDateTime::currentMSecsSinceEpoch()));
            root.appendChild(MakeElement(doc,"name", answer));

            if(scenario->active_scenario && !stop_request)
            {                
                root.appendChild(MakeElement(doc, "status", "1"));
                child = MakeElement(doc, "execute_status", "busy");
                child.setAttribute("code", "3");
                root.appendChild(child);
            }
            else
            {
                root.appendChild(MakeElement(doc, "status", "0"));
                child = MakeElement(doc, "execute_status", "accepted");
                child.setAttribute("code", "1");
                root.appendChild(child);
            }

            //qDebug() << "AnswerToServer: " << doc.toString();

            emit sAnswerToServer("...;" + doc.toString());

            if(stop_request)
            {
                scenario->web_break = true;
                scenario->slot_StopScenario();
            }
            else  emit sRun_Scenario(fn, map);

        }

        doc.clear();
    }
    else
    {
        qDebug() << "bad XML...";
    }

    map.clear();
}

//-------------------------------------------------------------------------------
//--- send_Message(QString message)
//-------------------------------------------------------------------------------
void Web_Interface::send_Message(QString message)
{
    emit message_ToServer(message);
}
//-------------------------------------------------------------------------------
//--- get_Protocol(QString str)
//-------------------------------------------------------------------------------
void Web_Interface::get_Protocol(QString str)
{

    emit get_protocol(str);
}
//-----------------------------------------------------------------------------
//--- WebProtocol_Get(QString str)
//-----------------------------------------------------------------------------
void Main_RT::WebProtocol_Get(QString str)
{
    QFile file;

    //qDebug() << "GET PROTOCOL ADD: " << str.length();

    if(str.length())
    {
        file.setFileName(qApp->applicationDirPath() +  "/qml/web.rt");

        if(file.open(QIODevice::WriteOnly))
        {
            QTextStream out(&file);

            out << str;
            file.close();

            emit Web_additional->send_ProtocolToAnalysis(file.fileName());
        }
    }
}
//-----------------------------------------------------------------------------
//--- WebProtocolOrder_Get(QString str)
//-----------------------------------------------------------------------------
void Main_RT::WebProtocolOrder_Get(QString str)
{
    QString text;
    QFile file;

    //qDebug() << "GET PROTOCOL: " << str.length();

    if(str.length())
    {
        text = str.mid(0,100);

        if(text.contains("<create_plate"))
        {
            file.setFileName(qApp->applicationDirPath() +  "/qml/web7.xml");
        }
        else
        {
            file.setFileName(qApp->applicationDirPath() +  "/qml/web.rt");
        }

        //qDebug() << "text: " << text;

        if(file.open(QIODevice::WriteOnly))
        {
            QTextStream out(&file);

            out << str;
            file.close();

            emit Web->send_ProtocolToSetup(file.fileName());
            if(scenario->active_scenario && scenario->current_point >= 0)
            {
                if(file.exists()) WebProperty_AddReplace(file.fileName());
                emit Web->send_ProtocolToScenario(file.fileName());
            }
        }
    }
}
//-----------------------------------------------------------------------------
//--- WebConfirmationFromSetup(QString type, QString str)
//-----------------------------------------------------------------------------
void Main_RT::WebConfirmationFromSetup(QString type, QString str)
{
    qDebug() << "type, confirmation:"  << type << str;

    QDomDocument doc;
    QDomElement  root;
    QString text,param;
    bool state = false;

    if(doc.setContent(str))
    {
        root = doc.documentElement();
        if(root.hasAttribute("status") && root.nodeName() == "notice")
        {
            text = root.attribute("status","");
            param = root.text();
            qDebug() << "attribute: " << text << param;

            if(text.contains("accepted")) state = true;
        }
        else
        {
            doc.clear();
            return;
        }
        doc.clear();
    }
    else return;


    msgBox.setStandardButtons(QMessageBox::Ok);
    //msgBox.button(QMessageBox::Ok)->animateClick(15000);
    if(state)
    {
        msgBox.setIcon(QMessageBox::Information);
        text = tr("Protocol was successfully saved!");
    }
    else
    {
        msgBox.setIcon(QMessageBox::Warning);
        text = tr("Attention! Problems detected...") + "\r\n(" + param + ")";
    }

    msgBox.setText(text);
    msgBox.exec();

}

//-----------------------------------------------------------------------------
//--- WebProperty_AddReplace
//-----------------------------------------------------------------------------
void Main_RT::WebProperty_AddReplace(QString fn)
{
    int i;
    QDomDocument doc;
    QDomElement  root;
    QDomElement  item;
    QDomElement  child;
    QDomElement  child_name;
    QDomNode     node;
    QString text;
    int count;

    QFile file(fn);
    if(file.exists() && file.open(QIODevice::ReadOnly))
    {
        if(doc.setContent(&file))
        {
            file.close();

            root = doc.documentElement();

            // find ID protocol:
            item = root.firstChildElement("regNumber");
            text = item.text();

            // find Web_Protocol property
            item = root.firstChildElement("properties");
            count = item.childNodes().size();
            for(i=0; i<count; i++)
            {
                node = item.childNodes().at(i);
                child_name = node.toElement().firstChildElement("name");
                if(!child_name.isNull() && child_name.text() == "Web_Protocol")
                {
                    item.removeChild(node);
                    break;
                }
            }
            child = doc.createElement("item");
            child.appendChild(MakeElement(doc,"name", "Web_Protocol"));
            child.appendChild(MakeElement(doc,"value", text));

            item.appendChild(child);

            if(file.open(QIODevice::WriteOnly))
            {
                QTextStream(&file) << doc.toString();
                file.close();
            }
        }
        else file.close();
    }
}

//-----------------------------------------------------------------------------
//--- WebList_add_Get
//-----------------------------------------------------------------------------
void Main_RT::WebList_add_Get(QString str, QString separator)
{
    int i;
    QString text;
    QStringList list_param;
    QStringList list = str.split(separator);
    if(str.isEmpty()) list.clear();

    list_WebProtocols.clear();
    for(i=0; i<list.size(); i++)
    {
        text = list.at(i);
        list_param = text.split(";");
        if(list_param.size() >= 3)
        {
            text = QString("%1;%2").arg(list_param.at(2)).arg(list_param.at(1));
            list_WebProtocols.insert(list_param.at(0), text);
        }
    }
    emit Web_additional->listProtocols_Changed();
    if(list_WebProtocols.size() && web_Protocols)
    {
        if(web_Protocols->isVisible() && web_Protocols->type == rtapp)
        {
            Sleep(300);
            web_Protocols->Load_Protocols(&list_WebProtocols);
        }
    }
    //qDebug() << "ADD: " << str.length();
}
//-----------------------------------------------------------------------------
//--- WebList_add_Get
//-----------------------------------------------------------------------------
void Main_RT::WebList_Get(QString str, QString separator)
{
    int i;
    QString text;
    QStringList list_param;
    QStringList list = str.split(separator);
    if(str.isEmpty()) list.clear();

    list_WebProtocolsOrder.clear();         //Web->list_Protocols.clear();
    for(i=0; i<list.size(); i++)
    {
        text = list.at(i);
        list_param = text.split(";");
        if(list_param.size() >= 3)
        {
            text = QString("%1;%2").arg(list_param.at(2)).arg(list_param.at(1));
            list_WebProtocolsOrder.insert(list_param.at(0), text);
            //Web->list_Protocols.insert(list_param.at(0), text);
        }
    }
    emit Web->listProtocols_Changed();
    if(list_WebProtocolsOrder.size() && web_Protocols)
    {
        if(web_Protocols->isVisible() && web_Protocols->type == consumer)
        {
            Sleep(300);
            web_Protocols->Load_Protocols(&list_WebProtocolsOrder);
        }
    }
    //qDebug() << "MAIN: " << str.length();
}
//-----------------------------------------------------------------------------
//--- WebConnection_add_Changed(bool)
//-----------------------------------------------------------------------------
void Main_RT::WebConnection_add_Changed(bool state)
{
    //qDebug() << "Web_add state: " << state;

    protocolOrder_to_Server->setEnabled(state);
    list_Protocols->setEnabled(state);    
}
//-----------------------------------------------------------------------------
//--- WebConnection_Changed(bool)
//-----------------------------------------------------------------------------
void Main_RT::WebConnection_Changed(bool state)
{
    //qDebug() << "Web->url_Server: " << Web->url_Server << state;

    if(state)
    {
        Web_Dialog->connect_button->setText(tr("Disconnect..."));
        Web_Dialog->edit_url->setText(Web->url_Server);
        obj_gif->start();
        connect_toServer->setMovie(obj_gif);
        server_label->setText(Web_Dialog->edit_url->text());


    }
    else
    {
        obj_gif->stop();
        Web_Dialog->connect_button->setText(tr("Connect..."));
    }

    connect_toServer->setVisible(state);
    client_label->setVisible(state);
    server_label->setVisible(state);

    protocol_to_Server->setEnabled(state);
    list_ProtocolsOrder->setEnabled(state);
    //protocolRequest_to_Server->setEnabled(state);
}
//-----------------------------------------------------------------------------
//--- WebListProtocolsOrder_Changed()
//-----------------------------------------------------------------------------
void Main_RT::WebListProtocolsOrder_Changed()
{
    //bool sts = list_WebProtocolsOrder.size();
    //list_ProtocolsOrder->setEnabled(sts);
}
//-----------------------------------------------------------------------------
//--- WebListProtocols_Changed()
//-----------------------------------------------------------------------------
void Main_RT::WebListProtocols_Changed()
{
    //bool sts = list_WebProtocols.size();
    //list_Protocols->setEnabled(sts);
}
//-----------------------------------------------------------------------------
//--- WebProtocolToSetup(QString fn)
//-----------------------------------------------------------------------------
void Main_RT::WebProtocolToSetup(QString fn)
{
    pagesWidget->setProperty("Web_to_setup", QVariant(fn));
    QEvent *e = new QEvent((QEvent::Type)3012);
    QApplication::sendEvent(p_setup, e);
}
//-----------------------------------------------------------------------------
//--- WebProtocolToAnalysis(QString fn)
//-----------------------------------------------------------------------------
void Main_RT::WebProtocolToAnalysis(QString fn)
{
    pagesWidget->setProperty("Web_to_analysis", QVariant(fn));
    QEvent *e = new QEvent((QEvent::Type)3011);
    QApplication::sendEvent(p_analysis, e);
}
//-----------------------------------------------------------------------------
//--- WebConnect()
//-----------------------------------------------------------------------------
void Main_RT::WebConnect()
{
    QVariant url = Web_Dialog->edit_url->text();
    bool state_connection = !Web->connection_Server;
    //qDebug() << "want state_connection: " << state_connection;
    QVariant returned_Value = NULL;
    QVariant state = state_connection;
    QObject *obj = (QObject*)(pweb->rootObject());

    //if(state_connection)
    //{
    QMetaObject::invokeMethod(obj, "set_SocketUrl",
                              Q_RETURN_ARG(QVariant, returned_Value),
                              Q_ARG(QVariant, url));
    //}

    //if(state_connection) Sleep(500);

    QMetaObject::invokeMethod(obj, "set_StateSocket",
                              Q_RETURN_ARG(QVariant, returned_Value),
                              Q_ARG(QVariant, state));
}
//-----------------------------------------------------------------------------
//--- ChangeStateProtocol_ToWeb(QString message)
//-----------------------------------------------------------------------------
void Main_RT::ChangeStateProtocol_ToWeb(QString message)
{
    if(message.isEmpty()) return;

    //qDebug() << "toWEB: " << message;

    QStringList list = message.split(";");
    QVariant id = list.at(0);
    QVariant returned_Value = NULL;

    list.removeFirst();
    QVariant state = list.join(";");

    //qDebug() << "To server --> " <<  state.toString();

    QObject *obj = (QObject*)(pweb->rootObject());
    QMetaObject::invokeMethod(obj, "send_StateProtocolToWeb",
                              Q_RETURN_ARG(QVariant, returned_Value),
                              Q_ARG(QVariant, id),
                              Q_ARG(QVariant, state));
}
//-----------------------------------------------------------------------------
//--- As_DisabledUser()
//-----------------------------------------------------------------------------
void Main_RT::As_DisabledUser()
{
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.button(QMessageBox::Ok)->animateClick(10000);
    msgBox.setIcon(QMessageBox::Warning);
    //QString text = tr("Attention! You are logged in as a disabled user...");
    QString text = tr("Attention! You cannot continue working as an unregistered user!");
    text += "\r\n" + tr("But You can continue to work as a Guest when You log in again...");
    msgBox.setText(text);
    msgBox.raise();
    msgBox.activateWindow();
    msgBox.exec();    
}
//-----------------------------------------------------------------------------
//--- As_DisabledUser()
//-----------------------------------------------------------------------------
void Main_RT::Check_Compatibility_WIN7()
{
    int i;
    bool ok;
    int ver;
    QSysInfo system_info;
    auto current = QOperatingSystemVersion::current();
    //QOperatingSystemVersion
    bool res = false;
    bool sys = false;

    QString text = APP_NAME;    //"DTmaster.exe";
    //qDebug() << "application name: " << text;

    QString str;

    QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags\\Layers", QSettings::NativeFormat);
    QStringList list_reg = settings.allKeys();
    foreach(str, list_reg)
    {
        //qDebug() << "key: " << str << settings.value(str).toString();
        /*if(str.contains(text))
        {
            res = true;
            break;
        }*/
    }

    //qDebug() << "applicationFilePath(): " << qApp->applicationFilePath();


    if(current >= QOperatingSystemVersion(QOperatingSystemVersion::Windows, 7))
    {
        sys = true;
    }

    //qDebug() << "sys ver: " << res << sys << current.majorVersion() << current.name() << current.minorVersion();
    //qDebug() << "sys ver: " << system_info.windowsVersion() << system_info.productVersion();

    str = system_info.productVersion();
    if(!str.isEmpty())
    {
        ver = str.toInt(&ok);
        if(!ok) ver = 0;
        if(ver > 7) sys = true;
    }


    HKEY hRootKey = HKEY_CURRENT_USER;
    HKEY hKey;
    const BYTE *lpData = reinterpret_cast<const BYTE *>("WIN7RTM");
    QString app_path = qApp->applicationFilePath();
    app_path = app_path.replace("/","\\");

    if(sys)
    {
        if(RegCreateKeyA(hRootKey,
                        "Software\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags\\Layers",
                        &hKey) == ERROR_SUCCESS)
        {
            RegSetValueExA(hKey,
                          app_path.toStdString().c_str(),
                          0,
                          REG_SZ,
                          lpData,
                          strlen("WIN7RTM"));

            RegCloseKey(hKey);
        }
    }

    if(sys)
    {

        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.button(QMessageBox::Ok)->animateClick(10000);
        msgBox.setIcon(QMessageBox::Warning);
        QString text = tr("Attention! For more stable operation of the application, You need to set the compatibility mode with Windows 7");
        msgBox.setText(text);
        msgBox.raise();
        msgBox.activateWindow();
        msgBox.exec();

        QTimer::singleShot(1000, this, SLOT(close()));
    }

    return;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Main_RT::Send_p_SplashScreenObject(SplashScreen *sp)
{
    // ... Setup ...
    if(p_setup && dll_setup)
    {
        Send_PointerSplash send_splash = reinterpret_cast<Send_PointerSplash>(
                                        ::GetProcAddress(dll_setup,"Splash_Object@8"));
        if(send_splash)
        {
            send_splash(p_setup, sp);
        }
    }
}
