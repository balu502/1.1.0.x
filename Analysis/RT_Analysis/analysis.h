#ifndef ANALYSIS_H
#define ANALYSIS_H

#include "analysis_global.h"

#include <QtWidgets>
#include <QWidget>
#include <QMainWindow>
#include <QGroupBox>
#include <QPushButton>
#include <QBoxLayout>
#include <QSplitter>
#include <QFont>
#include <QAction>
#include <QIcon>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QToolBar>
#include <QProgressBar>
#include <QLabel>
#include <QSettings>
#include <QSize>
#include <QList>
#include <QString>
#include <QFile>
#include <QMessageBox>
#include <QDir>
#include <QWindow>
#include <QTabWidget>
#include <QEvent>
#include <QResizeEvent>
#include <QVariant>
#include <QTranslator>
#include <QPrinter>
#include <QTemporaryDir>
#include <QMovie>
#include <QCryptographicHash>
#include <QStandardPaths>
#include <QtOpenGL/QtOpenGL>

#include <QDebug>

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>
#include <QXmlStreamReader>
#include <QUrlQuery>
#include <QDomDocument>

#include "windows.h"
#include <winver.h>

#include "protocol.h"
#include "alg_interface_qevent.h"
#include "alg_interface.h"
#include "generic_interface.h"
#include "analysis_interface.h"
#include "utility.h"
#include "graph.h"
#include "select_tubes.h"
#include "info.h"
#include "test_editor.h"
#include "common_pro.h"
#include "report_parameterization.h"
#include "report.h"
#include "video_archive.h"
#include "point_action.h"
#include "email.h"

#define ID_LANG_RU	0x0419
#define ID_LANG_EN	0x0409

typedef Alg_Interface*      (__stdcall *Interface_factory)();
typedef External_Interface* (__stdcall *ExtInterface_factory)();
typedef Analysis_Interface* (__stdcall *Analysis_factory)();

//.............................................................................

// DTReport !!!

typedef void (__stdcall callback_report)(int, int, void*, bool);        //callback
typedef void (__stdcall select_tube)(char* tube, char** err);           //callback
typedef void (__stdcall cb_setprogressbar)(bool, int, char**, char**);  //callback

typedef void (__stdcall *Init) (void* appl_handle, char *dir_forms, char *ver, int Lang, void *sr, void *si, void *r, cb_setprogressbar *sp, void *ss, void *sq);
typedef void (__stdcall *SetFont) (char *name, char *style, int size);
typedef void (__stdcall *SetUserFolder) (char *name);

typedef void (__stdcall *ShowResults)(HWND handle, char* path, char* FormsID, char* ARTTestID, select_tube *se, bool *visible, char** err);
typedef void (__stdcall *CloseResults)();
typedef void (__stdcall *ResizeResults)(HWND handle);

typedef void (__stdcall *Cr_Report) (char *p, char* FormsID, bool save, callback_report *proc, void*, char** err);
typedef void (__stdcall *ShowReport)(HWND handle, bool s, bool *modify, char** err);
typedef void (__stdcall *CloseReport)();

typedef void (__stdcall *ChangeStatusTubes)(char *p);
typedef void (__stdcall *SelectCurve)(char *p);
typedef void (__stdcall *GetGroupColor)(char** p);

//typedef void (__stdcall *ShowExportOptions)();
typedef void (__stdcall *ExportResultsEx)();

typedef void (__stdcall *GetDTRDisplayInfo)(char* FormID, char** DisplayName, char** Path, int* Release, int* Build);
typedef void (__stdcall *TranslateService) (int Lang, char *source, char **target);

// RDML !!!
typedef bool (__stdcall *RDML_IMPORT)(char*);


//.............................................................................

typedef struct
{
    QString idtask;
    QString uin;
    QString barcode;
    QString income;
    QString name;
    QString target_dt;
    QString suin;
    QString sent;
    QString sender;
    QString datasize;

    QString filename;

}xml_info;



