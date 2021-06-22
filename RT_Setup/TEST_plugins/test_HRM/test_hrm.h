#ifndef TEST_HRM_H
#define TEST_HRM_H

#include <QtCore/QObject>
#include <QApplication>
#include <QWindow>
#include <QTabWidget>
#include <QTableWidget>
#include <QGroupBox>
#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>
#include <QPainter>
#include <QHeaderView>
#include <QDebug>
#include <QMessageBox>
#include <QPoint>
#include <QAction>
#include <QMenu>
#include <QIcon>
#include <QEvent>
#include <QFont>
#include <QFontMetrics>
#include <QSplitter>
#include <QBoxLayout>
#include <QTranslator>
#include <QSettings>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QSplitter>
#include <QMainWindow>
#include <QToolBar>
#include <QFileDialog>
#include <QTime>
#include <QDoubleValidator>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QResizeEvent>

#include <QDebug>

#include "test_hrm_global.h"
#include "test_editor.h"
#include "test_interface.h"
#include "protocol_information.h"
#include "utility.h"
#include "define_PCR.h"

#include "common.h"
#include "header_test.h"

//#define Method "Base\\HRM"

typedef void (__stdcall *Interface_Editor)(std::string, bool&, std::string&);

//-----------------------------------------------------------------------------
class TEST_HRMSHARED_EXPORT Test_HRM: public QObject, public Test_Interface
{
    Q_OBJECT

public:
    Test_HRM();

    void *Create_Win(void *, void *);
    void Destroy_Win();
    void Show();
    void Destroy();

    void Disable_Action();
    void Reserve(void*);

    //void GetInformation(std::vector<std::string> *info);
    void GetInformation(QVector<QString> *info);
    void Set_Test(std::string,std::string);
    void Get_Test(std::string*,std::string*);

    void readCommonSettings();
    QTranslator translator;
    QString StyleApp;


private:

    QWidget *parent_widget;
    QVector<string> Level_Program;

    QTabWidget  *TabGrid;
    int active_ch;

    QGroupBox   *group_Others;          //Others
    QSpinBox    *quality_percent;       //
    QLabel      *label_quality;         //
    QCheckBox   *use_CpValidity;        //
    QDoubleSpinBox *CpValidity;          //


    QMainWindow *program_group;
    QSplitter   *program_spl;       // Program
    Plot_ProgramAmpl    *plot_Program;      //...
    Scheme_ProgramAmpl  *scheme_Program;    //...
    QToolBar    *program_tool;

    Header_Test *header_Test;   // Header
    Common *common_Tree;        // Common

    QAction     *open_program;
    QAction     *edit_program;

    QString     User_Folder;


    void create_HeaderTest(QWidget*);
    void create_Program(QWidget*);
    void create_Common(QWidget*);
    void create_Others(QWidget*);
    void fill_Test(rt_Test*);
    void Load_Properties(QDomNode &node);

private slots:

    void Open_Program(QString fn = "");
    void Edit_Program();
    void Change_UseCpValidity(bool);
    void Change_TabIndex(int);


};

#endif // TEST_HRM_H
