#ifndef RUN_H
#define RUN_H

#include "run_global.h"

#include <QtWidgets>
#include <QWidget>
#include <QMainWindow>
#include <QGroupBox>
#include <QPushButton>
#include <QBoxLayout>
#include <QWindow>
#include <QSplitter>
#include <QFont>
#include <QProgressBar>
#include <QString>
#include <QComboBox>
#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>
#include <QPainter>
#include <QToolBar>
#include <QIcon>
#include <QAction>
#include <QSettings>
#include <QEvent>
#include <QCheckBox>
#include <QTabWidget>
#include <QDebug>
#include <QTime>
#include <QTimer>
#include <QBasicTimer>
#include <QMap>
#include <QStringList>
#include <QTextEdit>
#include <QMessageBox>
#include <QByteArray>
#include <QTemporaryDir>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QTextStream>
#include <QQueue>
#include <QByteArray>
#include <QTextEdit>
#include <QPalette>
#include <QTextCursor>
#include <QTextBlock>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QTreeWidgetItemIterator>
#include <QTranslator>
#include <QAxObject>
#include <QByteArray>
#include <QStandardPaths>
#include <QQueue>
#include <QLibrary>

#include <QDomDocument>

#include <QThread>

#include <QTcpSocket>
#include <QAbstractSocket>
#include <QDataStream>
#include <QByteArray>
#include <QNetworkInterface>

#include <protocol.h>
#include <utility.h>
//#include "algorithm.h"

#include <graph_fluor.h>
#include <graph_temperature.h>
#include <listdev.h>
#include <protocol_information.h>
#include <wait_process.h>
#include <video.h>
#include <expo.h>
#include <contract.h>
//#include <crypto.h>

#include "define_PCR.h"
#include "request_dev.h"

#include "select_tubes.h"
#include "runtime.h"

#include "point_action.h"

#include "worker_wait.h"
#include "scandialog.h"
//#include "../../Main_RT/global_param.h"
//#include "../../Analysis/DockWidgetTitleBar/dockwidgettitlebar.h"


//... Crypto ...
//typedef void (__stdcall *Read_PubKey)(char*, BYTE*);

//...
#define ID_LANG_RU	0x0419
#define ID_LANG_EN	0x0409

#define ToleranceMinLevel 5

struct Fluor_SpectralInfo				// 30 byte
    {
        char fluor_name[12];
        short id_group;
        short coeff[8];
    };
struct SpectralCoeff_Compensation   	// 512 byte
    {
        char signature[30];
        short count;
        Fluor_SpectralInfo Info_Spectral[16];
    };
struct OpticalCoeff_Compensation        // 512 byte
    {
        char signature[30];
        char fluor_name[12];
        short coeff[192];
        char reserve[86];
    };
struct Fluor_UnequalInfo                // 18 byte
    {
       char fluor_name[12];
       short id_group;
       short coeff[2];
    };
struct UnequalCoeff_Compensation        // 512 byte
    {
        char signature[30];
        short count;
        Fluor_UnequalInfo Info_Unequal[16];
        char reserve[192];
    };


//--- Thread ------------------------------------------------------------------

class DeviceThread : public QThread
{
    Q_OBJECT

public:
    int get_count;
    QTimer timer;

public:
    DeviceThread() : get_count(0)
        {
        }

    void run() Q_DECL_OVERRIDE;

public slots:
    void slotGetInfoData();
    void slotResume();
    void slotSuspend();

signals:
    void Send_InfoData(int);

};

//-----------------------------------------------------------------------------

class ItemDelegate_InfoDevice: public QStyledItemDelegate
{
public:
    ItemDelegate_InfoDevice()
    {
    }

    void paint (QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    QString style;

};
//-----------------------------------------------------------------------------

#define LIST_uC "Optics,Temperature,Motor,Display,Server"

enum DevState {sUnknown = -1, sNothing, sRun, sPause, sHold, sWarming_up};
enum ID_SavePar {video = 0, expo};
enum ID_TypeApp {tMajor = 0, tLimited, tOEM};
enum ID_Reagent {rNormal = 0, rTimeLimited, rTestLimited, rLimited};


//----
typedef void (__stdcall *TranslateService) (int Lang, char *source, char **target);


//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
class RUNSHARED_EXPORT Run : public QMainWindow
{
    Q_OBJECT

public:

