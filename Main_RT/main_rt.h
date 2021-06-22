#ifndef MAIN_RT_H
#define MAIN_RT_H

#include <QMainWindow>

#include <QObject>
#include <QGroupBox>
#include <QBoxLayout>
#include <QString>
#include <QFont>
#include <QSettings>
#include <QTextCodec>
#include <QPoint>
#include <QApplication>
#include <QDesktopWidget>
#include <QTimer>
#include <QSplashScreen>
#include <QPixmap>
#include <QDateTime>
#include <QLabel>
#include <QStatusBar>
#include <QListWidget>
#include <QListWidgetItem>
#include <QStackedWidget>
#include <QPalette>
#include <QDebug>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QEventLoop>
#include <QEvent>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QCloseEvent>
#include <QTranslator>
#include <QProcess>
#include <QTemporaryFile>
#include <QThread>
#include <QGraphicsDropShadowEffect>
#include <QMovie>
#include <QWebSocketServer>
#include <QWebSocket>
#include <QUrl>
#include <QDialog>
#include <QLibraryInfo>
#include <QSystemTrayIcon>
#include <QGuiApplication>
#include <QSysInfo>
#include <QOperatingSystemVersion>
#include <QEventLoop>

//... Qml, Qt_script...
#include <QtScript>
#include <QQuickWidget>
#include <QQmlEngine>
#include <QQmlContext>
#include <QMetaObject>
#include <QVariant>
//.....................

#include <QDomDocument>

#include <QAxObject>

#include "windows.h"
#include "winreg.h"

#include <sp.h>
#include <sp_rt.h>
#include <preference.h>
#include <aboutmoduls.h>
#include <list_webprotocols.h>
#include <webdialog.h>
#include <wrapperax.h>
#include <scenario.h>
#include <httpprocess.h>
#include <protocol.h>
#include <utility.h>
//#include <email.h>

//#include "global_param.h"
//#include <point_action.h>


//#include <QtDataVisualization/Q3DBars>
//using namespace QtDataVisualization;

#include <stdio.h>

//extern "C"
//{
//#include <openssl/applink.c>
//}
//#include <openssl/pem.h>
//#include <openssl/evp.h>

#define ID_LANG_RU	0x0419
#define ID_LANG_EN	0x0409


// DTReport !!!
typedef void (__stdcall cb_setprogressbar)(bool, int, char**, char**);  //callback

typedef void (__stdcall *ShowExportOptions)();
typedef void (__stdcall *Init) (void* appl_handle, char *dir_forms, char *ver, int Lang, void *sr, void *si, void *r, cb_setprogressbar *sp, void *ss, void *sq);

//------------------------------------------------------
typedef void* (__stdcall *Interface_Page)();
typedef void  (__stdcall *Destroy_Page)(void*);
typedef bool  (__stdcall *IsExit)(void*);
typedef void  (__stdcall *Load_3Dobject)(void*,void*);
typedef void  (__stdcall *Load_UserTests)(void*,std::string);
typedef void  (__stdcall *Load_User)(void*,QAxObject*);
typedef void  (__stdcall *Load_Protocol)(void*, char*);
typedef void  (__stdcall *Load_TestVerification)(void*,QAxObject*);
typedef void  (__stdcall *Load_TestVerification_Analysis)(void*,QAxObject*);
typedef void  (__stdcall *Load_TestVerification_Run)(void*,QAxObject*);
typedef void  (__stdcall *Load_PointTestDBase)(void*,QAxObject*);

typedef void  (__stdcall *Load_Menu)(void*, void*);
typedef void  (__stdcall *Execution_Menu)(void*, QString);
typedef void  (__stdcall *Enable_Menu)(void*, void*);
typedef void  (__stdcall *App_Status)(void*, QString, bool);

typedef void  (__stdcall *Send_PointerProtocol)(void*, void*);
typedef void* (__stdcall *Get_PointerProtocol)(void*);

//------------------------------------------------------
class Main_RT;
//------------------------------------------------------

class StackedWidget: public QStackedWidget
{
    Q_OBJECT

signals:
    void sJump_SetupToRun(QString);
    void sJump_RunToAnalysis(QString);
    void sJump_AnalysisToSetup(QString);
    void sJump_SetupToAnalysis(QString);

    void sChange_StateProtocolToWeb(QString);
    void sSend_ResultProtocolToWeb(QString);
    void sSend_OrderProtocolToWeb(QString);

    void sSendRunButton();

protected:

    bool event(QEvent *e);
};

//------------------------------------------------------
class axRitaThread: public QThread
{
public:
        QAxObject *axRita;
        void run()
        {
            if(!axRita->control().isNull()) axRita->dynamicCall("execute(bool)", false);
        }
};

