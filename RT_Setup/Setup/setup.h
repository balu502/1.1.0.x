#ifndef SETUP_H
#define SETUP_H

#include "setup_global.h"

#include <QtWidgets>
#include <QWidget>
#include <QMainWindow>
#include <QGroupBox>
#include <QPushButton>
#include <QBoxLayout>
#include <QSplitter>
#include <QFont>
#include <QFontMetrics>
#include <QAction>
#include <QIcon>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QToolBar>
#include <QToolButton>
#include <QButtonGroup>
#include <QAbstractButton>
#include <QProgressBar>
#include <QLabel>
#include <QLineEdit>
#include <QSettings>
#include <QSize>
#include <QList>
#include <QString>
#include <QFile>
#include <QTemporaryDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QDir>
#include <QWindow>
#include <QTabWidget>
#include <QEvent>
#include <QResizeEvent>
#include <QVariant>
#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>
#include <QPainter>
#include <QPainterPath>
#include <QHeaderView>
#include <QTableWidget>
#include <QComboBox>
#include <QColor>
#include <QColorDialog>
#include <QGridLayout>
#include <QPixmap>
#include <QPalette>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QTreeWidgetItemIterator>
#include <QAbstractButton>
#include <QMap>
#include <QMapIterator>
#include <QHash>
#include <QDateTime>
#include <QClipboard>
#include <QErrorMessage>
#include <QScrollArea>
#include <QStyleOptionHeader>
#include <QTranslator>
#include <QTimer>
#include <QLibraryInfo>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QStandardPaths>
#include <QRegExpValidator>
#include <QDomNamedNodeMap>
#include <QMovie>

#include <QDebug>

#include <string>
#include <QAxObject>

#include <zoomplate.h>
#include <copy_blocktests.h>
#include <add_tests.h>
#include <test_editor.h>
#include <test_interface.h>
#include <common_pro.h>
#include <protocol.h>
#include <utility.h>
#include <protocol_information.h>
#include <define_PCR.h>
#include <algorithm>

#define LIST_DEVICE "96,384,48,192"
#define List_DEVICENAME "Prime_96,Prime_384,Lite_48,Lite_192"
#define UNDO 5

#define ID_LANG_RU	0x0419
#define ID_LANG_EN	0x0409

#define PATIENT "patient"
#define SEX     "sex"
#define AGE     "age"
#define ORG     "organization"
#define PHISIC  "phisician"
#define DATA    "date"
#define NOTE    "note"
#define ADD_PRO "additional"



typedef void (__stdcall *Interface_Editor)(std::string, bool&, std::string&);
typedef void (__stdcall *GetDTRDisplayName)(char* FormID, char** DisplayName);
typedef void (__stdcall *GetDTRDisplayInfo)(char* FormID, char** DisplayName, char** Path, int* Release, int* Build);
typedef void (__stdcall *Init) (void* appl_handle, char *dir_forms, char *ver, int Lang, void *sr, void *si, void *r, void *sp, void *ss, void *sq);
typedef void (__stdcall *SetFont) (char *name, char *style, int size);
typedef void (__stdcall *SetUserFolder) (char *name);
typedef void (__stdcall *TranslateService) (int Lang, char *source, char **target);
typedef void (__stdcall *GetSampleSettings)(char* FormID, char** Settings);


enum Plate_Action{clear,auto_locate,free_locate,selected,user_locate,simply};
enum Plate_View{v_color,v_numeration};

typedef struct
{
    QString name;
    bool type;
    QString date_time;

} TEMPLATE_NAME;

void Paint_ChannelsRegions(QPainter*, QRect, int);

//-----------------------------------------------------------------------------
class MyHeader: public QHeaderView
{
    Q_OBJECT

public:

        MyHeader(Qt::Orientation orientation, QWidget *parent = 0)
            : QHeaderView(orientation, parent) {selected = false;}

        bool selected;
        bool moving;
        QVector<int> Section_Selected;

protected:

        void mouseReleaseEvent(QMouseEvent* event);
        void mousePressEvent(QMouseEvent *event);
        void mouseMoveEvent(QMouseEvent *event);
        void paintSection(QPainter * painter, const QRect & rect, int logicalIndex) const;

signals:
        void sSectionsSelect(QVector<int>);
};