    Run(QWidget *parent = 0);
    ~Run();

    bool Is_Exit();
    void load_3D(void*);
    bool create_User(QAxObject*);       // create ax_User
    bool create_TestVerification(QAxObject*);   // create axgp - verification resource

    void Create_MenuActions(void*);             //
    void execution_MenuAction(QString);         //
    void Enable_MenuAction(void*);    
    void Application_status(QString, bool);

    void Send_pProtocol(void*);
    rt_Protocol *prot_FromSetup;
    void Create_SignatureForProtocol(rt_Protocol*, QVector<QString>*);

    HINSTANCE dll_3D;
    HINSTANCE ext_dll_handle;           // DTReport
    TranslateService translate_service;
    int ID_LANG;                        // lanquage for DTReport2.dll

    //... Crypto ...
    //HINSTANCE dll_Crypto;

    //Read_PubKey read_key;


    //...

private:

    rt_Protocol *prot;                  // Protocol
    QTemporaryDir dir_temp;
    QDir dir_Device;
    Action_Event *action_event;

    QString APP_NAME;

    ID_Reagent id_Reagent;
    ID_TypeApp id_Type;
    DevState Dev_State;
    int ActiveCh_Dev;
    bool flag_SavingOpticDev;
    bool flag_SavingResult;
    bool flag_Compatibility;
    bool flag_RunLoaded;
    bool flag_LastRun;
    bool flag_FACS;
    bool flag_ClosePrerunRun;
    bool flag_ControlRun;
    bool flag_DeviceSettings;
    bool flag_CMD;
    bool flag_ActivePoint;
    //bool flag_FirstConnectDevice;
    bool flag_WebProtocol;
    bool flag_SaveLocationWin;
    bool CALIBRATION_status;
    bool ALARM_Volume;
    bool ALARM_Program;
    bool ALARM_Exposure;
    bool ALARM_MinLevel;
    bool flag_SelectCatalogForResult;
    bool flag_SaveOnLine;
    int Try_NewExpozition;
    int Try_ReadInfoDevice;
    bool flag_EnableDev;
    bool flag_ContractDev;
    bool flag_LimitedDev;

    QQueue<int> Queue_State;
    QMap<int,QString> Queue_ReadAllSectors;


    QGroupBox *MainGroupBox;            // Main Box
    QGroupBox *ControlBox;              // Control Box
    QGroupBox *ChartBox;                // Chart Box
    QSplitter *main_spl;                // splitter between Chart and Control widget
    QMainWindow *ChartWindow;
    QMainWindow *ControlWindow;

    QSplitter *chart_spl;               // Chart group ...
    QMainWindow *Fluor_Chart;           //
    QMainWindow *Temperature_Chart;     //
    QGroupBox *Fluor_Box;
    QGroupBox *Temperature_Box;

    QSplitter *control_spl;             // Control group ...
    QMainWindow *Info_window;           //
    QMainWindow *Control_window;        //
    QGroupBox *Info_Box;
    QGroupBox *Control_Box;

    QLabel      *NameProt;
    QTabWidget  *Tab_Protocol;

    //--- parameters Protocols ---
    int FN;                         // fn in protocol
    int fn_dev;                     // fn in device
    double T_hold;
    double T_current;
    int active_ch;                  // active_channels
    QMessageBox message;

    //--- Device Parameters ---
    double ver_OPTICS;

    Select_tubes *sel;
    QMainWindow *selPlate;

    RunTime          *runtime_Program;    
    Plot_ProgramAmpl *plot_Program;    
    Plate_Protocol   *plate_Protocol;
    Info_Protocol    *info_Protocol;    
    QGroupBox        *group_Program;
    Scheme_ProgramAmpl  *scheme_Program;
    QSplitter        *program_spl;
    QTextEdit        *comments;