//------------------------------------------------------
class Web_Interface: public QObject
{
    Q_OBJECT

public:
    Web_Interface(QWidget *parent = 0);
    ~Web_Interface();    

    Q_PROPERTY(bool connection_Server READ get_connection WRITE set_connection NOTIFY connection_Changed)
    Q_PROPERTY(QString url_Server WRITE set_url)

    bool connection_Server;
    QString url_Server;
    //QMap<QString,QString> list_Protocols;

    void set_connection(const bool&);
    bool get_connection() const;
    void set_url(const QString&);

    QMessageBox mb;
    Scenario    *scenario;

signals:
    void connection_Changed(bool);
    void listProtocols_Changed();
    void get_List(QString, QString);
    void get_protocol(QString);
    void confirmation_FromServer(QString, QString);
    void message_FromServer(QString);
    void message_ToServer(QString);
    void sAnswerToServer(QString);
    void sRun_Scenario(QString, QMap<QString,QString>);

    void send_ProtocolToSetup(QString);
    void send_ProtocolToScenario(QString);
    void send_ProtocolToAnalysis(QString);

public slots:
    void change_connection(bool);
    void get_ListProtocols(QString, QString);
    void get_Protocol(QString);
    void get_Message(QString);
    void send_Message(QString);
    void exec_Command(QString);

    void confirmationNotice(QString, QString);

};

//------------------------------------------------------
class Main_RT : public QMainWindow
{
    Q_OBJECT

public:
    Main_RT(QWidget *parent = 0);
    ~Main_RT();

protected:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;
    void showEvent(QShowEvent *e) Q_DECL_OVERRIDE;
    bool event(QEvent *e);

    //void keyPressEvent(QKeyEvent *);

private slots:
    void update_Rita();
    void update_DateTime();         // Data&Time
    void check_WebConnection();     // Web connection

public slots:
    //void changePage(QListWidgetItem *current, QListWidgetItem *previous);
    void changePage(QWidget*);
    void changePage(int current);
    void changeHelpPage();

    void menu_SetupPage();
    void menu_RunPage();
    void menu_AnalysisPage();
    void menu_EditProgramm();
    void menu_ViewVideoArchive();

    void slot_RunToAnalysis(QString);
    void slot_SetupToRun(QString);
    void slot_AnalysisToSetup(QString);
    void slot_SetupToAnalysis(QString);

    void slot_PreferenceExport();
    void slot_General();
    void slot_UserAuthorization();
    void slot_ReplaceUserAuthorization();
    void slot_Help();
    void slot_About();
    void slot_AboutModuls();
    void slot_ConnectToServer();
    void slot_QtScript();
    void slot_ListWebProtocolsOrder();
    void slot_ListWebProtocols();
    void slot_ProtocolToServer(QString message = "");
    void slot_ProtocolOrderToServer(QString message = "");
    void slot_ProtocolFromWeb(QString,QString);
    void slot_HttpProcess();
    void slot_ScenarioAction();

    void slot_ReloadListWebProtocols();
    void slot_ReloadListWebProtocolsOrder();
    //void slot_EmailAction();

    void slot_SetupMenu(QAction*);
    void slot_RunMenu(QAction*);
    void slot_AnalysisMenu(QAction*);

    void slot_SetupShow();
    void slot_RunShow();
    void slot_AnalysisShow();

    void slot_ActivePoint(QMovie::MovieState);
    void load_CalibrationScenario();

    void slot_DisableMenu(int);

private slots:
    void WS_Connected();
    void WS_Disconnected();
    void WS_Received();

    void WebConnection_Changed(bool);
    void WebListProtocolsOrder_Changed();
    void WebProtocolToSetup(QString);
    void WebConnect();
    void WebList_Get(QString, QString);
    void WebProtocolOrder_Get(QString);
    void WebProperty_AddReplace(QString);
    void WebConfirmationFromSetup(QString, QString);

    void WebListProtocols_Changed();
    void WebConnection_add_Changed(bool);
    void WebList_add_Get(QString, QString);
    void WebProtocol_Get(QString);
    void WebProtocolToAnalysis(QString);

    void ChangeStateProtocol_ToWeb(QString);

    void set_ConnectionWeb();
    void set_ConnectionWeb_add();

    void Check_Compatibility_WIN7();

private:

    QGroupBox *MainGroupBox;        // Main Box
    QListWidget *contentsWidget;    // List Widget
    QListWidget *helpWidget;        // List Widget
    StackedWidget *pagesWidget;     // Pages Widget
    QHBoxLayout *main_layout;       // Main Layout