//-----------------------------------------------------------------------------
class SampleEditorDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    SampleEditorDelegate(QObject *parent=0) : QStyledItemDelegate(parent) {}

    QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    //void paint (QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    bool enable_editor;
};
//-----------------------------------------------------------------------------
class PropertyDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    PropertyDelegate(QObject *parent=0) : QStyledItemDelegate(parent) {}

    QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void paint (QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

};
//-----------------------------------------------------------------------------
class PlateItemDelegate: public QStyledItemDelegate
{
    Q_OBJECT

public:
    PlateItemDelegate()
    {
        view = v_numeration;
    }

    rt_Protocol *p_prot;
    Plate_View view;
    QString style;
    QRect *A1;


    void get_prot(rt_Protocol *p) {p_prot = p;}
    void paint (QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

};

//-----------------------------------------------------------------------------
class Template_FileDialog: public QFileDialog
{
    Q_OBJECT


public:
    QCheckBox *template_protocol;

    Template_FileDialog(QWidget* parent = 0)
            : QFileDialog(parent)
    {
        setOptions(QFileDialog::DontUseNativeDialog);
        template_protocol = new QCheckBox("template", this);
        QGridLayout *grid = qobject_cast<QGridLayout*>(layout());
        grid->addWidget(template_protocol,grid->rowCount(),0);
    }
};

//-----------------------------------------------------------------------------
class PlateTableWidget : public QTableWidget
{
    Q_OBJECT

public:
QAbstractButton* corner_btn;

public:
        PlateTableWidget(int rows, int cols, QWidget* parent = 0)
                : QTableWidget(rows, cols, parent)
        {
            QAbstractButton* btn = findChild<QAbstractButton*>();
            if(btn)
            {
                corner_btn = btn;
                corner_btn->installEventFilter(this);
            }            
        }

        bool ability_select;
        bool selected;
        bool moving;
        QPoint point_0, point_1;
        QPoint coord_0, coord_1;
        QRect A1;

        bool enable_editor;
        PlateItemDelegate *delegate;        
        rt_Protocol *p_prot;
        QButtonGroup    *Color_Group;

signals:
        void ReLoadColor(QMap<int,int>);

protected:
        void mousePressEvent(QMouseEvent *event);
        void mouseReleaseEvent(QMouseEvent *event);
        void mouseMoveEvent(QMouseEvent *event);
        void paintEvent(QPaintEvent *e);
        void resizeEvent(QResizeEvent *e);

        virtual bool eventFilter(QObject* o, QEvent* e);

};
//-----------------------------------------------------------------------------
class SamplesItemDelegate: public QStyledItemDelegate
{
    Q_OBJECT

public:

    void paint (QPainter *painter,
                const QStyleOptionViewItem &option,
                const QModelIndex &index) const Q_DECL_OVERRIDE;


    QWidget* createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const Q_DECL_OVERRIDE;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;


    int current_row;
    QStringList *list_ID;
    QStringList *list_NAME;
    QString *nameTest;

    bool *open_sample;
    bool enable_editor;
    bool disable_Standarts;


    QString style;

public slots:
    void Close_TypeEditor();
    void Close_CopiesEditor();
    void Close_LineEditor();

signals:
    void change_Type();
    void change_Copies();
    void change_NameSample();

};
//-----------------------------------------------------------------------------
class SampleTableWidget : public QTableWidget
{
    Q_OBJECT

public:
QAbstractButton* corner_btn;
bool open_samples;

public:
        SampleTableWidget(int rows, int cols, QWidget* parent = 0)
                : QTableWidget(rows, cols, parent)
        {
            QAbstractButton* btn = findChild<QAbstractButton*>();
            if(btn)
            {
                corner_btn = btn;
                corner_btn->installEventFilter(this);
            }

            open_samples = true;
        }

protected:

