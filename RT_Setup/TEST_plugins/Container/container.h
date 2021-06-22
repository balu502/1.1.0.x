#ifndef CONTAINER_H
#define CONTAINER_H

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
#include <QRegExpValidator>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QValidator>
#include <QDoubleValidator>
#include <QListWidget>
#include <QListWidgetItem>

#include <QDebug>

#include "container_global.h"
#include "test_editor.h"
#include "test_interface.h"
#include "protocol_information.h"
#include "utility.h"
#include "define_PCR.h"

#include "common.h"
#include "header_test.h"

#define  Method "Container"
//#define  CONTAINER_TYPE 0x0033


typedef void (__stdcall *Interface_Editor)(std::string, bool&, std::string&);


//-----------------------------------------------------------------------------
class ItemListDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    ItemListDelegate(QObject *parent=0) : QStyledItemDelegate(parent) {}

    void paint (QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    QMap<QString,QString> *map_tests;

};

//-----------------------------------------------------------------------------
class CONTAINERSHARED_EXPORT Container: public QObject, public Test_Interface
{
    Q_OBJECT

public:
    Container();

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

    void Load_listTests();
    QVector<rt_Test*>   *TESTs;
    QVector<QString>   checked_Tests;   // List of ID test
    QMap<QString,QString> map_Tests;
    QMap<QString,rt_Test*> map_TESTs;

    QString Name_Container;
    QString Container_ID;

    QIcon logo;
    QIcon logonull;

private:

    QWidget *parent_widget;
    QVector<string> Level_Program;
    int active_ch;

    QTabWidget  *TabGrid;
    QSplitter   *main_Spl;
    QTreeWidget *List_Tests;
    Scheme_ProgramAmpl  *scheme_Container;
    QListWidget *List_Container;
    ItemListDelegate *Item_Delegate;
    QSplitter   *list_Spl;

    QMainWindow *program_group;
    QSplitter   *program_spl;       // Program
    Plot_ProgramAmpl    *plot_Program;      //...
    Scheme_ProgramAmpl  *scheme_Program;    //...
    QToolBar    *program_tool;

    Header_Test *header_Test;   // Header
    Common *common_Tree;        // Common

    QAction     *open_program;
    QAction     *edit_program;

    void create_HeaderTest(QWidget*);
    void create_Program(QWidget*);
    void create_Common(QWidget*);

    void fill_Test(rt_Test*);

private slots:

    void ReDraw_Program();
    void CheckedTests(QVector<QString>&);
    void Open_Program(QString fn = "");
    void Edit_Program();
    void ChangedItem(QTreeWidgetItem*,int);

    //void Change_TabIndex(int);
    //void Get_ActiveChannels(int act_ch, QMap<int,QString> *map = NULL);
};


#endif // CONTAINER_H