    //QLabel      *Device_Locked;
    QLabel      *NameDev;
    QPushButton *ClearProtocol;
    QPushButton *OpenProtocol;
    QPushButton *LastProtocol;
    QPushButton *IsEqual;
    QPushButton *ListDevices;    
    QPushButton *RunDev;
    QPushButton *StopDev;
    QPushButton *PauseDev;
    QTabWidget  *Tab_Control;
    QCheckBox   *StandBy;
    QGroupBox   *StandBy_Box;
    QScrollArea *LastRun_Area;
    QGroupBox   *LastRun_Box;
    QPushButton *LastRun;
    QCheckBox   *Analyze_immediately;
    QGroupBox   *OpenClose_Box;
    QToolButton *OpenBlock;
    QToolButton *CloseBlock;
    QGroupBox   *CMD_Box;
    QTextEdit   *cmd_Edit;
    QString     LastCMD;
    QComboBox   *cmd_ComboBox;
    QScrollArea *Preference_Area;
    QGroupBox   *Preference_Box;
    QPushButton *VideoImage;
    QPushButton *ExpoValue;
    QPushButton *Meas_heigthTube;
    QPushButton *OpticDev_TMP;
    QTreeWidget *Error_Info;
    //QTextEdit   *info_device;
    QTreeWidget *info_device;
    QCheckBox   *SaveImage_rejime;
    //bool        SaveImage_rejime_Active;
    //bool        SaveImage_rejime_IsChecked;

    ItemDelegate_InfoDevice *Delegate_InfoDevice;

    QTimer timer_temp;
    QTimer dev_timer;
    DeviceThread dev_Thread;
    QTimer Web_timer;

    QList<QString> List_Requests;
    QMap<QString, QBasicTimer*> Map_TimerRequest;
    QStringList Catalogue_Requests;

    QAxObject*  ax_user;
    QAxObject*  axgp;
    QDir user_Dir;
    QString OnLine_FileName;
    QString Save_FileName;

    //QStringList list_status;        // Temporary
    QLabel Label_status;
    int Tube_Height;

    //--- Actions -----------
    QAction *open_prot;
    QAction *save_prot;
    QAction *new_prot;

    QAction *videoimage;
    QAction *expo;

    //... methods ...
    void create_ChartControlBox();
    void createMenus();         // Menu
    void createFluorToolBars(); // ToolBar
    void createStatusBar();     // StatusBar
    void createCONTROL();       // CONTROL
    void createINFO();          // INFO
    void createActions();       // Actions
    void createSelectTubes();   // Select

    void readSettings();        // readsettings
    void writeSettings();       // Write settings

    void create_TimerRequest(); // Timers Request

    void addProtocol_toList(QString);   // add last Protocol to list

    void create_RequestStatusInfo(QString,short);   // Request & Status

    void RefreshInfoProtocol(rt_Protocol*); // info protocol

    QString Create_INProtocol(QString);     // create template IN
    bool Create_RunProtocol();              // create RUN protocol (IN + device)
    bool Create_MeasureProtocol(QString text = "", bool online = false); // create Measure protocol (RUN + Measure)
    bool Create_LastRunProtocol();

    void RemoveChannels_InSourceset(QDomElement*, QVector<int>*);

    bool Check_ChangeDevice(int);           // checking change status device
    //bool Check_Compatibility();             // checking compatibility device and protocol

    bool Validate_OpenProtocol(rt_Protocol*, QString&);       // Validate Open Protocol

    bool Load_Expozition();
    bool Check_TestsExposition(rt_Protocol *);  // check tests in protocol on equal exposition and fluors
    bool Check_TestsPtrogramm(rt_Protocol *);   // check tests in protocol on equal programm
    bool Check_TestsVolumeTube(rt_Protocol *);  // check tests in protocol on equal volume tube
    bool Check_ValidTests(rt_Protocol *);       // check tests on valid for limited device
    bool IsExpositionEqual();                   // compare exposition in Protocol and Device
    bool WriteNewExpositionToDevice();          // write new exposition to device from protocol
    void Read_AllSectors();

    bool Validate_Test(rt_Test*);   // validate test

    void Generate_IDProtocol(rt_Protocol *); // generate id protocol

    bool Load_SpectralCoeff();  // load spectral coefficients
    bool Load_OpticCoeff();     // load optic coefficients
    bool Load_UnequalCoeff();   // load unequal coefficients
    bool Load_Mask();           //
    QString Load_OpticMask();      // load optical mask: status auto,radius,offset,mask(x,y)

    double Get_T_hold();        // Get T_hold
    bool Check_HoldLevel();     // checking temperature level in Hold    
    double Ver_uOPTICS();

    void wait_process(Wait_state, int ,QString); // run wait process