        virtual bool eventFilter(QObject* o, QEvent* e);

};

//-----------------------------------------------------------------------------
class SIMPLE_TEST : public rt_Test
{

public:
    SIMPLE_TEST()
    {
        QString fluor_name[COUNT_CH] = FLUOR_NAME;

        //...
        QString text, str;
        QStringList list;
        bool ok;
        int i,j;
        rt_Preference *preference;
        QString dir_path = qApp->applicationDirPath();
        QSettings *CommonSettings = new QSettings(dir_path + "/tools/ini/preference.ini", QSettings::IniFormat);

        CommonSettings->beginGroup("Simple_Test");

        text = CommonSettings->value("active_channels","").toString();
        if(text.isEmpty()) header.Active_channel = 0x00011;
        else header.Active_channel = text.toInt(&ok, 16);
        if(!ok) header.Active_channel = 0x00001;

        text = CommonSettings->value("volumeTube","25").toString();
        header.Volume_Tube = text.toInt(&ok);
        if(!ok) header.Volume_Tube = 25;

        text = CommonSettings->value("comments","").toString();
        header.comment = text.toStdString();

        header.program.clear();
        text = CommonSettings->value("program","").toString();
        if(!text.isEmpty())
        {
            list = text.split('#');
            for(i=0; i<list.size(); i++)
            {
                text = list.at(i);
                header.program.push_back(text.toStdString());
            }
        }

        list.clear();
        list << CRITERION_POSRES << CRITERION_VALIDITY << USE_AFF << CRITERION_AFF << CRITERION_AFF_MIN << CRITERION_AFF_MAX << SIGMOIDVAL_MIN <<\
                SIGMOIDVAL_MAX << USE_THRESHOLD << VALUE_THRESHOLD << EXPOSURE << FLUOROFORS << MC_FWHM_BORDER << MC_PEAKS_BORDER;
        for(i=0; i<list.size(); i++)
        {
            str = list.at(i);
            preference = new rt_Preference;
            switch(i)
            {
            case 0: text = CommonSettings->value("CriterionPositiveResult","80").toString();    break;
            case 1: text = CommonSettings->value("CriterionValidity","5").toString();           break;
            case 2: text = CommonSettings->value("Use_AFF","yes").toString();                   break;
            case 3: text = CommonSettings->value("CriterionAFF","1000").toString();             break;
            case 4: text = CommonSettings->value("CriterionAFF_MIN","10").toString();           break;
            case 5: text = CommonSettings->value("CriterionAFF_MAX","90").toString();           break;
            case 6: text = CommonSettings->value("BordersSigmoidValidityMin","7").toString();   break;
            case 7: text = CommonSettings->value("BordersSigmoidValidityMax","20").toString();  break;
            case 8: text = CommonSettings->value("Use_Threshold","no").toString();  break;
            case 9: text = CommonSettings->value("Value_Threshold","10").toString();  break;
            case 10: text = CommonSettings->value("Exposure","1 1 1 1 1").toString();            break;
            case 11: text = CommonSettings->value("Fluorofors","Fam Hex Rox Cy5 cy5.5").toString();            break;
            case 12: text = CommonSettings->value("FWHM","3").toString();                        break;
            case 13: text = CommonSettings->value("peaks_Border","20").toString();               break;
            default:    text = ""; break;
            }
            preference->name = str.toStdString();
            preference->value = text.toStdString();
            preference_Test.push_back(preference);
        }



        CommonSettings->endGroup();
        delete CommonSettings;
        //...

        //header.Active_channel = 0x01011;
        header.Name_Test = "simple";
        header.Type_analysis = 0;
        header.ID_Test = "-1";

        rt_TubeTest *tube = new rt_TubeTest();
        tubes.push_back(tube);
        for(int i=0; i<COUNT_CH; i++)
        {
            if(!(header.Active_channel & (1<<i*4))) continue;
            rt_ChannelTest *ch = new rt_ChannelTest();
            tube->channels.push_back(ch);
            ch->ID_Channel = i;
            ch->name = fluor_name[i].toStdString();
            ch->number = i;
        }
    }

};

//-----------------------------------------------------------------------------
class Add_PushButton: public QPushButton
{
    Q_OBJECT

public:

    Add_PushButton(QIcon icon, QString label = "", QWidget* parent = 0)
            : QPushButton(icon, label, parent) {;}

    bool user_Tests;
protected:

    virtual void paintEvent(QPaintEvent* e);
};

//-----------------------------------------------------------------------------
class Color_ToolButton: public QToolButton
{
    Q_OBJECT

public:

    Color_ToolButton(QColor col, QWidget* parent = 0)
            : QToolButton(parent)   {color = col;}

    QColor color;

protected:

