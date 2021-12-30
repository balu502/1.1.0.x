//#include "sp.h"
#include "main_rt.h"
#include <QApplication>
#include <QObject>

BOOL CALLBACK enumWindowCallback(HWND hwnd,LPARAM lParam);
QStringList *list_Win;
//bool Find_DTmaster;

int main(int argc, char *argv[])
{
    int i;
    int id_user;
    QApplication a(argc, argv);
    QStringList	cache;
    QTemporaryFile file_TESTs;
    QAxObject axRita;
    QAxObject* axUser = NULL;
    QAxObject axgp;
    QString text;
    QVector<int> vec;
    QString item;
    bool calibration = false;
    QEventLoop loop;


    HWND handle_win = NULL;
    LPCWSTR w_str;
    //QString name_class = "DTmaster";
    list_Win = new QStringList();
    list_Win->clear();

    QDomDocument doc;
    QDomDocument doc_item;
    QDomElement  root_item;
    QDomElement element_Owner;

    //qDebug() << "libraryPaths(): " << QCoreApplication::libraryPaths();

/*
    QSettings settings(R"(HKCU\Software\Microsoft\Windows NT\CurrentVersion\AppCompatFlags\Layers)", QSettings::NativeFormat);
    QStringList list_reg = settings.allKeys();
    QStringList list_value;
    for(i=0; i<list_reg.size(); i++)
    {
        list_value.append(settings.value(list_reg.at(i)).toString());
    }

    qDebug() << "LIST_REG: " << list_reg;
    qDebug() << "LIST_VALUE: " << list_value;
    qDebug() << "VALUE: " << settings.value("C:\Program Files (x86)\Dna-Technology\DTmaster_9\DTmaster.exe").toString();
*/
    //QMessageBox::information(NULL, QString("QTemporaryFile"),  file_TESTs.fileName(), QMessageBox::Ok);

    //a.setStyle(QStyleFactory::create("Fusion"));
    //QApplication::setPalette(QApplication::style()->standardPalette());


    QStringList list_arg = a.arguments();

    qDebug() << "list_arg: " << list_arg;

    QFileInfo fi;
    bool cmd_str = false;
    if(list_arg.size() == 2)
    {
        text = list_arg.at(1);
        fi.setFile(text);
        if(fi.exists() && fi.suffix() == "rt") cmd_str = true;

        qDebug() << cmd_str << fi.fileName();
    }    

#ifdef CALIBRATION
    calibration = true;
    cmd_str = false;
#endif

    //SplashScreen_Wait *splash_wait = new SplashScreen_Wait(&a);
    //splash_wait->hide();

    QProcess process_usb;
    process_usb.startDetached("Server_USBDev.exe");

    //QMessageBox::information(0,"info", "Server_USBDev.exe");

    //w_str = (const wchar_t*) name_class.utf16();
    //handle_win = FindWindow(NULL, w_str);
    //qDebug() << "handle_win: " << handle_win;
    EnumWindows(enumWindowCallback, NULL);
    qDebug() << "LIST_WIN: " << *list_Win;
    if(list_Win->size() == 0)
    {
        WinExec("taskkill.exe /f /im rita.exe",SW_HIDE);
        WinExec("taskkill.exe /f /im dnasig.exe",SW_HIDE);
    }
    delete list_Win;
    //QMessageBox::information(0,"info", "taskkill.exe");

    //qDebug() << "font: " << APP_VERSION << APP_NAME;
    //qDebug() << "font: " << a.font();
    //a.setApplicationVersion(APP_VERSION);
    Main_RT w;    
    qDebug() << "create Main_RT";

    //... Parameters of the commands string ...
    i=0;
    if(list_arg.size() > 1)
    {
        foreach(text, list_arg)
        {
            if(i>0)
            {
                if(text == "-tray")
                {
                    //w.trayIcon->setVisible(true);
                    w.set_ActiveTray(true);
                }
            }
            i++;
        }
    }
    //...

    SplashScreen *splash = new SplashScreen(&a);      // RT
    //SplashScreen_RT *splash = new SplashScreen_RT(&a);
    w.Send_p_SplashScreenObject(splash);
    if(!cmd_str) splash->show();


    if(calibration)
    {
        splash->show();

        while(splash->m_progress < 100)   // RT        
        {
            w.Sleep(150);            
            splash->progress->setValue(splash->m_progress);   // RT
            a.processEvents();
            //i++;
        }
    }

    if(!cmd_str && !calibration)
  {
    i=0;
    //while(splash->m_progress < 100)   // RT
    while(i<30)
    {
        w.Sleep(100);
        splash->progress->setValue(splash->m_progress);   // RT
        a.processEvents();
        i++;
    }

   }

    if(FAILED(CoInitialize(NULL)))
    {
        qDebug() << "Unable to initialize COM";
        QMessageBox::warning(0,"Warning", "Unable to initialize COM");
    }
    else        
    {        

        //  class dnasig
        axgp.setControl("{6DCA6D52-6D57-44ae-AFDC-2C4332680093}");
        //if(!axgp)
        if(axgp.control().isNull())
        {
            if(splash->isVisible()) splash->close();
            QMessageBox::warning(0,"Warning", "Unable to set COM control: axgp");
        }

        //
        //  Инициализация объекта по идентификатору
        //
        axRita.setControl("{14CB79BB-D11C-4e98-8CB8-450379A0B008}");   // class rita

        //if(!axRita)
        if(axRita.control().isNull())
        {
            if(splash->isVisible()) splash->close();
            QMessageBox::warning(0,"Warning", "Unable to set COM control: axRita");
        }
        else
        {
            //      Настройка локали (en|ru)
            //
            text = w.LanguageApp;
            //qDebug() << "axRita" << text;
            axRita.dynamicCall("setLocale(QString)", text);

            QFont app_font = w.app_font;
            qDebug() << "app_font:  " << app_font.family() << app_font.pointSize() << app_font.weight();

            text = QString("font-family: \"%1\"; font-size: %2pt; font-weight: %3").arg(app_font.family())
                                                                            .arg(app_font.pointSize())
                                                                            .arg(app_font.weight());
            qDebug() << "fontCSS: " << text;
            axRita.dynamicCall( "setOption(QString,QString)", "fontCSS", text);
            //axRita.dynamicCall( "setOption(QString,QString)", "font", app_font.family());
            //axRita.dynamicCall( "setOption(QString,QString)", "fontPointSize", QString::number(app_font.pointSize()));
            //axRita.dynamicCall( "setOption(QString,QString)", "fontWeight",	QString::number(app_font.weight()));

            // Add new properties
            /*axRita.dynamicCall("trPrivilege(QString,QString)", "SAVE_LOCATION_WIN", "Сохранить расположение окон");
            axRita.dynamicCall("registerPPrivilege(QString,bool)", "SAVE_LOCATION_WIN", false);
            axRita.dynamicCall("trPrivilege(QString,QString)", "ENABLE_PAGE_SETUP", "Доступна страница: Протокол");
            axRita.dynamicCall("registerPPrivilege(QString,bool)", "ENABLE_PAGE_SETUP", false);
            axRita.dynamicCall("trPrivilege(QString,QString)", "ENABLE_PAGE_RUN", "Доступна страница: Запуск");
            axRita.dynamicCall("registerPPrivilege(QString,bool)", "ENABLE_PAGE_RUN", false);
            axRita.dynamicCall("trPrivilege(QString,QString)", "ENABLE_ADD_ANALYSIS", "Дополнительный анализ");
            axRita.dynamicCall("registerPPrivilege(QString,bool)", "ENABLE_ADD_ANALYSIS", false);
            axRita.dynamicCall("trPrivilege(QString,QString)", "ENABLE_CROSSTABLE", "Кросс_Таблица");
            axRita.dynamicCall("registerPPrivilege(QString,bool)", "ENABLE_CROSSTABLE", false);
            axRita.dynamicCall("trPrivilege(QString,QString)", "ENABLE_CMD", "Командная строка");
            axRita.dynamicCall("registerPPrivilege(QString,bool)", "ENABLE_CMD", false);

            axRita.dynamicCall("trPrivilege(QString,QString)", "IGNORE_VOLUME", "Игнорировать: Объём в пробирке");
            axRita.dynamicCall("registerPPrivilege(QString,bool)", "IGNORE_VOLUME", false);
            axRita.dynamicCall("trPrivilege(QString,QString)", "IGNORE_PROGRAM", "Игнорировать: Программа амплификации");
            axRita.dynamicCall("registerPPrivilege(QString,bool)", "IGNORE_PROGRAM", false);
            axRita.dynamicCall("trPrivilege(QString,QString)", "IGNORE_EXPOSURE", "Игнорировать: Экспозиция в тестах");
            axRita.dynamicCall("registerPPrivilege(QString,bool)", "IGNORE_EXPOSURE", false);
            axRita.dynamicCall("trPrivilege(QString,QString)", "IGNORE_MIN_LEVEL", "Игнорировать: Минимальная оптическая полка");
            axRita.dynamicCall("registerPPrivilege(QString,bool)", "IGNORE_MIN_LEVEL", false);
            */
            //text = QObject::tr("Select catalog for results");
            /*
            text = "Выбрать каталог для результатов";
            axRita.dynamicCall("trPrivilege(QString,QString)", "ENABLE_SELECT_FN", text);
            axRita.dynamicCall("registerPrivilege(QString,bool,int,QString)", "ENABLE_SELECT_FN", false, 3, "");
            text = "Сохранять данные оптических измерений в файл во время запуска протокола";
            axRita.dynamicCall("trPrivilege(QString,QString)", "COPY_ONLINE", text);
            axRita.dynamicCall("registerPrivilege(QString,bool,int,QString)", "COPY_ONLINE", false, 3, "");
            */
            // ...
            //qDebug() << "add new properties:" ;
            text = QObject::tr("Backup Exposure");
            axRita.dynamicCall("trPrivilege(QString,QString)", "BACKUP_EXPOSURE", text);
            axRita.dynamicCall("registerPrivilege(QString,bool,int,QString)", "BACKUP_EXPOSURE", false, 3, "");

            if(splash->isVisible()) splash->hide();//splash->close();
            splash->progress->setVisible(false);

            if(/*!cmd_str &&*/ !calibration)
            {
                axRita.dynamicCall("execute()");                            //	Вызов модального окна диалога
                axUser = axRita.querySubObject("currentLogin");

                qDebug() << "axUser" << axUser;
                if(!cmd_str && !calibration) w.Load_USERTESTs(axUser, &file_TESTs);
                else
                {
                    if(axUser)
                    {
                        w.nameUser_label->setText(axUser->property("userName").toString());
                    }
                }
            }
            /*if(axUser)
            {

                w.nameUser_label->setText(axUser->property("userName").toString());

                if(file_TESTs.open())
                {
                    QDomNode xmlNode = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\"");
                    doc.insertBefore(xmlNode, doc.firstChild());
                    QDomElement  root = doc.createElement("TESTs");
                    doc.appendChild(root);

                    root.setAttribute("operator", axUser->property("userName").toString());

                    cache = axUser->dynamicCall("getCache(QString)", "test").toStringList();
                    //cache.sort();
                    //qDebug() << cache;
                    foreach(item, cache) vec.append(item.toInt());
                    qSort(vec);
                    text = "";
                    id_user = axUser->property("userId").toInt();

                    for(i=0; i<vec.size(); i++)
                    {                        
                        QAxObject*  axItemCache = axUser->querySubObject("getCacheEntry(int)", vec.at(i));                        
                        if(id_user == axItemCache->property("idOwner").toInt())
                        {
                            //axItemCache->dynamicCall("setPermitionsAll(int)", 0);

                            //qDebug() << "permission: " << axItemCache->property( "permitionsAll" ) << axItemCache->property("uinIncome").toString();

                            text = axItemCache->property("dataEntry").toString();
                            doc_item.clear();
                            doc_item.setContent(text);
                            //root_item = doc_item.documentElement();
                            //root_item.setAttribute("owner", axUser->property("userName").toString());
                            //root_item.setAttribute("permission", axUser->property("permitionsAll").toInt());

                            root.appendChild(doc_item);

                        }
                        delete axItemCache;
                    }                    
                    QTextStream(&file_TESTs) << doc.toString();
                    file_TESTs.close();
                }
            }
            else
            {
                qDebug() << "axUser is absent!";
                //QMessageBox::warning(0,"Warning!", "You are logged in as a disabled user...");
                w.As_DisabledUser();
            }*/
        }
    }



        /*qDebug() << axUser->property( "userId" );		//	int		идентификатор пользователя
        qDebug() << axUser->property( "isAdmin" );		//	QString		признак администратора
        qDebug() << axUser->property( "userName" );		//	QString		логин пользователя
        qDebug() << axUser->property( "dtCreated" );	//	QString		дата регистрации
        qDebug() << axUser->property( "attrNames" );	//	QStringList	список названий атрибутов
        qDebug() << axUser->property( "privNames" );	//	QStringList	список названий привелегий
    */

    if(!cmd_str && axUser != NULL)
    {
        //splash->restart = true;   // RT
        //splash->setHidden(false);        
        splash->show();        
        a.processEvents();                
    }    

    //qDebug() << "splash_Show: " << QTime::currentTime();
    // Tests Verification Resource
    if(!axgp.control().isNull())
    {        
        w.Load_TESTVERIFICATION(&axgp);        
    }
    //qDebug() << "Load_TESTVERIFICATION: " << QTime::currentTime();

    // Load Tests:    
    if(file_TESTs.exists()) text = file_TESTs.fileName();
    else text = "";
    w.Load_TESTs(text);

    //qDebug() << "Load Tests: " << QTime::currentTime();


    // Load User    
    w.axRita = &axRita;
    w.axUser = axUser;    
    w.Load_USER(axUser);
    w.Load_POINTERDBASE(&axRita);
    //w.http->ax_user = axUser;

    //qDebug() << "Load_POINTERDBASE: " << QTime::currentTime();

    axRita.dynamicCall("startHeartBeat(qlonglong,int)", qApp->applicationPid(), 10000); // Проверка отклика с периодичностью 10сек

    // Load Menu    
    w.Load_MENU();

    //qDebug() << "Load_MENU: " << QTime::currentTime();

    // APP_NAME
    a.processEvents();
    w.APP_STATUS();
    //qDebug() << "APP_STATUS: " << QTime::currentTime();

    w.SetConnectToServer();
    //qDebug() << "SetConnectToServer: " << QTime::currentTime();

    //splash->timer->stop();    // RT
    splash->close();
    delete splash;  // RT

    if(w.trayIcon && w.trayIcon->isVisible()) w.close();
    else
    {
        if(axUser == NULL && !cmd_str && !calibration) QTimer::singleShot(1000, &w, SLOT(close()));
        else
        {
            w.setWindowFlags(w.windowFlags() | Qt::WindowStaysOnTopHint );
            w.show();

            w.setWindowFlags(w.windowFlags() & ~Qt::WindowStaysOnTopHint);
            w.show();
        }
    }    

    if(cmd_str) w.Load_PROTOCOL(fi.absoluteFilePath());
    if(calibration)  QTimer::singleShot(1000, &w, SLOT(load_CalibrationScenario()));

    QTimer::singleShot(2000, &w, SLOT(Check_Compatibility_WIN7()));



    return a.exec();
}

BOOL CALLBACK enumWindowCallback(HWND hWnd, LPARAM lparam)
{
    WCHAR WindowName[80], ClassName[80];
    GetWindowText(hWnd, WindowName, 79);
    GetClassName(hWnd, ClassName, 79);

    QString name = QString::fromStdWString(WindowName);
    QString class_name = QString::fromStdWString(ClassName);

    if(name.startsWith("DTmaster") || name.startsWith("RealTime__PCR"))
    {
        list_Win->append(name);
    }

    return(true);
}