    bool Add_CalibrationProperties(QMap<QString,QString>);   // add calibration properties in IN.rt when using Calibration tests

    QDomElement SaveXML_VideoData(QDomDocument &doc, QString dir);   // create xml element for video data

    // Crypto
    QByteArray  PubKey_BUF;     // 16 byte
    bool Read_PubKey();
    void DeCryption_EnCryption(UCHAR *buf, UCHAR *key);    
    void Random_ByteBuffer(char *p, int size);
    void Read_CryptoDevice();
    void Check_PubkeyCrypto(QMap<QString,QByteArray>*);
    //

    // Translate
    void Create_TranslateList(rt_Test*, QStringList&);  // create translate list
    void Translate_Tests(QStringList&);                 // translate names and research
    QMap<QString,QString> Map_TestTranslate;            //
    //

    QProgressBar *run_progress;    // ProgressBar -> StatusBar
    QLabel *PrBar_run_status;           // Label -> ProgressBar
    QProgressBar *main_progress;    // ProgressBar -> StatusBar
    QLabel *PrBar_status;           // Label -> ProgressBar
    QToolBar *fluor_toolbar;
    QToolBar *fileToolBar;
    QSettings *ApplSettings;        // Settings

    GraphFluorPlot          *plot_fluor;
    GraphTemperaturePlot    *plot_temperature;
    ListDev                 *list_dev;
    Wait_Process            *wait_obj;
    ScanDialog              *scan_dialog;


    QTranslator translator;
    QTranslator qt_translator;
    void readCommonSettings();
    QString StyleApp;

    void Display_ProgressBar(int percent, QString text)
     {
        main_progress->setValue(percent);
        PrBar_status->setText(text);
        PrBar_status->setMinimumSize(PrBar_status->sizeHint());
        qApp->processEvents();
     }

    static bool Wrapper_To_WaitProcess(void*, int, QString);

public:
    Video                   *Video_Image;
    Expo                    *Expo_check;
    Contract                *Contract_Dev;


private:    

    QAction *select;
    QAction *video;
    QAction *UpMarker;
    QAction *DownMarker;

    //... NetWork ...
    QTcpSocket  *m_pTcpSocket;
    //ushort      m_nNextBlockSize;
    quint32      m_nNextBlockSize;

    QPushButton *send_message;
    QString ip_addres;
    int port_serverUSB;

    QMap<QString,QString> map_InfoDevice;
    QMap<QString,QString> map_InfoData;
    QMap<QString,QString> map_Run;
    QMap<QString,QByteArray> map_Measure;
    QMap<QString,QString> map_CMD;
    QMap<QString,QString> map_Picture;
    QMap<QString,QByteArray> map_PictureData;
    QMap<QString,QString> map_SavePar;
    QMap<QString,QString> map_temp;
    QMap<QString,QString> map_Closed;
    QMap<QString,QString> map_Sector;
    QMap<QString,QString> map_Median;
    QMap<QString,QString> map_Default;
    QMap<QString,QString> map_Logged;
    QMap<QString,QString> map_ReadOpticDev;
    QMap<QString,QByteArray> map_CRYPTO;

    QMap<QString,QString> *map_TMP;

    //QMap<QString,QString> Map_TestTranslate;

    // device parameters:
    QVector<QString> SpectralCoeff;
    QVector<QString> OpticCoeff;
    QVector<QString> UnequalCoeff;

    QVector<short> XY;          // centers
    qint16 Rx;                  // radius_x
    qint16 Ry;                  // radius_y
    QVector<qint8> offset_XY;   // offsts for channel

    // expozition
    QVector<quint16> Expozition;    // 2 expt for channel: 2*CountMax = 2*8
    QVector<quint16> Expozition_Def;    // default expositions

public slots:
    void Open_Protocol(QString fn = "");
    void clear_protocol();

    void open_protocol();
    void new_protocol();
    void save_protocol();
    void Save_OnLineProtocol(QString);

    void create_VideoImage();
    void check_Expo();
    void measure_TubesHeight();
    void change_SaveImageRejime();
    //void update_SaveImageActive();

    void send_CRYPTO();

private slots:
    void slotConnected();
    void slotDisconnected();
    void slotReadyRead();
    void slotSendToServer(QString);
    void slotError(QAbstractSocket::SocketError);