    //virtual void paintEvent(QPaintEvent* e);
    void mouseDoubleClickEvent(QMouseEvent* e);
    void mouseReleaseEvent(QMouseEvent* e);

};

//-----------------------------------------------------------------------------
class SETUPSHARED_EXPORT Setup : public QMainWindow
{
    Q_OBJECT

public:
    Setup(QWidget *parent = 0);
    ~Setup();

    QToolBar *fileToolBar;              // ToolBars:
    //QToolBar *fileToolBar_pro;              // ToolBars:
    QToolBar *programToolBar;           // ...
    QToolBar *sampleToolBar;            // ...
    QToolBar *plateToolBar;             // ...
    QSettings *ApplSettings;            // Settings
    QMessageBox message;                // Message
    QTemporaryDir dir_temp;             // Temporary Dir
    QTemporaryDir dir_UNDO;             // Temporary Dir for UNDO

    QMessageBox msgbox_ToAnalysis;
    QCheckBox   *cb_ToAnalysis;
    bool        ToAnalysis;

    int free_currentTube;
    int current_UNDO;
    int count_UNDO;
    bool flag_EditProtocol;
    bool flag_EditTest;
    bool flag_CommonSettings;
    bool flag_CopyBlockTests;
    bool flag_ReOpenProtocol;
    bool flag_SaveLocationWin;

    TEMPLATE_NAME template_NAME;
    QString StyleApp;

    QMovie  *obj_gif;
    QLabel  *label_gif;

    rt_Protocol *prot;                  // Protocol
    //QVector<rt_Tube*> Tubes;          // Tubes in Protocol
    QMap<int,int> Tubes_kind;           // Tubes kind: sample(0),K+(1),K-(2),St(3)
    QMap<int,rt_Tube*> Tubes_key;       // Tubes on Sample table
    QMap<int,rt_Tube*> Tubes_plate;     // Tubes on Plate table
    SIMPLE_TEST *Simple_Test;           // Simple Test
    QVector<rt_Test*> TESTs;            // List of Operators Tests
    QVector<rt_Test*> Pro_TESTs;        // List of Protocols Tests
    QString Operator;                   // Name of Operator
    QMap<int,QString> Map_Research;     // map: ID_research, Name_research
    QMap<QString,QString> Map_TestTranslate;  //
    QMap<int,QString> Map_Settings; //


    HINSTANCE dll_editor;
    HINSTANCE ext_dll_handle;           // DTReport
    GetSampleSettings get_Settings;
    GetDTRDisplayInfo get_Info;
    Init initialize;
    SetFont set_font;
    SetUserFolder set_folder;
    TranslateService translate_service;
    int ID_LANG;                        // lanquage for DTReport2.dll

    void change_Selection();            //
    void create_NamesResearch();        // Names research: my plugins(dll) + external mudules (dtr)

private:
    QGroupBox   *MainGroupBox;          // Main Box
    QMainWindow *MainListWindow;
    QMainWindow *MainControlWindow;
    QGroupBox   *ListSampleBox;         // ListSample Box
    QGroupBox   *ControlBox;            // Info Box
    QSplitter   *main_spl;
    QSplitter   *program_plate_spl;
    QPushButton *LoadToRun;

    Add_TESTs   *add_tests;             // Dialog: Add_Tests
    Test_editor *test_editor;           // Dialog: Test_Editor
    Common_Pro  *Pro_editor;            // Dialog: Pro_Editor
    Copy_BlockTests *copy_block;        // Dialog: Copy_BlockTests

    QLineEdit   *Name_Protocol;
    QLabel      *Label_Protocol;

    QTranslator translator;
    QTranslator qt_translator;

    QAxObject*  axgp;
    QAxObject*  ax_user;
    QAxObject*  ax_rita;
    QDir user_Dir;
    QDir templates_Dir;

    //... Program ...
    QMainWindow         *MainProgramWindow;
    QGroupBox           *ProgramBox;    
    Plot_ProgramAmpl    *plot_Program;
    Scheme_ProgramAmpl  *scheme_Program;
    QSplitter           *program_spl;
    QLabel              *Label_Program;
    QLabel              *Name_Program;