class GroupBox : public QGroupBox
{
    Q_OBJECT

public:
    GroupBox(QWidget *parent = 0)
        : QGroupBox(parent) {}

signals:
    void Resize(void);

protected:
    virtual void resizeEvent(QResizeEvent *e);
    virtual void showEvent(QShowEvent *e);


};
//-----------------------------------------------------------------------------
class ANALYSISSHARED_EXPORT Analysis : public QMainWindow
{    
    Q_OBJECT

public:
    Analysis(QWidget *parent = 0);
    ~Analysis();

    bool Is_Exit();
    bool create_User(QAxObject*);               // create ax_User
    bool create_TestVerification(QAxObject*);   // create axgp - verification resource
    QAxObject *ax_user;
    QAxObject *axgp;
    bool flag_ReOpenProtocol;
    bool flag_CommonSettings;
    bool flag_EditTest;
    bool flag_ActivePoint;
    bool flag_SaveLocationWin;
    bool flag_SaveChangesProtocol;

    QStringList FirstPCR_TESTs;

    QString ArchiveFile_path;
    QDir user_Dir;
    QTemporaryDir dir_temp;
    Action_Event *action_event;

    int ID_LANG;                                // lanquage for DTReport2.dll

    void Create_MenuActions(void*);             //
    void execution_MenuAction(QString);         //
    void Enable_MenuAction(void*);

    // Translate
    QMap<QString,QString> Map_TestTranslate;            //
    void Create_TranslateList(rt_Test*, QStringList&);  // create translate list
    void Translate_Tests(QStringList&);                 // translate names and research
    void Translate_Catalog(QVector<rt_Test*>*); // translate catalog from russian
    //

    // RDML
    HINSTANCE dll_rdml;

    //

public slots:

    void save_XML();
    void open();
    void open_Protocol(QString fn = "",             //
                       bool send = false,           // Open protocol from file with sending to ...
                       bool create_hash = false);   // Create hash after run

    void save();
    void view_listProtocols();
    void select();                      // Select tubes

    void info_Protocol();
    void report();
    void report_Param();
    void Export_Results();

    void load_to_setup();               // load protocol to setup
    void reopen(QString fn = "");       // reopen protocol with another plate
    void editTest();                    // edir preference tests
    void editPreferencePro();           // edit preference protocol
    void replaceTests(QString&);        // replace tests in file protocol for future reopen...
    void replaceProperties(QString&);   // replace properties in file protocol for future reopen...

    void preview_archive();             // preview video archive
    void email_Send();                  // send protocol using email

    void rdml_import();

    void resize_splitter(int,int);
    void resize_extResults();
    void resize_CpAllResults();

    void helpChart();
    void Ct_Chart();
    void Lg_Chart();
    void Norm_Chart();
    void Melting_Chart();
    void ColorMelting_Chart();
    void ColorMelting_Default();
    void dFdT_Negative();
    void Up_sizeMarker();
    void Down_sizeMarker();
    void Up_Thickness();
    void Down_Thickness();
    void Curve_Edit(bool);
    void Refresh_Edit();

    void Increase_PlotScale();
    void Decrease_PlotScale();
    void Auto_PlotScale();

    void Clear_Rejimes(int);
    void ShowHidden_Select();

    void Change_IndexTabGrid(int);

    void Save_ChangesProtocol();
    bool Color_IsChanged();
    bool Comments_IsChanged();

    bool Validate_OpenProtocol(rt_Protocol*, QString&);       // Validate Open Protocol

    //... network ...................
    void view_listXmlProtocols();       // Open list XML Protocols
    void replyFinished(QNetworkReply*);     //
    void replyFinished_POST(QNetworkReply*);//
    void get_XmlProtocol(xml_info*);        // Get single XML Protocol (as Base64 string)
    void send_XmlProtocol(xml_info*);       // Send single XML Protocol (as Base64 string) with results
    //...............................

private:

    QGroupBox   *MainGroupBox;          // Main Box
    QGroupBox *GridBox;                 // Grid Box

    QTabWidget *GridTab;                // Grid Tab
    GroupBox *GridBox_allcp;
    GroupBox *GridBox_ext_globe;
    GroupBox *GridBox_ext_all;
    GroupBox *GridBox_ext_dna;

    //QTabWidget *Tab_Analyser;
    QTabWidget *Tab_AnalyserGlobe;
    QMap<int, QWidget*> Map_TabAnalyser;


    //QPushButton *report_CpAll;

    QGroupBox *ChartBox;                // Chart Box
    QMainWindow *ChartWindow;
    QMainWindow *GridWindow;

    //--- Chart Box ---------
    QHBoxLayout *layout_Chart;
    QVBoxLayout *layout_ChartTool;

    //--- Grid Box ----------
    QVBoxLayout *layout_Grid;
    QVBoxLayout *layout_Grid_ext;
    QVBoxLayout *layout_Grid_globe;

    //--- Attention ---
    QTextEdit   *Attention_Browser;
    QString     Attention_Marker;
    QMessageBox message_Marker;
    QCheckBox   *cb_MarkerAttention;
    bool        MarkerAttention;

    //--- Others ------------    
    QSettings   *ApplSettings;      // Settings
    QTimer      *timer;             // Timer
    Info        *Info_Pro;          // Information
    Report_Parameterization *Report_Param;  // Report for Parameterization DATA
    Report                  *Report_All;    // Report for My
    Video_Archive *Archive;         // Video archive preview
    Email         *email;
    //NCReport *ncreport;

    QTranslator translator;
    void readCommonSettings();
    QString StyleApp;

    //--- Actions -----------
    QAction *open_ListXML;
    QAction *openXmlAct;
    QAction *saveXmlAct;
    QAction *open_LastProtocols;
    QAction *saveAct;
    QAction *selectAct;

    QAction *load_ToSetup;
    QAction *reopenXmlAct;
    QAction *editTestAct;
    QAction *edit_PreferenceProtocol;

    QAction *preview_videoarchive;
    QAction *EMail_send;
    QAction *RDML_import;

    QAction *infoAct;
    QAction *reportAct;
    QAction *reportParamAct;
    QAction *exportResults;
    QAction *excelAct;

    QAction *helpChartAct;
    QAction *CtAct;
    QAction *LgAct;
    QAction *NormalizationAct;
    QAction *MeltingAct;
    QAction *ColorAct;
    QAction *dFdT_negative;

    QAction *Increase_Scale;
    QAction *Decrease_Scale;
    QAction *Auto_Scale;
    QAction *UpMarker;
    QAction *DownMarker;
    QAction *UpThickness;
    QAction *DownThickness;

    QAction *curveEdit;
    QAction *refreshEdit;

    QAction *Validity_Hash;

    //... methods ...
    void createActions();       // Actions
    void createMenus();         // Menu
    void createToolBars();      // ToolBar
    void createStatusBar();     // StatusBar

    void readSettings();        // Read settings
    void writeSettings();       // Write settings

    void addProtocol_toList(QString fn);    // add opening Protocol to list

    void createChartGroupBox(); // ChartGroup Box
    void createSelectTubes();   // Select Tubes

    void Display_ProgressBar(int percent, QString text)
     {
        main_progress->setValue(percent);
        PrBar_status->setText(text);
        PrBar_status->setMinimumSize(PrBar_status->sizeHint());
        qApp->processEvents();
     }

    void Check_Marker();        // check for validity markers
    void Attention_Analysis();  //
    void Check_CpValidity(QString&);
    void Check_OverflowData(QString&);

    bool Validate_Test(rt_Test*);   // validate test
    bool Validate_Calibration();    // Calibration: VideoDir

