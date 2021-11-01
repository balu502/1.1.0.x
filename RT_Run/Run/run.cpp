#include "run.h"

#include <stdio.h>
//extern "C"
//{
//#include <openssl/applink.c>
//}
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/bn.h>
#include <openssl/bio.h>
#include <openssl/x509.h>
#include <openssl/engine.h>
#include <openssl/des.h>


extern "C" RUNSHARED_EXPORT Run* __stdcall create_page()
{    
    return(new Run());
}
extern "C" RUNSHARED_EXPORT HWND __stdcall create_Run()
{
    int argc = 0;
    char **argv = 0;
    //QApplication a(argc, argv);
    if(QApplication::instance() == 0)
    {
        new QApplication(argc, argv);
    }

    Run *run = new Run();
    return((HWND)run->winId());
}
extern "C" RUNSHARED_EXPORT bool __stdcall is_exit(void *p)
{
    return(((Run*)p)->Is_Exit());
}
extern "C" RUNSHARED_EXPORT void __stdcall load_3Dbars(void *p_run, void *p_bars)
{
    ((Run*)p_run)->load_3D(p_bars);
}
extern "C" RUNSHARED_EXPORT void __stdcall load_User(void *p_run, QAxObject* user)
{
    ((Run*)p_run)->create_User(user);
}
extern "C" RUNSHARED_EXPORT void __stdcall load_TestVerification(void *p_run, QAxObject* axgp_resource)
{
    ((Run*)p_run)->create_TestVerification(axgp_resource);
}
extern "C" RUNSHARED_EXPORT void __stdcall load_Menu(void *p_run, void *list_pinfo)
{
    ((Run*)p_run)->Create_MenuActions(list_pinfo);
}
extern "C" RUNSHARED_EXPORT void __stdcall action_Menu(void *p_run, QString type_action)
{
    ((Run*)p_run)->execution_MenuAction(type_action);
}
extern "C" RUNSHARED_EXPORT void __stdcall enable_Menu(void *p_run, void *enable_action)
{
    ((Run*)p_run)->Enable_MenuAction(enable_action);
}
extern "C" RUNSHARED_EXPORT void __stdcall app_Status(void *p_run, QString app_name, bool calibration_status)
{
    ((Run*)p_run)->Application_status(app_name, calibration_status);
}
extern "C" RUNSHARED_EXPORT void __stdcall Send_pPro(void *p_run, void *p_prot)
{
    ((Run*)p_run)->Send_pProtocol(p_prot);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Run::Run(QWidget *parent): QMainWindow(parent)
{
    readCommonSettings();
    setFont(qApp->font());

    if(StyleApp == "fusion")
    {
        setStyleSheet(
                    "QToolButton::hover {background: white;}"
                    "QToolButton::disabled {background-color: #ccc;}"
                    "QToolButton::checked {background-color: #FFFFFF; border: 0px solid black}"
                    "QToolButton#Transparent {border: 0px solid gray;}"
                    );
    }

    Dev_State = sUnknown;
    flag_SavingOpticDev= false;
    flag_SavingResult = false;
    flag_Compatibility = false;
    flag_RunLoaded = false;
    flag_LastRun = false;
    flag_FACS = false;
    flag_ClosePrerunRun = false;
    flag_ActivePoint = false;
    //flag_FirstConnectDevice = true;
    flag_WebProtocol = false;
    flag_LimitedDev = false;

    if(qApp->applicationName().contains("DTcheck")) message.setWindowIcon(QIcon(":/images/check.ico"));
    else
    {
        message.setWindowIcon(QIcon(":/images/DTm.ico"));
        //message.setWindowIcon(QIcon(":/images/RT.ico"));
    }

    message.setFont(qApp->font());

    //dll_Crypto = NULL;
    dll_3D = NULL;
    ax_user = NULL;    
    axgp = NULL;
    prot_FromSetup = NULL;

    Catalogue_Requests << CATALOGUE;
    create_TimerRequest();

    prot = Create_Protocol();
    action_event = NULL;

    ver_OPTICS = 0.;

    MainGroupBox = new QGroupBox();
    MainGroupBox->setObjectName("Transparent");
    setCentralWidget(MainGroupBox);
    QVBoxLayout *main_layout = new QVBoxLayout;
    main_layout->setMargin(0);
    main_layout->setSpacing(4);
    MainGroupBox->setLayout(main_layout);

    main_spl = new QSplitter(Qt::Horizontal);
    main_spl->setHandleWidth(6);
    main_spl->setChildrenCollapsible(false);

    connect(main_spl, SIGNAL(splitterMoved(int,int)), this, SLOT(resize_splitter(int,int)));

    main_layout->addWidget(main_spl);

    createActions();
    create_ChartControlBox();
    createStatusBar();

    //createFluorToolBars();
    createCONTROL();
    createINFO();
    createSelectTubes();
    slot_Select();
    Tab_Protocol->setCurrentIndex(1);

    //... Apply Settings ...
    QString dir_path = qApp->applicationDirPath();
    ApplSettings = new QSettings(dir_path + "/tools/ini/preference_run.ini", QSettings::IniFormat);
    readSettings();

    //...
    list_dev = new ListDev(this,ip_addres,port_serverUSB);
    wait_obj = new Wait_Process(this);
    Video_Image = new Video(this);
    Expo_check = new Expo(this);
    Contract_Dev = new Contract(this);
    Contract_Dev->map_CRYPTO = &map_CRYPTO;
    Contract_Dev->user_Dir = &user_Dir;
    //dll_3D = ::LoadLibraryW(L"plugins\\bars_3D.dll");


#ifdef QT_DEBUG
    dll_3D = NULL;
#else
    dll_3D = ::LoadLibraryW(L"plugins\\bars_3D.dll");
    //if(dll_3D) Video_Image->Load_lib(dll_3D);
#endif


    //... Crypto ...
    if(Read_PubKey())
    {
        qDebug() << "PubKey: " << PubKey_BUF;
    }
    else
    {

    }
    //...


    //... NetWork ...
    m_nNextBlockSize = 0;
    m_pTcpSocket = new QTcpSocket(this);

    connect(m_pTcpSocket, SIGNAL(connected()), SLOT(slotConnected()));
    connect(m_pTcpSocket, SIGNAL(disconnected()), SLOT(slotDisconnected()));
    connect(m_pTcpSocket, SIGNAL(readyRead()), SLOT(slotReadyRead()));
    connect(m_pTcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
                this,     SLOT(slotError(QAbstractSocket::SocketError)));



    connect(&dev_timer, SIGNAL(timeout()), this, SLOT(slotSendRequestInfo()));

    connect(&dev_Thread, SIGNAL(Send_InfoData(int)), this, SLOT(slotReadInfoData(int)));


    connect(this, SIGNAL(sRefresh(QMap<QString,QString>*)),
            plot_temperature, SLOT(slotRefreshData(QMap<QString,QString>*)));
    connect(this, SIGNAL(sRefresh(QMap<QString,QString>*)),
            this, SLOT(slot_RefreshMeasure(QMap<QString,QString>*)));

    connect(this, SIGNAL(sNewProtocol(rt_Protocol*)),
            plot_fluor, SLOT(slot_NewProtocol(rt_Protocol*)));
    connect(this, SIGNAL(sNewProtocol(rt_Protocol*)),
            this, SLOT(slot_Select()));

    connect(this, SIGNAL(sRefreshFluor()),
            plot_fluor, SLOT(slot_RefreshFluor()));

    connect(this, SIGNAL(sErrorInfo(QString)), this, SLOT(slot_GetErrorInfo(QString)));

    connect(this, SIGNAL(sChangeStateDevice(int)), this, SLOT(slot_ChangeStateDevice(int)));
    connect(this, SIGNAL(sChangeStateDevice(int)), plot_temperature, SLOT(slot_ChangeRejime(int)));
    connect(this, SIGNAL(sSet_PauseRejime(int)), plot_temperature, SLOT(slot_ChangeRejime(int)));

    connect(Video_Image, SIGNAL(MakeMeasure_Request(int,int,int)), this, SLOT(slot_GetPicture(int,int,int)));
    connect(this, SIGNAL(sSendPicture(QMap<QString,QByteArray>*)), Video_Image, SLOT(Get_Picture(QMap<QString,QByteArray>*)));
    connect(Expo_check, SIGNAL(MakeMeasure_Request(int,int,int)), this, SLOT(slot_GetPicture(int,int,int)));
    connect(this, SIGNAL(sSendDigit(QMap<QString,QByteArray>*)), Expo_check, SLOT(Get_Digitization(QMap<QString,QByteArray>*)));
    connect(Video_Image, SIGNAL(SavePar(int,QString)), this, SLOT(slot_SavePar(int,QString)));
    connect(Expo_check, SIGNAL(SavePar(int,QString)), this, SLOT(slot_SavePar(int,QString)));
    connect(Expo_check, SIGNAL(SaveParDefault(QString,QString)), this, SLOT(slot_SaveParDefault(QString,QString)));
    connect(Video_Image, SIGNAL(SaveParMedian(QString,QString)), this, SLOT(slot_SaveParMedian(QString,QString)));
    connect(Video_Image, SIGNAL(WriteSector(int,QString)), this, SLOT(slot_WriteSector(int,QString)));
    connect(Video_Image, SIGNAL(ReadSector(int,QString)), this, SLOT(slot_ReadSector(int,QString)));

    connect(Contract_Dev, SIGNAL(sSend_CRYPTO()), this, SLOT(send_CRYPTO()));
    connect(this, SIGNAL(sRead_CRYPTO(QMap<QString,QByteArray>*)), Contract_Dev, SLOT(Read_CRYPTO(QMap<QString,QByteArray>*)));

    timer_temp.setInterval(500);
    connect(&timer_temp, SIGNAL(timeout()), this, SLOT(slot_TimerTemp()));

    Web_timer.setInterval(10000);
    connect(&Web_timer, SIGNAL(timeout()), this, SLOT(update_ProtocolInfoToWeb()));

    connect(this, SIGNAL(sActionPoint(Action_Event*)), this, SLOT(slot_ReceivedActionPoint(Action_Event*)));
    connect(this, SIGNAL(sRunBarcode()), this, SLOT(slot_RecievedRunButton()));
    connect(this, SIGNAL(sMeasureTubeHeiht(int)), this, SLOT(slot_MeasuredTubeHeiht(int)));
    connect(this, SIGNAL(sStatusMeasureTubeHeiht(int)), this, SLOT(slot_SaveMeasuredTubeHeiht(int)));


}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Run::~Run()
{    
    QBasicTimer *timer;

    List_Requests.clear();
    foreach(timer, Map_TimerRequest)
    {
        timer->stop();
        delete timer;
    }
    Map_TimerRequest.clear();

    if(dev_Thread.isRunning()) dev_Thread.exit();

    list_dev->slotRefresh();
    if(m_pTcpSocket->state() == QAbstractSocket::ConnectedState)
    {
        m_pTcpSocket->disconnect();
        m_pTcpSocket->close();
        //qDebug() << "close";
    }

    writeSettings();

    Error_Info->clear();
    Map_TestTranslate.clear();

    delete Delegate_InfoDevice;

    delete runtime_Program;
    delete comments;

    delete plot_fluor;
    delete plot_temperature;    
    delete info_device;
    //delete group_online;
    delete ApplSettings;

    delete list_dev;
    delete wait_obj;    

    delete Video_Image;
    delete Expo_check;
    if(dll_3D) ::FreeLibrary(dll_3D);
    delete Contract_Dev;

    delete NameProt;
    delete NameDev;
    delete RunDev;
    delete StopDev;
    delete PauseDev;
    delete Tab_Control;

    delete sel;
    delete selPlate;
    delete Fluor_Chart;
    delete Temperature_Chart;
    delete Info_window;
    delete Control_window;
    delete ChartWindow;
    delete ControlWindow;

    delete main_progress;

    prot->Clear_Protocol();
    delete prot;

}
//-----------------------------------------------------------------------------
//--- create_TimerRequest()
//-----------------------------------------------------------------------------
void Run::create_TimerRequest()
{
    int i;
    int index;
    QString request;

    for(i=0; i<Catalogue_Requests.size(); i++)
    {
        request = Catalogue_Requests.at(i);
        QBasicTimer *timer = new QBasicTimer();
        Map_TimerRequest.insert(request, timer);
    }


}

//-----------------------------------------------------------------------------
//--- readCommonSettings()
//-----------------------------------------------------------------------------
void Run::readCommonSettings()
{
    QString text;
    int id;
    bool ok;
    QString dir_SysTranslate;
    QString dir_path = qApp->applicationDirPath();
    QSettings *CommonSettings = new QSettings(dir_path + "/tools/ini/preference.ini", QSettings::IniFormat);

    CommonSettings->beginGroup("Common");

        //... Language ...
        ID_LANG = ID_LANG_RU;
        text = CommonSettings->value("language","ru").toString();
        if(translator.load(":/translations/run_" + text + ".qm"))
        {
            qApp->installTranslator(&translator);
            if(text == "en") ID_LANG = ID_LANG_EN;
        }        

        // ... System Language ...
        dir_SysTranslate = dir_path + "/tools/translations";
        if(qt_translator.load(":/translations/qt_" + text + ".qm", dir_SysTranslate))
        {
            qApp->installTranslator(&qt_translator);
        }

        //... Style ...
        text = CommonSettings->value("style","xp").toString();
        StyleApp = text;

        //... Type ...
        text = CommonSettings->value("type","0").toString();
        id = text.toInt(&ok);
        if(!ok) id = 0;
        id_Type = (ID_TypeApp)id;

    CommonSettings->endGroup();
    delete CommonSettings;
}

//-----------------------------------------------------------------------------
//--- Actions
//-----------------------------------------------------------------------------
void Run::createActions()
{
    open_prot = new QAction(QIcon(":/images/open.png"), tr("Open protocol"), this);
    connect(open_prot,SIGNAL(triggered()), this,SLOT(open_protocol()));

    new_prot = new QAction(QIcon(":/images/new.png"), tr("New protocol"), this);
    connect(new_prot,SIGNAL(triggered()), this,SLOT(new_protocol()));

    save_prot = new QAction(QIcon(":/images/save.png"), tr("Save protocol as"), this);
    connect(save_prot,SIGNAL(triggered()), this,SLOT(save_protocol()));

    //---

}
//-----------------------------------------------------------------------------
//--- change_SaveImageRejime()
//-----------------------------------------------------------------------------
void Run::change_SaveImageRejime()
{
    int id = 1; // uC_Server

    int res = 0;
    if(SaveImage_rejime->isChecked()) res = 1;
    QString cmd = QString("DPIC %1").arg(res);

    /*if(res) SaveImage_rejime_IsChecked = true;

    SaveImage_rejime_Active = false;
    QTimer::singleShot(5000, this, SLOT(update_SaveImageActive()));
    */

    map_CMD.clear();
    map_CMD.insert(EXECCMD_CMD, cmd);
    map_CMD.insert(EXECCMD_UC, QString::number(id));

    slotSendToServer(EXECCMD_REQUEST);
}
//-----------------------------------------------------------------------------
//--- update_SaveImageActive()
//-----------------------------------------------------------------------------
/*void Run::update_SaveImageActive()
{
    //SaveImage_rejime_Active = true;
}*/

//-----------------------------------------------------------------------------
//--- measure_TubesHeight()
//-----------------------------------------------------------------------------
void Run::measure_TubesHeight()
{
    Preference_Box->setDisabled(true);
    slotSendToServer(HTMEAS_REQUEST);
    wait_process(wOpen, 100, tr("Measuring of the tube height..."));
}
//-----------------------------------------------------------------------------
//--- slot_MeasuredTubeHeiht(int)
//-----------------------------------------------------------------------------
void Run::slot_MeasuredTubeHeiht(int value)
{
    QString text;
    Tube_Height = 0;
    bool active_p = false;
    int res = 0;

    if(flag_ActivePoint &&
            action_event &&
            (action_event->point_action->Unique_Name == CHANGE_HEIGHT_TUBES ||
             action_event->point_action->Unique_Name == BACKUP_HEIGHT_TUBES)) active_p = true;


    if(value < 0)
    {
        message.setStandardButtons(QMessageBox::Ok);
        message.setIcon(QMessageBox::Warning);
        text = tr("Attention! Error in measuring of the tube height!");
        message.setText(text);
        message.exec();

        if(active_p) PA_Error(text);

    }
    else
    {
        if(!active_p)
        {
            text = QString("%1: (h = %2)\r\n%3").arg(tr("Attention! Measurement of the tube height passed successfully"))
                                            .arg(value)
                                            .arg(tr("Do you want to save this value in subsequent runs?"));
            message.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            message.setIcon(QMessageBox::Question);
            message.setText(text);
            res = message.exec();
        }

        if(res == QMessageBox::Yes || active_p)
        {
            Tube_Height = value;
            slotSendToServer(HIGHTUBE_SAVE);
            //qDebug() << "HIGHTUBE_SAVE: " << value;
        }
    }
}
//-----------------------------------------------------------------------------
//--- slot_SaveMeasuredTubeHeiht(int status)
//-----------------------------------------------------------------------------
void Run::slot_SaveMeasuredTubeHeiht(int status)
{
    QString text;
    message.setStandardButtons(QMessageBox::Ok);
    message.button(QMessageBox::Ok)->animateClick(5000);

    bool active_p = false;

    if(flag_ActivePoint &&
            action_event &&
            (action_event->point_action->Unique_Name == CHANGE_HEIGHT_TUBES ||
             action_event->point_action->Unique_Name == BACKUP_HEIGHT_TUBES)) active_p = true;

    if(active_p)
    {
       if(status == 0) PA_Ok();
       else PA_Error("Error during saving...");
       return;
    }
    //----------

    if(status == 0)     // Ok
    {
        message.setIcon(QMessageBox::Information);
        text = tr("The value is saved successfully!");
    }
    else
    {
        message.setIcon(QMessageBox::Critical);
        text = tr("Error during saving...");
    }
    message.setText(text);
    message.exec();

}

//-----------------------------------------------------------------------------
//--- SLOT check_Expo()
//-----------------------------------------------------------------------------
void Run::check_Expo()
{
    bool ok;
    int width = 600;
    int height = 400;

    if(Expo_check->isVisible()) return;
    if(Video_Image->isVisible()) Video_Image->close();

    if(dll_3D) Expo_check->Load_lib(dll_3D);   // create 3D

    QString text = tr("Expo_") + map_InfoDevice.value(INFODEV_serName,"");
    Expo_check->setWindowTitle(text);
    Expo_check->flag_DeviceSettings = &flag_DeviceSettings;
    //Expo_check->setWindowIcon(QIcon(":/images/RT.ico"));
    Expo_check->setWindowIcon(QIcon(":/images/DTm.ico"));
    if(CALIBRATION_status) Expo_check->setWindowIcon(QIcon(":/images/check.ico"));

    text = map_InfoDevice.value(INFODEV_serName,"000");
    int count_tubes = text.trimmed().mid(1,1).toInt();
    switch(count_tubes)
    {
    default:    count_tubes = 0;   break;
    case 5:     count_tubes = 96;  break;
    case 6:     count_tubes = 384; break;
    case 7:     count_tubes = 48;  break;
    case 8:     count_tubes = 192; break;
    }

    Expo_check->type_dev = count_tubes;
    Expo_check->Coef_Expo = Coef_Expo;
    Expo_check->expozition_Def = &Expozition_Def;

    Expo_check->active_channels = map_InfoDevice.value(INFODEV_devMask,"0").toInt(&ok,16);    
    Expo_check->Load_Fluor();    
    Expo_check->Get_InfoDevice(&map_InfoDevice);

    QSize size = qApp->screens().at(0)->size();
    int x,y;
    x = (size.width() - width)/2;
    y = (size.height() - height)/2;
    HWND handle = (HWND)Expo_check->winId();
    ::SetWindowPos(handle, HWND_TOP, x, y, width, height, SWP_SHOWWINDOW);

    //Expo_check->setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);

    Expo_check->show();

}

//-----------------------------------------------------------------------------
//--- SLOT create_VideoImage()
//-----------------------------------------------------------------------------
void Run::create_VideoImage()
{
    bool ok;

    if(Video_Image->isVisible()) return;
    if(Expo_check->isVisible()) Expo_check->close();

    if(dll_3D) Video_Image->Load_lib(dll_3D);   // create 3D

    QString text = tr("Video_") + map_InfoDevice.value(INFODEV_serName,"");
    Video_Image->setWindowTitle(text);
    Video_Image->flag_DeviceSettings = &flag_DeviceSettings;
    //Video_Image->setWindowIcon(QIcon(":/images/video.png"));

    text = map_InfoDevice.value(INFODEV_serName,"000");
    int count_tubes = text.trimmed().mid(1,1).toInt();
    switch(count_tubes)
    {
    default:    count_tubes = 0;   break;
    case 5:     count_tubes = 96;  break;
    case 6:     count_tubes = 384; break;
    case 7:     count_tubes = 48;  break;
    case 8:     count_tubes = 192; break;
    }
    Video_Image->type_dev = count_tubes;

    Video_Image->LEFT_OFFSET = LEFT_OFFSET;
    Video_Image->LEFT_OFFSET_DT48 = LEFT_OFFSET_DT48;
    Video_Image->W_IMAGE = W_IMAGE;
    Video_Image->H_IMAGE = H_IMAGE;
    Video_Image->COEF_IMAGE = COEF_IMAGE;
    Video_Image->W_IMAGE_COEF = W_IMAGE_COEF;
    Video_Image->W_REALIMAGE = W_REALIMAGE;
    Video_Image->H_REALIMAGE = H_REALIMAGE;
    Video_Image->TOP_OFFSET = TOP_OFFSET;


    Video_Image->active_channels = map_InfoDevice.value(INFODEV_devMask,"0").toInt(&ok,16);
    Video_Image->Load_Fluor();
    Video_Image->Get_InfoDevice(&map_InfoDevice);
    Video_Image->Clear_Image();

    Video_Image->show();

    QDesktopWidget *desktop = QApplication::desktop();
    int x,y;
    x = (desktop->width() - Video_Image->width())/2;
    y = (desktop->height() - Video_Image->height())/2;
    Video_Image->move(x,y);    
    //Video_Image->setFixedSize(Video_Image->size());

}
//-----------------------------------------------------------------------------
//--- Check_TestsPtrogramm(rt_Protocol *)
//-----------------------------------------------------------------------------
bool Run::Check_TestsPtrogramm(rt_Protocol *p)
{
    int i,j;
    bool sts = true;

    if(ALARM_Program) return(true);

    if(!p) return(true);
    //if(p->tests.size() <= 1) return(true);

    QVector<QString> Test_Pro;
    QVector<QString> Protocol_Pro;
    rt_Test         *ptest;
    string str;
    QString text;
    QStringList list;
    int vol = p->volume;    

    for(i=0; i<p->program.size(); i++)
    {
        text = QString::fromStdString(p->program.at(i)).trimmed();
        if(text.isEmpty()) continue;
        if(text.startsWith("XPRG "))
        {
            list = text.split(" ");
            if(list.size() > 2)
            {
                list.replace(2,QString::number(vol));
                text = list.join(" ");
            }
        }
        Protocol_Pro.append(text);
    }

    //qDebug() << Protocol_Pro.size();
    //for(j=0; j<Protocol_Pro.size(); j++) qDebug() << j << Protocol_Pro.at(j);

    for(i=0; i<p->tests.size(); i++)
    {
        ptest = p->tests.at(i);
        Test_Pro.clear();

        for(j=0; j<ptest->header.program.size(); j++)
        {
            text = QString::fromStdString(ptest->header.program.at(j)).trimmed();
            if(text.isEmpty()) continue;
            if(text.startsWith("XPRG "))
            {
                list = text.split(" ");
                if(list.size() > 2)
                {
                    list.replace(2,QString::number(vol));
                    text = list.join(" ");
                }
            }
            Test_Pro.append(text);
        }

        //qDebug() << Test_Pro.size();
        //for(j=0; j<Test_Pro.size(); j++) qDebug() << j << Test_Pro.at(j);


        if(Test_Pro != Protocol_Pro)
        {
            message.setStandardButtons(QMessageBox::Ok);
            message.button(QMessageBox::Ok)->animateClick(10000);
            message.setIcon(QMessageBox::Warning);
            text = tr("Attention! The Protocol program differs from the Test Program!");
            text += "\r\n" + tr("The current protocol cannot be run!");
            message.setText(text);
            message.exec();
            sts = false;
            break;
        }
    }

    Test_Pro.clear();
    Protocol_Pro.clear();

    return(sts);
}
//-----------------------------------------------------------------------------
//--- Check_TestsVolumeTube(rt_Protocol *)
//-----------------------------------------------------------------------------
bool Run::Check_TestsVolumeTube(rt_Protocol *p)
{
    int i;
    QString text;
    int volume;
    bool sts = true;

    if(ALARM_Volume) return(true);

    rt_Test *ptest;

    if(!p) return(true);
    if(p->tests.size() <= 1) return(true);

    for(i=0; i<p->tests.size(); i++)
    {
        ptest = p->tests.at(i);
        if(!i)
        {
            volume = ptest->header.Volume_Tube;
            continue;
        }

        if(volume != ptest->header.Volume_Tube)
        {
            message.setStandardButtons(QMessageBox::Ok);
            message.button(QMessageBox::Ok)->animateClick(10000);
            message.setIcon(QMessageBox::Warning);
            text = tr("Attention! Discovered different tubes volume for multiple tests!");
            text += "\r\n" + tr("The current protocol cannot be run!");
            message.setText(text);
            message.exec();
            sts = false;
            break;
        }
    }

    return(sts);
}

//-----------------------------------------------------------------------------
//--- Check_TestsExposition(rt_Protocol *)
//-----------------------------------------------------------------------------
bool Run::Check_TestsExposition(rt_Protocol *p)
{
    bool sts = true;
    if(!p) return(true);
    //if(p->tests.size() <= 1) return(true);


    int i,j,k;
    QStringList list;
    QString text, str;
    QVector<double> expo, expo_mask;
    double value;
    short val;
    bool ok;
    int act_ch = p->active_Channels;

    rt_Test         *ptest;
    rt_Preference   *preference;

    p->Exposure.clear();

    for(i=0; i<p->tests.size(); i++)
    {
        ptest = p->tests.at(i);

        for(j=0; j<ptest->preference_Test.size(); j++)
        {
            preference = ptest->preference_Test.at(j);

            if(preference->name == EXPOSURE)
            {
                text = QString::fromStdString(preference->value);
                list = text.split(QRegExp("\\s+"));
                k = 0;
                foreach(text, list)
                {
                    if(text == "0") list.replace(k,"1");
                    k++;
                }
                //qDebug() << "expo: " << list;
                expo.clear();

                for(k=0; k<COUNT_CH; k++)
                {
                    value = 0;

                    if(act_ch & (0x0f << k*4))
                    {
                        value = QString(list.at(k)).toDouble(&ok);
                        if(!ok) value = 0;
                        expo.push_back(value);
                    }

                    if(!i)
                    {
                        str = QString::number(value*100., 'f', 0);
                        val = str.toShort();
                        p->Exposure.push_back(val);
                    }
                }
                if(!i) expo_mask = expo.mid(0);
                //qDebug() << "expo: " << expo << expo_mask << list << p->Exposure;
                break;
            }
        }
        if(expo_mask.size() == expo.size())
        {
            ok = qEqual(expo.begin(), expo.end(), expo_mask.begin());
        }
        else ok = false;
        sts = ok;

        if(ALARM_Exposure) return(true);

        if(!ok)
        {
            p->Exposure.clear();
            message.setStandardButtons(QMessageBox::Ok);
            message.button(QMessageBox::Ok)->animateClick(10000);
            message.setIcon(QMessageBox::Warning);
            text = tr("Attention! Discovered different values of exposures for multiple tests!") + "\r\n";
            text += tr("The current protocol cannot be run!");
            message.setText(text);
            message.exec();
            break;
        }
    }    
    return(sts);
}
//-----------------------------------------------------------------------------
//--- clear_protcol()
//-----------------------------------------------------------------------------
void Run::clear_protocol()
{    
    //
    prot->Clear_Protocol();
    //prot->program.clear();
    NameProt->setText(tr("No Protocol..."));
    NameProt->setStyleSheet("QLabel{background-color: rgb(220, 220, 220)}");

    // 1. clear protocol info
    info_Protocol->clear_Info();

    // 2. program
    plot_Program->clear_Program();
    scheme_Program->clear_scheme();

    // 3. runtime
    runtime_Program->scheme_RTProgram->clear_scheme();
    runtime_Program->program_Name->setText("");

    // 4. plate
    sel->Select_Grid->clear();
    sel->Select_Grid->setColumnCount(0);
    sel->Select_Grid->setRowCount(0);
    sel->Test_tree->clear();
    sel->Sample_tree->clear();

    // 5. Fluor Graph
    plot_fluor->Delete_Curve();
    plot_fluor->label_fn->setText("");

    // 6. Temperature Graph
    plot_temperature->clearPlot();

    // 7. Comments
    comments->clear();

    Tab_Protocol->setCurrentIndex(1);

    RunDev->setDisabled(true);  //
}

//-----------------------------------------------------------------------------
//--- SLOT open_protocol()
//-----------------------------------------------------------------------------
void Run::open_protocol()
{
    Open_Protocol();
}

//-----------------------------------------------------------------------------
//--- SLOT Open_Protocol(file)
//-----------------------------------------------------------------------------
void Run::Open_Protocol(QString fileName)
{
    QString text;
    QString dirName = user_Dir.absolutePath();    
    QString selectedFilter;
    rt_Test* ptest;
    QString validate_str;
    QStringList list_tr;

    // LOGGED
    //map_Logged.clear();
    //map_Logged.insert(DTMASTERLOG_DATA, "Open_Protocol()");
    //slotSendToServer(DTMASTERLOG_REQUEST);
    //

    //rt_Preference *property;

    //qDebug() << "Open_Protocol: " << fileName;

    if(fileName.isEmpty())
    {
        fileName = QFileDialog::getOpenFileName(this, tr("Open Protocol"),
                                dirName,
                                tr("Protocols File (*.rt *.trt)"),
                                &selectedFilter);        
    }

    if(!fileName.isEmpty()) addProtocol_toList(fileName);
    if(fileName.isEmpty()) return;
    QFileInfo file_Info(fileName);

    fileName = Create_INProtocol(fileName);
    if(fileName.isEmpty()) return;

    prot->Clear_Protocol();    
    Read_XML(this, NULL, prot, fileName, "", true);

    /*for(int i=0; i<prot->program.size(); i++)
    {
        qDebug() << i << QString::fromStdString(prot->program.at(i));
    }*/

    if(!Validate_OpenProtocol(prot, validate_str))
    {
        message.setStandardButtons(QMessageBox::Ok);
        message.button(QMessageBox::Ok)->animateClick(10000);
        message.setIcon(QMessageBox::Warning);
        message.setText(validate_str);
        message.exec();

        clear_protocol();
        return;
    }

    //qDebug() << "Run, OpenProtocol: " << fileName << prot->count_PCR << prot->count_MC;

    //foreach(property, prot->preference_Pro)
    //{
    //    qDebug() << "Properies: " << QString::fromStdString(property->name) << QString::fromStdString(property->value);
    //}

    foreach(ptest, prot->tests)
    {
        Validate_Test(ptest);
    }

    //... Translate Tests and Research
    if(ID_LANG != ID_LANG_RU)
    {
        foreach(ptest, prot->tests)
        {
            if(ptest->header.Type_analysis > 0x20000)
            {
                Create_TranslateList(ptest, list_tr);
            }
        }
        if(list_tr.size()) Translate_Tests(list_tr);        // create Map_TestsTranslate
        list_tr.clear();
    }
    //
    //... Change ID-protocol for Calibration (Scorcina)
    foreach(ptest, prot->tests)
    {
        if(ptest->header.Type_analysis == 0x0020)   // Calibration
        {
            prot->regNumber = QDateTime::currentDateTime().toString("hh-mm_dd-MM-yy").toStdString();
            //qDebug() << "calibr: " << QString::fromStdString(prot->regNumber);
            break;
        }
    }
    //...

    if(QString::fromStdString(prot->name).isEmpty())
    {
        text = file_Info.completeBaseName() + "." + file_Info.suffix();
        prot->name = text.toStdString();
    }
    NameProt->setStyleSheet("QLabel{background-color: rgb(128, 255, 128)}");
    NameProt->setText(QString::fromStdString(prot->name));

    //Generate_IDProtocol(prot);

    runtime_Program->scheme_RTProgram->draw_Program(prot);  // Online
    runtime_Program->program_Name->setText(QString::fromStdString(prot->PrAmpl_name));

    plot_Program->draw_Program(prot);   // Program
    scheme_Program->draw_Program(prot);

    sel->clear_SelectGrid();
    sel->load_SelectGrid(prot);         // Plate

    RefreshInfoProtocol(prot);          // Information

    comments->clear();                  // Comments

    emit sNewProtocol(prot);

    FN = 0;    
    T_current = -1;
    T_hold = Get_T_hold();
    OnLine_FileName = "";    

    //... check expositions,programms and volumes
    if(!Check_TestsExposition(prot) ||
       !Check_TestsPtrogramm(prot)  ||
       !Check_TestsVolumeTube(prot))
    {        
        clear_protocol();
    }
    //...

    Check_Compatibility();    
    slot_ChangeStateDevice(Dev_State);

    Tab_Protocol->setCurrentIndex(1);

    prot->state = mWait;

    if(prot->count_MC == 0 && prot->count_PCR == 0)
    {
        message.setStandardButtons(QMessageBox::Ok);
        message.button(QMessageBox::Ok)->animateClick(10000);
        message.setIcon(QMessageBox::Warning);
        message.setText(tr("Attention! There are no optical measurements in the Protocol!"));
        message.exec();
    }

    //QVector<double> pp;

    //pp = QVector<double>::fromStdVector(prot->PrAmpl_time);
    //qDebug() << "level_count:" << pp;

    // temporary

    //this->parentWidget()->setProperty("measurements_file",
    //                                  QVariant(file_Info.absoluteFilePath()));
    //QEvent *e = new QEvent((QEvent::Type)3003);
    //QApplication::sendEvent(this->parentWidget(), e);

}
//-----------------------------------------------------------------------------
//--- Create_TranslateList(rt_Test *, QStringList &)
//-----------------------------------------------------------------------------
void Run::Create_TranslateList(rt_Test *p_test, QStringList &list)
{
    QString text;
    rt_TubeTest     *tube_test;
    rt_ChannelTest  *channel_test;

    // 1. name test
    text = QString::fromStdString(p_test->header.Name_Test);
    if(text.contains(QRegularExpression("[А-Яа-я]+")) && !list.contains(text))
    {
        list.append(text);
        //qDebug() << "name: " << text;
    }

    // 2. name research
    foreach(tube_test, p_test->tubes)
    {
        foreach(channel_test, tube_test->channels)
        {
            text = QString::fromStdString(channel_test->name);
            if(text.contains(QRegularExpression("[А-Яа-я]+")) && !list.contains(text))
            {
                list.append(text);
                //qDebug() << "name_research: " << text;
            }
        }
    }

    // перевод каталогов делаем сами...
    // 3. name Catalog
    //text = QString::fromStdString(p_test->header.Catalog);
    //if(text.contains(QRegularExpression("[А-Яа-я]+")) && !list.contains(text)) list.append(text);
}
//-----------------------------------------------------------------------------
//--- Translate_Tests
//-----------------------------------------------------------------------------
void Run::Translate_Tests(QStringList &list)
{
    QString text, str = "";
    char *p;
    QByteArray ba;
    QStringList list_in, list_out;
    int id;
    QString key, value;

    Map_TestTranslate.clear();

    foreach(text, list)
    {
        if(str.length()) str += "\r\n";
        str += text;
    }

    if(str.length())
    {
        ext_dll_handle = ::LoadLibraryW(L"DTReport2.dll");
        if(ext_dll_handle)
        {
            translate_service = (TranslateService)(::GetProcAddress(ext_dll_handle,"TranslateStr"));
            if(translate_service)
            {
                //text = "Balu... - Я";
                list_in = str.split("\r\n");
                translate_service(ID_LANG, str.toUtf8().data(), &p);

                ba.clear();
                ba.append(p);

                text = QString::fromLocal8Bit(ba);                      // ok
                //text = QString::fromStdString(ba.toStdString());      // ok
                list_out = text.split("\r\n");


                id = 0;
                foreach(text, list_in)
                {
                    key = text;
                    if(id >= list_out.size()) break;
                    value= list_out.at(id);
                    Map_TestTranslate.insert(key, value);

                    id++;
                }
            }
            ::FreeLibrary(ext_dll_handle);
        }
    }

    qDebug() << "TranslateService - ok" << Map_TestTranslate;

}
//-----------------------------------------------------------------------------
//--- Validate_OpenProtocol(QString &str)
//-----------------------------------------------------------------------------
bool Run::Validate_OpenProtocol(rt_Protocol *Prot, QString &str)
{
    int i,j,k;
    rt_GroupSamples *group;
    rt_Sample       *sample;
    rt_Tube         *tube;
    bool res = true;

    if(!Prot) {str = "NULL Protocol!"; return(false);}

    str = "";

    // Tube Position - ???
    for(i=0; i<Prot->Plate.groups.size(); i++)
    {
        group = Prot->Plate.groups.at(i);
        for(j=0; j<group->samples.size(); j++)
        {
            sample = group->samples.at(j);
            for(k=0; k<sample->tubes.size(); k++)
            {
                tube = sample->tubes.at(k);
                if(tube->pos < 0)
                {
                    str = tr("Attention! Invalid tubes position in the Protocol!");
                    res = false;
                    break;
                }
            }
            if(str.length()) break;
        }
        if(str.length()) break;
    }

    //

    return(res);
}

//-----------------------------------------------------------------------------
//--- Validate_Test(rt_Test*)
//-----------------------------------------------------------------------------
bool Run::Validate_Test(rt_Test *ptest)
{
    bool res = false;
    QString xml_str;
    QString status;
    rt_Preference   *preference;

    if(!axgp) return(res);

    foreach(preference, ptest->preference_Test)
    {
        if(preference->name == "xml_node")
        {
            xml_str = QString::fromStdString(preference->value);
            res = axgp->dynamicCall("loadXmlTest(QString)", xml_str).toBool();
            if(res) status = "ok";
            else status = "none";
            ptest->header.Hash = status.toStdString();
            break;
        }
    }

    return(res);
}
//-----------------------------------------------------------------------------
//--- Generate_IDProtocol(rt_Protocol *p)
//-----------------------------------------------------------------------------
void Run::Generate_IDProtocol(rt_Protocol *p)
{
    if(!p) return;
    if(!QString::fromStdString(p->regNumber).trimmed().isEmpty()) return;

    QString text = QString("%1_%2").arg(GetRandomString(5)).arg(QDateTime::currentDateTime().toString("ddMMyy_hmmss"));
    p->regNumber = text.toStdString();
}

//-----------------------------------------------------------------------------
//--- Check_HoldLevel()
//-----------------------------------------------------------------------------
bool Run::Check_HoldLevel()
{
    double value;

    if(T_hold > 0 && T_current > 0)
    {
        value = qAbs(T_current - T_hold)/T_hold;
        if(value < 0.25) return(true);
    }

    return(false);
}

//-----------------------------------------------------------------------------
//--- Get_T_hold()
//-----------------------------------------------------------------------------
double Run::Get_T_hold()
{
    if(!prot) return(-1);

    int i;
    int count = prot->PrAmpl_value.size();
    double res = -1;

    for(i=0; i<count; i++)
    {
        if(prot->PrAmpl_color.at(i) == 3)
        {
            res = prot->PrAmpl_value.at(i);
            break;
        }
    }
    return(res);
}
//-----------------------------------------------------------------------------
//--- SLOT slot_GetPicture(int, int, int)
//-----------------------------------------------------------------------------
void Run::slot_GetPicture(int ch, int expo, int ctrl)
{
    map_Picture.clear();
    map_Picture.insert(GETPIC_CHANNEL, QString::number(ch));
    map_Picture.insert(GETPIC_EXP, QString::number(expo));
    map_Picture.insert(GETPIC_CTRL, QString::number(ctrl));

    slotSendToServer(GETPIC_REQUEST);
    wait_process(wPic, 100, tr("Get Picture..."));
}
//-----------------------------------------------------------------------------
//--- SLOT slot_SavePar(int, QString)
//-----------------------------------------------------------------------------
void Run::slot_SavePar(int ctrl, QString data)
{
    ID_SavePar id = (ID_SavePar)ctrl;
    map_SavePar.clear();
    map_SavePar.insert(SAVEPAR_CTRL, QString::number(ctrl));
    map_SavePar.insert(SAVEPAR_DATA, data);
    map_SavePar.insert(ID_PARAM, QString::number(id));

    //qDebug() << map_SavePar;

    slotSendToServer(SAVEPAR_REQUEST);
    wait_process(wPic, 100, tr("Save Parameters..."));
}
//-----------------------------------------------------------------------------
//--- SLOT slot_SaveParMedian(QString, QString)
//-----------------------------------------------------------------------------
void Run::slot_SaveParMedian(QString key, QString data_base64)
{
    map_Median.clear();
    map_Median.insert(key, data_base64);
}
//-----------------------------------------------------------------------------
//--- SLOT slot_SaveParDefault(QString, QString)
//-----------------------------------------------------------------------------
void Run::slot_SaveParDefault(QString key, QString data_base64)
{
    map_Default.clear();
    map_Default.insert(key, data_base64);
}

//-----------------------------------------------------------------------------
//--- SLOT slot_WriteSector(int, QString)
//-----------------------------------------------------------------------------
void Run::slot_WriteSector(int sector, QString data)
{
    map_Sector.clear();
    map_Sector.insert(SECTOR_CMD, QString("PWRS %1").arg(sector));
    map_Sector.insert(SECTOR_DATA, data);

    //qDebug() << "SECTORWRITE_REQUEST: " << map_Sector;

    slotSendToServer(SECTORWRITE_REQUEST);
    wait_process(wPic, 100, tr("Write Sector Data..."));
}
//-----------------------------------------------------------------------------
//--- SLOT slot_ReadSector(int, QString)
//-----------------------------------------------------------------------------
void Run::slot_ReadSector(int sector, QString owner)
{        
    map_Sector.clear();
    map_Sector.insert(SECTOR_CMD, QString("PRDS %1").arg(sector));
    map_Sector.insert(ID_PARAM, owner);

    //qDebug() << "SECTORREAD_REQUEST: " << map_Sector;

    slotSendToServer(SECTORREAD_REQUEST);
    wait_process(wPic, 100, tr("Read Sector Data..."));
}
//-----------------------------------------------------------------------------
//--- Read_AllSectors()
//-----------------------------------------------------------------------------
void Run::Read_AllSectors()
{
    int i;

    QStringList  list_param;
    QVector<int> list_id;

    list_id << 0x0419 << 0x0418;
    list_param << "median" << "default";

    Queue_ReadAllSectors.clear();

    for(i=0; i<list_id.size(); i++)
    {
        Queue_ReadAllSectors.insert(list_id.at(i), list_param.at(i));
    }

    slot_Read_CurrentSector();
}
//-----------------------------------------------------------------------------
//--- slot_Read_CurrentSector()
//-----------------------------------------------------------------------------
void Run::slot_Read_CurrentSector()
{
    int id;
    QString text;

    //qDebug() << "slot_Read_CurrentSector(): " << Queue_ReadAllSectors;

    if(!Queue_ReadAllSectors.isEmpty())
    {
        id = Queue_ReadAllSectors.keys().at(0);
        text = Queue_ReadAllSectors.value(id);
        Queue_ReadAllSectors.remove(id);

        slot_ReadSector(id, text);
    }
}

//-----------------------------------------------------------------------------
//--- SLOT slot_Processing_ReadSector(QMap<QString, QString> &map)
//-----------------------------------------------------------------------------
void Run::slot_Processing_ReadSector(QMap<QString, QString> &map)
{
    int i,j;
    QTreeWidgetItem *item;
    QList<QTreeWidgetItem*> list;
    int value;
    QString str;
    bool status_def = false;
    bool device_enable = false;
    QString name_Dev;
    QChar ch_0, ch_1;
    bool reload_dev = false;

    //flag_EnableDev = false;

    union
    {
        Device_ParDefault Device_ParamDefault;
        unsigned char byte_buf[512];
    }DEVICE_PAR_DEFAULT;
    QByteArray buf_default;

    //... map_Sector ...

    QString text;
    QString str_default;
    QString owner = map.value(ID_PARAM,"");
    int status = map.value("status","0").toInt();

    //qDebug() << "slot_Processing_ReadSector: " << owner << status;


    if(owner == "median")
    {
        text = map.value(SECTOR_DATA,"");
        //QByteArray ba = QByteArray::fromBase64(text.toStdString().c_str());
        //qDebug() << "Sector Data: " << ba;
        if(!text.isEmpty()) map_InfoDevice.insert("median", text);
    }
    if(owner == "default")
    {
        if(status != 0)
        {
            if(flag_ActivePoint) PA_Error("error ReadSector!");

            text = tr("Attention! It is not possible to read data from the device!") + "\r\n";
            text += tr("You need to restart the current application.") + "\r\n";
            text += tr("Please, contact customer support...");
            message.setStandardButtons(QMessageBox::Ok);
            message.setIcon(QMessageBox::Critical);
            //message.button(QMessageBox::Ok)->animateClick(30000);
            message.setText(text);
            message.exec();

            return;         // ERROR!!!!!!!!
        }

        //flag_FirstConnectDevice = true;
        //flag_EnableDev = true;

        text = map.value(SECTOR_DATA,"");
        //QByteArray ba = QByteArray::fromBase64(text.toStdString().c_str());
        //qDebug() << "Sector Data: " << ba;

        if(!text.isEmpty())
        {
            //map_InfoDevice.insert("default", text);

            buf_default = QByteArray::fromBase64(text.toStdString().c_str());
            if(buf_default.size() == 512)
            {
                for(i=0; i<512; i++) DEVICE_PAR_DEFAULT.byte_buf[i] = buf_default.at(i);
                str_default = QString(DEVICE_PAR_DEFAULT.Device_ParamDefault.id_default);

                // TEMPORARY //
                //if(first_cycle) str_default.clear();
                //first_cycle = false;
                //

                if(str_default.contains("default"))
                {
                    Expozition_Def.clear();
                    for(j=0; j<COUNT_CH*2; j++) Expozition_Def.append(DEVICE_PAR_DEFAULT.Device_ParamDefault.Expo_def[j]);
                    status_def = true;
                    flag_EnableDev = true;
                }

                if(!status_def)
                {
                    //...
                    if(flag_ActivePoint) PA_Error("Basic exposure values!");

                    //... 1. Device is made after 2017 ???
                    name_Dev = map_InfoDevice.value(INFODEV_serName, "").trimmed();

                    qDebug() << "name_Dev: " << name_Dev;
                    if(name_Dev.size() == 6)
                    {
                        ch_0 = name_Dev.at(1);

                        switch(ch_0.unicode())
                        {
                        case '5':           // dt96
                        case '6':           // dt384
                                    ch_0 = name_Dev.at(2);
                                    ch_1 = name_Dev.at(3);
                                    if((ch_0.isLetter() && ch_0 > 'F') ||
                                      ((ch_0.isLetter() && ch_0 == 'F') && ((ch_1.isDigit() && (ch_1 >= '2' || ch_1 == '0')) || ch_1 == 'N' || ch_1 == 'D')))
                                    {
                                        device_enable = true;
                                    }

                                    if(ch_0 >= 'V')  device_enable = false; // V,W,X,Y,Z !!!!!!!!!!!!!!!!!!!


                                    break;
                        case '7':           // dt48

                                    ch_0 = name_Dev.at(2);
                                    ch_1 = name_Dev.at(3);
                                    if((ch_0.isLetter() && ch_0 > 'F') ||
                                      ((ch_0.isLetter() && ch_0 == 'F') && ((ch_1.isDigit() && (ch_1 >= '6' || ch_1 == '0')) || ch_1 == 'N' || ch_1 == 'D')))
                                    {
                                        device_enable = true;
                                    }

                                    break;

                        case '8':           // dt192
                                    break;

                        default:
                                    break;
                        }
                    }
                    else
                    {
                        reload_dev = true;
                    }

                    //... 2. ...


                    if(device_enable)
                    {
                        QTimer::singleShot(500, this, SLOT(slot_SaveDefExposition()));

                        text = tr("Attention! Basic exposure values were not detected in the device!") + "\r\n";
                        text += tr("Please, contact customer support...");
                        message.setStandardButtons(QMessageBox::Ok);
                        message.setIcon(QMessageBox::Warning);
                        message.button(QMessageBox::Ok)->animateClick(30000);
                        message.setText(text);
                        //message.exec();

                        flag_EnableDev = false;
                    }
                    else
                    {
                        // device is disable
                        text = tr("Attention! The device cannot work with the current version of the program!") + "\r\n";
                        text += tr("Please, contact customer support...");
                        if(reload_dev)
                        {
                            text = tr("Attention! You need to restart the device!");
                        }
                        message.setStandardButtons(QMessageBox::Ok);
                        message.setIcon(QMessageBox::Warning);
                        message.button(QMessageBox::Ok)->animateClick(10000);
                        message.setText(text);
                        message.exec();

                        flag_EnableDev = false;
                    }
                }

                //if(!status_def && flag_FirstConnectDevice) QTimer::singleShot(200, this, SLOT(slot_SaveDefExposition()));
                //flag_FirstConnectDevice = false;
            }
        }
        //qDebug() << "ReadSector Expo: " << Expozition << Expozition_Def << status_def;
        list = info_device->findItems(tr("Exposition"), Qt::MatchContains, 0);
        if(list.size())
        {
            item = list.at(0);
            i = 0;
            text = "";
            foreach(value, Expozition)
            {
                if(div(i,2).rem == 0)
                {
                    if(!text.isEmpty()) text += ",";
                    text += QString("%1").arg(value);
                }
                i++;
            }

            i = 0;
            str = "";
            foreach(value, Expozition_Def)
            {
                if(div(i,2).rem == 0)
                {
                    if(!str.isEmpty()) str += ",";
                    str += QString("%1").arg(value);
                }
                i++;
            }
            str = QString("  (def: %1)").arg(str);

            text += str;
            item->setText(1, text);
        }
    }

}

//-----------------------------------------------------------------------------
//--- SLOT new_protocol()
//-----------------------------------------------------------------------------
void Run::new_protocol()
{

}
//-----------------------------------------------------------------------------
//--- SLOT save_protocol()
//-----------------------------------------------------------------------------
void Run::save_protocol()
{

}
//-----------------------------------------------------------------------------
//--- SLOT save_protocol()
//-----------------------------------------------------------------------------
void Run::Save_OnLineProtocol(QString fn)
{
    if(!flag_SaveOnLine || OnLine_FileName.isEmpty()) return;



}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Run::resize_splitter(int pos, int index)
{
    main_progress->setFixedWidth(pos);
    run_progress->setFixedWidth(statusBar()->width() - pos - 10);
}
//-------------------------------------------------------------------------------
//--- resizeEvent
//-------------------------------------------------------------------------------
void Run::resizeEvent(QResizeEvent *e)
{    
    main_progress->setFixedWidth(ChartWindow->width());
    run_progress->setFixedWidth(ControlWindow->width());
}
//-------------------------------------------------------------------------------
//--- showEvent
//-------------------------------------------------------------------------------
void Run::showEvent(QShowEvent *e)
{
    main_progress->setFixedWidth(ChartWindow->width());
    run_progress->setFixedWidth(ControlWindow->width());
}
//-------------------------------------------------------------------------------
//--- timerEvent
//-------------------------------------------------------------------------------
void Run::timerEvent(QTimerEvent *event)
{
    bool sts = false;
    QBasicTimer *timer;
    QString text = "", str;
    int index;
    bool view_message = true;

    int id = event->timerId();

    foreach(timer, Map_TimerRequest)
    {
        if(id == timer->timerId())
        {
            timer->stop();
            sts = true;
            text = Map_TimerRequest.key(timer);
            break;
        }
    }
    if(sts)
    {
        index = List_Requests.indexOf(text);
        if(index >= 0) List_Requests.removeAt(index);
        emit finished_WaitProcess();

        if(text == INFO_DATA && (Dev_State < sRun || Dev_State == sHold)) view_message = false;

        if(view_message)
        {
            message.setStandardButtons(QMessageBox::Ok);
            message.setIcon(QMessageBox::Warning);
            message.button(QMessageBox::Ok)->animateClick(3000);
            str = tr("Attention! The answer is not received! ");
            message.setText(str + "(" + text + ")");
            message.exec();
        }

        create_RequestStatusInfo(text, EXTERNAL_TIMEOUT);
        if(text == OPENBLOCK_REQUEST || text == CLOSEBLOCK_REQUEST) OpenClose_Box->setEnabled(true);
        if(text == CLOSEBLOCK_REQUEST) flag_ClosePrerunRun = false;

    }
    else QWidget::timerEvent(event);
}

//-----------------------------------------------------------------------------
//--- create_ChartControlBox
//-----------------------------------------------------------------------------
void Run::create_ChartControlBox()
{
    //... chart ...
    ChartWindow = new QMainWindow();
    ChartBox = new QGroupBox(ChartWindow);
    ChartBox->setObjectName("Transparent");
    ChartBox->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Ignored);
    ChartBox->setMinimumWidth(200);
    //ChartBox->setStyleSheet("border:0;");
    ChartWindow->setCentralWidget(ChartBox);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(0);
    layout->setSpacing(4);
    ChartBox->setLayout(layout);

    chart_spl = new QSplitter(Qt::Vertical);
    chart_spl->setHandleWidth(6);
    Fluor_Chart = new QMainWindow();
    Fluor_Box = new QGroupBox(Fluor_Chart);
    Fluor_Box->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Ignored);
    Fluor_Chart->setCentralWidget(Fluor_Box);
    Temperature_Chart = new QMainWindow();
    Temperature_Box = new QGroupBox(Temperature_Chart);
    Temperature_Box->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Ignored);
    Temperature_Chart->setCentralWidget(Temperature_Box);

    plot_fluor = new GraphFluorPlot(this);
    QVBoxLayout *fluor_layout = new QVBoxLayout;
    Fluor_Box->setLayout(fluor_layout);
    fluor_layout->addWidget(plot_fluor);

    plot_temperature = new GraphTemperaturePlot(this);
    QVBoxLayout *temperature_layout = new QVBoxLayout;
    Temperature_Box->setLayout(temperature_layout);
    temperature_layout->addWidget(plot_temperature);


    chart_spl->addWidget(Fluor_Chart);
    chart_spl->addWidget(Temperature_Chart);
    layout->addWidget(chart_spl);

    //ChartWindow->setDisabled(true);

    //... control ...
    ControlWindow = new QMainWindow();
    ControlBox = new QGroupBox(ControlWindow);
    ControlBox->setObjectName("Transparent");
    ControlBox->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Ignored);
    ControlBox->setMinimumWidth(200);
    ControlWindow->setCentralWidget(ControlBox);

    QVBoxLayout *control_layout = new QVBoxLayout;
    control_layout->setMargin(0);
    control_layout->setSpacing(4);
    ControlBox->setLayout(control_layout);

    control_spl = new QSplitter(Qt::Vertical);
    control_spl->setHandleWidth(6);
    Info_window = new QMainWindow();
    Info_Box = new QGroupBox(Info_window);
    Info_Box->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Ignored);
    Info_window->setCentralWidget(Info_Box);
    /*
    fileToolBar = Info_window->addToolBar("Open&Save");
    //fileToolBar->setOrientation(Qt::Vertical);
    //fileToolBar->setAllowedAreas(Qt::RightToolBarArea);
    fileToolBar->addAction(new_prot);
    fileToolBar->addAction(open_prot);
    fileToolBar->addAction(save_prot);
    */

    Control_window = new QMainWindow();
    Control_Box = new QGroupBox(Control_window);
    Control_Box->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Ignored);
    Control_window->setCentralWidget(Control_Box);

    control_spl->addWidget(Info_window);
    control_spl->addWidget(Control_window);
    control_layout->addWidget(control_spl);
    //send_message = new QPushButton("send message",ControlWindow);
    //connect(send_message, SIGNAL(clicked()), this, SLOT(slotSendToServer()));

    main_spl->addWidget(ChartWindow);
    main_spl->addWidget(ControlWindow);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Run::createCONTROL()
{
    QPalette palette;
    QPalette pal;
    QStringList list_uC = QString(LIST_uC).split(",");    
    QFont f = qApp->font();

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setSpacing(0);
    Control_Box->setLayout(layout);

    QHBoxLayout *layout_device = new QHBoxLayout;
    layout_device->setSpacing(0);
    layout_device->setMargin(0);
    //Device_Locked = new QLabel(this);
    //Device_Locked->setVisible(false);
    NameDev = new QLabel(tr("No Device..."));
    NameDev->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    NameDev->setAlignment(Qt::AlignCenter);
    NameDev->setStyleSheet("QLabel{background-color: rgb(220, 220, 220)}");    
    f.setBold(true);
    NameDev->setFont(f);
    NameDev->setTextFormat(Qt::RichText);
    if(StyleApp == "fusion")
    {
        ListDevices = new QPushButton(QIcon(":/images/flat/open_flat.png"), NULL, this);
        ListDevices->setObjectName("Transparent");
        ListDevices->setIconSize(QSize(24,24));
        ListDevices->setFlat(true);
        ListDevices->setToolTip(tr("Open Device"));
        ListDevices->setStyleSheet("QPushButton::hover {background-color: white; border: 1px solid #ddd;}");
    }
    else ListDevices = new QPushButton(QIcon(":/images/open_new.png"),tr("ListDev"));
    connect(ListDevices, SIGNAL(clicked()), this, SLOT(ShowListDevice()));
    //layout_device->addWidget(Device_Locked);
    layout_device->addWidget(NameDev,1);
    layout_device->addWidget(ListDevices);

    //RunDev = new QPushButton(QIcon(":/images/start_1.png"), tr("Run"),this);
    QIcon icon_run;
    icon_run.addPixmap(QPixmap(":/images/start_1.png"), QIcon::Normal);
    icon_run.addPixmap(QPixmap(":/images/start_1_disable.png"), QIcon::Disabled);
    QIcon icon_pause;
    icon_pause.addPixmap(QPixmap(":/images/pause_1.png"), QIcon::Normal);
    icon_pause.addPixmap(QPixmap(":/images/pause_1_disable.png"), QIcon::Disabled);
    QIcon icon_stop;
    icon_stop.addPixmap(QPixmap(":/images/stop_1.png"), QIcon::Normal);
    icon_stop.addPixmap(QPixmap(":/images/stop_1_disable.png"), QIcon::Disabled);

    RunDev = new QPushButton(tr("Run"),this);
    RunDev->setIcon(icon_run);
    RunDev->setFixedHeight(33);
    RunDev->setIconSize(QSize(24,24));
    RunDev->setDisabled(true);
    StopDev = new QPushButton(tr("Stop"),this);
    StopDev->setIcon(icon_stop);
    StopDev->setFixedHeight(33);
    StopDev->setIconSize(QSize(24,24));
    StopDev->setDisabled(true);
    PauseDev = new QPushButton(QIcon(":/images/pause_1.png"), tr("Pause"),this);
    PauseDev->setIcon(icon_pause);
    PauseDev->setFixedHeight(33);
    PauseDev->setIconSize(QSize(24,24));
    PauseDev->setCheckable(true);
    PauseDev->setDisabled(true);

    if(StyleApp == "fusion")
    {
        OpenBlock = new QToolButton(this);
        OpenBlock->setIcon(QIcon(":/images/open_block.png"));
        //OpenBlock->setObjectName("Transparent");
        //OpenBlock->setFlat(true);
        OpenBlock->setToolTip(tr("Open block"));
        OpenBlock->setIconSize(QSize(24,24));
        OpenBlock->setStyleSheet("QToolButton::hover {background-color: white; border: 1px solid #ddd;}");
    }
    else
    {
        OpenBlock = new QToolButton(this);
        OpenBlock->setIcon(QIcon(":/images/open_block.png"));
        OpenBlock->setFixedSize(77,33);
    }

    if(StyleApp == "fusion")
    {
        CloseBlock = new QToolButton(this);
        CloseBlock->setIcon(QIcon(":/images/close_block.png"));
        //CloseBlock->setObjectName("Transparent");
        //CloseBlock->setFlat(true);
        CloseBlock->setToolTip(tr("Close block"));
        CloseBlock->setIconSize(QSize(24,24));
        CloseBlock->setStyleSheet("QToolButton::hover {background-color: white; border: 1px solid #ddd;}");
    }
    else
    {
        CloseBlock = new QToolButton(this);
        CloseBlock->setIcon(QIcon(":/images/close_block.png"));
        CloseBlock->setFixedSize(77,33);
    }

    OpenClose_Box = new QGroupBox(this);
    OpenClose_Box->setStyleSheet("border:none;");
    QHBoxLayout *layout_OC = new QHBoxLayout;
    OpenClose_Box->setLayout(layout_OC);
    layout_OC->addWidget(OpenBlock);
    layout_OC->addWidget(CloseBlock);
    OpenClose_Box->setDisabled(true);


    QHBoxLayout *layout_button = new QHBoxLayout;
    layout_button->setMargin(0);
    layout_button->addWidget(RunDev);
    layout_button->addWidget(PauseDev);
    layout_button->addWidget(StopDev);
    layout_button->addSpacing(30);
    layout_button->addWidget(OpenClose_Box, 0, Qt::AlignRight);


    Tab_Control = new QTabWidget();
    StandBy = new QCheckBox(tr("Stanby after run"), this);
    StandBy->setEnabled(false);
    StandBy_Box = new QGroupBox(this);
    StandBy_Box->setObjectName("Transparent");
    QVBoxLayout *layout_SB = new QVBoxLayout;
    StandBy_Box->setLayout(layout_SB);
    layout_SB->addWidget(StandBy, 1, Qt::AlignLeft);
    layout_SB->addStretch();
    LastRun = new QPushButton(tr("Last Protocol"),this);
    LastRun->setDisabled(true);
    //LastRun->setFixedWidth(150);
    LastRun_Box = new QGroupBox(this);
    LastRun_Box->setObjectName("Transparent");
    LastRun_Area = new QScrollArea(this);
    LastRun_Area->setWidget(LastRun_Box);
    LastRun_Area->setWidgetResizable(true);
    //LastRun->setFixedHeight(33);
    //LastRun->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Expanding);
    Analyze_immediately = new QCheckBox(tr("Analyze immediately"), this);
    QVBoxLayout *layout_LR = new QVBoxLayout;
    QHBoxLayout *layout_LRbutton = new QHBoxLayout;
    layout_LRbutton->addWidget(LastRun, 1, Qt::AlignLeft);
    LastRun_Box->setLayout(layout_LR);
    //layout_LR->addWidget(LastRun,1);
    layout_LR->addLayout(layout_LRbutton);
    layout_LR->addWidget(Analyze_immediately);
    layout_LR->addStretch();

    CMD_Box = new QGroupBox(this);
    CMD_Box->setObjectName("Transparent");
    cmd_Edit = new QTextEdit(this);
    cmd_Edit->append(">");
    QVBoxLayout *layout_CMD = new QVBoxLayout;
    layout_CMD->setMargin(0);
    CMD_Box->setLayout(layout_CMD);
    layout_CMD->addWidget(cmd_Edit);    
    cmd_Edit->setStyleSheet("background-color: darkBlue; color: white; selection-color: black;");
    cmd_Edit->installEventFilter(this);    
    LastCMD = "";
    cmd_ComboBox = new QComboBox(this);
    cmd_ComboBox->addItems(list_uC);
    cmd_ComboBox->setCurrentIndex(0);
    cmd_ComboBox->setStyleSheet("background-color: #ffffff; color: black; selection-background-color: #d7d7ff; selection-color: black;");
    //cmd_ComboBox->setFont(f);
    //cmd_ComboBox->setFocusPolicy(Qt::NoFocus);
    QHBoxLayout *layout_cmdCB = new QHBoxLayout;
    layout_cmdCB->setMargin(20);
    cmd_Edit->setLayout(layout_cmdCB);
    layout_cmdCB->addWidget(cmd_ComboBox,0,Qt::AlignRight | Qt::AlignTop);

    Preference_Area = new QScrollArea(this);
    Preference_Area->setWidgetResizable(true);
    Preference_Box = new QGroupBox(this);
    Preference_Box->setObjectName("Transparent");
    Preference_Area->setWidget(Preference_Box);

    if(StyleApp == "fusion")
    {
        VideoImage = new QPushButton(QIcon(":/images/flat/video_flat.png"), tr("create VideoImage"), this);
        VideoImage->setIconSize(QSize(24,24));
        VideoImage->setFixedHeight(33);

        ExpoValue = new QPushButton(QIcon(":/images/flat/timer_flat.png"), tr("check Exposure"), this);
        ExpoValue->setIconSize(QSize(24,24));
        ExpoValue->setFixedHeight(33);

        Meas_heigthTube = new QPushButton(QIcon(":/images/flat/measure_tube_32.png"), tr("measure Tubes height"), this);
        Meas_heigthTube->setIconSize(QSize(24,24));
        Meas_heigthTube->setFixedHeight(33);
    }
    else
    {
        VideoImage = new QPushButton(QIcon(":/images/video.png"), tr("create VideoImage"), this);
        ExpoValue = new QPushButton(QIcon(":/images/expo.png"), tr("check Exposure"), this);
        Meas_heigthTube = new QPushButton(QIcon(":/images/flat/measure_tube_32.png"), tr("measure Tubes height"), this);
        VideoImage->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
        ExpoValue->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
        Meas_heigthTube->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
    }

    SaveImage_rejime = new QCheckBox(tr("Save Image rejime"), this);
    SaveImage_rejime->setDisabled(true);
    //SaveImage_rejime_Active = true;


    OpticDev_TMP = new QPushButton(this);
    OpticDev_TMP->setVisible(false);

    QVBoxLayout *layout_Pre = new QVBoxLayout;
    Preference_Box->setLayout(layout_Pre);
    if(StyleApp == "fusion")
    {
        layout_Pre->addWidget(VideoImage,0,Qt::AlignTop | Qt::AlignLeft);
        layout_Pre->addWidget(ExpoValue,0,Qt::AlignTop | Qt::AlignLeft);
        layout_Pre->addWidget(Meas_heigthTube,0,Qt::AlignTop | Qt::AlignLeft);
        layout_Pre->addWidget(OpticDev_TMP,1,Qt::AlignTop | Qt::AlignRight);

        layout_Pre->addWidget(SaveImage_rejime, 1, Qt::AlignBottom | Qt::AlignLeft);
    }
    else
    {
        layout_Pre->addWidget(VideoImage);
        layout_Pre->addWidget(ExpoValue);
        layout_Pre->addWidget(Meas_heigthTube);
        layout_Pre->addWidget(SaveImage_rejime);
    }

    connect(VideoImage, SIGNAL(clicked(bool)), this, SLOT(create_VideoImage()));
    connect(ExpoValue, SIGNAL(clicked(bool)), this, SLOT(check_Expo()));
    //connect(SaveImage_rejime, SIGNAL(clicked(bool)), this, SLOT(change_SaveImageRejime()));
    connect(SaveImage_rejime, SIGNAL(stateChanged(int)), this, SLOT(change_SaveImageRejime()));
    connect(Meas_heigthTube, SIGNAL(clicked(bool)), this, SLOT(measure_TubesHeight()));

    connect(OpticDev_TMP, SIGNAL(clicked(bool)), this, SLOT(slot_ReadOpticDev()));

#ifdef QT_DEBUG
    ExpoValue->setDisabled(true);
    VideoImage->setDisabled(true);
#endif

    Error_Info = new QTreeWidget(this);
    Error_Info->setColumnCount(4);
    QStringList header;
    header << tr("Request") << tr("Value") << tr("uC") << tr("Description");
    Error_Info->setHeaderLabels(header);
    Error_Info->header()->setSectionResizeMode(0,QHeaderView::ResizeToContents);
    //Error_Info->header()->setSectionResizeMode(1,QHeaderView::ResizeToContents);
    Error_Info->clear();
    Error_Info->setFont(qApp->font());
    Error_Info->header()->setFont(qApp->font());

    info_device = new QTreeWidget(this);
    info_device->setFont(qApp->font());
    info_device->header()->setFont(qApp->font());
    Delegate_InfoDevice = new ItemDelegate_InfoDevice();
    Delegate_InfoDevice->style = StyleApp;
    info_device->setItemDelegate(Delegate_InfoDevice);

    //Tab_Control->addTab(OpenClose_Box,"Open_Close");
    Tab_Control->addTab(StandBy_Box,tr("StandBy"));
    Tab_Control->addTab(new QWidget(this),tr("Add and Skip"));
    Tab_Control->addTab(LastRun_Area,tr("Last Run"));
    Tab_Control->addTab(CMD_Box,tr("Command string"));
    Tab_Control->addTab(Preference_Area,tr("Preference"));
    Tab_Control->addTab(Error_Info,tr("Error"));
    Tab_Control->addTab(info_device,tr("Information"));

    //Tab_Control->setTabEnabled(0,false);
    Tab_Control->setTabEnabled(1,false);

    Tab_Control->setStyleSheet("QTabWidget::tab-bar {left: 10px;}");
    //Tab_Control->setStyleSheet("background-color: rgb(240, 240, 240);");
    Tab_Control->setDisabled(true);
    Tab_Control->setCurrentWidget(info_device);


    layout->addLayout(layout_device);
    layout->addLayout(layout_button);
    layout->addWidget(Tab_Control,1);    

    //connect(RunDev, SIGNAL(clicked(bool)), this, SLOT(slot_CheckExposition_Run()));
    connect(RunDev, SIGNAL(clicked(bool)), this, SLOT(slot_ProtocolIsEqual()));
    connect(this, SIGNAL(sRunAfterPrerun()), this, SLOT(slot_RUN()));
    //connect(this, SIGNAL(sPrerunAfterClose()), this, SLOT(slot_PreRUN));
    connect(PauseDev, SIGNAL(clicked(bool)), this, SLOT(slot_PAUSE()));
    connect(StopDev, SIGNAL(clicked(bool)), this, SLOT(slot_STOP()));

    connect(LastRun, SIGNAL(clicked(bool)), this, SLOT(slot_PrepareLastRun()));
    connect(StandBy, SIGNAL(stateChanged(int)), this, SLOT(slot_StandBy()));

    connect(OpenBlock, SIGNAL(clicked(bool)), this, SLOT(slot_OpenBlock()));
    connect(CloseBlock, SIGNAL(clicked(bool)), this, SLOT(slot_CloseBlock()));
    connect(Tab_Control,SIGNAL(tabBarClicked(int)), this, SLOT(slot_SelectTab(int)));

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Run::createINFO()
{
    QFont f = qApp->font();
    f.setBold(true);

    QVBoxLayout *layout = new QVBoxLayout;
    Info_Box->setLayout(layout);

    QHBoxLayout *layout_protocol = new QHBoxLayout;
    layout_protocol->setSpacing(0);

    NameProt = new QLabel(tr("No Protocol..."));
    NameProt->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    NameProt->setAlignment(Qt::AlignCenter);
    NameProt->setStyleSheet("QLabel{background-color: rgb(220, 220, 220)}");
    NameProt->setFont(f);

    ClearProtocol = new QPushButton(QIcon(":/images/flat/clear_flat_16.png"), NULL, this);
    ClearProtocol->setObjectName("Transparent");
    ClearProtocol->setIconSize(QSize(24,24));
    ClearProtocol->setFlat(true);
    ClearProtocol->setToolTip(tr("Clear Protocol"));
    ClearProtocol->setStyleSheet("QPushButton::hover {background-color: white; border: 1px solid #ddd;}");

    connect(ClearProtocol, SIGNAL(clicked()), this, SLOT(clear_protocol()));

    if(StyleApp == "fusion")
    {
        OpenProtocol = new QPushButton(this);
        OpenProtocol->setIcon(QIcon(":/images/flat/open_flat.png"));
        OpenProtocol->setObjectName("Transparent");
        OpenProtocol->setIconSize(QSize(24,24));
        OpenProtocol->setFlat(true);
        OpenProtocol->setToolTip(tr("Open Protocol"));
        OpenProtocol->setStyleSheet("QPushButton::hover {background-color: white; border: 1px solid #ddd;}");
    }
    else
    {
        //OpenProtocol = new QToolButton(QIcon(":/images/open_new.png"),tr("Open"),this);
    }
    connect(OpenProtocol, SIGNAL(clicked()), this, SLOT(open_protocol()));
    LastProtocol = new QPushButton(QIcon(":/images/list.png"),"",this);
    LastProtocol->setToolTip(tr("Last Protocols"));
    //LastProtocol->setFixedWidth(13);
    if(StyleApp == "fusion")
    {
        LastProtocol->setObjectName("Transparent");
        LastProtocol->setFlat(true);
        LastProtocol->setIconSize(QSize(13,24));
        LastProtocol->setMaximumWidth(13);
        LastProtocol->setStyleSheet("QPushButton::hover {background-color: white; border: 1px solid #ddd;}");
    }
    connect(LastProtocol, SIGNAL(clicked(bool)), this, SLOT(slot_LastProtocols()));

    //IsEqual = new QPushButton(QIcon(":/images/list.png"),"",this);
    //connect(IsEqual, SIGNAL(clicked(bool)), this, SLOT(slot_ProtocolIsEqual()));

    layout_protocol->addWidget(NameProt,1);
    layout_protocol->addWidget(ClearProtocol);
    layout_protocol->addWidget(OpenProtocol);
    layout_protocol->addWidget(LastProtocol);
    //layout_protocol->addWidget(IsEqual);

    selPlate = new QMainWindow();

    Tab_Protocol = new QTabWidget();

    //group_Program = new QGroupBox(this);
    //QHBoxLayout *layout_program = new QHBoxLayout;
    //group_Program->setLayout(layout_program);
    program_spl = new QSplitter(Qt::Horizontal, this);
    program_spl->setHandleWidth(6);
    program_spl->setChildrenCollapsible(false);

    plot_Program = new Plot_ProgramAmpl(this);
    scheme_Program = new Scheme_ProgramAmpl(QColor(1,1,1,1), this);
    //layout_program->setSpacing(2);
    //layout_program->setMargin(2);
    //layout_program->addWidget(plot_Program);
    //layout_program->addWidget(scheme_Program);
    program_spl->addWidget(plot_Program);
    program_spl->addWidget(scheme_Program);

    info_Protocol = new Info_Protocol(this);
    info_Protocol->setFont(qApp->font());
    info_Protocol->header()->setFont(qApp->font());

    runtime_Program = new RunTime(this);    
    //group_online = new QGroupBox(this);
    //QHBoxLayout *layout_online = new QHBoxLayout;
    //group_online->setLayout(layout_online);
    //layout_online->addWidget(runtime_Program);

    comments = new QTextEdit(this);
    comments->copyAvailable(true);

    Tab_Protocol->setStyleSheet("QTabWidget::tab-bar {left: 10px;}");
    //Tab_Protocol->setStyleSheet("background-color: rgb(240, 240, 240);");
    Tab_Protocol->addTab(runtime_Program,tr("On-Line"));
    //Tab_Protocol->addTab(group_Program,"Program");
    Tab_Protocol->addTab(program_spl,tr("Program"));
    Tab_Protocol->addTab(selPlate,tr("Plate"));
    Tab_Protocol->addTab(info_Protocol,tr("Information"));
    Tab_Protocol->addTab(comments, tr("Comments"));

    //Tab_Protocol->setTabEnabled(0,false);



    //QFont f = app_font;
    //f.setBold(false);
    //f.setPointSize(8);
    //Tab_Protocol->setFont(f);

    layout->addLayout(layout_protocol);
    //layout->addWidget(NameProt);
    //layout->addWidget(info_Prot,1);
    layout->addWidget(Tab_Protocol,1);

}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Run::Show_Contract()
{
    Contract_Dev->exec();
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Run::ShowListDevice()
{
    list_dev->dev_state = Dev_State;
    list_dev->exec();    
}

//-----------------------------------------------------------------------------
//--- create StatusBar
//-----------------------------------------------------------------------------
void Run::createStatusBar()
{
    QString text;

    main_progress = new QProgressBar();

    if(StyleApp != "fusion")
    {
        main_progress->setStyleSheet("QProgressBar:horizontal {border: 1px solid gray; border-radius: 3px; background: #E8E8E8; padding: 2px;}"
                    "QProgressBar::chunk:horizontal {background: qlineargradient(x1: 0, y1: 0.1, x2: 1, y2: 0.1, stop: 0 #C4FFC4, stop: 1 lime);}");
    }

    main_progress->setFixedWidth(600);


    //QLabel *version_label = new QLabel(this);
    //version_label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    //version_label->setText("ver. " + QCoreApplication::applicationVersion() + " ");
    text = VERS;
    //version_label->setText("ver. " + text);
    //version_label->setText("");

    //Label_status.setText(list_status.join(" "));

    statusBar()->addWidget(main_progress);

    //statusBar()->addWidget(version_label);
    //statusBar()->addWidget(&Label_status);

    main_progress->setTextVisible(false);
    main_progress->setRange(0,100);
    PrBar_status = new QLabel(this);
    PrBar_status->setAlignment(Qt::AlignCenter);

    QHBoxLayout *probar_layout = new QHBoxLayout();

    main_progress->setLayout(probar_layout);
    probar_layout->addWidget(PrBar_status);

    //
    run_progress = new QProgressBar();

    run_progress->setStyleSheet(
    "QProgressBar:horizontal {border: 1px solid gray; background: #FAFAFA; padding: 2px;}"
    "QProgressBar::chunk:horizontal {background: qlineargradient(x1: 0, y1: 0.1, x2: 1, y2: 0.1, stop: 0 #B5D8E6, stop: 1 #409BBD);}"
    );
    run_progress->setFixedWidth(600);
    run_progress->setTextVisible(false);
    run_progress->setRange(0,100);

    statusBar()->addWidget(run_progress);

    PrBar_run_status = new QLabel(this);
    PrBar_run_status->setAlignment(Qt::AlignCenter);
    QHBoxLayout *probar_layout_1 = new QHBoxLayout();

    run_progress->setLayout(probar_layout_1);
    probar_layout_1->addWidget(PrBar_run_status);

    //QFont f = app_font;
    //f.setBold(false);
    //statusBar()->setFont(f);

    statusBar()->setSizeGripEnabled(false);

}
//-----------------------------------------------------------------------------
//--- createSelectTubes()
//-----------------------------------------------------------------------------
void Run::createSelectTubes()
{
    sel = NULL;     // pointer for Select_Tubes object
    sel = new Select_tubes(tr("Test&&Sample&&Select"),this);
    sel->setAllowedAreas(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
    sel->setFeatures(QDockWidget::NoDockWidgetFeatures);
    //sel->setAllowedAreas(Qt::NoDockWidgetArea);
    //sel->setFloating(true);
    sel->hide();
    //sel->mTitleBar->list_MainWindow.append(Fluor_Chart);
    sel->mTitleBar->list_MainWindow.append(selPlate);
    sel->mTitleBar->updateStyleChange();
    sel->mTitleBar->aClose->setVisible(false);
    sel->mTitleBar->aFloat->setVisible(false);
    //sel->mTitleBar->setVisible(false);
    //sel->setWindowFlags(sel->windowFlags() | Qt::FramelessWindowHint);

    sel->Select_Grid->setFont(qApp->font());

    sel->map_TestTranslate = &Map_TestTranslate;

}

//-----------------------------------------------------------------------------
//--- create FluorToolBar
//-----------------------------------------------------------------------------
void Run::createFluorToolBars()
{
    int i;
    QString fluor_name[COUNT_CH] = FLUOR_NAME;
    QIcon icon;

    fluor_toolbar = Fluor_Chart->addToolBar(tr("File"));
    Fluor_Chart->setContextMenuPolicy(Qt::NoContextMenu);

    QWidget* spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    //... actions ...
    select = new QAction(QIcon(":/images/color.png"), tr("Select"), this);
    video = new QAction(QIcon(":/images/video.png"), tr("Video"), this);

    UpMarker = new QAction(QIcon(":/images/marker_up.png"), tr("marker up"), this);
    connect(UpMarker, SIGNAL(triggered(bool)), this, SLOT(Up_sizeMarker()));

    DownMarker = new QAction(QIcon(":/images/marker_down.png"), tr("marker down"), this);
    connect(DownMarker, SIGNAL(triggered(bool)), this, SLOT(Down_sizeMarker()));

    fluor_toolbar->addWidget(spacer);
    fluor_toolbar->addAction(UpMarker);
    fluor_toolbar->addAction(DownMarker);
    fluor_toolbar->addSeparator();
    fluor_toolbar->addAction(select);
    fluor_toolbar->addAction(video);
    //fluor_toolbar->addSeparator();
    //fluor_toolbar->addWidget(fluor_select);

    connect(select, SIGNAL(triggered(bool)), this, SLOT(slot_Select()));

}
//-----------------------------------------------------------------------------
//--- Up_sizeMarker()
//-----------------------------------------------------------------------------
void Run::Up_sizeMarker()
{
    if(!prot || !plot_fluor) return;

    plot_fluor->size_symbol++;
    plot_fluor->Change_SizeMarker();
}
//-----------------------------------------------------------------------------
//--- Down_sizeMarker()
//-----------------------------------------------------------------------------
void Run::Down_sizeMarker()
{
    if(!prot || !plot_fluor) return;
    if(plot_fluor->size_symbol == 0) return;

    plot_fluor->size_symbol--;
    plot_fluor->Change_SizeMarker();
}
//-----------------------------------------------------------------------------
//--- slot_Select
//-----------------------------------------------------------------------------
void Run::slot_Select()
{
    Tab_Protocol->setCurrentIndex(2);
    if(!sel->isHidden()) return;

    sel->show();
    sel->raise();
    sel->activateWindow();

    selPlate->addDockWidget(Qt::TopDockWidgetArea, sel);

}

//-----------------------------------------------------------------------------
//--- readSettings
//-----------------------------------------------------------------------------
void Run::readSettings()
{
    int i;
    QStringList list_str;
    QAbstractButton *a_button;
    Color_ToolButton *button;
    QColor color;
    unsigned int val;
    bool ok;

    //... MainWindow ..........................................................
    ApplSettings->beginGroup("MainWindow");

    int width = (main_spl->size().width() - 4)/2;
    QList<int> list_spl;
    QString text = ApplSettings->value("splitter","").toString();
    if(text.isEmpty()) list_spl << width << width;
    else
    {
        list_str = text.split(" ");
        for(i=0; i<list_str.count(); i++) list_spl.append(list_str.at(i).toInt());
    }
    main_spl->setSizes(list_spl);

    text = ApplSettings->value("chart_splitter","").toString();
    if(!text.isEmpty())
    {
        list_str.clear();
        list_spl.clear();
        list_str = text.split(" ");
        for(i=0; i<list_str.count(); i++) list_spl.append(list_str.at(i).toInt());
        chart_spl->setSizes(list_spl);
    }

    text = ApplSettings->value("control_splitter","").toString();
    if(!text.isEmpty())
    {
        list_str.clear();
        list_spl.clear();
        list_str = text.split(" ");
        for(i=0; i<list_str.count(); i++) list_spl.append(list_str.at(i).toInt());
        control_spl->setSizes(list_spl);
    }

    text = ApplSettings->value("program_splitter","").toString();
    if(!text.isEmpty())
    {
        list_str.clear();
        list_spl.clear();
        list_str = text.split(" ");
        for(i=0; i<list_str.count(); i++) list_spl.append(list_str.at(i).toInt());
        program_spl->setSizes(list_spl);
    }

    ApplSettings->endGroup();

    //... Server_USB ..........................................................
    ApplSettings->beginGroup("Server_USB");
    ip_addres = ApplSettings->value("address","localhost").toString();
    port_serverUSB = ApplSettings->value("port","9011").toInt();
    ApplSettings->endGroup();
    //.........................................................................

    //... ColorRejime ................................................................
    ApplSettings->beginGroup("ColorRejime");
    i = 0;
    foreach(a_button, sel->Fluor_Buttons->buttons())
    {
        button = (Color_ToolButton*)a_button;
        text = ApplSettings->value(QString("color_%1").arg(i), "").toString();
        i++;
        if(text.isEmpty()) continue;
        val = text.toInt(&ok,16);
        if(!ok) val = 0x0;
        color = val;
        if(color.isValid()) button->color = color;
    }
    emit sel->Fluor_Buttons->buttonClicked(0);
    ApplSettings->endGroup();
}

//-----------------------------------------------------------------------------
//--- writeSettings
//-----------------------------------------------------------------------------
void Run::writeSettings()
{
    QString text = "";
    int i;    
    QAbstractButton *a_button;
    Color_ToolButton *button;
    unsigned int val;

    if(!flag_SaveLocationWin) return;

    //--- MainWindow ---
    ApplSettings->beginGroup("MainWindow");

    QList<int> list_spl = main_spl->sizes();
    for(i=0; i<list_spl.count(); i++) text += QString::number(list_spl.at(i)) + " ";
    ApplSettings->setValue("splitter", text);

    list_spl.clear();
    text = "";
    list_spl = chart_spl->sizes();
    for(i=0; i<list_spl.size(); i++) text += QString::number(list_spl.at(i)) + " ";
    ApplSettings->setValue("chart_splitter", text);

    list_spl.clear();
    text = "";
    list_spl = control_spl->sizes();
    for(i=0; i<list_spl.size(); i++) text += QString::number(list_spl.at(i)) + " ";
    ApplSettings->setValue("control_splitter", text);

    list_spl.clear();
    text = "";
    list_spl = program_spl->sizes();
    for(i=0; i<list_spl.size(); i++) text += QString::number(list_spl.at(i)) + " ";
    ApplSettings->setValue("program_splitter", text);

    ApplSettings->endGroup();

    //--- ColorRejime ---
    ApplSettings->beginGroup("ColorRejime");
    i=0;
    foreach(a_button, sel->Fluor_Buttons->buttons())
    {
        button = (Color_ToolButton*)a_button;
        val = button->color.rgb() & 0xffffff;
        text = QString::number(val,16);
        ApplSettings->setValue(QString("color_%1").arg(i), text);
        i++;
    }
    ApplSettings->endGroup();
}

//-----------------------------------------------------------------------------
//--- create_RequestStatusInfo(QString, short)
//-----------------------------------------------------------------------------
void Run::create_RequestStatusInfo(QString request, short status)
{
    int i;
    QString text;
    CodeErrors dev_error;
    QTreeWidgetItem *item;
    QList<QTreeWidgetItem *> list = Error_Info->findItems(request, Qt::MatchContains, 0);

    if(list.isEmpty())
    {
        // 0. Request
        item = new QTreeWidgetItem(Error_Info);
        item->setText(0, request);
    }
    else
    {
        item = list.at(0);
    }

    if(status) item->setTextColor(0, Qt::red);
    else item->setTextColor(0, Qt::black);

    // 1. Value Error
    i = status;
    i = i & 0xffff;
    //text = QString::number(i,16);
    text = QString("0x%1").arg(i,1,16);
    item->setText(1, text);

    // 2. uC
    text = "";
    if(status)
    {
        if(status & 0x800) text += "USB ";
        if(status & 0x1000) text += "Optical ";
        if(status & 0x2000) text += "Temp ";
        if(status & 0x4000) text += "Motor ";
        if(status & 0x8000) text += "TFT ";

        text = text.trimmed();
        text = text.replace(" ",",");

        item->setTextAlignment(2,Qt::AlignHCenter);
    }
    item->setText(2, text);

    // 3. Description    
    dev_error = (CodeErrors)(status & 0x7ff);

    //qDebug() << "dev_error:" << status;

    switch(dev_error)
    {
    case NONE:                  text = "";  break;
    case INITIALISE_ERROR:      text = tr("Error of initialization");   break;
    case USB_ERROR:             text = tr("Error of USB");  break;
    case CAN_ERROR:             text = tr("Error of CAN");  break;
    case USBBULK_ERROR:         text = tr("Error of the reading/writing a bulk in USB");    break;
    case NOTREADY:              text = tr("Device don't ready, waiting termination of initialisation...");  break;
    case DEVHW_ERROR:           text = tr("Device ERROR! Necessary to switch off device...");   break;
    case OPENINRUN:             text = tr("Opening device! Programm is running...");    break;
    case MOTORALARM:            text = tr("Drive ERROR!");  break;
    case STARTONOPEN:           text = tr("ERROR - Device open!"); emit sErrorInfo(text);  break;
    case PRERUNFAULT_ERROR:     text = tr("PreRUN ERROR!"); break;
    case PRERUNSEMIFAULT_ERROR: text = tr("PreRUN WARNING!");   break;
    case DATAFRAME_ERROR:       text = tr("ERROR - Get block data!"); break;
    case LEDSSETUP_ERROR:       text = tr("ERROR - LED drive!");    break;

    case UNKNOWN_ERROR:
    default:                    text = tr("Unknown ERROR!");    break;

    case EXTERNAL_TIMEOUT:      text = tr("External TIMEOUT!"); break;
    }

    item->setText(3, text);

    if(status && Tab_Control->currentWidget() != CMD_Box)  Tab_Control->setCurrentWidget(Error_Info);

}
//-----------------------------------------------------------------------------
//--- slot_GetErrorInfo(QString)
//-----------------------------------------------------------------------------
void Run::slot_GetErrorInfo(QString err_info)
{
    emit finished_WaitProcess();

    QString text = tr("Attention! ") + err_info;

    message.setStandardButtons(QMessageBox::Ok);
    message.setIcon(QMessageBox::Warning);
    message.button(QMessageBox::Ok)->animateClick(7000);
    message.setText(text);
    message.exec();
}
//-----------------------------------------------------------------------------
//--- addProtocol_toList(QString)
//-----------------------------------------------------------------------------
void Run::addProtocol_toList(QString fn)
{
    int i,k;
    QString text;
    int count_border = 10;
    QStringList list_fn;
    QFileInfo fi;

    ApplSettings->beginGroup("LastProtocols");

    QStringList list = ApplSettings->childKeys();

    for(i=0; i<list.count(); i++)
    {
        text = ApplSettings->value(list.at(i),"").toString();
        fi.setFile(text);
        if(!fi.exists()) continue;
        if(fi.baseName() == "manual_setup") continue;
        list_fn.append(text);
    }

    k = list_fn.indexOf(fn);

    if(k >= 0) list_fn.removeAt(k);
    list_fn.insert(0,fn);

    ApplSettings->endGroup();
    ApplSettings->remove("LastProtocols");

    ApplSettings->beginGroup("LastProtocols");
    k = 0;
    for(i=0; i<list_fn.count(); i++)
    {
        text = list_fn.at(i).trimmed();
        if(text.isEmpty()) continue;

        ApplSettings->setValue(QString::number(k), text);
        k++;
        if(count_border <= k) break;
    }
    ApplSettings->endGroup();
}

//-----------------------------------------------------------------------------
//--- slot_LastProtocols()
//-----------------------------------------------------------------------------
void Run::slot_LastProtocols()
{
    int i;
    QMenu menu;
    QString text;
    QAction *act = NULL;
    QFileInfo fi;

    menu.setStyleSheet("QMenu::item:selected {background-color: #d7d7ff; color: black;}");
    menu.setFont(qApp->font());

    ApplSettings->beginGroup("LastProtocols");
    QStringList list = ApplSettings->childKeys();

    for(i=0; i<list.count(); i++)
    {
        text = ApplSettings->value(list.at(i),"").toString();
        if(text.isEmpty()) continue;
        // check of file exists...
        fi.setFile(text);
        if(!fi.exists()) continue;
        //...
        act = new QAction(text, this);
        menu.addAction(act);
    }
    text = "";
    if(menu.actions().size())
    {
        act = menu.exec(QCursor::pos());
        if(act) text = act->text();
    }
    ApplSettings->endGroup();

    if(act)
    {
        QFile file(text);
        if(file.exists())
        {
            Open_Protocol(text);
        }
        else
        {
            text = tr("FILE (") + text + tr(")  IS ABSENT!");
            QMessageBox::warning(this, tr("Attention"), text);
        }
    }

    menu.clear();
}

//======================== NETWORK ============================================

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Run::slotConnected()
{
    //qDebug() << "Received the connected() signal";

    //map_Logged.clear();
    //map_Logged.insert(DTMASTERLOG_DATA, "Received the connected() signal");
    //slotSendToServer(DTMASTERLOG_REQUEST);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Run::slotDisconnected()
{
    //qDebug() << "Received the Disconnected() signal";
    flag_ContractDev = false;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Run::Contract_Check()
{
    /*int id = 2;         // uC Optic
    QString cmd = "CRYC";

    map_CMD.clear();
    map_CMD.insert(EXECCMD_CMD, cmd);
    map_CMD.insert(EXECCMD_UC, QString::number(id));

    slotSendToServer(EXECCMD_REQUEST);*/

    map_CRYPTO.clear();
    map_CRYPTO.insert(CRYPTO_CTRL,"CRYC");

    QByteArray ba("yes");
    map_CRYPTO.insert("RejimeCryptoDevice", ba);

    send_CRYPTO();
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Run::slot_CheckContractDevice()
{
    Check_ContractDevice(&map_CRYPTO);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Run::Check_ContractDevice(QMap<QString, QByteArray> *map)
{
    int i;
    QByteArray ba;
    QString text, answer;
    int first_par, second_par;
    QStringList list;
    int id_uC;
    int status = -1;
    bool ok_0, ok_1;
    int value;
    int count_days;
    int border_days = 8;    

    id_Reagent = rNormal;

    QString name_Dev = map_InfoDevice.value(INFODEV_serName,"");
    ba = map->value(CRYPTO_DATA,"");
    answer = QString::fromUtf8(ba);
    list = answer.split(";");

    if(answer.trimmed().isEmpty() || answer.contains("?") || list.size() < 2) flag_ContractDev = false;
    else flag_ContractDev = true;

    if(list.size() > 1) {first_par = list.at(0).toInt(&ok_0); second_par = list.at(1).toInt(&ok_1);}


    //qDebug() << "Check_ContractDevice:" << list << flag_ContractDev << ok_0 << ok_1;

    if(flag_ContractDev && ok_0 && ok_1)
    {
        NameDev->setText("<img src=\":/images/flat/unlock_16.png\"/>  " + name_Dev);
        count_days = second_par;
        Contract_Dev->count_days = second_par;

        if(first_par == second_par)
        {
            id_Reagent = rTimeLimited;
            if(second_par == 0) id_Reagent = rLimited;
        }
        if(first_par > 0 && second_par == 0)  id_Reagent = rTestLimited;

        //qDebug() << "id_Reagent: " << id_Reagent << count_days;

        switch(id_Reagent)
        {
        case rTimeLimited:                  // Time Limited
                            if(count_days > 0 && count_days <= border_days)
                            {
                                text = tr("Attention! According to the contract restrictions, the device will be blocked in: ") + "  " +
                                QString::number(count_days) + tr(" days.") + "\r\n" +
                                        tr("Please apply to seller!");
                                message.setIcon(QMessageBox::Information);
                                message.setStandardButtons(QMessageBox::Ok);
                                message.setText(text);
                                message.exec();
                            }
                            break;

        case rLimited:                      // Limited ?
        case rTestLimited:                  // Test Limited
                            Read_CryptoDevice();
                            break;

        default:            break;
        }

    }






}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Run::slot_RefreshCMD(QMap<QString, QString> *map)
{
    //qDebug() << "slot_RefreshCMD: " << *map;
    int i;
    QString cmd, answer;
    QString text;
    int id_uC;
    int status = -1;
    bool ok;
    int value;
    int count_days;
    int border_days = 8;

    QString name_Dev = map_InfoDevice.value(INFODEV_serName,"");

    cmd = map->value(EXECCMD_CMD,"");
    answer = map->value(EXECCMD_ANSWER, "");
    id_uC = map->value(EXECCMD_UC,"").toInt(&ok);
    status = map->value("status","").toInt(&ok);
    if(!ok) status = -1;

    // 1. CRYC
    /*if(cmd == "CRYC")
    {
        //Device_Locked->setPixmap(lock);
        //Device_Locked->setVisible(true);
        //NameDev->setText(tr("<img src=\":/images/flat/clear_flat_16.png\"/> Terminal settings"));
        //NameDev->setText("<img src=":/myimage.png">Hello!");
        //NameDev->setText("some senseless text with an image <img src=":/images/flat/clear_flat_16.png"/>");

        if(answer.contains("?")) flag_ContractDev = false;
        else flag_ContractDev = true;

        if(status == 0 && flag_ContractDev && answer.trimmed().length())        
        {
            NameDev->setText("<img src=\":/images/flat/unlock_16.png\"/>  " + name_Dev);
            count_days = answer.toInt(&ok);
            Contract_Dev->count_days = count_days;

            switch(id_Type)
            {
            case tLimited:
                            // 1. Limited on Functions
                            Read_CryptoDevice();
                            break;

            default:
            case tMajor:
                            // 2. Limited on Time (Major)
                            if(ok)
                            {
                                if(count_days <= 0)
                                {
                                    text = tr("Attention! According to the contract restrictions, you have exceeded the quota of permitted runs") +
                                                "\r\n" + tr("Please apply to seller!");
                                    message.setIcon(QMessageBox::Warning);
                                    NameDev->setText("<img src=\":/images/flat/lock_16.png\"/>  " + name_Dev);
                                    NameDev->setStyleSheet("QLabel{background-color: rgb(255, 128, 128)}");
                                }
                                if(count_days > 0 && count_days <= border_days)
                                {
                                    text = tr("Attention! According to the contract restrictions, the device will be blocked in: ") + "  " +
                                    QString::number(count_days) + tr(" days.") + "\r\n" +
                                            tr("Please apply to seller!");
                                    message.setIcon(QMessageBox::Information);
                                }
                                if(count_days <= border_days)
                                {
                                    message.setStandardButtons(QMessageBox::Ok);
                                    message.setText(text);
                                    message.exec();
                                }
                            }

                            break;
            }
        }
    }*/

    // 2. FACS
    if(cmd == "FACS")
    {
        if(flag_FACS == true)
        {
            value = answer.toInt(&ok);
            if(status != 0 || !ok) {flag_FACS = false; return;}

            ActiveCh_Dev = 0;
            for(i=0; i<COUNT_CH; i++)
            {
                if(value & (0x01<<i)) ActiveCh_Dev += 0x01<<4*i;
            }

            if(ActiveCh_Dev > 0) slot_LastRun();


            //qDebug() << "FACS: " << ActiveCh_Dev << value;
        }
        flag_FACS = false;
    }

}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Run::slotReadyRead()
{    
    int i;
    int index;
    QString text, str;
    QString key_str;
    QByteArray ba;
    int id;
    int fn;
    int value;
    bool ok;
    bool skip;
    short status;
    QBasicTimer *timer_req;
    char status_byte[] = {0x01};
    QByteArray status_ba;

    rt_Test *ptest;

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
        m_nNextBlockSize = 0;


        QString answer;
        in >> answer >> status;

        status_ba.setNum(status);

        //qDebug() << "get Request: " << answer << status;

        if(answer == CONNECT_REQUEST)
        {
            in >> map_temp;
            map_temp.clear();
        }


        if(!List_Requests.contains(answer))     // Event from Device...
        {

            in >> map_temp;
            if(answer == DEVICE_REQUEST)
            {
                // 1. Press Button_Run ...
                if(map_temp.keys().contains(PRESS_BTN_RUN))
                {
                    if(map_temp.value(PRESS_BTN_RUN) == "1")
                    {
                        emit sRunBarcode();
                    }
                }
            }
            map_temp.clear();
            break;
        }

        if(Map_TimerRequest.contains(answer))
        {
            timer_req = Map_TimerRequest.value(answer);
            timer_req->stop();
        }

        //
        skip = false;
        if(answer == EXECCMD_REQUEST)
        {
            in >> map_CMD;
            if(map_CMD.value(EXECCMD_CMD,"") == "CRYC") skip = true;
        }

        if(!skip) create_RequestStatusInfo(answer, status);   // Error Info ->

        index = Catalogue_Requests.indexOf(answer);

        //qDebug() << "Request: " << answer << status;

        switch(index)
        {
        default:
                    if(answer == CONTINUE_REQUEST)
                    {
                        emit sSet_PauseRejime(1);
                    }
                    break;

        case 0:             //INFO_DATA

                    map_InfoData.clear();
                    in >> map_InfoData;
                    if(map_InfoDevice.value(INFODEV_serName,"").trimmed().isEmpty()) {map_InfoData.clear(); break;}
                    //qDebug() << "Info" << status << map_InfoData;
                    if(map_InfoData.isEmpty() || status == 5) break;
                    emit sRefresh(&map_InfoData);
                    break;
        case 1:             //INFO_DEVICE

                    map_InfoDevice.clear();
                    in >> map_InfoDevice;
                    //qDebug() << "Dev" << status << Try_ReadInfoDevice << map_InfoDevice.value(INFODEV_TempProgram,"");

                    //

                    if(map_InfoDevice.isEmpty() || status == 5)
                    {                        
                        Try_ReadInfoDevice++;
                        if(Try_ReadInfoDevice > 4)
                        {
                            message.setStandardButtons(QMessageBox::Ok);
                            message.setIcon(QMessageBox::Critical);
                            text = tr("Cannot read the INFO_DEVICE...") + "\r\n";
                            text += tr("Please, overload the device!");
                            message.button(QMessageBox::Ok)->animateClick(5000);
                            message.setText(text);
                            message.exec();
                            break;
                        }
                        else  QTimer::singleShot(1000,this,SLOT(slotGetInfoDevice()));
                    }
                    else
                    {
                        flag_EnableDev = false;
                        Try_ReadInfoDevice = 0;
                        slotRefreshInfoDevice();

                        Read_AllSectors();                  // read all sectors with additional parameters
                        //slotResume();

                        Contract_Check();
                        //QTimer::singleShot(1000,this,SLOT(Contract_Check()));   // Check on Contract

                        if(!dev_timer.isActive()) dev_timer.start(1000);    // reconnect
                    }

                    //slot_ReadSector(0x0419, "median");  // read sector with additional parameters: median offset                   


                    //qDebug() << "OK - Read InfoDevice...";

                    break;

        case 2:             //RUN

                    //qDebug() << answer;
                    if(Create_RunProtocol()) flag_RunLoaded = true;
                    //update_ProtocolInfoToWeb("Start RUN");     // to Web server
                    break;

        case 3:             //STOP

                    qDebug() << answer;
                    runtime_Program->scheme_RTProgram->draw_Program(prot);
                    //update_ProtocolInfoToWeb("Stop RUN");     // to Web server
                    break;

        case 4:             //PAUSE

                    //qDebug() << answer;
                    emit sSet_PauseRejime(2);
                    break;

        case 5:             // MEASURE                    
                    map_Measure.clear();
                    in >> fn >> map_Measure;
                    //qDebug() << map_Measure;
                    if(slot_GetMeasure(fn))
                    {
                        if(flag_WebProtocol) slot_SendMeasureToWeb(fn);
                    }
                    emit sRefreshFluor();                    
                    break;
        case 6:             // OPEN                    
                    //qDebug() << answer;
                    OpenClose_Box->setEnabled(true);
                    emit finished_WaitProcess();
                    if(flag_ActivePoint && action_event && action_event->point_action->Unique_Name == OPENBLOCK) PA_Ok();
                    break;
        case 7:             // CLOSE                    
                    //qDebug() << "CLOSE BLOCK: " << answer << status;
                    map_Closed.clear();
                    in >> map_Closed;
                    prot->barcode = map_Closed.value(barcode_name).toStdString();
                    OpenClose_Box->setEnabled(true);
                    emit finished_WaitProcess();
                    if(!status && flag_ClosePrerunRun) slot_PreRUN();
                    if(status && flag_ClosePrerunRun && flag_ActivePoint) PA_Error("Close block Error");

                    if(flag_ActivePoint && action_event && action_event->point_action->Unique_Name == CLOSEBLOCK) PA_Ok();
                    if(flag_ActivePoint &&
                       action_event &&
                       action_event->point_action->Unique_Name == GET_BARCODE) PA_BarCode();

                    flag_ClosePrerunRun = false;

                    break;
        case 8:             // EXECCMD_REQUEST                    
                    //in >> map_CMD;

                    if(flag_ActivePoint &&
                       action_event &&
                       action_event->point_action->Unique_Name == CHANGE_HEIGHT_TUBES)
                    {
                        PA_RunTubeHeiht(map_CMD.value(EXECCMD_ANSWER,""));
                        break;
                    }

                    cmd_Edit->insertPlainText(map_CMD.value(EXECCMD_ANSWER));
                    cmd_Edit->append(">");
                    //qDebug() << map_CMD;

                    map_CMD.insert("status", QString::number(status));
                    slot_RefreshCMD(&map_CMD);

                    break;
        case 9:             // PRERUN_REQUEST                    
                    emit finished_WaitProcess();
                    if(!status) emit sRunAfterPrerun();
                    else
                    {

                    }
                    if(status && flag_ActivePoint) PA_Error("PreRun Error");

                    break;
        case 10:            // GETPIC_REQUEST                    
                    emit finished_WaitProcess();
                    map_PictureData.clear();
                    in >> map_PictureData;                    
                    if(map_PictureData.contains(GETPIC_VIDEO)) emit sSendPicture(&map_PictureData);     // Video
                    if(!map_PictureData.contains(GETPIC_VIDEO) &&                                       // Expo
                        map_PictureData.contains(GETPIC_DATA)) emit sSendDigit(&map_PictureData);                                                                           // Expo
                    break;
        case 11:            // SAVEPAR_REQUEST
                    map_SavePar.clear();
                    in >> map_SavePar;
                    //qDebug() << "map_SavelPar: " << map_SavePar;
                    emit finished_WaitProcess();
                    if(flag_ActivePoint &&
                            action_event &&
                            action_event->point_action->Unique_Name == BACKUP_EXPOSURE) PA_Ok();
                    break;

        case 12:            // SECTORREAD_REQUEST
                    emit finished_WaitProcess();
                    map_Sector.clear();

                    //qDebug() << "SECTORREAD_REQUEST: " << answer << status;



                    in >> map_Sector;
                    map_Sector.insert("status", QString("%1").arg((short)status));

                    slot_Processing_ReadSector(map_Sector);

                    //QTimer::singleShot(100, this, SLOT(slot_Read_CurrentSector()));

                    break;
        case 13:            // SECTORWRITE_REQUEST
                    emit finished_WaitProcess();
                    slotSendToServer(INFO_DEVICE);
                    break;


        case 14:            // HTMEAS_REQUEST
                    //qDebug() << "HTMEAS_REQUEST: " << answer << status;
                    if(status == 0) in >> value;
                    else value = -1;
                    //qDebug() << "HTMEAS_REQUEST value: " << value;
                    emit finished_WaitProcess();
                    Preference_Box->setDisabled(false);
                    emit sMeasureTubeHeiht(value);
                    break;

        case 15:            // HIGHTUBE_SAVE
                    //qDebug() << "HIGHTUBE_SAVE: ";
                    emit sStatusMeasureTubeHeiht(status);
                    break;

        case 16:            // GETALLVIDEO_REQUEST

                    emit finished_WaitProcess();
                    List_Requests.removeOne(GETALLVIDEO_REQUEST);

                    if(flag_SavingOpticDev)
                    {
                        flag_SavingOpticDev = false;
                        if(flag_LastRun) slot_ChangeStateDevice(Dev_State);
                        else CreateMeasure_And_GoToAnalysis();
                    }                    
                    break;

        case 17:            // CRYPTO_REQUEST
                    emit finished_WaitProcess();
                    map_CRYPTO.clear();
                    in >> map_CRYPTO;

                    map_CRYPTO.insert("status", status_ba);

                    //qDebug() << "CRYPTO_REQUEST: " << status << map_CRYPTO.value(CRYPTO_CTRL,"") << map_CRYPTO.value("status","");
                    //qDebug() << "RejimeCryptoDevice: " << map_CRYPTO.value("RejimeCryptoDevice");

                    // 1. rejime CryptoDevuce (read/write for Limited on Function
                    if(map_CRYPTO.value("RejimeCryptoDevice") == "yes")
                    {
                        if(map_CRYPTO.value(CRYPTO_CTRL,"").trimmed() == "CRYR")
                        {                            
                            //qDebug() << "CRYPTO_REQUEST: " << map_CRYPTO;

                            QTimer::singleShot(300, this, SLOT(slot_CheckPubkeyCrypto()));
                            //Check_PubkeyCrypto(&map_CRYPTO);
                            break;
                        }
                        if(map_CRYPTO.value(CRYPTO_CTRL,"").trimmed() == "CRYS")
                        {
                            //qDebug() << "CRYS: " << map_CRYPTO.value(CRYPTO_DATA);
                            if(id_Reagent == rLimited && status == 0) id_Reagent = rTestLimited;
                            break;
                        }
                        if(map_CRYPTO.value(CRYPTO_CTRL,"").trimmed() == "CRYC")
                        {
                            //qDebug() << "CRYC: " << map_CRYPTO.value(CRYPTO_DATA);
                            QTimer::singleShot(250, this, SLOT(slot_CheckContractDevice()));
                            //Check_ContractDevice(&map_CRYPTO)
                            break;
                        }

                        break;
                    }

                    // 2. other rejimes: Contract module
                    emit sRead_CRYPTO(&map_CRYPTO);
                    break;
        }

        index = List_Requests.indexOf(answer);
        if(index >= 0) List_Requests.removeAt(index);

        //...
        if(answer == SECTORREAD_REQUEST) slot_Read_CurrentSector();


        //...
        if(answer == GETALLVIDEO_REQUEST && SaveImage_rejime->isChecked())
        {
            SaveImage_rejime->setCheckState(Qt::Unchecked);
        }

        //... continuous data request ...
        if(answer == MEASURE_REQUEST)
        {
            if(fn_dev > FN)
            {
                active_ch = prot->active_Channels;
                slotSendToServer(MEASURE_REQUEST);
            }
            else
            {                
                emit finished_WaitProcess();
                Display_ProgressBar(0,"");
                if(flag_LastRun)
                {
                    ptest = (rt_Test*)prot->tests.at(0);
                    if(ptest->header.Type_analysis == 0x0020)   // Verification protocol
                    {
                        flag_SavingOpticDev = true;             //  send request: GETALLVIDEO_REQUEST
                        slot_ReadOpticDev();                    //
                    }
                    else slot_ChangeStateDevice(Dev_State);
                }

                if(flag_SaveOnLine && !OnLine_FileName.isEmpty() && !flag_LastRun)
                {
                    //qDebug() << "SaveOnLine: " << FN << OnLine_FileName;
                    Create_MeasureProtocol(OnLine_FileName, true);
                }
            }
        }
        //... reload Info_Device after SavePar ...
        if(answer == SAVEPAR_REQUEST)
        {
            id = map_SavePar.value(ID_PARAM, "-1").toInt(&ok);
            if(ok)
            {
                switch(id)
                {
                case 0:     // video
                            if(map_Median.size() == 0) slotSendToServer(INFO_DEVICE);
                            else
                            {
                                text = map_Median.value("median", "");
                                slot_WriteSector(0x0419, text);
                                map_Median.clear();
                            }
                            break;
                case 1:     // expo
                            if(map_Default.size() == 0) slotSendToServer(INFO_DEVICE);
                            else
                            {
                                text = map_Default.value("default", "");
                                slot_WriteSector(0x0418, text);
                                map_Default.clear();
                            }
                            break;
                default:    break;
                }
            }
        }
     }
}

//-----------------------------------------------------------------------------
//--- RefreshInfoProtocol(rt_Protocol *)
//-----------------------------------------------------------------------------
void Run::RefreshInfoProtocol(rt_Protocol *p)
{
    info_Protocol->clear_Info();
    info_Protocol->fill_Info(p);
}

//-----------------------------------------------------------------------------
//--- slotRefreshInfoDevice()
//-----------------------------------------------------------------------------
void Run::slotRefreshInfoDevice()
{
    int i;
    quint16 value;
    QString text;
    QTreeWidgetItem *item;
    QStringList header;
    header << tr("Parameter") << tr("Value");
    QString B96b = "A5X112,A5X206,A5Y201,A5Y809,A5Z202,A5Z806,A5Z910,A5AN14,A5AN15,A5BN09,A5BD06,A5C113,A5C302,A5CD11,A5CD26";
    QString B96c = "A5X111,A5X207,A5X409,A5Z008,A5A603,A5AN09,A5B730,A5BN01,A5C510,A5CD15,A5CD16";
    QStringList list_B96b = B96b.split(",");
    QStringList list_B96c = B96c.split(",");
    QString name_dev = map_InfoDevice.value(INFODEV_serName);;

    info_device->clear();
    info_device->setColumnCount(2);
    info_device->setHeaderLabels(header);
    info_device->header()->setSectionResizeMode(0,QHeaderView::ResizeToContents);
    info_device->header()->setSectionResizeMode(1,QHeaderView::ResizeToContents);

    // FHW
    LEFT_OFFSET = 66;
    LEFT_OFFSET_DT48 = 7;
    W_IMAGE = 752;
    H_IMAGE = 292;
    COEF_IMAGE = 1.93;
    W_IMAGE_COEF = 390;
    W_REALIMAGE = 825;
    H_REALIMAGE = 312;
    TOP_OFFSET = 5;
    Coef_Expo = COEF_EXPO;

    if(map_InfoDevice.value(INFODEV_devHW,"").contains("v4.0"))
    {
        LEFT_OFFSET = 0;
        LEFT_OFFSET_DT48 = 0;
        W_IMAGE = 640;
        H_IMAGE = 480;
        COEF_IMAGE = 1.00;
        W_IMAGE_COEF = 640;
        W_REALIMAGE = 640;
        H_REALIMAGE = 480;
        TOP_OFFSET = 0;
        Coef_Expo = 0.154;  // new coef_expo
    }


    // Name
    item = new QTreeWidgetItem(info_device);
    item->setText(0, tr("Name of Device"));
    item->setText(1, map_InfoDevice.value(INFODEV_serName));
    // Version
    item = new QTreeWidgetItem(info_device);
    item->setText(0, tr("Version"));
    item->setText(1, map_InfoDevice.value(INFODEV_version));
    // FluorMask
    item = new QTreeWidgetItem(info_device);
    item->setText(0, tr("FluorMask"));
    item->setText(1, map_InfoDevice.value(INFODEV_devMask));
    // TypeBlock
    item = new QTreeWidgetItem(info_device);
    item->setText(0, tr("TypeBlock"));
    item->setText(1, map_InfoDevice.value(INFODEV_thermoBlock));    
    if(map_InfoDevice.value(INFODEV_thermoBlock) == "UNKNOWN")
    {        
        if(list_B96b.indexOf(name_dev) >= 0) item->setText(1, "B96B");
        if(list_B96c.indexOf(name_dev) >= 0) item->setText(1, "B96C");

        if(item->text(1) == "UNKNOWN" && name_dev.at(1) == '5')
        {
            item->setText(1, "B96A");
        }
    }    

    // Version Optics uController
    ver_OPTICS = Ver_uOPTICS();

    // Load Expozition
    Load_Expozition();

    // Exposition
    item = new QTreeWidgetItem(info_device);
    item->setText(0, tr("Exposition"));
    i = 0;
    text = "";
    foreach(value, Expozition)
    {
        if(div(i,2).rem == 0)
        {
            if(!text.isEmpty()) text += ",";
            text += QString("%1").arg(value);
        }
        i++;
    }
    item->setText(1, text);

    Expozition_Def.clear();
    Expozition_Def.resize(COUNT_CH*2);
    Expozition_Def.fill(0);

    // Load Coefficients
    bool coeff_Spectral = Load_SpectralCoeff();
    bool coeff_Optic = Load_OpticCoeff();
    bool coeff_Unequal = Load_UnequalCoeff();

    if(!coeff_Spectral || !coeff_Optic || !coeff_Unequal)
    {
        message.setStandardButtons(QMessageBox::Ok);
        message.setIcon(QMessageBox::Critical);
        text = tr("Attention! Optical unevenness coefficients readout fail...") +"\r\n" +
               tr("Please contact to service engineer");
        message.setText(text);
        message.exec();
    }

    // Load Optical Mask
    bool load_Mask = Load_Mask();

    if(!load_Mask)
    {
        message.setStandardButtons(QMessageBox::Ok);
        message.setIcon(QMessageBox::Critical);
        text = tr("Attention! Optical geometry indices data readout fail...") +"\r\n" +
               tr("Please check optical geometry indices and correct if necessary");
        message.setText(text);
        message.exec();
    }

    //... dir_dev ...
    bool ok;
    text = qApp->applicationDirPath() + "/device/";
    dir_Device.setPath(text);
    if(!dir_Device.exists()) ok = dir_Device.mkdir(text);

    text = qApp->applicationDirPath() + "/device/" + map_InfoDevice.value(INFODEV_serName);
    dir_Device.setPath(text);
    if(!dir_Device.exists()) ok = dir_Device.mkdir(text);    

    Check_Compatibility();
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Run::slotSendToServer(QString request)
{
    qint64 out_byte;
    QBasicTimer *timer_req;
    int interval;

    //if(request == CRYPTO_REQUEST) qDebug() << "List_Requests: " << List_Requests;


    if(List_Requests.contains(RUN_REQUEST)) return; // when RUN_REQUEST is active, ALL other are disabled
    if(List_Requests.contains(MEASURE_REQUEST)) return; // when MEASURE_REQUEST is active, ALL other are disabled
    if(List_Requests.contains(GETPIC_REQUEST)) return;  // when GETPIC_REQUEST is active, ALL other are disabled
    if(List_Requests.contains(SAVEPAR_REQUEST)) return;  // when SAVEPAR_REQUEST is active, ALL other are disabled    
    //if(List_Requests.contains(SECTORREAD_REQUEST)) return;  // when SECTORREAD_REQUEST is active, ALL other are disabled
    //if(List_Requests.contains(SECTORWRITE_REQUEST)) return;  // when SECTORREAD_REQUEST is active, ALL other are disabled
    if(List_Requests.contains(HTMEAS_REQUEST) && request != HIGHTUBE_SAVE) return;
    if(List_Requests.contains(GETALLVIDEO_REQUEST)) return;
    if(List_Requests.contains(CRYPTO_REQUEST)) return;

    if(List_Requests.contains(request)) return; //don't get answer on previous request
    else List_Requests.append(request);

    if(List_Requests.contains(DTMASTERLOG_REQUEST)) List_Requests.removeAll(DTMASTERLOG_REQUEST);

    QByteArray  arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_7);
    out << quint32(0) << request;

    if(request == RUN_REQUEST) out << map_Run;                          // Run
    //if(request == RUN_REQUEST) SaveImage_rejime_IsChecked = false;

    if(request == MEASURE_REQUEST)                                      // Measure
    {
        out << (FN+1) << active_ch;
        //qDebug() << "FN,active_ch: " << FN << active_ch;
    }

    if(request == EXECCMD_REQUEST) out << map_CMD;                      // CMD

    if(request == GETPIC_REQUEST) out << map_Picture;                   // Get Picture

    if(request == SAVEPAR_REQUEST) out << map_SavePar;                  // Save Parameters to Device

    if(request == SECTORWRITE_REQUEST) out << map_Sector;               // Write Sector Data
    if(request == SECTORREAD_REQUEST) out << map_Sector;                // Read Sector Data

    if(request == HIGHTUBE_SAVE) out << Tube_Height;

    if(request == DTMASTERLOG_REQUEST) out << map_Logged;               // write to Log

    if(request == GETALLVIDEO_REQUEST) out << map_ReadOpticDev;        // Read Optic Picture from Device


    if(request == CRYPTO_REQUEST)
    {
        out << map_CRYPTO;                    // CRYPTO
        //qDebug() << "slotSendToServer:" << map_CRYPTO.value(CRYPTO_CTRL,"");
    }



    //... save image ...
    if(request == RUN_REQUEST)
    {
        SaveImage_rejime->blockSignals(true);
        SaveImage_rejime->setCheckState(Qt::Unchecked);
        SaveImage_rejime->blockSignals(false);
    }
    //if(request == STOP_REQUEST && SaveImage_rejime->checkState() == Qt::Checked)
    //{
        //SaveImage_rejime->setCheckState(Qt::Unchecked);
    //}
    //...

    out.device()->seek(0);
    out << quint32(arrBlock.size() - sizeof(quint32));

    /*if(request == EXECCMD_REQUEST)
    {
        qDebug() << request << map_CMD;
    }*/
    /*if(request == DTMASTERLOG_REQUEST)
    {
        qDebug() << request << map_Logged;
    }*/
    //

    out_byte = m_pTcpSocket->write(arrBlock);

    //qDebug() << "send request: <- " << request;

    /*if(request == DTMASTERLOG_REQUEST || request == INFO_DEVICE)
    {
        qDebug() << out_byte << " : " << arrBlock << m_pTcpSocket->state();
    }*/

    // Timer ...

    if(request == DTMASTERLOG_REQUEST) return; // No timer
    if(request == GETALLVIDEO_REQUEST) return; // No timer

    if(Map_TimerRequest.contains(request))
    {
        timer_req = Map_TimerRequest.value(request);

        if(request == INFO_DATA)    interval = 50000;
        if(request == INFO_DEVICE)  interval = 10000;
        if(request == RUN_REQUEST)  interval = 15000;
        if(request == STOP_REQUEST) interval = 15000;
        if(request == PAUSE_REQUEST)  interval = 15000;
        if(request == MEASURE_REQUEST)  interval = 18000;
        if(request == OPENBLOCK_REQUEST)  interval = 35000;
        if(request == CLOSEBLOCK_REQUEST)  interval = 35000;
        if(request == EXECCMD_REQUEST)  interval = 15000;
        if(request == PRERUN_REQUEST)  interval = 15000;
        if(request == GETPIC_REQUEST)  interval = 17000;
        if(request == SAVEPAR_REQUEST)  interval = 17000;
        if(request == SECTORREAD_REQUEST)  interval = 17000;
        if(request == SECTORWRITE_REQUEST)  interval = 17000;
        if(request == HTMEAS_REQUEST) interval = 75000;
        if(request == HIGHTUBE_SAVE)  interval = 20000;
        if(request == CRYPTO_REQUEST)  interval = 10000;

        timer_req->start(interval, this);
    }

}
//-----------------------------------------------------------------------------
//--- slotSendRequestInfo (timeOut)
//-----------------------------------------------------------------------------
void Run::slotSendRequestInfo()
{
    if(m_pTcpSocket->state() == QAbstractSocket::ConnectedState)
    {
        slotSendToServer(INFO_DATA);
    }
}
//-----------------------------------------------------------------------------
//--- slotGetInfoDevice
//-----------------------------------------------------------------------------
void Run::slotGetInfoDevice()
{
    if(m_pTcpSocket->state() == QAbstractSocket::ConnectedState)
    {
        slotSendToServer(INFO_DEVICE);
    }
}
//-----------------------------------------------------------------------------
//--- slot_PAUSE
//-----------------------------------------------------------------------------
void Run::slot_PAUSE()
{
    bool state = PauseDev->isChecked();
    QString text, request;


    if(state)
    {
        text = tr("Continue");
        request = PAUSE_REQUEST;
    }
    else
    {
        text = tr("Pause");
        request = CONTINUE_REQUEST;
    }
    PauseDev->setText(text);
    PauseDev->clearFocus();

    if(m_pTcpSocket->state() == QAbstractSocket::ConnectedState)
    {
        slotSendToServer(request);
    }

}
//-----------------------------------------------------------------------------
//--- slot_STOP
//-----------------------------------------------------------------------------
void Run::slot_STOP(bool msg)
{
    int res;

    if(m_pTcpSocket->state() == QAbstractSocket::ConnectedState)
    {
        if(msg && Dev_State == sHold) msg = false;

        if(msg)
        {
        message.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);        
        if(flag_ActivePoint) {message.setDefaultButton(QMessageBox::Yes); message.button(QMessageBox::Yes)->animateClick(7000);}
        else {message.setDefaultButton(QMessageBox::Cancel); message.button(QMessageBox::Cancel)->animateClick(7000);}
        message.setIcon(QMessageBox::Information);        
        message.setText(tr("Program is executed In instrument... Do You want to finish performing the program?"));
        res = message.exec();

        if(res == QMessageBox::Cancel) return;
        }

        StopDev->setDisabled(true);
        StopDev->clearFocus();
        slotSendToServer(STOP_REQUEST);

        wait_process(wStop, 100, tr("Stopping..."));
    }
}
//-----------------------------------------------------------------------------
//--- slot_StandBy
//-----------------------------------------------------------------------------
void Run::slot_StandBy()
{
    int id = 3;
    bool sts = StandBy->isChecked();
    QString cmd = QString("TSNS %1").arg((int)sts);

    map_CMD.clear();
    map_CMD.insert(EXECCMD_CMD, cmd);
    map_CMD.insert(EXECCMD_UC, QString::number(id));

    slotSendToServer(EXECCMD_REQUEST);
}
//-----------------------------------------------------------------------------
//--- slot_PrepareLastRun()
//-----------------------------------------------------------------------------
void Run::slot_PrepareLastRun()
{
    int id = 2;         // uC Optic
    QString cmd = "FACS";

    map_CMD.clear();
    map_CMD.insert(EXECCMD_CMD, cmd);
    map_CMD.insert(EXECCMD_UC, QString::number(id));

    flag_FACS = true;
    slotSendToServer(EXECCMD_REQUEST);
}
//-----------------------------------------------------------------------------
//--- slot_LastRun
//-----------------------------------------------------------------------------
void Run::slot_LastRun()
{
    bool find_run = false;

    // 1. open run.rt protocol (last protocol for this device)
    QString filename = qApp->applicationDirPath() + "/device/" +
                       map_InfoDevice.value(INFODEV_serName) + "/run.rt";

    QFile fn(filename);
    QFileInfo fi(filename);

    if(fi.exists())
    {
        QDateTime last_modificate = fi.lastModified();
        QDateTime current = QDateTime::currentDateTime();
        int count_sec = last_modificate.secsTo(current);
        double hour = (double)count_sec/3600.;
        if(hour < 24.) find_run = true;

        //qDebug() << "hour: " << hour;
    }

    //qDebug() << "find_run: " << find_run;

    if(find_run)        // run.rt
    {
        Open_Protocol(filename);
    }


    if(!find_run)
    {
        Create_LastRunProtocol();        
        Open_Protocol(filename);
    }


    /*if(!fn.exists())
    {
        message.setStandardButtons(QMessageBox::Ok);
        message.setIcon(QMessageBox::Warning);
        message.button(QMessageBox::Ok)->animateClick(7000);
        message.setText(tr("Information about Last Run is absent!"));
        message.exec();
        return;
    }*/



    active_ch = prot->active_Channels;
    //qDebug() << fn_dev << FN << active_ch;

    flag_WebProtocol = false;
    flag_LastRun = true;
    slotSendToServer(MEASURE_REQUEST);
    wait_process(wMeasure, 1000, "");
}
//-------------------------------------------------------------------------------------------------
//--- eventFilter
//-------------------------------------------------------------------------------------------------
bool Run::eventFilter(QObject *obj, QEvent *evt )
{
    QTextBlock block;
    QTextCursor cursor;
    QString cmd_str;
    int id;

    if(evt->type() == QEvent::KeyPress && obj == cmd_Edit)
    {
      QKeyEvent *keyEvent = static_cast<QKeyEvent*>(evt);

      if(keyEvent->key() == Qt::Key_Up ||
         keyEvent->key() == Qt::Key_Down ||
         keyEvent->key() == Qt::Key_Left ||
         keyEvent->key() == Qt::Key_Right ||
         keyEvent->key() == Qt::Key_Tab)
        {
            return(true);
        }
      else
        {

          if(keyEvent->key() == Qt::Key_Return)
          {
              cursor = cmd_Edit->textCursor();
              block = cursor.block();
              cmd_str = block.text();
              cmd_str.remove(QChar('>'));
              cmd_str = cmd_str.simplified();

              if(!cursor.atEnd())
               {
                  cmd_Edit->moveCursor(QTextCursor::End);
                  cmd_str = "";
                  LastCMD = "";
               }
              if(cmd_str.count() > 0) cmd_Edit->append("");

              if(cmd_str.count() > 0) LastCMD = cmd_str;
              else
               {
                if(LastCMD.count() > 0) cmd_str = LastCMD;
                else
                 {
                   cmd_Edit->append(">");
                   return(true);
                 }
               }

              map_CMD.clear();
              id = cmd_ComboBox->currentIndex() + 2;
              if(cmd_ComboBox->currentIndex() == 4) id = 1;
              map_CMD.insert(EXECCMD_CMD, cmd_str);
              map_CMD.insert(EXECCMD_UC, QString::number(id));

              slotSendToServer(EXECCMD_REQUEST);
              return(true);
          }
        }
    }
    return(QWidget::eventFilter(obj, evt));

}
//-----------------------------------------------------------------------------
//--- slot_SelectTab
//-----------------------------------------------------------------------------
void Run::slot_SelectTab(int index)
{
    switch(index)
    {
        case 4:             // command string
                    cmd_Edit->setFocus();
                    break;

        default:
                    break;
    }
}

//-----------------------------------------------------------------------------
//--- slot_OpenBlock
//-----------------------------------------------------------------------------
void Run::slot_OpenBlock()
{
    OpenClose_Box->setEnabled(false);
    slotSendToServer(OPENBLOCK_REQUEST);
    wait_process(wOpen, 100, tr("Open block..."));
}

//-----------------------------------------------------------------------------
//--- slot_CloseBlock
//-----------------------------------------------------------------------------
void Run::slot_CloseBlock()
{
    OpenClose_Box->setEnabled(false);
    slotSendToServer(CLOSEBLOCK_REQUEST);
    wait_process(wClose, 100, tr("Close block..."));
}
//-----------------------------------------------------------------------------
//--- slot_ProtocolIsEqual()
//-----------------------------------------------------------------------------
void Run::slot_ProtocolIsEqual()
{
    bool state = true;
    QString text;
    int res;
    QVector<QString> source_pro, target_pro;

    string s;
    QString str;
    QVector<QString> s_pro, t_pro;
    bool s1,s2,s3;

    //qDebug() << "slot_ProtocolIsEqual: " << CALIBRATION_status << flag_ActivePoint << prot << prot_FromSetup;

    if(!CALIBRATION_status && !flag_ActivePoint && prot && prot_FromSetup)
    {
        //qDebug() << "Is_Equal: " << QString::fromStdString(prot->regNumber) << QString::fromStdString(prot_FromSetup->regNumber);
        if(prot->regNumber == prot_FromSetup->regNumber && prot_FromSetup->Plate.groups.size())
        {
            Create_SignatureForProtocol(prot, &target_pro);
            Create_SignatureForProtocol(prot_FromSetup, &source_pro);

            foreach(s, prot->program)
            {
                str = QString::fromStdString(s);
                if(str.startsWith("XPRG")) continue;
                s_pro.append(str);
            }
            foreach(s, prot_FromSetup->program)
            {
                str = QString::fromStdString(s);
                if(str.startsWith("XPRG")) continue;
                t_pro.append(str);
            }
            //qDebug() << "program: " << s_pro << t_pro;
            //qDebug() << "name: " << QString::fromStdString(prot->name) << QString::fromStdString(prot_FromSetup->name);
            //qDebug() << "source: " << target_pro << source_pro;


            s1 = s2 = s3 = true;
            if(s_pro != t_pro) s1 = false;
            if(prot->name != prot_FromSetup->name) s2 = false;
            if(target_pro != source_pro) s3 = false;

            //qDebug() << "bool: " << s1 << s2 << s3;

            /*if(prot->program != prot_FromSetup->program ||
               prot->name != prot_FromSetup->name       ||
               target_pro != source_pro)*/
            if(!s1 || !s2 || !s3)
            {
                text = tr("Attention! You have changed the original protocol. Do You want to continue running the protocol without these changes?");
                message.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                message.button(QMessageBox::No)->animateClick(20000);
                message.setIcon(QMessageBox::Warning);
                message.setText(text);
                res = message.exec();

                if(res == QMessageBox::No) state = false;
            }
        }
    }

    if(state) slot_CheckExposition_Run();
}
//-----------------------------------------------------------------------------
//--- Create_SignatureForProtocol(rt_Protocol *P, QVector<QString> *vec)
//-----------------------------------------------------------------------------
void Run::Create_SignatureForProtocol(rt_Protocol *P, QVector<QString> *vec)
{
    foreach(rt_GroupSamples *group, P->Plate.groups)
    {
        foreach(rt_Sample *sample, group->samples)
        {
            vec->append(QString::fromStdString(sample->Unique_NameSample));
            vec->append(QString::fromStdString(sample->ID_Test));
            foreach(rt_Tube *tube, sample->tubes)
            {
                vec->append(QString::number(tube->ID_Tube));
                vec->append(QString::number(tube->pos));
                vec->append(QString::number(tube->color));
            }
        }
    }
}
//-----------------------------------------------------------------------------
//--- slot_CheckExposition_Run()
//-----------------------------------------------------------------------------
void Run::slot_CheckExposition_Run()
{
    QString text;
    int res;

    //qDebug() << "start slot_CheckExposition_Run():" << Try_NewExpozition;

    if(!flag_EnableDev)
    {
        NameDev->setStyleSheet("QLabel{background-color: rgb(255, 128, 128)}");

        text = tr("Attention! The device cannot work with the current version of the program!") + "\r\n";
        text += tr("Please, contact customer support...");
        message.setStandardButtons(QMessageBox::Ok);
        message.setIcon(QMessageBox::Warning);
        message.button(QMessageBox::Ok)->animateClick(15000);
        message.setText(text);
        message.exec();

        return;
    }

    if(flag_ContractDev && Contract_Dev->count_days <= 0 && id_Reagent == rTimeLimited)
    {
        NameDev->setStyleSheet("QLabel{background-color: rgb(255, 128, 128)}");

        text = tr("Attention! According to the contract restrictions, you have exceeded the quota of permitted runs") +
               "\r\n" + tr("Please apply to seller!");
        message.setStandardButtons(QMessageBox::Ok);
        message.setIcon(QMessageBox::Warning);
        message.button(QMessageBox::Ok)->animateClick(15000);
        message.setText(text);
        message.exec();

        return;
    }

    if(flag_ContractDev && flag_LimitedDev && id_Reagent == rTestLimited)
    {
        NameDev->setStyleSheet("QLabel{background-color: rgb(255, 128, 128)}");

        text = tr("Attention! Invalid the PublicKey...") + "\r\n" +
               tr("According to the contract restrictions, Device is blocked!");

        message.setStandardButtons(QMessageBox::Ok);
        message.setIcon(QMessageBox::Warning);
        message.button(QMessageBox::Ok)->animateClick(15000);
        message.setText(text);
        message.exec();

        return;
    }

    if(flag_ContractDev && flag_LimitedDev && id_Reagent == rLimited)
    {
        NameDev->setStyleSheet("QLabel{background-color: rgb(255, 128, 128)}");

        text = tr("According to the contract restrictions, Device is blocked!");

        message.setStandardButtons(QMessageBox::Ok);
        message.setIcon(QMessageBox::Warning);
        message.button(QMessageBox::Ok)->animateClick(15000);
        message.setText(text);
        message.exec();

        return;
    }

    if(Try_NewExpozition > 3)
    {
        message.setStandardButtons(QMessageBox::Ok);
        message.setIcon(QMessageBox::Critical);
        text = tr("Cannot change the exposure value in the device...") + "\r\n";
        text += tr("Please, overload the device!");
        message.button(QMessageBox::Ok)->animateClick(20000);
        message.setText(text);
        res = message.exec();

        Try_NewExpozition = 0;
        if(flag_ActivePoint) PA_Error("Run canceled!");

        return;
    }

    //... check on valid tests in the protocol ...
    if(flag_ContractDev && !flag_LimitedDev && id_Reagent == rTestLimited)
    {
        if(!Check_ValidTests(prot))
        {
            text = tr("According to the contract restrictions, you can't use unsigned tests.");

            message.setStandardButtons(QMessageBox::Ok);
            message.setIcon(QMessageBox::Warning);
            message.button(QMessageBox::Ok)->animateClick(15000);
            message.setText(text);
            message.exec();

            return;
        }
    }



    if(IsExpositionEqual())
    {
       //qDebug() << "start slot_ClosePrerunRun()";;
       slot_ClosePrerunRun();       // preRun and Run
       Try_NewExpozition = 0;
    }
    else
    {
        //qDebug() << "write new expo:";
        Try_NewExpozition++;
        if(WriteNewExpositionToDevice())
        {
            QTimer::singleShot(2000, this, SLOT(slot_CheckExposition_Run()));
        }

        return;

        //==================== The END ======================================


        text = tr("Attention! To start the Protocol, you need to change the current exposure values in the device!") + "\r\n";
        text += tr("Do You want to do it?");
        /*if(ALARM_Exposure)
        {
            message.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
            text += tr("you can change the exposure in the device or immediately start the Protocol with the current exposure values");

            message.setButtonText(QMessageBox::Yes, tr("Change expo"));
            message.setButtonText(QMessageBox::No, tr("Run with current expo"));
        }
        else
        {*/
            message.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
            //text += tr("You must change the exposure in the device to start the protocol!") + "\r\n";
            //text += tr("Will you want to write a new exposition in Device?");
        //}


        message.setDefaultButton(QMessageBox::Cancel);
        message.setIcon(QMessageBox::Warning);
        message.button(QMessageBox::Cancel)->animateClick(20000);


        message.setText(text);
        res = message.exec();

        if(res == QMessageBox::Cancel)  // Cancel
        {
            //clear_protcol();
            //Check_Compatibility();
            //slot_ChangeStateDevice(Dev_State);

            return;
        }
        /*if(res == QMessageBox::No)      // No...  Don't change expo and run protocol
        {
            slot_ClosePrerunRun();
        }*/
        if(res == QMessageBox::Yes)     // Yes... Change expo
        {
            // write new exposition
            /*message.setStandardButtons(QMessageBox::Ok);
            message.setDefaultButton(QMessageBox::Ok);
            message.setIcon(QMessageBox::Information);
            message.button(QMessageBox::Ok)->animateClick(7000);
            text = tr("After changing the exposure, Protocol will start automatically...");
            message.setText(text);
            message.exec();*/

            if(WriteNewExpositionToDevice())
            {
                QTimer::singleShot(1000, this, SLOT(slot_CheckExposition_Run()));
            }
        }
    }
}
//-----------------------------------------------------------------------------
//--- Check_ValidTests(rt_Protocol *p)
//-----------------------------------------------------------------------------
bool Run::Check_ValidTests(rt_Protocol *p)
{
    rt_Test *ptest;
    bool state = true;

    foreach(ptest, p->tests)
    {
        if(ptest->header.Hash != "ok") {state = false; break;}
    }

    return(state);
}
//-----------------------------------------------------------------------------
//--- WriteNewExpositionToDevice()
//-----------------------------------------------------------------------------
bool Run::WriteNewExpositionToDevice()
{
    int i,j;
    double dvalue;
    qint16 value;
    quint32 val;
    QString data = "", text;
    QVector<quint16> NewExposition;
    int act_ch = prot->active_Channels;

    NewExposition = Expozition.mid(0);

    for(i=0; i<COUNT_CH; i++)
    {
        if(act_ch & (0x0f<<i*4))
        {
            //value = prot->Exposure.at(i);
            //if(value == 0) value = Expozition_Def.at(2*i);
            dvalue = ((double)(prot->Exposure.at(i)))/100.;
            value =  dvalue * Expozition_Def.at(2*i);

            //qDebug() << "WriteNewExpositionToDevice: " << i << dvalue << value << prot->Exposure.at(i);

            NewExposition.replace(2*i, value);
            NewExposition.replace(2*i+1, value/5.);

            if(value > 10000 || value <= 10)
            {
                message.setStandardButtons(QMessageBox::Ok);
                message.setDefaultButton(QMessageBox::Ok);
                message.setIcon(QMessageBox::Critical);
                message.button(QMessageBox::Ok)->animateClick(7000);
                text = tr("Attention! Discovered invalid values of the exposition... (Device,Protocol)");
                message.setText(text);
                message.exec();
                return(false);
            }
        }
        for(j=0; j<COUNT_EXP; j++)
        {
            val = qRound((double)(NewExposition.at(2*i+j)/Coef_Expo));  // COEF_EXPO
            text = QString(" %1").arg(val);
            data += text;
        }
    }
    //qDebug() << "exp_device, new_Exp: " << Expozition << NewExposition;
    //qDebug() << "write new expo: " << data;
    slot_SavePar(1, data);

    return(true);
}

//-----------------------------------------------------------------------------
//--- IsExpositionEqual()
//-----------------------------------------------------------------------------
bool Run::IsExpositionEqual()
{
    QVector<int> expo_Protocol, expo_Device, expo_Device_Def, expo_NULL;
    int i;
    int value;
    int val;
    double dvalue;
    QString text;
    bool sts;
    bool ok;
    int act_ch = prot->active_Channels;
    //int act_ch_Dev = map_InfoDevice.value(INFODEV_devMask, "0").toInt(&ok,16);


    //qDebug() << "act_ch: " << act_ch << prot->Exposure << Expozition << Expozition_Def << act_ch_Dev;

    for(i=0; i<COUNT_CH; i++)
    {
        if(act_ch & (0x0f<<i*4))
        {
            value = prot->Exposure.at(i);
            //if(value == 0) value = Expozition_Def.at(i*2);

            expo_Protocol.append(value);
            expo_Device.append(Expozition.at(i*2));
            expo_Device_Def.append(Expozition_Def.at(i*2));
            //expo_NULL.append(0);
        }
    }

    //qDebug() << "expo_1:" << expo_Protocol << expo_NULL;
    //qDebug() << "expo_2:" << expo_Protocol << expo_Device << expo_Device_Def;

    //if(qEqual(expo_Protocol.begin(), expo_Protocol.end(), expo_NULL.begin()) /*&&
    //   qEqual(expo_Device.begin(), expo_Device.end(), expo_Device_Def.begin())*/)  return(true);
    //else
    //{
    //   sts = qEqual(expo_Protocol.begin(), expo_Protocol.end(), expo_Device.begin());
    //}

    //... new ...
    i = 0;
    sts = true;
    foreach(value, expo_Protocol)
    {
        /*if(value == 0)
        {
            if(expo_Device.at(i) != expo_Device_Def.at(i)) {sts = false; break;}
        }
        else
        {
            if(value != expo_Device.at(i)) {sts = false; break;}
        }*/
        if(expo_Device_Def.at(i) == 0) {sts = false; break;}

        dvalue = ((double)expo_Device.at(i))/((double)expo_Device_Def.at(i));
        text = QString("%1").arg(dvalue,0,'f',2);
        dvalue = text.toDouble();
        text = QString::number(dvalue * 100,'f',0);
        val = text.toInt();

        //qDebug() << "expo(i): " << value << val << dvalue;

        if(value != val) {sts = false; break;}

        i++;
    }
    //...

    //qDebug() << "IsExpositionEqual res: " << sts;
    return(sts);
}
//-----------------------------------------------------------------------------
//--- slot_SaveDefExposition()
//-----------------------------------------------------------------------------
void Run::slot_SaveDefExposition()
{
    int i;
    QString text = "";
    int expo_value = 1000;
    union
    {
        Device_ParDefault Device_ParamDefault;
        unsigned char byte_buf[512];
    }DEVICE_PAR_DEFAULT;
    QVector<quint16> Expozition_def;

    QString name_Dev = map_InfoDevice.value(INFODEV_serName, "").trimmed();
    QChar ch = name_Dev.at(1);

    if(ch == '7' || ch == '8') expo_value = 500;
    for(i=0; i<COUNT_CH; i++)
    {
        Expozition_def.append(expo_value);
        Expozition_def.append(expo_value/5);
    }

    for(i=0; i<512; i++) DEVICE_PAR_DEFAULT.byte_buf[i] = 0;
    strcpy(DEVICE_PAR_DEFAULT.Device_ParamDefault.id_default, "default");
    for(i=0; i<Expozition_def.size(); i++)
    {
        DEVICE_PAR_DEFAULT.Device_ParamDefault.Expo_def[i] = Expozition_def.at(i);
    }
    QByteArray ba = QByteArray((char*)(DEVICE_PAR_DEFAULT.byte_buf), 512);
    text = ba.toBase64();

    //...
    QString str_0 = tr("Attention! Basic exposure values were not detected in the device!");
    QString str_1 = tr("The base values will be the default exposure values for this type device!");
    //QString str_2 = tr("Please, contact customer support...");
    QString str = QString("%1\r\n%2").arg(str_0).arg(str_1);


    message.setStandardButtons(QMessageBox::Ok);
    message.button(QMessageBox::Ok)->animateClick(30000);
    message.setIcon(QMessageBox::Warning);
    message.setText(str);
    message.exec();

    //...

    slot_WriteSector(0x0418, text);
}

//-----------------------------------------------------------------------------
//--- slot_ClosePrerunRun()
//-----------------------------------------------------------------------------
void Run::slot_ClosePrerunRun()
{     
    flag_ClosePrerunRun = true;
    slot_CloseBlock();
}
//-----------------------------------------------------------------------------
//--- slot_PreRUN
//-----------------------------------------------------------------------------
void Run::slot_PreRUN()
{       
    slotSendToServer(PRERUN_REQUEST);
    wait_process(wPreRun, 100, tr("PreRUN Starting..."));
}

//-----------------------------------------------------------------------------
//--- slot_RUN
//-----------------------------------------------------------------------------
void Run::slot_RUN()
{
    int i;
    QString text = "";
    QString str;

    map_Run.clear();

    //
    //... Select user catalog ...
    QString FileName = QString::fromStdString(prot->name);
    if(FileName.trimmed().isEmpty()) FileName = "Protocol_";
    QString dirName = user_Dir.absolutePath() + "/" + QString::fromStdString(prot->name) + ".rt";

    Save_FileName = "";

    if(flag_SelectCatalogForResult)
    {
        FileName = QFileDialog::getSaveFileName(this,
                                                tr("Save Protocol with results as ..."),
                                                dirName,
                                                tr("Protocols File (*.rt)"));
        if(!FileName.isEmpty())
        {
            Save_FileName = FileName;
        }
        else
        {
            message.setStandardButtons(QMessageBox::Ok);
            message.button(QMessageBox::Ok)->animateClick(8000);
            text = tr("Attention! You stopped running the current protocol...");
            message.setText(text);
            message.setIcon(QMessageBox::Warning);
            message.exec();

            return;
        }
    }
    //...

    // 0. Name
    text = QString::fromStdString(prot->name);
    map_Run.insert(run_name, text);

    // 1. Programm
    text = "";
    QVector<string> prog = QVector<string>::fromStdVector(prot->program);
    for(i=0; i<prog.size(); i++)
    {
        str = QString::fromStdString(prog.at(i)).trimmed();
        if(str.isEmpty()) continue;
        if(i) text += "\t";        
        text += str;
    }
    map_Run.insert(run_programm, text);

    // 2. ActiveChannel
    text = QString::number(prot->active_Channels,16);
    map_Run.insert(run_activechannel, text);

    // 3. ID_Protocol
    text = QString::fromStdString(prot->regNumber);
    map_Run.insert(run_IDprotocol, text);

    // 4. Operator
    text = QString::fromStdString(prot->owned_by);
    map_Run.insert(run_operator, text);

    qDebug() << "Map Run:" << map_Run;

    if(m_pTcpSocket->state() == QAbstractSocket::ConnectedState)
    {
        RunDev->setDisabled(true);
        RunDev->clearFocus();
        FN = 0;
        fn_dev = 0;
        flag_SavingResult = false;
        flag_SavingOpticDev = false;
        flag_RunLoaded = false;
        plot_fluor->current_fn = 0;
        plot_fluor->Clear_Graph();
        plot_fluor->label_fn->setText(QString("fn=%1").arg(plot_fluor->current_fn));

        slotSendToServer(RUN_REQUEST);

        wait_process(wRun, 100, tr("RUN Loading..."));
    }
}
//-----------------------------------------------------------------------------
//--- slot_RefreshMeasure
//-----------------------------------------------------------------------------
void Run::slot_RefreshMeasure(QMap<QString, QString> *map)
{
    bool ok;
    int current_state = Dev_State;

    T_current = map->value(INFO_Temperature,"-1").split(" ").at(0).toDouble()/100.;   // current temperature
    int fn = map->value(INFO_fn,"0").toInt();                   // fn
    int state = map->value(INFO_status,"0").mid(0,1).toInt();   // state - status(nothing,run,hold,pause,warm,...)

    if(state == sRun || state == sHold) SaveImage_rejime->setDisabled(false);
    else SaveImage_rejime->setDisabled(true);
    //if(SaveImage_rejime_Active) SaveImage_rejime->setChecked((bool)(map->value(INFO_fmode,"0").toInt(&ok)));

    fn_dev = fn;

    if(state > sUnknown && state <= sWarming_up)
    {
        Queue_State.enqueue(state);
        if(Queue_State.size() > 10) Queue_State.dequeue();

        if(Check_ChangeDevice(state)/* || (Dev_State == sUnknown && Dev_State != state)*/)  emit sChangeStateDevice(state);
        //if(current_state == sUnknown) return;

    }
    //else dev_state = sUnknown;

    //... XID - info about current block,level,status ...
    if(map->contains(INFO_Levels) && prot->program.size()) runtime_Program->XID_info(map->value(INFO_Levels, ""), prot);


    //... TIM - info about time
    if(map->contains(INFO_time)) runtime_Program->TIM_info(map->value(INFO_time, ""), prot);

    //... XGS - info about cycles
    if(map->contains(INFO_status)) runtime_Program->XGS_info(map->value(INFO_status, ""), prot);


    //... Hold ...
    if(state == sHold && Check_HoldLevel() && FN == fn) emit sChangeStateDevice(state);

    //... Run ProgressBar ...
    if(state >= sRun && state <= sHold)
    {
        qint64 msec;
        int value;
        int tim_0 = 0;
        int all_fn = prot->count_MC + prot->count_PCR;
        QDateTime tim = QDateTime::currentDateTime();
        QString text = map->value(INFO_time, "");
        if(!text.isEmpty())
        {
            sscanf(text.toStdString().c_str(),"%d", &tim_0);
            msec = runtime_Program->prot_Start.msecsTo(tim);
            value = (msec/1000. * 100.)/(msec/1000. + tim_0);
            //qDebug() << "INFO_time: " << text << tim_0 << msec << value;
            slot_RunProcess(value, QString("%1% %2(%3)").arg(value).arg(fn).arg(all_fn));
        }
    }



    if(List_Requests.contains(MEASURE_REQUEST)) return;
    if(!flag_Compatibility) return;

    if(fn > FN && (Dev_State == sRun || Dev_State == sHold))    // In rejime RUN,HOLD
    {        
        active_ch = prot->active_Channels;
        slotSendToServer(MEASURE_REQUEST);
        wait_process(wMeasure, 1000, "");
    }    
}
//-----------------------------------------------------------------------------
//--- Check_Compatibility()
//-----------------------------------------------------------------------------
bool Run::Check_Compatibility()
{
    int i,j;
    int count_tubes;
    int active_ch, mask, res, active_ch_New = 0x00;
    QString text, kod;
    QStringList list;
    bool ok;
    QString fluor_name[COUNT_CH] = FLUOR_NAME;
    int measured_Time = 0;
    int count_expo = 2;
    rt_Test *ptest;

    QVector<string> programm;
    int grad = 0;
    bool gradient = false;
    QTreeWidgetItem *item;
    QString name_Block;    

    message.setStandardButtons(QMessageBox::Ok);    

    flag_Compatibility = false;
    kod = "";

    // 1. protocol - isNull ???
    if(!prot || prot->program.size() < 1)
    {
        //qDebug() << "compatibility: protocol - Null";

        if(map_InfoDevice.value(INFODEV_serName,"").isEmpty()) NameDev->setStyleSheet("QLabel{background-color: rgb(220, 220, 220)}");
        else NameDev->setStyleSheet("QLabel{background-color: rgb(128, 255, 128)}");
        NameProt->setStyleSheet("QLabel{background-color: rgb(220, 220, 220)}");
        return(false);
    }

    // 2. device - isNull ???
    if(map_InfoDevice.value(INFODEV_serName,"").isEmpty())
    {
        //qDebug() << "compatibility: device - Null";

        if(!prot || prot->program.size() < 1) NameProt->setStyleSheet("QLabel{background-color: rgb(128, 255, 128)}");
        else NameProt->setStyleSheet("QLabel{background-color: rgb(128, 255, 128)}");
        NameDev->setStyleSheet("QLabel{background-color: rgb(220, 220, 220)}");
        return(false);
    }



    //........................
    while(1)
    {
        // 3. count of tubes:
        text = map_InfoDevice.value(INFODEV_serName,"000");
        count_tubes = text.trimmed().mid(1,1).toInt();
        switch(count_tubes)
        {
        default:    count_tubes = 0;   break;
        case 5:     count_tubes = 96;  break;
        case 6:     count_tubes = 384; break;
        case 7:     count_tubes = 48;  break;
        case 8:     count_tubes = 192; break;
        }


        if(prot->count_Tubes != count_tubes)
        {
            kod = tr("Count_Tubes");
            message.setIcon(QMessageBox::Critical);
            break;
        }

        /*
        // 4. active channels
        active_ch = prot->active_Channels;
        mask = map_InfoDevice.value(INFODEV_devMask,"0").toInt(&ok,16);
        for(i=0; i<COUNT_CH; i++)
        {
            if(active_ch & (0x0f << i*4))
            {
                if(!(mask & (1 << i)))
                {
                    kod = tr("Active channels");
                    message.setIcon(QMessageBox::Warning);
                    break;
                }
            }
        }
        if(!kod.isEmpty()) break;
        */


        // 6. Minimum temperature level        
        for(i=0; i<COUNT_CH; i++)
        {
            if(active_ch & (0x0f << i*4))
            {
                measured_Time += 1000;  // On/Off LEDs, turn filters
                for(j=0; j<count_expo; j++)
                {
                    measured_Time += Expozition.at(i*2+j);      // value of expo
                    measured_Time += 500;                       // read and calculate data
                }
            }
        }
        measured_Time += 1000;  // reserve

        measured_Time = (double)(measured_Time)/1000. + 1 - ToleranceMinLevel;
        if(measured_Time > prot->PrAmpl_minimumLevel && !CALIBRATION_status && !ALARM_MinLevel)
        {
            kod = QString("%1 %2").arg(measured_Time).arg(tr("sec: Minimum temperature level"));
            break;
        }


        // 5. Type_ThermoBlock ???
        programm = QVector<string>::fromStdVector(prot->program);
        for(i=0; i<programm.size(); i++)
        {
            text = QString::fromStdString(programm.at(i));
            if(text.startsWith("XPRG "))
            {
                list = text.split(QRegExp("\\s+"));
                if(list.size() > 3)
                {
                    grad = QString(list.at(3)).toInt();
                }
                continue;
            }
            if(text.startsWith("XTCH ")) {gradient = true; break;}
        }
        item = info_device->topLevelItem(3);
        name_Block = item->text(1);
        if(gradient && !name_Block.startsWith("B96")) {kod = tr("Current device don't support gradient..."); break;}
        if(gradient && name_Block == "UNKNOWN") {kod = tr("Current device don't support gradient..."); break;}
        if(gradient && grad == 4 && name_Block == "B96A") {kod = tr("Current device don't support this gradient..."); break;}
        if(gradient && grad < 4 && (name_Block == "B96B" || name_Block == "B96C")) {kod = tr("Current device don't support this gradient..."); break;}

        break;
    }

    //qDebug() << "compatibility:" << kod;

    if(!kod.isEmpty())
    {
        text = tr("Device and Protocol are incompatible (") + kod + ")...";

        if(flag_ActivePoint) PA_Error(text);

        NameDev->setStyleSheet("QLabel{background-color: rgb(255, 255, 128)}");
        NameProt->setStyleSheet("QLabel{background-color: rgb(255, 255, 128)}");        
        message.button(QMessageBox::Ok)->animateClick(10000);
        message.setText(text);
        message.exec();
        return(false);
    }


    // Check on active channels and optic mask of the device

    //... init prot->active_Channels
    active_ch = 0;
    foreach(ptest, prot->tests)
    {
        active_ch |= ptest->header.Active_channel;
    }
    prot->active_Channels = active_ch;

    //...

    //active_ch = prot->active_Channels;
    mask = map_InfoDevice.value(INFODEV_devMask,"0").toInt(&ok,16);
    //mask = 0x0f;
    kod = "";
    for(i=0; i<COUNT_CH; i++)
    {
        if(active_ch & (0x0f << i*4))
        {
            if(!(mask & (1 << i)))
            {
                if(kod.length()) kod += ",";
                kod += fluor_name[i];
            }
            else active_ch_New |= (0x01 << i*4);
        }
    }
    if(!kod.isEmpty())
    {
        text = tr("Attention! Results for channels: ") + kod + tr("  will be missing...") + "\r\n";
        text += tr("Are You ready to continue executing the Protocol?");
        message.setIcon(QMessageBox::Warning);
        message.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        message.setDefaultButton(QMessageBox::No);
        message.button(QMessageBox::No)->animateClick(20000);
        message.setText(text);
        res = message.exec();

        if(res == QMessageBox::No)
        {
            NameDev->setStyleSheet("QLabel{background-color: rgb(255, 255, 128)}");
            NameProt->setStyleSheet("QLabel{background-color: rgb(255, 255, 128)}");
            if(flag_ActivePoint) PA_Error(tr("Device and Protocol are incompatible (") + "optic mask)...");
            //return(false);
        }
        else
        {
            prot->active_Channels = active_ch_New;
            emit sNewProtocol(prot);    // for Fluor Chart
        }

        //qDebug() << "prot->active_Channels: " << prot->active_Channels;

        QTreeWidgetItem *item = info_Protocol->topLevelItem(6); // active_Channels
        if(item)
        {
            text = QString("0x%1").arg(prot->active_Channels,5,16);
            item->setText(1, text);
        }

        if(res == QMessageBox::No) return(false);

    }


    //--- Ok ---
    flag_Compatibility = true;
    NameDev->setStyleSheet("QLabel{background-color: rgb(128, 255, 128)}");
    NameProt->setStyleSheet("QLabel{background-color: rgb(128, 255, 128)}");
    return(true);
}

//-----------------------------------------------------------------------------
//--- Check_ChangeDevice()
//-----------------------------------------------------------------------------
bool Run::Check_ChangeDevice(int state)
{
    int i;
    int current_state = Dev_State;
    int new_state = state;
    int count = Queue_State.size();

    if(current_state == new_state) return(false);
    if(Queue_State.size() < 3) return(false);

    for(i=0; i<2; i++)
    {
        if(new_state != Queue_State.at(count-1-i)) return(false);
    }

    return(true);
}

//-----------------------------------------------------------------------------
//--- slot_ChangeStateDevice
//-----------------------------------------------------------------------------
void Run::slot_ChangeStateDevice(int state)
{
    DevState current_state = Dev_State;
    Dev_State = (DevState)state;
    int res;
    QString fn;
    QString question_0, question_1;
    rt_Preference *property;

    emit finished_WaitProcess();

    //qDebug() << "Dev_State: " << Dev_State << current_state << flag_Compatibility << flag_ControlRun;

    // 1. Run
    if(Dev_State == sNothing && flag_Compatibility && flag_ControlRun) RunDev->setDisabled(false);
    else RunDev->setDisabled(true);

    // 2. Pause
    if(Dev_State == sRun) PauseDev->setDisabled(false);
    else PauseDev->setDisabled(true);

    // 3. Stop
    if(Dev_State > sNothing && Dev_State != sPause) StopDev->setDisabled(false);
    else StopDev->setDisabled(true);

    // 4. Tab_Control
    if(Dev_State > sUnknown) {Tab_Control->setDisabled(false); OpenClose_Box->setDisabled(false);}
    else {Tab_Control->setDisabled(true); OpenClose_Box->setDisabled(true);}

    // 5. OpenProtocol
    if(Dev_State < sRun) {OpenProtocol->setDisabled(false); LastProtocol->setDisabled(false); ClearProtocol->setDisabled(false);}
    else {OpenProtocol->setDisabled(true); LastProtocol->setDisabled(true); ClearProtocol->setDisabled(true);}
    if(Dev_State == sRun)
    {
        Tab_Protocol->setCurrentIndex(0);
    }

    // 6. LastRun
    if(Dev_State == sNothing) LastRun->setDisabled(false);
    else  LastRun->setDisabled(true);

    // 7. Error Info
    if(Dev_State < sNothing) Error_Info->clear();


    switch(current_state)
    {
    default:
    case sUnknown:      if(state > sNothing && state <= sWarming_up)
                        {
                            message.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                            message.setDefaultButton(QMessageBox::Yes);
                            message.setIcon(QMessageBox::Question);
                            message.button(QMessageBox::Yes)->animateClick(7000);
                            question_0 = tr("Attention! Program is running in the device!");
                            question_1 = tr("Would you like to continue the program execution?");
                            message.setText(question_0 + "\r\n" + question_1);
                            res = message.exec();

                            if(res == QMessageBox::No)
                            {
                                slotSendToServer(STOP_REQUEST);
                                //Check_Compatibility();
                            }
                            if(res == QMessageBox::Yes)
                            {
                                //... open new protocol ...
                                fn = qApp->applicationDirPath() + "/device/" +
                                     map_InfoDevice.value(INFODEV_serName) + "/run.rt";
                                Open_Protocol(fn);
                                Tab_Protocol->setCurrentWidget(runtime_Program);
                                //if(!Web_timer.isActive()) Web_timer.start();
                            }
                        }
                        //else Check_Compatibility();

                        break;

    case sNothing:      if(state == sNothing && flag_LastRun)
                        {
                            if(Create_MeasureProtocol("last_run.rt"))
                            {
                                flag_LastRun = false;
                                if(Analyze_immediately->isChecked())
                                {
                                    QEvent *e = new QEvent((QEvent::Type)3003);
                                    QApplication::sendEvent(this->parentWidget(), e);
                                }
                            }
                         }

                        break;

    case sHold:
    case sRun:
                        if((state == sNothing || state == sHold) && !flag_SavingResult && FN > 0)
                        {                            
                            if(state == sHold && !Check_HoldLevel()) break;

                            //qDebug() << "flag: " << flag_ActivePoint;

                            if(SaveImage_rejime->isChecked() && !flag_SavingOpticDev)
                            {
                                qDebug() << "send GETALLVIDEO_REQUEST: ";
                                flag_SavingOpticDev = true;
                                slot_ReadOpticDev();
                            }
                            else CreateMeasure_And_GoToAnalysis();

                            /*if(Create_MeasureProtocol())
                            {
                                flag_SavingResult = true;                                

                                if(flag_ActivePoint) PA_RunProtocol();
                                else
                                {
                                    message.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                                    message.setDefaultButton(QMessageBox::Yes);
                                    message.setIcon(QMessageBox::Information);
                                    message.button(QMessageBox::Yes)->animateClick(10000);
                                    message.setText(tr("Will You analyze the optical data?"));
                                    res = message.exec();

                                    if(res == QMessageBox::Yes)
                                    {
                                        QEvent *e = new QEvent((QEvent::Type)3003);
                                        QApplication::sendEvent(this->parentWidget(), e);
                                    }
                                }
                            }*/
                        }
                        if((state == sNothing || state == sHold) &&
                           !flag_SavingResult &&
                           FN == 0 &&
                           flag_ActivePoint)
                        {
                            PA_RunProtocol();
                        }
                        break;

    case sPause:        break;

    case sWarming_up:   break;
    }

    if(Dev_State > sNothing) plot_temperature->set_Enable(true);
    else plot_temperature->set_Enable(false);

    if(Dev_State == sRun) StandBy->setEnabled(true);
    else StandBy->setEnabled(false);

    //... WebProtocol ...
    if(Dev_State >= sRun)
    {        
        foreach(property, prot->preference_Pro)
        {
            if(property->name == "Web_Protocol")
            {
                //qDebug() << QString::fromStdString(property->name);

                if(property->value == prot->regNumber) flag_WebProtocol = true;
                else flag_WebProtocol = false;


                break;
            }
        }
    }
    if(Dev_State < sRun) flag_WebProtocol = false;
    if(flag_WebProtocol && !Web_timer.isActive()) Web_timer.start();        // Web_Timer
    if(!flag_WebProtocol) Web_timer.stop();
    //qDebug() << "flag_WebProtocol: " << flag_WebProtocol << Web_timer.isActive();
    //...

    //qDebug() << "Dev_State: " << Dev_State << current_state << flag_ActivePoint;

    if(Dev_State == sRun && (current_state < sRun || current_state >= sWarming_up) && flag_ActivePoint)
    {
        //qDebug() << "SaveImage_rejime: " << action_event->point_action->Param << action_event->point_action->Unique_Name;

        if(action_event &&
           action_event->point_action->Unique_Name == RUN_PROTOCOL &&
           action_event->point_action->Param.value("VideoImage","") == "On")
        {
            SaveImage_rejime->setCheckState(Qt::Checked);
            //QTimer::singleShot(1000, this, SLOT(change_SaveImageRejime()));
        }
    }

    if(Dev_State < sRun || Dev_State > sHold) slot_RunProcess(0, "");   // ProgressBar for RUN

}
//-----------------------------------------------------------------------------
//--- CreateMeasure_And_GoToAnalysis()
//-----------------------------------------------------------------------------
void Run::CreateMeasure_And_GoToAnalysis()
{
    int res;
    QString text;

    if(Create_MeasureProtocol())
    {
        flag_SavingResult = true;

        if(flag_ActivePoint) PA_RunProtocol();
        else
        {
            if(Dev_State == sHold)
            {
                message.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                message.setDefaultButton(QMessageBox::No);
                message.button(QMessageBox::No)->animateClick(10000);
                message.setIcon(QMessageBox::Question);
                text = tr("We are starting to analyze the optical data...") + "\r\n";
                text += tr("Do you want to finish the standby mode?");
            }
            else
            {
                message.setStandardButtons(QMessageBox::Ok);
                message.button(QMessageBox::Ok)->animateClick(10000);
                message.setIcon(QMessageBox::Information);
                text = tr("The program has finished, we are starting to analyze the optical data.");
            }

            message.setText(text);
            //message.setText(tr("Will You analyze the optical data?"));
            res = message.exec();

            if(Dev_State == sHold && res == QMessageBox::Yes)
            {
                // request: Stop Run
                slotSendToServer(STOP_REQUEST);
            }


            // to Analysis
            QEvent *e = new QEvent((QEvent::Type)3003);
            QApplication::sendEvent(this->parentWidget(), e);

        }
    }
}
//-----------------------------------------------------------------------------
//--- slot_ReadOpticDev()
//-----------------------------------------------------------------------------
void Run::slot_ReadOpticDev()
{
    map_ReadOpticDev.clear();

    //int mask = 0x1111;
    //int fn = 22;

    map_ReadOpticDev.insert(OPTICMASK_DATA, QString("%1").arg(prot->active_Channels));
    map_ReadOpticDev.insert(OPTICFN_DATA, QString("%1").arg(fn_dev));

    //map_ReadOpticDev.insert(OPTICMASK_DATA, QString("%1").arg(mask));
    //map_ReadOpticDev.insert(OPTICFN_DATA, QString("%1").arg(fn));

    qDebug() << "send GETALLVIDEO_REQUEST: " << map_ReadOpticDev;

    slotSendToServer(GETALLVIDEO_REQUEST);
    wait_process(wOpen, 100, tr("Read Video_Picture..."));
}

//-----------------------------------------------------------------------------
//--- Ver_uOPTICS()
//-----------------------------------------------------------------------------
double Run::Ver_uOPTICS()
{
    double ver = 0;
    bool ok;

    QString text = map_InfoDevice.value(INFODEV_version);
    QStringList list = text.split("\r\n");

    foreach(text, list)
    {
        if(text.startsWith("OPTICS"))
        {
            text = text.mid(7,4);
            ver = text.toDouble(&ok);
            if(!ok) ver = 0;

            break;
        }
    }

    return(ver);
}
//-----------------------------------------------------------------------------
//--- slot_GetMeasure
//-----------------------------------------------------------------------------
void Run::slot_SendMeasureToWeb(int fn_meas)
{
    int i;
    int count_ch = 0;
    short value;
    QDomDocument doc;
    QVector<short> measure;
    rt_Measurement *meas;
    QByteArray data;
    QString text;
    QString id_prot = QString::fromStdString(prot->regNumber);
    QString message;
    QString dev_name;

    QStringList list_ch, list;
    list << "fam" << "hex" << "rox" << "cy5" << "cy55";

    QDomNode xmlNode = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\"");
    doc.insertBefore(xmlNode, doc.firstChild());

    QDomElement  root = doc.createElement("measurements");
    doc.appendChild(root);

    root.setAttribute("time", QString("%1").arg(QDateTime::currentMSecsSinceEpoch()));
    root.setAttribute("fn", fn_meas);

    dev_name = map_InfoDevice.value(INFODEV_serName,"");
    root.appendChild(MakeElement(doc,"id_device",dev_name));

    for(i=0; i<COUNT_CH; i++)
    {
        if(prot->active_Channels & (0x0f << i*4)) {count_ch++; list_ch << list.at(i);}
    }
    for(i=0; i<count_ch; i++)
    {
        meas = prot->meas.at(count_ch*COUNT_EXP*(fn_meas-1) + COUNT_EXP*i);
        measure = QVector<short>::fromStdVector(meas->measurements);
        /*
        data.clear();
        QDataStream stream(&data, QIODevice::WriteOnly);
        stream << measure;  // data size = size (4 byte) + measure size (96*2)bytes

        //data = QByteArray::fromRawData(reinterpret_cast<const char*>(measure.constData()), sizeof(short) * measure.size());

        text = data.toBase64();
        */
        text = "";
        foreach(value, measure)
        {
            text += QString("%1 ").arg(value & 0x0fff);
        }

        QDomElement item = MakeElement(doc,"item",text);
        root.appendChild(item);
        item.setAttribute("ch", list_ch.at(i));        
    }    

    message = id_prot + ";" + doc.toString();
    parentWidget()->setProperty("ProtocolInfo_ToWeb", QVariant(message));
    QEvent *e = new QEvent((QEvent::Type)3013);
    QApplication::sendEvent(this->parentWidget(), e);


    //qDebug() << "Measure to Web: " << message;

    /*
    data = QByteArray::fromBase64(text.toStdString().c_str());
    qDebug() << "to Web_: " << data.size();    
    QDataStream stream(data);    
    stream >> measure;
    */

    /*QFile file(dir_temp.path() + "/to_Web.rt");
    if(file.open(QIODevice::WriteOnly))
    {
        QTextStream(&file) << doc.toString();
        file.close();
    }*/
}

//-----------------------------------------------------------------------------
//--- slot_GetMeasure
//-----------------------------------------------------------------------------
bool Run::slot_GetMeasure(int fn_meas)
{
    int i,j,k,m;
    int id;
    double value;
    int count = map_Measure.size();
    QByteArray b;    
    QString text;
    int count_ch = 0;
    rt_Measurement* measure;

    //... offset exposition ...
    int offset_expo = 40;
    if(ver_OPTICS >= 3.03) offset_expo = 118;
    //...

    int num_tubes = 96;
    if(prot->count_Tubes == 48) num_tubes = 48;

    int all_measure = prot->count_MC + prot->count_PCR;
    int count_tubes = prot->count_Tubes;

    int*    p_int;
    short*  p_short;

    int sec_type;
    int sec_sequence_number;
    short fn;                // number of measuring
    short type_meas;         // type of measuring
    short block_number;      // number of block in temperature program
    short cycle_rep;		 // current cycle in block
    short optical_channel;   // 0 - Fam, 1 - Hex, 2 - Rox, 3 - Cy, ...
    short exp_number;        // count of exposition
    short blk_exp;		     // black level
    short ovf_exp;			 // flag overflow
    unsigned short value_exp;         // value of exp

    message.setStandardButtons(QMessageBox::Ok);
    message.setIcon(QMessageBox::Warning);

    int num_sector = div(prot->count_Tubes,96).quot;
    if(num_sector < 1) num_sector = 1;
    for(i=0; i<COUNT_CH; i++)
    {
        if(prot->active_Channels & (0x0f<<i*4)) count_ch++;
    }

    //qDebug() << "count,count_ch,num_sector" << count << count_ch << num_sector << prot->count_Tubes << fn_meas;

    if(count_ch*num_sector != count)
    {        
        message.button(QMessageBox::Ok)->animateClick(1000);
        message.setText(tr("Invalid data format...(num_sector)"));
        message.exec();
        return(false);
    }

    QByteArray single_block;
    single_block.reserve(512*num_sector);

    for(i=0; i<count_ch; i++)
    {
        single_block.clear();
        for(j=0; j<num_sector; j++)
        {
            text = QString("_%1").arg(i*num_sector+j);
            text = MEASURE_Data + text;
            b = map_Measure.value(text,"");
            single_block.append(b);
        }
        for(j=0; j<COUNT_EXP; j++)
        {
            measure = new rt_Measurement();
            prot->meas.push_back(measure);
            measure->measurements.reserve(prot->count_Tubes);

            sec_type = *(int*)(single_block.mid(0,4).data());                   // 4
            sec_sequence_number = *(int*)(single_block.mid(4,4).data());        // 8
            fn = *(short*)(single_block.mid(8,2).data());                       // 10
            type_meas = *(short*)(single_block.mid(10,2).data());               // 12
            block_number = *(short*)(single_block.mid(12,2).data());            // 14
            cycle_rep = *(short*)(single_block.mid(14,2).data());               // 16
            optical_channel = *(short*)(single_block.mid(16,2).data());         // 18
            exp_number = *(short*)(single_block.mid(18,2).data());              // 20

            blk_exp = *(short*)(single_block.mid(20+j*4,2).data());             // 22
            ovf_exp = *(short*)(single_block.mid(22+j*4,2).data());             // 24
            //...                                                               // 26-28
            value_exp = *(unsigned short*)(single_block.mid(offset_expo+j*2,2).data());           // 30-32
            value_exp = qRound((double)value_exp * Coef_Expo);      // COEF_EXPO
            if(value_exp <= 0.)
            {
                value_exp = Expozition.at(i*COUNT_EXP+j);
            }
            //value_exp = QString::number((double)value_exp*0.308,'f',0).toInt();
            //...
            //reserve char[96]                                                  // 128
            //...

            measure->fn = fn;
            measure->type_meas = type_meas;
            measure->block_number = block_number;
            measure->cycle_rep = cycle_rep;
            measure->optical_channel = optical_channel;
            measure->num_exp = j;
            measure->exp_value = value_exp;
            measure->blk_exp = blk_exp;

            for(m=0; m<num_sector; m++)
            {                
                for(k=0; k<num_tubes; k++)
                {
                    p_short = (short*)(single_block.mid(128+j*96*2+m*512+k*2,2).data());
                    measure->measurements.push_back(*p_short);
                }
            }


            /*qDebug() << "meas_" << i*2+j;
            qDebug() << sec_type << sec_sequence_number << fn << type_meas << block_number
                     << cycle_rep << optical_channel
                     << exp_number << blk_exp << ovf_exp << value_exp;*/

            //qDebug() << temp;


            //if(!j)  // only first expo
            if(j == 1)  // only second expo
            {
                for(m=0; m<prot->count_Tubes; m++)
                {
                    value = 0x0fff & measure->measurements.at(m);
                    value *= COEF_EXPO_1;
                    id = i*count_tubes*all_measure + m*all_measure + (fn_meas-1);
                    plot_fluor->Y.replace(id,value);                    
                }                
            }
        }
    }

    //qDebug() << "fn =" << fn_meas << plot_fluor->Y;

    plot_fluor->current_fn = fn_meas;
    FN = fn_meas;

    if(fn_dev > 0)
    {
        k = (100.*FN)/fn_dev;
        text = QString(tr("Measurements %1(%2)")).arg(FN).arg(fn_dev);
        Display_ProgressBar(k, text);
    }


    //... for Holding ...
    /*if(Dev_State == sHold &&
       FN == all_measure &&
       FN > 0)
    {
        emit sChangeStateDevice(sHold);
    }*/
    return(true);
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Run::slotError(QAbstractSocket::SocketError err)
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
    qDebug() << strError;

    if(err == QAbstractSocket::RemoteHostClosedError)
    {
        //list_dev->dev_reconnected.start(1000);
        list_dev->name_reconnect = NameDev->text().trimmed();
        QTimer::singleShot(1000, list_dev, SLOT(sReConnected()));
        NameDev->setStyleSheet("QLabel{background-color: rgb(255, 128, 128)}");
        slotSuspend();
        map_InfoDevice.clear();
        emit sChangeStateDevice(sUnknown);
    }

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Run::slot_RecievedRunButton()
{
    //if(!flag_ActivePoint && Dev_State < sRun)
    //{
        QEvent *e = new QEvent((QEvent::Type)3016);
        QApplication::sendEvent(this->parentWidget(), e);
    //}
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Run::slot_ReceivedActionPoint(Action_Event *p_event)
{
    int i,res;
    QString text;
    QStringList list_points;
    short value;
    int count_ActiveCh;
    bool def_param = false;

    list_points << LIST_ACTIONPOINTS_RUN;

    int id = list_points.indexOf(p_event->point_action->Unique_Name);
    action_event = p_event;
    flag_ActivePoint = true;

    QMap<QString,QString> map_pro;

    //qDebug() << "IN: " << p_event->point_action->Unique_Name << flag_ActivePoint;

    switch(id)
    {
    default:    PA_Error("???");    break;

    case -1:    PA_Error("Unknown Point Action"); break;

    case 0:                         // OpenProtocol
                if(Dev_State < sRun)
                {
                    text = p_event->point_action->Param.value("FileName");
                    if(text.startsWith("../"))
                    {
                        text.replace("../","/");
                        text = qApp->applicationDirPath() + text;
                    }
                    //qDebug() << "Param.value(FileName): " << text;
                    Open_Protocol(text);
                    QTimer::singleShot(3000, this, SLOT(PA_OpenProtocol()));
                }
                else
                {
                    PA_Error("Device is run");
                }

                break;

    case 1:                         // OpenDevice
                if(Dev_State < sRun)
                {
                    if(m_pTcpSocket->state() == QAbstractSocket::ConnectedState &&
                       (map_InfoDevice.value(INFODEV_serName) == action_event->point_action->Param.value("DeviceName") ||
                        action_event->point_action->Param.value("DeviceName","").isEmpty()))
                    {
                        PA_Ok();
                    }
                    else
                    {
                        if(!action_event->point_action->Param.value("DeviceName","").isEmpty())
                        {
                            if(m_pTcpSocket->state() == QAbstractSocket::ConnectedState)    // disconnect
                            {
                                list_dev->PA_active = true;
                                list_dev->PA_DeviceName = action_event->point_action->Param.value("DeviceName");
                                list_dev->dev_state = Dev_State;
                                list_dev->exec();
                            }
                            list_dev->PA_active = true;                                     // connect
                            list_dev->PA_DeviceName = action_event->point_action->Param.value("DeviceName");
                        }

                        list_dev->dev_state = Dev_State;
                        list_dev->exec();
                        QTimer::singleShot(7000, this, SLOT(PA_OpenDevice()));
                    }                    
                }
                else
                {                    
                    PA_Error("Device is run");
                }

                break;

    case 2:                         // RunProtocol
                Sleep(1000);
                QApplication::processEvents();
                //Sleep(2000);
                if(!RunDev->isEnabled()) {PA_Error("Run is disable"); break;}

                // 1.
                //slot_ClosePrerunRun();

                RunDev->click();
                break;

    case 3:                         // Run Stop
                if(StopDev->isEnabled())
                {
                    StopDev->setDisabled(true);
                    StopDev->clearFocus();
                    slotSendToServer(STOP_REQUEST);
                    wait_process(wStop, 100, tr("Stopping..."));
                }

                flag_ActivePoint = false;
                break;

    case 4:                         // Get BarCode
                if(m_pTcpSocket->state() == QAbstractSocket::ConnectedState && Dev_State < sRun)
                {
                    slot_CloseBlock();
                }
                else  PA_Error("");
                break;

    case 5:                         // OpenProtocolBarcode
    case 8:                         // OpenProtocolID
                text = action_event->point_action->Param.value("FileName", "");
                if(text.isEmpty())
                {
                    PA_Error("File is absent");
                }
                else
                {
                    if(Dev_State < sRun)
                    {
                        Open_Protocol(p_event->point_action->Param.value("FileName"));

                        QTimer::singleShot(3000, this, SLOT(PA_OpenProtocol()));
                    }
                    else PA_Error("Device is run");
                }
                break;

    case 6:                         // ScanCoefficients
                //qDebug() << "ScanCoefficients...";
                count_ActiveCh = 0;
                for(i=0; i<COUNT_CH; i++)
                {
                    if(prot->active_Channels & (0x0f<<4*i)) count_ActiveCh++;
                }
                text = action_event->point_action->Param.value("Default_Param", "");
                if(text.startsWith("yes")) def_param = true;

                //qDebug() << "def_Param: " << def_param;

                scan_dialog = new ScanDialog(this, def_param);
                scan_dialog->count_ActiveCh = count_ActiveCh;
                connect(scan_dialog, SIGNAL(sReadSettings(QMap<QString,QString>*)),
                                            this, SLOT(Read_CalibrationSettings(QMap<QString,QString>*)));
                scan_dialog->name_Dev = map_InfoDevice.value(INFODEV_serName);
                scan_dialog->exec();

                //slot_CloseBlock();
                //Sleep(3000);

                res = scan_dialog->Result;
                switch(res)
                {
                default:        PA_Error(tr("???"));
                                break;

                case 0:         map_pro.insert("TPeaks", scan_dialog->TPeaks_values->text().trimmed());
                                map_pro.insert("Amplitude", scan_dialog->Amplitude_values->text().trimmed());
                                map_pro.insert("ID_plate", scan_dialog->ID_plate->text().trimmed());
                                map_pro.insert("DTcheck_LOT", scan_dialog->sn_LOT.trimmed());
                case 1:
                                if(Add_CalibrationProperties(map_pro)) PA_Ok();
                                else PA_Error(tr("unable to create calibration property..."));

                                break;
                }
                delete scan_dialog;


                break;

    case 7:                         // SetPlate
                if(m_pTcpSocket->state() == QAbstractSocket::ConnectedState)
                {
                    slot_OpenBlock();
                    Sleep(12000);

                    message.setStandardButtons(QMessageBox::Ok);
                    message.setIcon(QMessageBox::Information);
                    text = tr("Set the plate into the device and only after that you can continue...");
                    message.setText(text);
                    message.exec();
                    //QMessageBox::information(this, tr("Attention"), tr("Set the plate into the device and only after that you can continue..."), QMessageBox::Ok);

                    slot_CloseBlock();
                    Sleep(15000);
                    PA_Ok();
                }

                break;

    case 9:                         // ChangeExposure

                text = action_event->point_action->Param.value("Exposure", "0");
                value = text.toInt();
                if(change_Exposure(value, &Expozition)) PA_Ok();
                else  PA_Error("");

                break;


    case 10:                        // ChangeHeightTubes
                PA_TubeHeiht();
                //measure_TubesHeight();
                break;

    case 11:                        // BackupExposure

                qApp->processEvents();
                QTimer::singleShot(5000, this, SLOT(backup_Exposure()));
                break;

    case 12:                        // BackupHeightTubes

                qApp->processEvents();
                QTimer::singleShot(3000, this, SLOT(backup_TubeHeiht()));
                break;

    case 13:                        // OpenBlock
                text = p_event->point_action->Param.value("DeviceName","");
                /*if(text.trimmed().isEmpty() || text.trimmed() != map_InfoDevice.value(INFODEV_serName,""))
                {
                    PA_Error("Device is not connected");
                    break;
                }*/
                slot_OpenBlock();
                break;

    case 14:                        // CloseBlock
                text = p_event->point_action->Param.value("DeviceName","");
                /*if(text.trimmed().isEmpty() || text.trimmed() != map_InfoDevice.value(INFODEV_serName,""))
                {
                    PA_Error("Device is not connected");
                    break;
                }*/
                slot_CloseBlock();
                break;
    }
}
//-----------------------------------------------------------------------------
//--- PA_RunTubeHeiht(QString text)
//-----------------------------------------------------------------------------
void Run::PA_RunTubeHeiht(QString text)
{
    //qDebug() << "PA_RunTubeHeiht, execCMD RTUB: " << text;
    int value = 0;
    QTextStream(&text) >> value;

    if(value < 100 || value > 100000)
    {
        PA_Error(QString("read RTUB: %1").arg(value));
        return;
    }
    text = QString::number(value);

    QString dir_path = qApp->applicationDirPath();
    QString fn = dir_path + "/calibration" + "/Calibration.ini";
    QSettings *CalibrSettings = new QSettings(fn, QSettings::IniFormat);

    CalibrSettings->beginGroup("Change_Exposure");
    CalibrSettings->setValue("TubeHeiht", text);
    CalibrSettings->endGroup();
    delete CalibrSettings;


    // go to measuring
    measure_TubesHeight();
}

//-----------------------------------------------------------------------------
//--- PA_TubeHeiht()
//-----------------------------------------------------------------------------
void Run::PA_TubeHeiht()
{
    int id = 4; // Motor uC

    map_CMD.clear();
    map_CMD.insert(EXECCMD_CMD, "RTUB");
    map_CMD.insert(EXECCMD_UC, QString::number(id));

    slotSendToServer(EXECCMD_REQUEST);
}
//-----------------------------------------------------------------------------
//--- backup_TubeHeiht()
//-----------------------------------------------------------------------------
void Run::backup_TubeHeiht()
{
    QString text;
    int value;
    bool ok;

    QString dir_path = qApp->applicationDirPath();
    QString fn = dir_path + "/calibration" + "/Calibration.ini";
    QSettings *CalibrSettings = new QSettings(fn, QSettings::IniFormat);

    CalibrSettings->beginGroup("Change_Exposure");
    text = CalibrSettings->value("TubeHeiht", "").toString().trimmed();
    CalibrSettings->setValue("TubeHeiht", "");
    CalibrSettings->endGroup();

    delete CalibrSettings;

    //qDebug() << "TubeHeiht: " << text;
    if(text.isEmpty()) {PA_Ok(); return;}

    value = text.toInt(&ok);
    if(ok) slot_MeasuredTubeHeiht(value);
}

//-----------------------------------------------------------------------------
//--- backup_Exposure()
//-----------------------------------------------------------------------------
bool Run::backup_Exposure()
{
    QString text;
    QString data = "";
    QStringList list;
    short value;
    bool ok;

    QString dir_path = qApp->applicationDirPath();
    QString fn = dir_path + "/calibration" + "/Calibration.ini";
    QSettings *CalibrSettings = new QSettings(fn, QSettings::IniFormat);

    CalibrSettings->beginGroup("Change_Exposure");
    text = CalibrSettings->value("Exposure", "").toString().trimmed();
    CalibrSettings->setValue("Exposure", "");
    CalibrSettings->endGroup();

    delete CalibrSettings;

    qDebug() << "backup expo: " << text;
    if(text.isEmpty()) {PA_Ok(); return(true);}

    list = text.split(" ");
    foreach(text, list)
    {
        value = text.toInt(&ok);
        if(!ok) return(true);

        value = qRound((double)(value)/Coef_Expo);    // COEF_EXPO
        data += QString(" %1").arg(value);
    }
    data = data.trimmed();    
    slot_SavePar(1, data);    

    return(true);

}

//-----------------------------------------------------------------------------
//--- change_Exposure(short value, QVector<quint16> *vector)
//-----------------------------------------------------------------------------
bool Run::change_Exposure(short value, QVector<quint16> *vector)
{
    int i;
    quint16 val, qval;
    bool sts = true;
    QVector<quint16> vec;
    QString data = "";
    QString data_old = "";    

    qDebug() << "expo: " << *vector << value;

    //if(value < 100 || value >= 10000) return(false);

    QString dir_path = qApp->applicationDirPath();
    QString fn = dir_path + "/calibration" + "/Calibration.ini";
    QSettings *CalibrSettings = new QSettings(fn, QSettings::IniFormat);
    CalibrSettings->beginGroup("Change_Exposure");    
    CalibrSettings->setValue("Exposure", "");

    foreach(val, *vector)
    {
        data_old += QString(" %1").arg(val);
    }
    CalibrSettings->setValue("Exposure", data_old);

    CalibrSettings->endGroup();
    delete CalibrSettings;

    return(sts);


    /*for(i=0; i<vector->size(); i++)
    {
        if(div(i,2).rem) continue;
        if(div(i,2).quot >= 4) break;       // only 4 channels

        val = vector->at(i);
        vec.append(val);
    }

    qDebug() << "save expo: " << vec;

    foreach(val, vec)
    {
        if(fabs((double)(val - value)/(double)(value)) > 0.01)
        {
            i = 0;
            foreach(val, *vector)
            {
                data_old += QString(" %1").arg(val);
                if(div(i,2).rem == 0) qval = value;
                else qval = qRound((double)(value)/COEF_EXPO_1);

                if(val > 0) val = qRound((double)(qval)/(double)(COEF_EXPO));
                data += QString(" %1").arg(val);
                i++;
            }
            data = data.trimmed();
            CalibrSettings->beginGroup("Change_Exposure");
            CalibrSettings->setValue("Exposure", data_old);
            CalibrSettings->endGroup();

            //qDebug() << "expo to device: " << data;
            //slot_SavePar(1, data);

            break;
        }
    }

    delete CalibrSettings;
    //Sleep(5000);

    return(sts);*/
}

//-----------------------------------------------------------------------------
//--- Read_CalibrationSettings(QMap<QString, QString> *map)
//-----------------------------------------------------------------------------
void Run::Read_CalibrationSettings(QMap<QString, QString> *map)
{
    QString text;
    double a,b,c;
    QString dir_path = qApp->applicationDirPath();
    QString fn = dir_path + "/calibration" + "/Calibration.ini";
    QSettings *CalibrSettings = new QSettings(fn, QSettings::IniFormat);

    // Coefficients
    CalibrSettings->beginGroup("Coefficients");

    text = CalibrSettings->value("TPeaks", "100 100 100 100").toString();
    map->insert("TPeaks",text);
    text = CalibrSettings->value("Amplitude", "1000 1000 1000 1000").toString();
    map->insert("Amplitude",text);
    text = CalibrSettings->value("TCorrection", "1 1 1 1").toString();
    map->insert("TCorrection",text);

    CalibrSettings->endGroup();

    // Borders_mask
    CalibrSettings->beginGroup("Borders_mask");
    a = CalibrSettings->value("mask_quality","1").toDouble();
    b = CalibrSettings->value("distance_channels","1").toDouble();
    c = CalibrSettings->value("difference_device","1").toDouble();
    text = QString("%1 %2 %3").arg(a,0,'f',1).arg(b,0,'f',1).arg(c,0,'f',1);
    map->insert("Borders_mask",text);
    CalibrSettings->endGroup();

    // Borders_temperature
    CalibrSettings->beginGroup("Borders_temperature");
    a = CalibrSettings->value("abs_deviation","1").toDouble();
    b = CalibrSettings->value("irregularity_plate","1").toDouble();
    c = CalibrSettings->value("plate_deviation","1").toDouble();
    text = QString("%1 %2 %3").arg(a,0,'f',1).arg(b,0,'f',1).arg(c,0,'f',1);
    map->insert("Borders_temperature",text);
    CalibrSettings->endGroup();

    // Borders_Optic
    CalibrSettings->beginGroup("Borders_Optic");
    a = CalibrSettings->value("abs_deviation","100").toDouble();
    b = CalibrSettings->value("irregularity_plate","100").toDouble();
    c = CalibrSettings->value("plate_deviation","100").toDouble();
    text = QString("%1 %2 %3").arg(a,0,'f',1).arg(b,0,'f',1).arg(c,0,'f',1);
    map->insert("Borders_Optic",text);
    CalibrSettings->endGroup();

    delete CalibrSettings;
}

//-----------------------------------------------------------------------------
//--- Add_CalibrationProperties()
//-----------------------------------------------------------------------------
bool Run::Add_CalibrationProperties(QMap<QString,QString> map)
{
    bool res = false;
    QString str, key;
    QString name_dev;
    QMap<QString,QString> calibr_Settings;

    QDomDocument    doc;
    QDomElement     root;
    QDomElement     property;
    QDomElement     item;

    // 1. read from file.ini
    Read_CalibrationSettings(&calibr_Settings);

    // 2. replace scanning value
    foreach(key, map.keys())
    {
        str = map.value(key);
        calibr_Settings.insert(key,str);
    }

    // 3. add property about video data location:
    key = "VideoData_Catalogue";
    name_dev = map_InfoDevice.value(INFODEV_serName);
    str = qApp->applicationDirPath() + "/VIDEO/" + name_dev;
    calibr_Settings.insert(key,str);

    // 4. add to properties
    QFile file(dir_temp.path() + "/in.rt");
    //qDebug() << "fname: " << file.fileName() << file.exists();
    if(file.exists() && file.open(QIODevice::ReadOnly))
    {
        if(doc.setContent(&file))
        {
            file.close();

            root = doc.documentElement();
            property = root.firstChildElement("properties");
            if(!property.isNull())
            {
                foreach(key, calibr_Settings.keys())
                {
                    str = calibr_Settings.value(key);
                    item = doc.createElement("item");
                    item.appendChild(MakeElement(doc,"name",key));
                    item.appendChild(MakeElement(doc,"value",str));
                    property.appendChild(item);
                }

                if(file.open(QIODevice::WriteOnly))
                {
                    QTextStream(&file) << doc.toString();
                    file.close();
                    res = true;
                }
            }
        }
    }

    return(res);
}
//-----------------------------------------------------------------------------
//--- SaveXML_VideoData(QDomDocument &doc, QString dir)
//-----------------------------------------------------------------------------
QDomElement Run::SaveXML_VideoData(QDomDocument &doc, QString dir)
{
    QString text, fn;
    QString base_name;
    QDomElement res = doc.createElement("video_data");
    QFile file;
    QFileInfo fi;
    QByteArray ba;
    int count_f, k = 0, num = 0;

    //dir = "c:/RealTime/VIDEO/A5X611";
    QDir dir_video(dir);

    qDebug() << "dir:" << dir;

    if(dir_video.exists())
    {
        QStringList list_image = dir_video.entryList(QDir::Files);
        count_f = list_image.size();
        foreach(fn, list_image)
        {
            num++;
            k = (100.*num)/count_f;            
            text = QString(tr("Saving video data..."));
            Display_ProgressBar(k, text);
            text = dir_video.absolutePath() + "/" + fn;

            fi.setFile(text);
            if(!fi.baseName().endsWith("_2")) continue;

            file.setFileName(text);
            if(file.exists() && file.open(QIODevice::ReadOnly))
            {
                ba = file.readAll();
                file.close();

                //qDebug() << "BA: " << fn << ba.size() << ba.toBase64().size() << qCompress(ba).size();
                //text = ba.toBase64();
                QDomElement item = doc.createElement("item");
                item.appendChild(MakeElement(doc,"name", fn));
                item.appendChild(MakeElement(doc,"value",  qCompress(ba).toBase64()));//QString::fromStdString(qCompress(ba.toBase64()).toStdString())));
                res.appendChild(item);
            }
        }
        Display_ProgressBar(0, "");

    }

    return(res);
}

//-----------------------------------------------------------------------------
//--- PA_Ok()
//-----------------------------------------------------------------------------
void Run::PA_Ok()
{
    flag_ActivePoint = false;
    //qDebug() << "OUT: PA_Ok " << action_event->point_action->Unique_Name << flag_ActivePoint;
    action_event->point_action->Param.insert("status","1");
    QApplication::sendEvent(action_event->point_action->sender, action_event);
}

//-----------------------------------------------------------------------------
//--- PA_Error()
//-----------------------------------------------------------------------------
void Run::PA_Error(QString text)
{
    flag_ActivePoint = false;
    //qDebug() << "OUT: PA_Error " << action_event->point_action->Unique_Name << flag_ActivePoint;
    action_event->point_action->Param.insert("status","0");
    action_event->point_action->Param.insert("error",text);
    QApplication::sendEvent(action_event->point_action->sender, action_event);    
}

//-----------------------------------------------------------------------------
//--- PA_OpenProtocol()
//-----------------------------------------------------------------------------
void Run::PA_OpenProtocol()
{
    bool sts = false;
    if(prot && prot->program.size() > 1) sts = true;
    flag_ActivePoint = false;

    //qDebug() << "OUT: PA_OpenProtocol() " << action_event->point_action->Unique_Name << flag_ActivePoint;

    action_event->point_action->Param.insert("status", QString::number((int)sts));
    QApplication::sendEvent(action_event->point_action->sender, action_event);
}

//-----------------------------------------------------------------------------
//--- PA_OpenDevice()
//-----------------------------------------------------------------------------
void Run::PA_OpenDevice()
{
    bool sts = false;
    flag_ActivePoint = false;

    //qDebug() << "OUT: PA_OpenDevice() " << action_event->point_action->Unique_Name << flag_ActivePoint;

    if(m_pTcpSocket->state() == QAbstractSocket::ConnectedState &&
       map_InfoDevice.value(INFODEV_serName) == action_event->point_action->Param.value("DeviceName")) sts = true;
    if(m_pTcpSocket->state() == QAbstractSocket::ConnectedState &&
       action_event->point_action->Param.value("DeviceName","").isEmpty()) sts = true;

    if(!sts) action_event->point_action->Param.insert("error", "Device is not found");
    action_event->point_action->Param.insert("status", QString::number((int)sts));
    QApplication::sendEvent(action_event->point_action->sender, action_event);    
}

//-----------------------------------------------------------------------------
//--- PA_RunProtocol()
//-----------------------------------------------------------------------------
void Run::PA_RunProtocol()
{
    if(flag_ActivePoint && action_event->point_action->Unique_Name == RUN_PROTOCOL)
    {
        // check on Hold...        
        if(Dev_State == sHold &&
           StopDev->isEnabled() &&
           action_event->point_action->Param.value("Hold","") == "stop_after_execution")
        {            
            slot_STOP(false);
        }

        flag_ActivePoint = false;
        //qDebug() << "OUT: PA_RunProtocol " << action_event->point_action->Unique_Name << flag_ActivePoint;

        // continue...
        QFile file;
        file.setFileName(dir_Device.path() + "/measurements.rt");
        if(file.exists())
        {
            action_event->point_action->Param.insert("FileName_Measurements", file.fileName());
        }

        QTimer::singleShot(3000, this, SLOT(PA_RunProtocol_afterStop()));
    }

    //qDebug() << "OUT: PA_RunProtocol " << action_event->point_action->Unique_Name;
}
//-----------------------------------------------------------------------------
//--- PA_RunProtocol_Stop()
//-----------------------------------------------------------------------------
void Run::PA_RunProtocol_Stop()
{
    //slot_STOP(false);
}

//-----------------------------------------------------------------------------
//--- PA_RunProtocol_afterStop()
//-----------------------------------------------------------------------------
void Run::PA_RunProtocol_afterStop()
{
    action_event->point_action->Param.insert("status", "1");
    QApplication::sendEvent(action_event->point_action->sender, action_event);
}

//-----------------------------------------------------------------------------
//--- PA_BarCode()
//-----------------------------------------------------------------------------
void Run::PA_BarCode()
{
    QString text = QString::fromStdString(prot->barcode);
    if(text.isEmpty()) PA_Error("BarCode is empty");
    else
    {
        action_event->point_action->Param.insert("barcode", text);
        PA_Ok();
    }
}

//-------------------------------------------------------------------------------
//--- User events
//-------------------------------------------------------------------------------
bool Run::event(QEvent *e)
{
    QString text, name;
    int num;

    // 1. connect/disconnect device
    if(e->type() == 2001)
    {
        //qDebug() << "event = " << e->type();

        Connect_Status *p_event = static_cast<Connect_Status *>(e);

        if(p_event)
        {

            if(p_event->connect && m_pTcpSocket->state() == QAbstractSocket::UnconnectedState)        //... connect:
            {

                switch(p_event->connect)
                {
                default:   break;
                case 1:     //qDebug() << "event = connect";

                            foreach(QWidget *widget, QApplication::topLevelWidgets())
                            {
                                text = widget->windowTitle();
                                if(text.startsWith(APP_NAME))
                                {
                                    num = APP_NAME.length();
                                    name = text.mid(num).trimmed();
                                    if(!name.isEmpty())
                                    {
                                        m_pTcpSocket->connectToHost(p_event->ip, p_event->port);
                                        slotResume();
                                        NameDev->setText(name);
                                        NameDev->setStyleSheet("QLabel{background-color: rgb(128, 255, 128)}");
                                        QTimer::singleShot(1000,this,SLOT(slotGetInfoDevice()));
                                    }
                                    break;
                                }
                            }

                            break;

                case 2:     //qDebug() << "event = re_connect";

                            m_pTcpSocket->connectToHost(p_event->ip, p_event->port);
                            //slotResume();
                            //NameDev->setStyleSheet("QLabel{background-color: rgb(128, 255, 128)}");
                            QTimer::singleShot(1000,this,SLOT(slotGetInfoDevice()));

                            break;
                }

                return(true);
            }
            else                        //... disconnect:
            {

                //if(m_pTcpSocket->state() != QAbstractSocket::ConnectedState) return(true);

                //qDebug() << "event = disconnect";

                m_pTcpSocket->disconnectFromHost();
                NameDev->setText(tr("No Device..."));
                NameDev->setStyleSheet("QLabel{background-color: rgb(220, 220, 220)}");
                slotSuspend();
                map_InfoDevice.clear();
                emit sChangeStateDevice(sUnknown);
                QTimer::singleShot(1000,this,SLOT(Check_Compatibility()));
                return(true);
            }
        }
    }

    // 1.1 Point_Action Event
    if(e->type() == 2002)
    {
        Action_Event *p_event = static_cast<Action_Event *>(e);
        if(p_event)
        {
            //qDebug() << "p_event: " << p_event->point_action->Unique_Name;
            emit sActionPoint(p_event);
        }

        return(true);
    }

    // 2. hide child windows
    if(e->type() == 9001)
    {
        if(!sel->isHidden() & !sel->allowedAreas()) sel->hide();
        return(true);
    }

    // 3. // Enable tubes
    if(e->type() == 1100)       // Enable tubes
    {
        EnableTubes *p_event = static_cast<EnableTubes *>(e);
        if(p_event)
        {
            plot_fluor->Change_Enable(prot, &p_event->enable_tubes);
            return(true);
        }
    }

    // 4.
    if(e->type() == 3002)       // open protocol (file from Setup)
    {        
        if(Dev_State > sNothing)
        {
            text = tr("You cann't reopen protocol! Program is executed in device...");
            emit sErrorInfo(text);
            return(true);
        }

        text = this->parentWidget()->property("pcrdocument").toString();
        Open_Protocol(text);
        return(true);
    }




    return QWidget::event(e);
}

//=== QThread =================================================================

//-------------------------------------------------------------------------------
//---
//-------------------------------------------------------------------------------
void DeviceThread::run()
{

    connect(&timer, SIGNAL(timeout()), SLOT(slotGetInfoData()));
    //this->timer.start(1000);
    //timer.start(1000);

    exec();
}
//-------------------------------------------------------------------------------
//---
//-------------------------------------------------------------------------------
void DeviceThread::slotGetInfoData()
{

    get_count++;
    //qDebug() << "dev... = " << get_count;

    emit Send_InfoData(get_count);
}
void DeviceThread::slotResume()
{
    this->timer.start(1000);
}
void DeviceThread::slotSuspend()
{
    this->timer.stop();
}




void Run::slotReadInfoData(int count)
{
    //qDebug() << "GUI... = " << count;
}
void Run::slotReadInfoData_()
{
    //qDebug() << "GUI... = main";
    //slotSendToServer();
}
void Run::slotResume()
{
    //dev_timer.start(1000);
    plot_temperature->clearPlot();
}
void Run::slotSuspend()
{
    dev_timer.stop();
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Run::slot_Emulated()
{
    QString text;
    double value;
    QTime time;

    value = QTime::currentTime().second()/10.;
    value = qSin(value)*100;
    value = qAbs(value);
    text = QString::number(value,'f',1);

    //emit sRefresh(text);
}
//-----------------------------------------------------------------------------
//--- Create_INProtocol
//-----------------------------------------------------------------------------
QString Run::Create_INProtocol(QString fn)
{
    QString res = "";
    int i,j,k,m,count;

    QDomDocument    doc;
    QDomElement     root;
    QDomElement     source;
    QDomElement     samples;
    QDomElement     tubes;
    QDomElement     channels;
    QDomElement     result;
    QDomNode        item;

    QFile file(fn);

    if(file.exists() && file.open(QIODevice::ReadOnly))
    {
        if(doc.setContent(&file))
        {
            file.close();

            root = doc.documentElement();

            // SourceSet: Results
            source = root.firstChildElement("sourceSet");
            if(!source.isNull())
            {
                for(i=0; i<source.childNodes().size(); i++)
                {
                    samples = source.childNodes().at(i).firstChildElement("samples");
                    if(samples.isNull()) continue;
                    for(j=0; j<samples.childNodes().size(); j++)
                    {
                        item = samples.childNodes().at(j);
                        result = item.firstChildElement("sample_results");
                        if(!result.isNull()) item.removeChild(result);
                        tubes = item.firstChildElement("tubes");
                        if(tubes.isNull()) continue;
                        for(k=0; k<tubes.childNodes().size(); k++)
                        {
                            item = tubes.childNodes().at(k);
                            result = item.firstChildElement("tube_results");
                            if(!result.isNull()) item.removeChild(result);
                            channels = item.firstChildElement("channels");
                            for(m=0; m<channels.childNodes().size(); m++)
                            {
                                item = channels.childNodes().at(m);
                                result = item.firstChildElement("channel_results");
                                if(!result.isNull()) item.removeChild(result);
                            }
                        }
                    }
                }
            }


            // 1. device
            item = root.firstChildElement("device");
            if(!item.isNull()) root.removeChild(item);            

            // 2. measurements
            item = root.firstChildElement("measurements");
            if(!item.isNull()) root.removeChild(item);

            // 3. Analysis_Curves
            item = root.firstChildElement("Analysis_Curves");
            if(!item.isNull()) root.removeChild(item);

            QFile file_out(dir_temp.path() + "/in.rt");
            if(file_out.open(QIODevice::WriteOnly))
            {
                QTextStream(&file_out) << doc.toString();
                file_out.close();
                res = file_out.fileName();                
            }
        }
        else file.close();
    }

    return(res);
}
//-----------------------------------------------------------------------------
//--- RemoveChannels_InSourceset
//-----------------------------------------------------------------------------
void Run::RemoveChannels_InSourceset(QDomElement *sourceset, QVector<int> *vec_removed)
{
    int i,j,k,m,n,p,q;
    QString text;
    int id;
    bool ok;

    QDomNode    item_group;
    QDomNode    item_Samples;
    QDomNode    item_Tubes;
    QDomNode    item_Channels;
    QDomNode    item_sample;
    QDomNode    item_tube;
    QDomNode    item_channel;

    QDomElement id_channel;
    QVector<QDomNode> vec_item;


    for(i=0; i<sourceset->childNodes().size(); i++)
    {
        item_group = sourceset->childNodes().at(i);
        if(item_group.nodeName() != "item") continue;

        for(j=0; j<item_group.childNodes().size(); j++)
        {
            item_Samples = item_group.childNodes().at(j);
            if(item_Samples.nodeName() != "samples") continue;

            for(k=0; k<item_Samples.childNodes().size(); k++)
            {
                item_sample = item_Samples.childNodes().at(k);
                if(item_sample.nodeName() != "item") continue;

                for(m=0; m<item_sample.childNodes().size(); m++)
                {
                    item_Tubes = item_sample.childNodes().at(m);
                    if(item_Tubes.nodeName() != "tubes") continue;

                    for(n=0; n<item_Tubes.childNodes().size(); n++)
                    {
                        item_tube = item_Tubes.childNodes().at(n);
                        if(item_tube.nodeName() != "item") continue;

                        for(p=0; p<item_tube.childNodes().size(); p++)
                        {
                            item_Channels = item_tube.childNodes().at(p);
                            if(item_Channels.nodeName() != "channels") continue;

                            vec_item.clear();
                            for(q=0; q<item_Channels.childNodes().size(); q++)
                            {
                                item_channel = item_Channels.childNodes().at(q);

                                id_channel = item_channel.firstChildElement("IDChannel");
                                if(!id_channel.isNull())
                                {
                                    text = id_channel.text();
                                    id = text.toInt(&ok);

                                    if(ok)
                                    {
                                        if(vec_removed->contains(id)) vec_item.append(item_channel);
                                    }
                                }
                            }
                            // remove channels
                            for(q=0; q<vec_item.size(); q++)
                            {
                                item_Channels.removeChild(vec_item.at(q));
                            }
                        }
                    }
                }
            }
        }
    }

    vec_item.clear();
}
//-----------------------------------------------------------------------------
//--- Create_LastRunProtocol()
//-----------------------------------------------------------------------------
bool Run::Create_LastRunProtocol()
{
    int i,j;
    bool ok;
    QString text;
    QStringList list;
    QString fluor_name[COUNT_CH] = FLUOR_NAME;    

    prot->Clear_Protocol();

    // Header Protocol ...
    prot->name = "LastRun";
    prot->regNumber = GetRandomString().toStdString();
    if(ax_user) prot->owned_by = ax_user->property("userName").toString().toStdString();
    else prot->owned_by = "???";
    prot->time_created = QDateTime::currentDateTime().toString("ddMMyy_hmmss").toStdString();
    prot->state = mRun;

    prot->active_Channels = ActiveCh_Dev;   // from FACS

    qDebug() << "active_ch: " << prot->active_Channels;

    text = map_InfoDevice.value(INFODEV_serName,"000");
    int count_tubes = text.trimmed().mid(1,1).toInt();
    switch(count_tubes)
    {
    default:    count_tubes = 0;   break;
    case 5:     count_tubes = 96;  break;
    case 6:     count_tubes = 384; break;
    case 7:     count_tubes = 48;  break;
    case 8:     count_tubes = 192; break;
    }
    if(count_tubes == 0) return(false);

    prot->count_Tubes = count_tubes;
    prot->color_tube.clear();
    prot->enable_tube.clear();
    QColor color = QColor(Qt::black);
    int key = color.rgb();
    for(i=0; i<prot->count_Tubes; i++)
    {
        prot->color_tube.push_back(key);
        prot->enable_tube.push_back(1);
    }

    // Program of Amplification
    text = map_InfoDevice.value(INFODEV_TempProgram, "");
    list = text.split(";");
    //list << "XPRG 0 35 0" << "XLEV 3000 15 0 0 0 0" << "XCYC 1" << "XLEV 3000 16 100 0 0 2" << "XCYC 10" << "XLEV 2000 15 0 0 0 0" << "XHLD" << "XSAV Example";
    qDebug() << "program: " << list;
    prot->program.clear();
    foreach(text, list)
    {
        if(text.trimmed().isEmpty()) continue;
        prot->program.push_back(text.toStdString());
    }
    //

    // Test
    rt_Test *ptest = new rt_Test();
    ptest->header.ID_Test = "-1";
    ptest->header.Type_analysis = 0;
    ptest->header.Name_Test = "simple";
    ptest->header.Active_channel = prot->active_Channels;

    ptest->header.program.clear();
    for(i=0; i<prot->program.size(); i++)
    {
        ptest->header.program.push_back(prot->program.at(i));
    }

    rt_TubeTest *ptube_test = new rt_TubeTest();
    ptube_test->ID_Tube = 0;
    for(i=0; i<COUNT_CH; i++)
    {
        if(!(prot->active_Channels & (0x0f<<4*i))) continue;
        rt_ChannelTest *channel = new rt_ChannelTest();
        channel->ID_Channel = i;
        channel->number = i;
        channel->name = fluor_name[i].toStdString();
        ptube_test->channels.push_back(channel);
    }
    ptest->tubes.push_back(ptube_test);
    prot->tests.push_back(ptest);    


    // Groups, samples ...
    for(i=0; i<count_tubes; i++)
    {
        rt_GroupSamples *group = new rt_GroupSamples();
        group->ID_Group = QString::number(i).toStdString();
        group->Unique_NameGroup = QString("Sample_%1").arg(i).toStdString();
        rt_Sample *sample = new rt_Sample();
        sample->ID_Sample = QString::number(i).toStdString();
        sample->ID_Test = "-1";
        sample->Unique_NameSample = group->Unique_NameGroup;
        sample->p_Test = ptest;
        rt_Tube *ptube = new rt_Tube();
        ptube->ID_Tube = 0;
        ptube->color = 0;
        ptube->pos = i;
        ptube->active = 1;
        for(j=0; j<COUNT_CH; j++)
        {
            if(!(prot->active_Channels & (0x0f<<4*j))) continue;
            rt_Channel *pchannel = new rt_Channel();
            pchannel->ID_Channel = j;
            pchannel->Unique_NameChannel = fluor_name[j].toStdString();
            ptube->channels.push_back(pchannel);
        }
        sample->tubes.push_back(ptube);
        group->samples.push_back(sample);
        prot->Plate.groups.push_back(group);
    }

    // device info
    prot->SerialName =  map_InfoDevice.value(INFODEV_serName).toStdString();
    prot->uC_Versions =  map_InfoDevice.value(INFODEV_version).toStdString();
    prot->Type_ThermoBlock = map_InfoDevice.value(INFODEV_thermoBlock).toStdString();
    prot->FluorDevice_MASK = map_InfoDevice.value(INFODEV_devMask).toInt(&ok,16);

    // device coeff ...
    prot->COEFF_Spectral.clear();
    for(i=0; i<SpectralCoeff.size(); i++) prot->COEFF_Spectral.push_back(SpectralCoeff.at(i).toStdString());
    prot->COEFF_Optic.clear();
    for(i=0; i<OpticCoeff.size(); i++) prot->COEFF_Optic.push_back(OpticCoeff.at(i).toStdString());
    prot->COEFF_UnequalCh.clear();
    for(i=0; i<UnequalCoeff.size(); i++) prot->COEFF_UnequalCh.push_back(UnequalCoeff.at(i).toStdString());

    // device mask ...
    prot->DeviceParameters = Load_OpticMask().toStdString();


    // 2. Save as XML
    SaveAsXML(this, NULL, prot, dir_Device.path() + "/run.rt", "", true);
    Sleep(200);

    QDomDocument    doc;
    QDomElement     root;

    QFile file(dir_Device.path() + "/run.rt");
    if(file.exists() && file.open(QIODevice::ReadOnly))
    {
        if(doc.setContent(&file))
        {
            file.close();
            root = doc.documentElement();

            QDomElement device = SaveXML_Device(doc, prot);
            root.appendChild(device);

            Sleep(200);
            if(file.open(QIODevice::WriteOnly))
            {
                QTextStream(&file) << doc.toString();
                file.close();
            }
        }
    }


    return(true);
}
//-----------------------------------------------------------------------------
//--- Create_RunProtocol()
//-----------------------------------------------------------------------------
bool Run::Create_RunProtocol()
{
    int i;
    bool res = false;
    bool ok;
    QString text, str;
    QString path;
    QString ID_plate = "";
    int act_ch, test_ActCh;
    QVector<int> vec_RemovedCh;
    rt_Test *ptest;
    rt_Preference *property;

    
    QDomDocument    doc;
    QDomElement     root;

    QDomElement     status_element;
    QDomElement     status_new;
    QDomElement     created_element;
    QDomElement     created_new;
    QDateTime       run_Start = QDateTime::currentDateTime();

    QDomElement     program_element;
    QDomElement     program_new;
    QDomElement     property_element;
    QDomNode        item_property;
    QDomElement     item_name;
    QDomElement     item_value;
    QDomElement     item_ActiveCh;
    QDomElement     item_ActiveCh_new;
    QDomElement     item_SourceSet;


    //... Protocol: device parameters ...
    if(prot != NULL)
    {
        // clear measure if make it earlier
        for(i=0; i<prot->meas.size(); i++) delete prot->meas.at(i);
        prot->meas.clear();

        // device info
        prot->SerialName =  map_InfoDevice.value(INFODEV_serName).toStdString();
        prot->uC_Versions =  map_InfoDevice.value(INFODEV_version).toStdString();
        prot->Type_ThermoBlock = map_InfoDevice.value(INFODEV_thermoBlock).toStdString();
        prot->FluorDevice_MASK = map_InfoDevice.value(INFODEV_devMask).toInt(&ok,16);

        // device coeff ...
        prot->COEFF_Spectral.clear();
        for(i=0; i<SpectralCoeff.size(); i++) prot->COEFF_Spectral.push_back(SpectralCoeff.at(i).toStdString());
        prot->COEFF_Optic.clear();
        for(i=0; i<OpticCoeff.size(); i++) prot->COEFF_Optic.push_back(OpticCoeff.at(i).toStdString());
        prot->COEFF_UnequalCh.clear();
        for(i=0; i<UnequalCoeff.size(); i++) prot->COEFF_UnequalCh.push_back(UnequalCoeff.at(i).toStdString());

        // device mask ...
        prot->DeviceParameters = Load_OpticMask().toStdString();
    }
    //...................................

    QFile file(dir_temp.path() + "/in.rt");
    if(file.exists() && file.open(QIODevice::ReadOnly))
    {
        if(doc.setContent(&file))
        {
            file.close();

            root = doc.documentElement();

            //... change status -> run
            status_element = root.firstChildElement("status");
            status_new = MakeElement(doc, "status", "run");
            if(status_element.isNull()) root.insertAfter(status_new, root.firstChildElement("regNumber"));
            else root.replaceChild(status_new, status_element);
            //...

            //... change created: QDateTime::currentDateTime()
            created_element = root.firstChildElement("created");
            created_new = MakeElement(doc, "created", run_Start.toString("d MMMM yyyy, H:mm:ss"));
            if(created_element.isNull()) root.insertAfter(created_new, root.firstChildElement("status"));
            else root.replaceChild(created_new, created_element);

            runtime_Program->prot_Start = run_Start;
            //...

            //... reLoad Active Channels ...
            item_ActiveCh = root.firstChildElement("activeChannels");
            text = item_ActiveCh.text();
            act_ch = text.toInt(&ok, 16);
            if(!ok) act_ch = 0;
            if(act_ch != prot->active_Channels)
            {

                qDebug() << "reLoad Active Channels: " << act_ch << prot->active_Channels;

                item_ActiveCh_new = MakeElement(doc, "activeChannels", QString("%1").arg(prot->active_Channels,0,16));
                root.replaceChild(item_ActiveCh_new, item_ActiveCh);

                foreach(ptest, prot->tests)
                {
                    test_ActCh = ptest->header.Active_channel;
                    for(i=0; i<COUNT_CH; i++)
                    {
                        if((test_ActCh & (0x0f<<4*i)) && !(prot->active_Channels & (0x0f<<4*i)))
                        {
                            if(!vec_RemovedCh.contains(i)) vec_RemovedCh.append(i);
                        }
                    }
                }

                qDebug() << "vec_RemovedCh: " << vec_RemovedCh;

                if(vec_RemovedCh.size())
                {
                    item_SourceSet = root.firstChildElement("sourceSet");
                    if(!item_SourceSet.isNull()) RemoveChannels_InSourceset(&item_SourceSet, &vec_RemovedCh);
                }
            }



            //... reLoad Program ...
            text = "";
            QVector<string> prog = QVector<string>::fromStdVector(prot->program);
            for(i=0; i<prog.size(); i++)
            {
                if(!text.isEmpty()) text += "\r\n";
                text += QString::fromStdString(prog.at(i));
            }
            program_element = root.firstChildElement("amProgramm");
            program_new = MakeElement(doc, "amProgramm" , text);
            root.replaceChild(program_new, program_element);
            //...

            //... Properties ...
            property_element = root.firstChildElement("properties");
            //foreach(item_property, property_element.childNodes())
            for(i=0; i<property_element.childNodes().size(); i++)
            {
                item_property = property_element.childNodes().at(i);
                item_name = item_property.firstChildElement("name");
                if(item_name.isNull()) continue;
                else
                {
                    if(item_name.text() == "Users_Catalog")
                    {                        
                        property_element.removeChild(item_property);
                        continue;
                    }
                    if(item_name.text() == "Save_OnLine")
                    {
                        property_element.removeChild(item_property);
                        continue;
                    }
                    if(item_name.text() == "ID_plate")
                    {
                        item_value = item_property.firstChildElement("value");
                        if(!item_value.isNull()) ID_plate = item_value.text();
                        continue;
                    }
                }
            }

            //... Select user catalog ...
            QString FileName = QString::fromStdString(prot->name);
            if(FileName.trimmed().isEmpty()) FileName = "Protocol_";
            QString dirName = user_Dir.absolutePath() + "/" + QString::fromStdString(prot->name) + ".rt";

            if(flag_SelectCatalogForResult || CALIBRATION_status)
            {                
                //FileName = QFileDialog::getSaveFileName(this,
                //                                        tr("Save Protocol with results as ..."),
                //                                        dirName,
                //                                        tr("Protocols File (*.rt)"));
                FileName = Save_FileName;

                if(CALIBRATION_status)
                {
                    QStringList locations = QStandardPaths::standardLocations(QStandardPaths::AppConfigLocation);
                    foreach(str, locations)
                        {
                            if(str.contains("ProgramData"))
                            {
                                FileName = QString("%1/Verification/%2_%3.rt").
                                                    arg(str).
                                                    arg(QString::fromStdString(prot->SerialName)).
                                                    arg(ID_plate);
                            }
                        }
                }

                if(!FileName.isEmpty())
                {
                    QDomElement item_pro = MakeElement(doc,"item","");
                    item_pro.appendChild(MakeElement(doc,"name","Users_Catalog"));
                    item_pro.appendChild(MakeElement(doc,"value",FileName));
                    property_element.appendChild(item_pro);
                    dirName = FileName;
                }
                else
                {
                    /*message.setStandardButtons(QMessageBox::Ok);
                    message.button(QMessageBox::Ok)->animateClick(8000);
                    text = tr("The Protocol will be saved in the user's home directory: ") + "\r\n" + user_Dir.absolutePath();
                    message.setText(text);
                    message.setIcon(QMessageBox::Information);
                    message.exec();*/
                }
            }
            QTreeWidgetItem *item = info_Protocol->topLevelItem(8); // FileName of the protocol
            if(item)
            {
                item->setText(1, dirName);
            }

            //... Save_OnLine ...
            OnLine_FileName = "";
            if(flag_SaveOnLine)
            {
                QFileInfo fi(dirName);
                path = fi.absoluteDir().absolutePath();
                QDir online_dir(path + "/OnLine");
                if(!online_dir.exists())
                {
                    ok = online_dir.mkdir(path + "/OnLine");
                    if(!ok) online_dir.setPath(path);
                }

                //qDebug() << "Save_FileName: " << Save_FileName;

                if(!Save_FileName.isEmpty())
                {
                    QFileInfo fi(Save_FileName);
                    str = fi.baseName();
                }
                else str = QString::fromStdString(prot->owned_by);

                OnLine_FileName = online_dir.absolutePath() + "/"
                                                     + QString("%1_%2_%3.rt").arg(QString::fromStdString(prot->SerialName))
                                                                             .arg(str)
                                                                             .arg(QDateTime::currentDateTime().toString("ddMMyy_HHmmss"));

                //qDebug() << "OnLine_FileName: " << OnLine_FileName;

                QDomElement item_pro = MakeElement(doc,"item","");
                item_pro.appendChild(MakeElement(doc,"name","Save_OnLine"));
                item_pro.appendChild(MakeElement(doc,"value",OnLine_FileName));
                property_element.appendChild(item_pro);
            }

            //...

            QDomElement device = SaveXML_Device(doc, prot);
            root.appendChild(device);
            
            QFile file_out(dir_Device.path() + "/run.rt");
            if(file_out.open(QIODevice::WriteOnly))
            {
                QTextStream(&file_out) << doc.toString();
                file_out.close();
                if(file_out.exists())
                {
                    res = true;
                    addProtocol_toList(file_out.fileName());
                }
            }
        }
        else file.close();
    }



    return(res);
}

//-----------------------------------------------------------------------------
//--- Create_MeasureProtocol()
//-----------------------------------------------------------------------------
bool Run::Create_MeasureProtocol(QString text, bool save_online)
{
    int i;
    bool res = false;
    bool ok;
    QString str;
    QString dir_video = qApp->applicationDirPath() + "/VIDEO/" + map_InfoDevice.value(INFODEV_serName);

    QDomDocument    doc;
    QDomElement     root;
    QDomElement     status_elemment;
    QDomElement     status_new;
    QDomElement     regNumber;
    QDomElement     regNumber_new;
    QDomElement     properties_elemment;
    QDomElement     item_property;

    QDomNode        child_property;
    QDomElement     child_name;

    //qDebug() << "Create_MeasureProtocol()";

    //qDebug() << "check & rejime" << SaveImage_rejime->checkState() << SaveImage_rejime_Active << SaveImage_rejime_IsChecked;


    QFile file(dir_Device.path() + "/run.rt");
    //QFile file(qApp->applicationDirPath() + "/device/Template_LastRun.trt");

    QFile file_out;
    QFileInfo file_info;

    if(text.isEmpty()) QFile::remove(dir_Device.path() + "/measurements.rt");
    else QFile::remove(dir_Device.path() + "/" +text);

    if(text.isEmpty())  file_out.setFileName(dir_Device.path() + "/measurements.rt");
    else                file_out.setFileName(dir_Device.path() + "/" + text);

    if(save_online && !text.isEmpty())
    {
        file_out.setFileName(text);
    }

    //file_info.setFile(dir_Device.path() + "/measurements.rt");

    //qDebug() << "Create_MeasureProtocol: " << sts << file_info.size() << file_info.absoluteFilePath();


    if(file.exists() && file.open(QIODevice::ReadOnly))
    {
        if(doc.setContent(&file))
        {
            file.close();

            root = doc.documentElement();

            //... check regNumber on empty value ...
            regNumber = root.firstChildElement("regNumber");
            if(!regNumber.isNull())
            {
                if(regNumber.text().trimmed().isEmpty())
                {
                    Generate_IDProtocol(prot);
                    regNumber_new = MakeElement(doc,"regNumber",QString::fromStdString(prot->regNumber));
                    root.replaceChild(regNumber_new, regNumber);
                }
            }

            //... change status -> measurements
            status_elemment = root.firstChildElement("status");
            status_new = MakeElement(doc,"status","measurements");
            if(status_elemment.isNull()) root.insertAfter(status_new, root.firstChildElement("regNumber"));
            else root.replaceChild(status_new, status_elemment);

            //... add Protocol property ... Comments ...
            bool find_comments = false;
            str = comments->toPlainText().trimmed();

            properties_elemment = root.firstChildElement("properties");
            if(!properties_elemment.isNull())
            {
                item_property = doc.createElement("item");
                item_property.appendChild(MakeElement(doc, "name", "Comments"));
                item_property.appendChild(MakeElement(doc, "value", str));

                for(i=0; i<properties_elemment.childNodes().size(); i++)
                {
                    child_property = properties_elemment.childNodes().at(i);
                    child_name = child_property.firstChildElement("name");
                    if(!child_name.isNull())
                    {
                        if(child_name.text() == "Comments")
                        {
                            find_comments = true;
                            properties_elemment.replaceChild(item_property, child_property);
                            break;
                        }
                    }
                }
                if(!find_comments) properties_elemment.appendChild(item_property);
             }

            //...
            QDomElement measurements = SaveXML_Measurements(doc, prot);

            //qDebug() << "meas_file:" << prot->meas.size();

            root.appendChild(measurements);

            //qDebug() << "file_out: " << file_out.fileName();

            if(file_out.open(QIODevice::WriteOnly))
            {
                QTextStream(&file_out) << doc.toString();
                file_out.close();
                i = 0;
                while(file_out.isOpen())
                {
                    Sleep(100);
                    qApp->processEvents();
                    i++;
                    if(i > 3) break;
                }

                if(file_out.exists()) res = true;
                //qDebug() << "close_measure";
            }

        }
        else file.close();
    }

    if(res && !save_online)
    {
        file_info.setFile(file_out);
        this->parentWidget()->setProperty("measurements_file",
                                          QVariant(file_info.absoluteFilePath()));

        if(text.isEmpty())      // When Run is finished...
        {
            QStringList locations = QStandardPaths::standardLocations(QStandardPaths::AppConfigLocation);
            foreach(str, locations)
            {
                if(str.contains("ProgramData"))
                {
                    QDir dir;
                    QString path = QString("%1/Archive").arg(str);
                    dir.setPath(path);
                    if(!dir.exists()) dir.mkpath(dir.absolutePath());
                    if(dir.exists())
                    {
                        str = QString("%1/%2_%3_%4.rt").arg(dir.absolutePath()).
                                                        arg(QString::fromStdString(prot->SerialName)).
                                                        arg(QString::fromStdString(prot->owned_by)).
                                                        arg(QDateTime::currentDateTime().toString("ddMMyy_HHmmss"));

                        if(!file_out.copy(str)) str = "";
                        this->parentWidget()->setProperty("measurements_ArchiveFile", QVariant(str));
                    }
                    break;
                }
            }
        }
    }

    return(res);
}
//-----------------------------------------------------------------------------
//--- Load_OpticMask()
//-----------------------------------------------------------------------------
QString Run::Load_OpticMask()
{
    if(!Video_Image) return("");

    int i;
    int count;
    QString mask;
    QString text;

    // 1. auto status
    mask += "0 ";

    // 2. radius
    mask += QString("%1 %2 ").arg(Rx).arg(Rx);

    // 3. offset
    text = "";
    count = offset_XY.size();
    for(i=0; i<count; i++)
    {
        text += QString("%1 ").arg(offset_XY.at(i));
    }
    mask += text;

    // 4. data
    mask += "centers: ";
    count = XY.size();
    for(i=0; i<count; i++)
    {
       mask += QString("%1 ").arg(XY.at(i));
    }

    return(mask);
}
//-----------------------------------------------------------------------------
//--- Load_Mask()
//-----------------------------------------------------------------------------
bool Run::Load_Mask()
{
    int i,j;    
    int num_sec;
    int left;
    int count_sec = 96;
    QString text;
    QByteArray buf;
    std::vector<short> std_vec;
    bool state = true;
    int col,row;


    union
    {
        Device_Par Device_Param;
        unsigned char byte_buf[512];
    }DEVICE_PAR;

    text = map_InfoDevice.value(INFODEV_parameters,"");
    if(text.isEmpty()) return(false);

    buf = QByteArray::fromBase64(text.toStdString().c_str());
    num_sec = div(buf.size(),512).quot;

    text = map_InfoDevice.value(INFODEV_serName,"000");
    int count_tubes = text.trimmed().mid(1,1).toInt();
    switch(count_tubes)
    {
    default:    count_tubes = 0;   break;
    case 5:     count_tubes = 96;  break;
    case 6:     count_tubes = 384; break;
    case 7:     count_tubes = 48;  break;
    case 8:     count_tubes = 192; break;
    }

    if(count_tubes == 0) return(false);

    prot->Plate.PlateSize(count_tubes, row, col);

    left = LEFT_OFFSET;
    if(count_tubes == 48 || count_tubes == 192) left = LEFT_OFFSET_DT48;
    if(count_tubes == 48) count_sec = 48;


    XY.clear();
    offset_XY.clear();

    //... Geometry ...
    for(i=0; i<num_sec; i++)
    {
        for(j=0; j<512; j++) DEVICE_PAR.byte_buf[j] = buf.at(j+i*512);

        std_vec.clear();
        std_vec.assign(DEVICE_PAR.Device_Param.X_Y, DEVICE_PAR.Device_Param.X_Y + count_sec*2);

        for(j=0; j<count_sec; j++)
        {
            XY.append(std_vec.at(2*j) - left);
            XY.append(std_vec.at(2*j+1) - TOP_OFFSET);
        }
        if(i == 0)  // only for first sector
        {
            Rx = DEVICE_PAR.Device_Param.Rx;
            Ry = DEVICE_PAR.Device_Param.Ry;
            for(j=0; j<COUNT_CH; j++)
            {
                offset_XY.append(DEVICE_PAR.Device_Param.d_xy[j*2+1]); // * (-1));
                offset_XY.append(DEVICE_PAR.Device_Param.d_xy[j*2]);
            }
        }
    }
    //qDebug() << "XY: " << Rx << Ry << offset_XY << XY;
    std_vec.clear();

    //--- Validate
    int x,y;
    int lim_x = W_IMAGE;
    int lim_y = H_IMAGE;

    // 1.
    for(i=0; i<count_tubes; i++)
    {
        x = XY.at(i*2);
        y = XY.at(i*2+1);

        if(x > (lim_x - Rx) || x < Rx) {state = false; break;}
        if(y > (lim_y - Ry) || y < Ry) {state = false; break;}
    }
    //qDebug() << "1.: " << i << lim_x << lim_y << state;
    if(!state) return(state);

    // 2.
    if(Rx < 2 || Rx > lim_x/(2*col) || Ry < 2 || Ry > lim_y/(2*row)) return(false);

    // 3.
    int x1 = XY.at(0);
    int x2 = XY.at((col-1)*2);
    int x3 = XY.at((row-1)*2*col);
    int x4 = XY.at((count_tubes-1)*2);

    int y1 = XY.at(0+1);
    int y2 = XY.at((col-1)*2 + 1);
    int y3 = XY.at((row-1)*2*col + 1);
    int y4 = XY.at((count_tubes-1)*2 + 1);

    //qDebug() << "3. " << x1 << x2 << x3 << x4 << y1 << y2 << y3 << y4;

    if(abs(x1-x3) > 3*Rx || abs(x2-x1) < (col-1)*Rx*2*0.80) state = false;
    if(abs(x4-x2) > 3*Rx || abs(x4-x1) < (col-1)*Rx*2*0.80) state = false;
    if(abs(y1-y2) > 3*Ry || abs(y1-y3) < (row-1)*Ry*2*0.80) state = false;
    if(abs(y3-y4) > 3*Ry || abs(y2-y4) < (row-1)*Ry*2*0.80) state = false;
    //--- Validate


    return(state);
}

//-----------------------------------------------------------------------------
//--- Load_Expozition()
//-----------------------------------------------------------------------------
bool Run::Load_Expozition()
{
    int i;
    QString text;
    QByteArray buf;
    std::vector<short> std_vec;

    union
    {
        Device_Par Device_Param;
        unsigned char byte_buf[512];
    }DEVICE_PAR;

    Expozition.clear();

    text = map_InfoDevice.value(INFODEV_parameters,"");
    if(text.isEmpty()) return(false);

    buf = QByteArray::fromBase64(text.toStdString().c_str());
    if(buf.size() < 512) return(false);

    //... read data ...
    for(i=0; i<512; i++) DEVICE_PAR.byte_buf[i] = buf.at(i);
    std_vec.clear();
    std_vec.assign((short int*)DEVICE_PAR.Device_Param.optics_ch,(short int*)DEVICE_PAR.Device_Param.optics_ch + 8*6);

    for(i=0; i<COUNT_CH; i++)
    {
        Expozition.append(qRound((double)(std_vec.at(i*6+4))*Coef_Expo));   // first expo for i-channel
        Expozition.append(qRound((double)(std_vec.at(i*6+5))*Coef_Expo));   // seecond expo for i-channel
    }

    //qDebug() << "Expozition: " << Expozition;

    return(true);
}

//-----------------------------------------------------------------------------
//--- Load_SpectralCoeff()
//-----------------------------------------------------------------------------
bool Run::Load_SpectralCoeff()
{
    int i,j;
    int count;
    double value;
    bool sts = true;

    union
        {
            SpectralCoeff_Compensation SpectralCOEFF;
            unsigned char byte_buf[512];
        }SPECTRAL_COEFF;

    SpectralCoeff.clear();

    QString text = map_InfoDevice.value(INFODEV_SpectralCoeff,"");
    if(text.isEmpty()) return(false);

    QByteArray buf = QByteArray::fromBase64(text.toStdString().c_str());
    for(i=0; i<512; i++) SPECTRAL_COEFF.byte_buf[i] = buf.at(i);
    count = SPECTRAL_COEFF.SpectralCOEFF.count;

    for(i=0; i<count; i++)
    {
        text = (QString)SPECTRAL_COEFF.SpectralCOEFF.Info_Spectral[i].fluor_name;
        text += "\t" + QString::number(SPECTRAL_COEFF.SpectralCOEFF.Info_Spectral[i].id_group);
        text += "\t";
        for(j=0; j<COUNT_CH; j++)
        {
            value = SPECTRAL_COEFF.SpectralCOEFF.Info_Spectral[i].coeff[j];
            value /= 1000.;
            text += " " + QString::number(value,'f',3);

            if(value > 2. || value < 0.) sts = false;
        }

        SpectralCoeff.append(text);
        //qDebug() << text;
    }

    return(sts);
}
//-----------------------------------------------------------------------------
//--- Load_OpticCoeff
//-----------------------------------------------------------------------------
bool Run::Load_OpticCoeff()
{
    int i,j,k;
    int count;
    int count_block = 1;
    int count_tube;
    double value;
    bool sts = true;

    union
        {
            OpticalCoeff_Compensation OpticalCOEFF;
            unsigned char byte_buf[512];
        }OPTICAL_COEFF;

    OpticCoeff.clear();

    QString text = map_InfoDevice.value(INFODEV_OpticalCoeff,"");
    if(text.isEmpty()) return(false);

    QByteArray buf = QByteArray::fromBase64(text.toStdString().c_str());

    text = map_InfoDevice.value(INFODEV_serName,"");
    k = text.trimmed().mid(1,1).toInt();
    if(k == 6) count_block = 2;
    switch(k)
    {
    default:
    case 5:     count_tube = 96;    break;
    case 6:     count_tube = 384;   break;
    case 7:     count_tube = 48;    break;
    case 8:     count_tube = 192;    break;
    }

    for(i=0; i<COUNT_CH; i++)
    {
        text = "";
        for(j=0; j<count_block; j++)
        {
            for(k=0; k<512; k++) OPTICAL_COEFF.byte_buf[k] = buf.mid(512*count_block*i + 512*j, 512).at(k);

            //if(!j) text = (QString)OPTICAL_COEFF.OpticalCOEFF.fluor_name + "\t";

            for(k=0; k<192; k++)
            {
                if(k >= count_tube) break;
                value = OPTICAL_COEFF.OpticalCOEFF.coeff[k];
                value /= 1000.;
                //text += QString::number(value,'f',3) + " ";
                text += QString("%1 ").arg(value,0,'f',3);

                if(value > 2. || value < 0.1) sts = false;
            }
        }
        OpticCoeff.append(text);
        //qDebug() << text;
    }    

    return(sts);
}
//-----------------------------------------------------------------------------
//--- Load_UnequalCoeff()
//-----------------------------------------------------------------------------
bool Run::Load_UnequalCoeff()
{
    int i,j;
    int count;
    double value;
    bool sts = true;

    union
        {
            UnequalCoeff_Compensation UnequalCOEFF;
            unsigned char byte_buf[512];
        }UNEQUAL_COEFF;

    UnequalCoeff.clear();

    QString text = map_InfoDevice.value(INFODEV_UnequalCoeff,"");
    if(text.isEmpty()) return(false);

    QByteArray buf = QByteArray::fromBase64(text.toStdString().c_str());
    for(i=0; i<512; i++) UNEQUAL_COEFF.byte_buf[i] = buf.at(i);
    count = UNEQUAL_COEFF.UnequalCOEFF.count;

    for(i=0; i<count; i++)
    {
        text = (QString)UNEQUAL_COEFF.UnequalCOEFF.Info_Unequal[i].fluor_name;
        text += "\t" + QString::number(UNEQUAL_COEFF.UnequalCOEFF.Info_Unequal[i].id_group);
        text += "\t";
        for(j=0; j<2; j++)  // 2 coefficients
        {
            value = UNEQUAL_COEFF.UnequalCOEFF.Info_Unequal[i].coeff[j];
            if(!j) {value /= 1000.; text += " " + QString::number(value,'f',3);}
            else text += " " + QString::number(value,'f',0);
        }

        UnequalCoeff.append(text);
        //qDebug() << text;
    }

    return(sts);
}

//-----------------------------------------------------------------------------
//--- Is_Exit()
//-----------------------------------------------------------------------------
bool Run::Is_Exit()
{
    int res;
    bool sts = true;
    QString question_0 =  tr("Attention!  Program is running in the device...");
    QString question_1 = tr("Would you like to stop the program before exit?");

    if(Dev_State >= sRun && Dev_State <= sWarming_up && Dev_State != sHold)
    {
        message.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        message.setDefaultButton(QMessageBox::Cancel);
        message.setIcon(QMessageBox::Question);
        message.button(QMessageBox::Cancel)->animateClick(7000);
        message.setText(question_0 + "\r\n" + question_1);
        res = message.exec();
        if(res == QMessageBox::Cancel) sts = false;
        else sts = true;
        if(res == QMessageBox::Yes)
        {
            slotSendToServer(STOP_REQUEST);
            wait_process(wStop, 100, tr("Stopping..."));
        }
    }
    return(sts);
}
//-----------------------------------------------------------------------------
//--- load_3D(void *3D)
//-----------------------------------------------------------------------------
void Run::load_3D(void *p_3d)
{
    //Video_Image->plate_3D->load_3Dobject(p_3d);
}
//-----------------------------------------------------------------------------
//--- create_User
//-----------------------------------------------------------------------------
bool Run::create_User(QAxObject *user)
{
    ax_user = user;
    user_Dir.setPath(qApp->applicationDirPath() + "/user");   
    QString temp;

    if(ax_user)
    {
        QStringList locations = QStandardPaths::standardLocations(QStandardPaths::AppConfigLocation);
        foreach(QString str, locations)
        {
            if(str.contains("ProgramData"))
            {
                QDir dir;
                QString path = QString("%1/Users/%2").arg(str).arg(ax_user->property("userName").toString());
                dir.setPath(path);
                if(!dir.exists()) dir.mkpath(dir.absolutePath());
                if(dir.exists()) user_Dir.setPath(dir.absolutePath());                
                break;
            }
        }
    }

    // set user properties
    flag_DeviceSettings = false;
    flag_ControlRun = false;
    flag_SaveLocationWin = false;
    flag_CMD = false;
    ALARM_Volume = false;
    ALARM_Program = false;
    ALARM_Exposure = false;
    ALARM_MinLevel = false;
    flag_SelectCatalogForResult = false;
    flag_SaveOnLine = false;
    Try_NewExpozition = 0;
    Try_ReadInfoDevice = 0;
    if(ax_user)
    {
        flag_DeviceSettings = ax_user->dynamicCall("getPriv(QString)", "CHANGE_DEVICE_PREF").toBool();
        flag_ControlRun = ax_user->dynamicCall("getPriv(QString)", "CONTROL_RUN").toBool();
        flag_SaveLocationWin = ax_user->dynamicCall("getPriv(QString)", "SAVE_LOCATION_WIN").toBool();
        flag_CMD = ax_user->dynamicCall("getPriv(QString)", "ENABLE_CMD").toBool();

        ALARM_Volume = ax_user->dynamicCall("getPriv(QString)", "IGNORE_VOLUME").toBool();
        ALARM_Program = ax_user->dynamicCall("getPriv(QString)", "IGNORE_PROGRAM").toBool();
        ALARM_Exposure = ax_user->dynamicCall("getPriv(QString)", "IGNORE_EXPOSURE").toBool();
        ALARM_MinLevel = ax_user->dynamicCall("getPriv(QString)", "IGNORE_MIN_LEVEL").toBool();

        flag_SelectCatalogForResult = ax_user->dynamicCall("getPriv(QString)", "ENABLE_SELECT_FN").toBool();
        flag_SaveOnLine = ax_user->dynamicCall("getPriv(QString)", "COPY_ONLINE").toBool();

        temp = ax_user->dynamicCall("getAttr(QString)","dir:PR_HOME").toString().trimmed();
        if(!temp.isEmpty() && QDir(temp).exists()) user_Dir.setPath(temp);
    }

//#ifdef CALIBRATION
//    flag_StatusRun = true;
//   flag_EditDevicePreference = true;
//#endif    

    //Tab_Control->setTabEnabled(4,flag_DeviceSettings);
    Tab_Control->setTabEnabled(3,flag_CMD);

    return(true);

}
//-----------------------------------------------------------------------------
//--- create_TestVerification
//-----------------------------------------------------------------------------
bool Run::create_TestVerification(QAxObject *axgp_resource)
{
    axgp = axgp_resource;

    return(true);
}

//-----------------------------------------------------------------------------
//--- static wrapper-function to WaitProcess
//-----------------------------------------------------------------------------
bool Run::Wrapper_To_WaitProcess(void *pt2Object, int percent, QString text)
{
    bool res = true;

//--- explicitly cast to a pointer to TClassA

    Run *mySelf = (Run *) pt2Object;

//--- call member

    mySelf->Display_ProgressBar(percent, text);

    if(percent < 0 || percent > 100) res = false;
    if(text.isEmpty()) res = false;
    if(mySelf->flag_RunLoaded) res = false;

    return(res);
}

//-----------------------------------------------------------------------------
//--- wait_process
//-----------------------------------------------------------------------------
void Run::wait_process(Wait_state w_st, int msec, QString label)
{
    QThread* thread = new QThread;
    Worker_wait *woker = new Worker_wait(w_st, label);
    QTimer *timer_wait = new QTimer;
    timer_wait->setInterval(msec);

    woker->moveToThread(thread);

    connect(timer_wait, SIGNAL(timeout()), woker, SLOT(slot_timeout()));
    connect(thread, SIGNAL(started()), timer_wait, SLOT(start()));
    connect(this, SIGNAL(finished_WaitProcess()), timer_wait, SLOT(stop()));

    connect(woker, SIGNAL(cursor_wait(bool)), this, SLOT(slot_CursorWait(bool)));
    connect(thread, SIGNAL(started()), woker, SLOT(start_process()));
    connect(woker, SIGNAL(display_process(int,QString)),
            this, SLOT(slot_DisplayProcess(int,QString)));
    connect(this, SIGNAL(finished_WaitProcess()), woker, SLOT(stop_process()));
    connect(woker, SIGNAL(finished()), thread, SLOT(quit()));
    connect(woker, SIGNAL(finished()), woker, SLOT(deleteLater()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    connect(thread, SIGNAL(finished()), timer_wait, SLOT(deleteLater()));

    thread->start();
}
//-----------------------------------------------------------------------------
//--- slot_CursorWait
//-----------------------------------------------------------------------------
void Run::slot_CursorWait(bool state)
{
    if(state) QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    else QApplication::restoreOverrideCursor();
}
//-----------------------------------------------------------------------------
//--- slot_DisplayProcess
//-----------------------------------------------------------------------------
void Run::slot_DisplayProcess(int percent, QString text)
{
    main_progress->setValue(percent);
    PrBar_status->setText(text);
    PrBar_status->setMinimumSize(PrBar_status->sizeHint());
    qApp->processEvents();
}
//-----------------------------------------------------------------------------
//--- slot_DisplayRun
//-----------------------------------------------------------------------------
void Run::slot_RunProcess(int percent, QString text)
{
    run_progress->setValue(percent);
    PrBar_run_status->setText(text);
    PrBar_run_status->setMinimumSize(PrBar_run_status->sizeHint());
    qApp->processEvents();
}
//-----------------------------------------------------------------------------
//--- TEMPORARY
//-----------------------------------------------------------------------------
void Run::slot_TimerTemp()
{
    //qDebug() << "timer_wait = " << timer_wait;
}
//-----------------------------------------------------------------------------
//--- execution_MenuAction
//-----------------------------------------------------------------------------
void Run::execution_MenuAction(QString type)
{
    if(type == "select_protocol") {open_protocol(); return;}
    if(type == "select_device") {ShowListDevice(); return;}
    if(type == "contract") {Show_Contract(); return;}
}

//-----------------------------------------------------------------------------
//--- Create_MenuActions(void *)
//-----------------------------------------------------------------------------
void Run::Create_MenuActions(void *list_pinfo)
{
    QStringList *list = (QStringList*)list_pinfo;

    list->append(tr("Select Protocol") + "\t" + "select_protocol" + "\t" + "1");
    list->append(tr("Select Device") + "\t" + "select_device" + "\t" + "1");
    list->append("...");
    list->append(tr("Contract") + "\t" + "contract" + "\t" + "1");
}
//-----------------------------------------------------------------------------
//--- Enable_MenuAction(void *)
//-----------------------------------------------------------------------------
void Run::Enable_MenuAction(void *map_enable)
{
    QMap<QString, int> *map = (QMap<QString, int>*)map_enable;

    map->insert("select_protocol", OpenProtocol->isEnabled());
    map->insert("select_device", ListDevices->isEnabled());
    map->insert("contract", flag_ContractDev);
}
//-----------------------------------------------------------------------------
//--- Application_status
//-----------------------------------------------------------------------------
void Run::Application_status(QString app_name, bool calibration_status)
{
    APP_NAME = app_name;
    list_dev->application_name = app_name;
    CALIBRATION_status = calibration_status;

    if(calibration_status)
    {
       flag_ControlRun = true;
       flag_DeviceSettings = true;
       //Tab_Control->setTabEnabled(4,flag_DeviceSettings);
    }

    //qDebug() << "APP_NAME: " << APP_NAME << calibration_status;
}
//-----------------------------------------------------------------------------
//--- Send_pProtocol(void *p)
//-----------------------------------------------------------------------------
void Run::Send_pProtocol(void *p)
{
    prot_FromSetup = (rt_Protocol*)p;

    //qDebug() << "Send_Pro:" << p << prot_FromSetup;
}
//-----------------------------------------------------------------------------
//--- update_ProtocolInfoToWeb()
//-----------------------------------------------------------------------------
void Run::update_ProtocolInfoToWeb(QString state)
{
    QString message;
    QString id_prot = QString::fromStdString(prot->regNumber);
    QDomDocument doc;
    QString fn, temp, time, dev_name;

    if(state.isEmpty())
    {
        switch(Dev_State)
        {
        case sRun:      state = "Run";      break;
        case sPause:    state = "Pause";    break;
        case sHold:     state = "Hold";     break;
        case sWarming_up:   state = "Warming_up"; break;
        default:        state = "Unknown";  break;
        }
    }
    fn = QString("%1(%2)").arg(fn_dev).arg(prot->count_MC + prot->count_PCR);
    temp = QString("%1 °C").arg(T_current,0,'f',1);
    time = runtime_Program->list_Time.at(1);
    dev_name = map_InfoDevice.value(INFODEV_serName,"");

    QDomNode xmlNode = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\"");
    doc.insertBefore(xmlNode, doc.firstChild());

    QDomElement  root = doc.createElement("run");
    doc.appendChild(root);
    root.setAttribute("time", QString("%1").arg(QDateTime::currentMSecsSinceEpoch()));

    root.appendChild(MakeElement(doc,"id_device",dev_name));
    root.appendChild(MakeElement(doc,"state",state));
    root.appendChild(MakeElement(doc,"fn",fn));
    root.appendChild(MakeElement(doc,"Temperature",temp));
    root.appendChild(MakeElement(doc,"Time",time));


    message = id_prot + ";" + doc.toString();
    parentWidget()->setProperty("ProtocolInfo_ToWeb", QVariant(message));
    QEvent *e = new QEvent((QEvent::Type)3013);
    QApplication::sendEvent(this->parentWidget(), e);

    //qDebug() << "Web_Timer: " << message;

}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void ItemDelegate_InfoDevice::paint(QPainter *painter,
                                     const QStyleOptionViewItem &option,
                                     const QModelIndex &index) const
{
    QString text;
    int active_ch;
    bool ok;
    int i;

    QPixmap pixmap(":/images/fam.png");

    QPalette pal = option.palette;
    QStyleOptionViewItem  viewOption(option);
    viewOption.state &= ~QStyle::State_HasFocus;    // disable state HasFocus

    QStyledItemDelegate::paint(painter, viewOption, index);

    text = index.data().toString();
    //qDebug() << "tree: " << index.row() << index.column() << text;
    if(index.row() == 2 && index.column() == 1)
    {
        text = text.replace(" ","").trimmed();
        active_ch = text.toInt(&ok, 16);

        for(i=0; i<COUNT_CH; i++)
        {
            if(active_ch & (0x1<<i))
            {
                if(style == "fusion")
                {
                    switch(i)
                    {
                    case 0:  pixmap.load(":/images/flat/fam_flat.png");    break;
                    case 1:  pixmap.load(":/images/flat/hex_flat.png");    break;
                    case 2:  pixmap.load(":/images/flat/rox_flat.png");    break;
                    case 3:  pixmap.load(":/images/flat/cy5_flat.png");    break;
                    case 4:  pixmap.load(":/images/flat/cy55_flat.png");   break;
                    default: pixmap.load(":/images/flat/disable_flat.png");   break;
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
                if(style == "fusion") pixmap.load(":/images/flat/disable_flat.png");
                else pixmap.load(":/images/disable.png");
            }


            painter->drawPixmap(option.rect.x() + 13*i + 100, option.rect.y(), pixmap);
        }
    }
}

//-----------------------------------------------------------------------------
//--- send_CRYPTO(QMap<QString, QByteArray> *map)
//-----------------------------------------------------------------------------
void Run::send_CRYPTO()
{

    slotSendToServer(CRYPTO_REQUEST);
    wait_process(wOpen, 100, tr("send CRYPTO_REQUEST..."));
}
//-----------------------------------------------------------------------------
//--- send_CRYPTO(QMap<QString, QByteArray> *map)
//-----------------------------------------------------------------------------
bool Run::Read_PubKey()
{
    int k;
    bool res = false;
    X509 *cert = NULL;
    EVP_PKEY *key = NULL;
    FILE *fp_key = NULL;
    RSA *rsa;
    QString fn_cer = qApp->applicationDirPath() + "\\test_verification\\certificate.cer";    //"47722632_privkey.cer";

    const BIGNUM *n;

    PubKey_BUF.resize(16);
    PubKey_BUF.fill('0',16);

    fp_key = fopen(fn_cer.toStdString().c_str(), "r");
    if(fp_key != NULL)
    {

        cert = PEM_read_X509(fp_key,NULL,NULL,NULL);
        if(cert != NULL)
        {
            key = X509_get_pubkey(cert);
            rsa = EVP_PKEY_get1_RSA(key);


            if(rsa) qDebug() << "RSA - ok" << RSA_size(rsa);

            RSA_get0_key(rsa, &n, NULL, NULL);

            unsigned char *binary = (unsigned char*) OPENSSL_malloc(BN_num_bytes(n)*sizeof(unsigned char));
            k = BN_bn2bin(n, binary);

            memcpy(PubKey_BUF.data(), binary, 16);	// copy 16 bytes

            OPENSSL_free(binary);
            RSA_free(rsa);
        }

        qDebug() << "certificate.cer" << PubKey_BUF;


        fclose(fp_key);
    }

    return(res);
}
//-----------------------------------------------------------------------------
//--- DeCryption_EnCryption(UCHAR *buf, UCHAR *key)
//-----------------------------------------------------------------------------
void Run::DeCryption_EnCryption(UCHAR *buf, UCHAR *pubkey)
{
    int i;

    UCHAR tmp[8];
    UCHAR in[8];
    UCHAR out[8];
    UCHAR random_key[8];
    UCHAR key_crypt[8];
    UCHAR endian_buf[512];

    const UCHAR *verbuf = reinterpret_cast<const UCHAR *>("DT CRYPT V1.00 \nSET");
    //QString s = "DT CRYPT V1.00 \nSET";
    //verbuf = const_cast<UCHAR*>(s.toStdString().c_str());

    int run_cnt = rand()%100 + 100; //100-200
    DES_cblock seed = {0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10};

    DES_cblock key;
    DES_key_schedule keysched;

    //--- 1. Endian_buffer ---
    swab((char*)buf, (char*)endian_buf, 512);

    //--- 2. Key ---
    memcpy((char*)key_crypt, (char*)(endian_buf + 1*8), 8);
    for(i=0; i<8; i++) key[i] = key_crypt[i];
    DES_set_key(&key, &keysched);

    //--- 3. Unique random key value ---
    memcpy((char*)in, (char*)(endian_buf + 5*8), 8);
    DES_ecb_encrypt((const_DES_cblock*)in, (DES_cblock*)random_key, &keysched, DES_DECRYPT);

    //--- PubKey ---  7-block & 8-block
    memcpy((char*)in, (char*)(endian_buf + 7*8), 8);
    DES_ecb_encrypt((const_DES_cblock*)in, (DES_cblock*)out, &keysched, DES_DECRYPT);
    memcpy((char*)pubkey, (char*)out, 8);
    memcpy((char*)in, (char*)(endian_buf + 8*8), 8);
    DES_ecb_encrypt((const_DES_cblock*)in, (DES_cblock*)out, &keysched, DES_DECRYPT);
    memcpy((char*)(pubkey+8), (char*)out, 8);


    //--- Output buffer ---
    memset((char*)endian_buf, 0, 512);	// NULL

    //--- Version ---
    memcpy((char*)(endian_buf + 480), (char*)verbuf, 19);

    //--- Key ---
    memcpy((char*)(endian_buf + 8), (char*)key_crypt, 8);

    //--- Unique random key value ---
    DES_ecb_encrypt((const_DES_cblock*)random_key, (DES_cblock*)out, &keysched, DES_ENCRYPT);
    memcpy((char*)(endian_buf + 5*8), (char*)out, 8);

    //--- Random ---
    Random_ByteBuffer((char *)endian_buf,8);			// 0 - block
    Random_ByteBuffer((char *)(endian_buf + 2*8),8);	// 2 - block
    Random_ByteBuffer((char *)(endian_buf + 3*8),8);	// 3 - block

    //--- Run Count ---
    Random_ByteBuffer((char *)tmp,8);
    tmp[0] = ((run_cnt>>24)&0xff);
    tmp[1] = ((run_cnt>>16)&0xff);
    tmp[2] = ((run_cnt>>8)&0xff);
    tmp[3] = (run_cnt&0xff);
    DES_ecb_encrypt((const_DES_cblock*)tmp, (DES_cblock*)out, &keysched, DES_ENCRYPT);
    memcpy((char*)(endian_buf + 4*8), (char*)out, 8);

    //--- Endian_buffer ---
    swab((char*)endian_buf, (char*)buf, 512);


}
//-----------------------------------------------------------------------------
//--- DeCryption_EnCryption(UCHAR *buf, UCHAR *key)
//-----------------------------------------------------------------------------
void Run::Random_ByteBuffer(char *p, int size)
{
    int i;

    for(i=0; i<size; i++)
    {
        p[i] = rand();
    }
}
//-----------------------------------------------------------------------------
//--- Read_CryptoDevice()
//-----------------------------------------------------------------------------
void Run::Read_CryptoDevice()
{
    map_CRYPTO.clear();
    map_CRYPTO.insert(CRYPTO_CTRL,"CRYR");
    //qDebug() << "send CRYR: ";

    QByteArray ba("yes");
    map_CRYPTO.insert("RejimeCryptoDevice", ba);
    send_CRYPTO();


}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
//void Run::slot_CheckPubkeyCrypto()
void Run::Check_PubkeyCrypto(QMap<QString,QByteArray> *map)
{
    //Check_PubkeyCrypto(&map_CRYPTO);
}
//-----------------------------------------------------------------------------
//--- Check_PubkeyCrypto(QMap<QString,QByteArray>*)
//-----------------------------------------------------------------------------
//void Run::Check_PubkeyCrypto(QMap<QString,QByteArray> *map)
void Run::slot_CheckPubkeyCrypto()
{
    QByteArray ba = map_CRYPTO.value(CRYPTO_DATA, "");
    QByteArray status = map_CRYPTO.value("status", "");
    QDataStream ds(status);
    short state;
    ds >> state;
    UCHAR pubkey[16];
    UCHAR *p_ba;
    QString name_Dev = map_InfoDevice.value(INFODEV_serName,"");
    QString text;

    //qDebug() << "Check_PubkeyCrypto: " << map_CRYPTO << state;

    if(!status.isEmpty() && status.at(0) == 0x00) state = true;
    memset((char*)pubkey,0,16); 	// NULL

    if(ba.size() == 512 && state == 0)
    {
        p_ba = (UCHAR*)ba.data();
        DeCryption_EnCryption(p_ba, pubkey);

        QByteArray ba_dev = QByteArray::fromRawData(reinterpret_cast<char*>(pubkey),16);    // or (char*)buf

        //qDebug() << "crypto pubkey: " << ba_dev << PubKey_BUF;

        if(ba_dev != PubKey_BUF)
        {
            NameDev->setText("<img src=\":/images/flat/lock_16.png\"/>  " + name_Dev);
            NameDev->setStyleSheet("QLabel{background-color: rgb(255, 128, 128)}");
            if(id_Reagent == rTestLimited)  // CRYC: 100  CRYC: 0
            {
                text = tr("Attention! Invalid the PublicKey...") + "\r\n" +
                       tr("According to the contract restrictions, Device is blocked!");
            }
            if(id_Reagent == rLimited)       // CRYC: 0   CRYC: 0
            {
                text = tr("According to the contract restrictions, Device is blocked!");
            }

            message.setStandardButtons(QMessageBox::Ok);
            message.setIcon(QMessageBox::Warning);
            message.button(QMessageBox::Ok)->animateClick(15000);
            message.setText(text);
            message.exec();

            flag_LimitedDev = true;
        }
        else
        {
            NameDev->setText("<img src=\":/images/flat/unlock_16.png\"/>  " + name_Dev);
            map_CRYPTO.clear();
            map_CRYPTO.insert(CRYPTO_CTRL,"CRYS");
            map_CRYPTO.insert(CRYPTO_DATA, ba);
            QByteArray ba_rejime("yes");
            map_CRYPTO.insert("RejimeCryptoDevice", ba_rejime);

            QTimer::singleShot(1000, this, SLOT(send_CRYPTO()));
        }
    }
    else
    {
        // message...  Off/On device
        //qDebug() << "CRYR: bytearray is empty or state = false!" << state << ba.size() << map_CRYPTO.value("RejimeCryptoDevice","") << id_Reagent;

        if(map_CRYPTO.value("RejimeCryptoDevice","") == "yes")
        {
            NameDev->setText("<img src=\":/images/flat/lock_16.png\"/>  " + name_Dev);
            NameDev->setStyleSheet("QLabel{background-color: rgb(255, 128, 128)}");
            text = tr("According to the contract restrictions, Device is blocked!");
            message.setStandardButtons(QMessageBox::Ok);
            message.setIcon(QMessageBox::Warning);
            message.button(QMessageBox::Ok)->animateClick(15000);
            message.setText(text);
            message.exec();

            flag_LimitedDev = true;
        }
    }    
}