    //... List of Samples ...
    QGroupBox           *InfoBox;
    QTabWidget          *TabSamples;
    SampleTableWidget   *Samples_Table;
    SamplesItemDelegate *Samples_Delegate;
    QGroupBox           *TestSampleBox;
    QSplitter           *info_sample_spl;
    QSplitter           *test_sample_spl;
    QTreeWidget         *Test_Tree;
    QTreeWidget         *Sample_Tree;
    SampleEditorDelegate *Samples_EditorDelegate;
    QGroupBox           *InformationBox;    
    Info_Protocol       *Info_Tree;
    QStringList         list_ID;
    QStringList         list_NAME;    
    QTreeWidget         *Sample_Properties;
    PropertyDelegate    *property_Delegate;

    QLabel              *LabelTests_ADDSample;
    Add_PushButton      *ADD_Sample;
    QPushButton         *ADD_Tube;
    QPushButton         *DELETE_Sample;
    QPushButton         *DELETE_AllSamples;
    QPushButton         *UNDO_Action;

    QIcon logo;
    QIcon logonull;

    //... Plate ...
    QMainWindow         *MainPlateWindow;
    QGroupBox           *PlateBox;
    QComboBox           *type_plate;
    QWidget             *spacer;
    QWidget             *spacer_fix;
    PlateTableWidget    *Type_Plate;
    PlateItemDelegate   *Plate_Delegate;
    MyHeader            *header_hor;
    MyHeader            *header_ver;
    QLabel              *label_Plate;
    QVector<int>        User_Position;
    QCursor             cursor_FREE;
    ZoomPlate           *zoom;

    //... Actions ...
    QAction *open_DBProtocol;
    QAction *open_Protocol;
    QAction *last_Protocols;
    QAction *save_Protocol;
    QAction *clear_Protocol;
    QAction *edit_Test;
    QAction *edit_PreferenceProtocol;
    QAction *load_ToRun;
    QAction *load_ToAnalysis;
    QAction *load_ToPipet;
    QAction *copy_BlockTests;
    QAction *open_XML;

    QAction *open_Program;
    QAction *last_Programs;
    QAction *edit_Program;

    QAction *add_Sample;
    QAction *add_Tube;
    QAction *delete_Sample;
    QAction *delete_Tube;
    QAction *delete_AllSamples;
    QAction *Undo;

    QAction *user_Filling;
    QAction *auto_Filling;
    QAction *free_Filling;
    QAction *order_Filling;
    QAction *clear_Plate;
    QAction *type_View;
    QAction *zoom_Plate;

    QButtonGroup        *Color_Group;
    QGroupBox           *Color_Box;
    QCursor             cursor_ColorFill;

    Color_ToolButton    *first_ColorButton;
    Color_ToolButton    *second_ColorButton;
    Color_ToolButton    *third_ColorButton;
    Color_ToolButton    *fourth_ColorButton;
    Color_ToolButton    *fifth_ColorButton;
    Color_ToolButton    *sixth_ColorButton;
    Color_ToolButton    *seventh_ColorButton;

    QAction *paste_from_clipboard;
    QAction *increment_digits;
    QAction *send_protocol_to_web;
    QAction *look_TIS_Tests;
    bool check_ValidProtocol();

    QAction *invert_Row;
    QAction *invert_Column;
    QAction *invert_Plate;
    QAction *tests_layout;

    //... methods ...
    void createActions();               // Actions
    void createToolBars();              // ToolBar
    void create_SampleProgramPlate();   // Sample & Program & Plate
    void create_SampleBlock();          // Table of Samples
    void create_ProgramBlock();         // Program
    void create_PlateBlock();           // Plate

    void fill_SampleTable();            // Load Sample Table
    void fill_TestSample();             // Load TreeWidget Test&Sample
    void fill_Plate();                  // Load Plate
    void fill_Information();            // Load Information about Protocol
    void fill_SampleProperties();       // Load Information about Sample
    void change_Position(Plate_Action); //
    void set_NextTube(int);             //
    void set_FirstTube();               //
    void set_PreviousTube();

    int  Tube_CodeChannels(rt_Tube*);   // code (bit for channel) for active channels in tube

    void Clear_AfterDelete();           // clear tubes on the Plate after deleting tubes in the Sample
    void Clear_UNDO();