    QMenu *file_menu;
    QMenu *test_menu;
    QMenu *setup_menu;
    QMenu *run_menu;
    QMenu *analysis_menu;
    QMenu *preference_menu;
    QMenu *authorization_menu;
    QMenu *help_menu;
    QMenu *network_menu;
    QMenu *tools_menu;

    QAction *exit;
    QAction *setup_Page;
    QAction *run_Page;
    QAction *analysis_Page;
    QAction *edit_Programm;
    QAction *view_VideoArchive;

    QAction *general;    
    QAction *user_authorization;
    QAction *user_replace_authorization;
    QAction *preference_export;
    QAction *RT_help;
    QAction *about;
    QAction *about_moduls;
    QAction *connect_WSServer;
    QAction *qtScript;
    QAction *list_ProtocolsOrder;
    QAction *list_Protocols;
    QAction *protocol_to_Server;
    QAction *protocolOrder_to_Server;
    QAction *httpprocess;

    QAction *scenario_action;
    //QAction *email_action;

    QAction *subMenu;

    General     *general_menu;
    AboutModuls *aboutmoduls_menu;
    SplashScreen *splash;
    Scenario    *scenario;    
    //Email       *email;

//--- WebSocket ---
    QWebSocket  *WS_Client;
    WebDialog   *Web_Dialog;
    List_WebProtocols *web_Protocols;
    QQuickWidget *pweb;
    QQuickWidget *pweb_add;

    //Q3DBars *widgetgraph;           // 3DBars object

//--- Others ------------

    QSettings *ApplSettings;        // Settings
    QTimer *timer;                  // Timer
    QTranslator translator;
    QTranslator qt_translator;
    QTimer *timer_Web;
    QTimer *timer_Rita;

//--- Flags ---
    bool flag_SaveLocationWin;
    bool flag_ApplicationSettings;
    bool flag_EnableSetup;
    bool flag_EnableRun;


//--- Methods ---
    void create_ListWidget();
    void create_PagesWidget();
    void load_Pages();
    void create_Icons();
    void readSettings();            // Read settings
    void writeSettings();           // Write settings
    void create_StatusBar();        // create StatusBar
    void create_Menu();             // create Menu
    void create_WebSocket();        // create WS Client

public:
//--- TrayIcon ---
    QSystemTrayIcon *trayIcon;
    QMenu           *trayIconMenu;
    QAction         *minimizeAction;
    QAction         *restoreAction;
    QAction         *quitAction;

    void set_ActiveTray(bool);

private slots:
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    void setTrayIconActions();
    void showTrayIcon();



public:

    QWidget *statusBar_widget;
    QLabel *mode_label;             // Label -> StatusBar
    QLabel *active_label;           // Active Point label
    QMovie *ActivePoint_gif;
    QLabel *spacer;
    QLabel *connect_toServer;       // Label -> StatusBar
    QMovie *obj_gif;
    QLabel *client_label;           // Client_Label -> StatusBar
    QLabel *server_label;           // Server_Label -> StatusBar
    QLabel *PrBar_status;           // Label -> ProgressBar
    QLabel *account_label;          // Account label (png)
    QLabel *nameUser_label;

    int last_page;
    QAxObject *axRita;              // COM object for operators authorization
    QAxObject* axUser;
    axRitaThread rita;
    HttpProcess *http;

    QString LanguageApp;            // language application
    QString StyleApp;               // style application

    void Load_POINTERDBASE(QAxObject*);     // Load pointer of the tests dBase
    void Load_TESTVERIFICATION(QAxObject*); // Load Test Verification resource
    void Load_TESTs(QString fn);            // Load Users TESTs
    void Load_USER(QAxObject*);             // Load User
    void load_user(QAxObject*);             // Load User properties for Main
    void Load_PROTOCOL(QString fn);         // Load Protocol from command-line arguments
    void Load_MENU();
    void Load_USERTESTs(QAxObject*, QTemporaryFile*); // Load tests from dbase

    void APP_STATUS();

    void As_DisabledUser();
    void SetConnectToServer();


    HINSTANCE dll_analysis;
    HINSTANCE dll_setup;
    HINSTANCE dll_run;
    HINSTANCE dll_saver;
    QWidget *p_analysis;
    QWidget *p_setup;
    QWidget *p_run;
    QWidget *p_saver;

    ShowExportOptions show_export_options;  // menu
    Init initialize;

    QMessageBox msgBox;
    QFont app_font;                 // Font

    Web_Interface *Web;
    Web_Interface *Web_additional;
    QMap<QString,QString> list_WebProtocolsOrder;
    QMap<QString,QString> list_WebProtocols;

    void Sleep(int ms);

    int ID_LANG;                                // lanquage for DTReport2.dll


//signals:
    //void load_CalibrationScenario();
};

#endif // MAIN_RT_H