    void slotReadInfoData(int);
    void slotReadInfoData_();
    void slotSendRequestInfo();
    void slotRefreshInfoDevice();

    void slotResume();
    void slotSuspend();

    void slot_Emulated();
    void slotGetInfoDevice();
    void slot_RUN();
    void slot_PreRUN();
    void slot_ProtocolIsEqual();        // compare protocol with protocol from Setup
    void slot_CheckExposition_Run();
    void slot_ClosePrerunRun();
    void slot_PAUSE();
    void slot_STOP(bool msg = true);
    void slot_PrepareLastRun();
    void slot_LastRun();
    void slot_StandBy();
    void slot_RefreshMeasure(QMap<QString,QString>*);
    bool slot_GetMeasure(int);
    void slot_SendMeasureToWeb(int);
    void slot_ChangeStateDevice(int);
    void slot_OpenBlock();
    void slot_CloseBlock();
    void slot_SelectTab(int);
    void slot_GetPicture(int, int, int);
    void slot_SavePar(int, QString);
    void slot_WriteSector(int, QString);
    void slot_ReadSector(int,QString);
    void slot_Read_CurrentSector();
    void slot_Processing_ReadSector(QMap<QString,QString>&);
    void slot_SaveParMedian(QString, QString);
    void slot_SaveParDefault(QString, QString);
    void slot_SaveDefExposition();
    void slot_MeasuredTubeHeiht(int);
    void slot_SaveMeasuredTubeHeiht(int);

    void slot_Select();

    bool Check_Compatibility();             // checking compatibility device and protocol

    void ShowListDevice();
    void Show_Contract();

    void Up_sizeMarker();
    void Down_sizeMarker();

    void slot_CursorWait(bool);
    void slot_DisplayProcess(int, QString);
    void slot_RunProcess(int, QString);

    void slot_TimerTemp();
    void slot_GetErrorInfo(QString);
    void slot_LastProtocols();

    void Contract_Check();                  // CONTARCT!!!!!!!!!!!!
    void Check_ContractDevice(QMap<QString,QByteArray>*);
    void slot_CheckContractDevice();
    void slot_CheckPubkeyCrypto();
    void slot_RefreshCMD(QMap<QString,QString>*);

    void Read_CalibrationSettings(QMap<QString,QString>*);    // read calibration properties from file (default value)

    void CreateMeasure_And_GoToAnalysis();
    void slot_ReadOpticDev();

    // ... Point_Actions ...
    void slot_ReceivedActionPoint(Action_Event*);
    void PA_Error(QString msg = "");
    void PA_Ok();
    void PA_OpenProtocol();
    void PA_OpenDevice();
    void PA_RunProtocol();
    void PA_RunProtocol_afterStop();
    void PA_RunProtocol_Stop();
    void PA_BarCode();
    void PA_TubeHeiht();
    void PA_RunTubeHeiht(QString);

    void slot_RecievedRunButton();
    bool change_Exposure(short, QVector<quint16>*);
    bool backup_Exposure();
    void backup_TubeHeiht();
    // .....................

    void update_ProtocolInfoToWeb(QString state = "");

    //void slot_TimeOutRequest(QString);

public slots:
    void resize_splitter(int,int);

signals:
    //void sRefresh(QString str);
    void sRefresh(QMap<QString,QString>*);
    void sNewProtocol(rt_Protocol*);
    void sRefreshFluor(void);
    void sChangeStateDevice(int);
    void sRunAfterPrerun(void);
    void sPrerunAfterClose(void);
    void sSendPicture(QMap<QString,QByteArray>*);
    void sSendDigit(QMap<QString,QByteArray>*);
    void sSet_PauseRejime(int);

    void finished_WaitProcess();

    void sErrorInfo(QString);

    void sActionPoint(Action_Event*);
    void sRunBarcode();
    void sMeasureTubeHeiht(int);
    void sStatusMeasureTubeHeiht(int);

    void sRead_CRYPTO(QMap<QString,QByteArray>*);

protected:
    bool eventFilter( QObject *, QEvent * );
    bool event(QEvent *e);
    void resizeEvent(QResizeEvent *e);
    void showEvent(QShowEvent *e);
    void timerEvent(QTimerEvent *event) Q_DECL_OVERRIDE;
};

#endif // RUN_H