    void Load_CatalogProperties(rt_Test*);      // Check&Load Catalog Property
    void Load_DropperTableTests();
    void readCommonSettings();          // read Common settings
    void readTemplateSettings();        // read TemplateName Settings
    void readSettings();                // readsettings
    void writeSettings();               // Write settings
    void addProtocol_toList(QString fn);// add opening Protocol to list
    void addProgram_toList(QString fn); // add Program to list

    QString Check_ValidNameFile(QString);
    QString Generate_NameProtocol(int); // generate name of protocol
    void Generate_IDProtocol(rt_Protocol *); // generate id protocol

    void Check_TestsExposition(rt_Protocol *);  // check tests in protocol on equal exposition and fluors
    void Check_TestsPtrogramm(rt_Protocol *);   // check tests in protocol on equal programm
    void Check_TestsVolumeTube(rt_Protocol *);  // check tests in protocol on equal volume tube
    void Check_ProtocolProgramm_TestsProgramm(rt_Protocol *); // check protocol programm with tests programm

    void Create_GroupSamplesProperties(rt_GroupSamples *);
    QString GroupSettings_XML2String(QString);

public:

    bool create_TestDBase(QAxObject*);          // create axRita - tests dbase resource
    bool create_TestVerification(QAxObject*);   // create axgp - verification resource
    bool create_User(QAxObject*);               // create ax_User
    bool create_ListTESTs(QString);             // create list of Operators TESTs
    void Translate_Tests(QStringList&);         // translate names and research
    void Translate_Catalog(QVector<rt_Test*>*); // translate catalog from russian
    void Create_TranslateList(rt_Test*, QStringList&);
    void CreateCopy_Test(rt_Test*, rt_Test*);   // create copy test

    void Create_MenuActions(void*);             //
    void execution_MenuAction(QString);         //
    void Enable_MenuAction(void*);              //
    void* Get_pProtocol();

private slots:

    void open_DBprotocol();
    QString open_protocol(QString dir = "", QString fn = "", bool temporary_file = false);
    void Load_Protocol();
    void open_XMLprotocol(QString fn = "");
    void last_protocols();
    void save_protocol();
    void clear_protocol(bool temporary_file = false);
    void edit_tests();
    void edit_preferencePro();
    void copy_block_tests();
    void load_torun();
    void load_toanalysis();
    void change_ToAnalysis(int);
    void load_topipet();

    void open_program(QString fn = "");
    void last_programs();
    void edit_program();
    void edit_program_separate();

    void cellSample_Selected();
    void cellSample_Changed(int,int);
    void add_sample();
    void add_tube();
    void delete_sample();
    void delete_all_samples();
    void delete_tube();
    void undo();
    void save_undo();
    void add_test(rt_Test*,int,int,int,int,int,int);
    void add_ForSample(QString, QVector<rt_Test*>*, int count_sample, int kind);

    void zoom_plate();
    void change_user();
    void change_auto();
    void change_free();
    void changED_free();
    void change_order();
    void clear_plate();
    void change_view();
    void change_plate(int, bool without_saveUndo = false);
    void cellSelected(int,int);    
    void columnSelected(int);
    void rowSelected(int);
    void allSelected();

    void ColumnsSelect(QVector<int>);
    void RowsSelect(QVector<int>);

    void columnMoved(int);
    void rowMoved(int);

    void change_NameProtocol(QString);
    void contextMenu_SamplesTable();
    void contextMenu_Plate(QPoint);
    void contextMenu_Test_Tree();    
    void from_ClipBoard();
    void Increment_Digits();
    void send_Protocol_To_Web();
    void look_TISTests();

    void ChangeColor(QMap<int,int>);
    void ChangeTypeSample();
    void ChangeCopiesSample();
    void ChangeNameSample();

    void SampleItemChange(QTreeWidgetItem*,int);
    void OpenCloseSamples();
    void PropertyItemChange(QTreeWidgetItem*,int);

    void resize_splitter(int,int);

    void change_StatusEditProtocol(bool);
    void close_Label();
    void open_Label(QString);

    void Color_Button(int);

    void InvertRow(int);
    void InvertColumn(int);
    void InvertPlate();
    void Tests_Layout();



protected:

    bool event(QEvent *e);
    void resizeEvent(QResizeEvent *e);
    void showEvent(QShowEvent *e);
    //void mouseDoubleClickEvent(QMouseEvent *e);

signals:
    void Operation_Complited();
};

#endif // SETUP_H