    void Load_InfoAnalyser();
    void Load_InfoEXTAnalyser();
    void Load_DTRVersion(rt_Protocol*, QVector<QString>*);

    bool Create_HashProtocol(QString);
    QString Original_FileName(QString);
    QString Check_ValidNameFile(QString);
    QString Unique_FileName(QString);

    void ResultColor_Melting(QString, int);
    void RemoveChannels_InSourceset(QDomElement*, QVector<int>*);

    void Save_ColorComments(rt_Protocol*, QString, bool, bool);

    static void Wrapper_To_Display_ProgressBar(void* pt2Object, int percent, QString text);     // for my DLL
    static void __stdcall Wrapper_To_ReportStatus(int, int, void* pt2Object, bool);             // for DTReport
    static void __stdcall Callback_SelectTube(char* tube,char** err);                           // for DTReport
    static void __stdcall Callback_SetProgressBar(bool s, int pos, char** mes, char** err);     // for DTReport

public:

    QSplitter *main_spl;            // splitter between Chart and Grid widget
    QProgressBar *main_progress;    // ProgressBar -> StatusBar
    QLabel *PrBar_status;           // Label -> ProgressBar
    QLabel *FileName_Protocol;
    QLabel *DirLabel_Protocol;
    bool original_FileName;
    QToolBar *fileToolBar;
    QToolBar *viewToolBar;
    QToolBar *chartToolBar;
    QToolBar *helpToolBar;

    QMovie  *obj_gif;
    QLabel  *label_gif;

    QLineEdit   *Name_Protocol;
    QLabel      *Label_Protocol;

    //--- NetWork -----------
    QNetworkAccessManager *net_manager_GET;
    QNetworkAccessManager *net_manager_POST;
    QUrl net_adress;
    void create_NetWork();
    xml_info current_XML;

    Select_tubes *sel;                  // Select
    Test_editor  *test_editor;          // Dialog: Test_Editor
    Common_Pro   *Pro_editor;           // Dialog: Pro_Editor

    Alg_Interface *alg;
    HINSTANCE dll_handle;

    External_Interface *ext_alg;
    HINSTANCE ext_dll_handle;
    bool ext_dll_handle_VALID;
    QMap<int, QString> Map_EXTanalyserNAME;

    Analysis_Interface *analyser;
    HINSTANCE analyser_handle;
    QMap<int, Analysis_Interface*> Map_analyser;
    QMap<int, QString> Map_analyserNAME;
    QVector<HINSTANCE> List_analyserHandle;

    GraphPlot *plot;                // Plot
    QwtSymbol *symbol;
    QwtPlotCurve *curve;
    rt_Protocol *prot;              // Protocol

    QString fn_IN, fn_OUT, fn_OUTTMP;
    QMessageBox message;
    QVector<short> EnableTubes_Prev;
    QMap<QString,QString> *map_TestTranslate;

    //... External moduls ...
    Init initialize;
    SetFont set_font;
    SetUserFolder set_folder;

    ShowResults show_results;
    CloseResults close_results;
    ResizeResults resize_results;
    ChangeStatusTubes enable_tubes;
    SelectCurve select_curve;
    GetGroupColor get_color;
    TranslateService translate_service;

    Cr_Report report_XMLfile;
    ShowReport show_report;
    CloseReport close_report;

    //ShowExportOptions show_export_options;
    ExportResultsEx export_results;

    GetDTRDisplayInfo get_Info;

    //.......................
private slots:
    void change_MarkerAttention(int);
    void restore_Cursor();

    // ... Point_Actions ...
    void slot_ReceivedActionPoint(Action_Event*);
    void PA_Error(QString msg = "");
    void PA_AnalysisProtocol();


    //.......................
signals:
    void sActionPoint(Action_Event*);

protected:

    bool event(QEvent *e);
    void resizeEvent(QResizeEvent *e);
    void showEvent(QShowEvent *e);
};

#endif // ANALYSIS_H
