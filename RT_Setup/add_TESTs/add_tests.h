#ifndef ADD_TESTS_H
#define ADD_TESTS_H

#include "add_tests_global.h"

#include <QtWidgets>
#include <QWidget>
#include <QMainWindow>
#include <QDialog>
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
#include <QProgressBar>
#include <QLabel>
#include <QTreeWidget>
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
#include <QSpinBox>
#include <QPalette>
#include <QShowEvent>
#include <QTranslator>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMovie>
#include <QTreeWidget>
#include <QTreeWidgetItemIterator>
#include <QTreeWidgetItemIterator>
#include <QRadioButton>

#include <protocol.h>

class ADD_TESTSSHARED_EXPORT Add_TESTs: public QDialog
{
    Q_OBJECT

public:
    Add_TESTs(QWidget *parent = 0);
    virtual ~Add_TESTs();

    QPushButton     *add_test;
    QPushButton     *ok_button;
    QPushButton     *cancel_button;

    QTabWidget      *Tab_ADD;

    QGroupBox       *Box_samples;
    QLabel          *Label_SAMPLE;
    QLineEdit       *Edit_Sample;
    QTreeWidget     *Tree_Tests;
    QRadioButton    *sample_Kind;
    QRadioButton    *Kplus_Kind;
    QRadioButton    *Kminus_Kind;

    QGroupBox       *Box_containers;
    QLabel          *Label_CONTAINER;
    QLineEdit       *Edit_Container;
    QSpinBox        *Count_Container;
    QLabel          *Label_CONTAINER_plus;
    QTreeWidget     *Tree_Container;
    QRadioButton    *sample_container_Kind;
    QRadioButton    *Kplus_container_Kind;
    QRadioButton    *Kminus_container_Kind;

    QGroupBox       *Box_tests;
    QComboBox       *Combo_Tests;
    QLabel          *Label_Name;
    //QLineEdit       *Line_Type;
    QComboBox       *Combo_Type;
    QLabel          *Label_Type;
    QTextEdit       *Text_Comments;
    QLabel          *Label_Comments;
    QLabel          *Label_Samples;
    QLabel          *Label_countTubes;
    QSpinBox        *Count_Samples;
    QSpinBox        *Count_Double;
    QLabel          *Label_Double;
    QLabel          *Label_Kpos;
    QLabel          *Label_countKpos;
    QSpinBox        *Count_Kpos;
    QLabel          *Label_Kneg;
    QLabel          *Label_countKneg;
    QSpinBox        *Count_Kneg;
    QLabel          *Label_St;
    QLabel          *Label_countSt;
    QSpinBox        *Count_St;
    QSpinBox        *Count_DoubleSt;
    QLabel          *Label_DoubleSt;
    QListWidget     *List_Types;
    QLabel          *Label_ListTypes;

    QTreeWidget     *List_Tests;
    QLabel          *Label_List;

    QProgressBar    *main_progress;

    QVector<rt_Test*> ListTests_ForSample;
    QVector<rt_Test*> *TESTs;
    QVector<rt_Test*> current_ListTests;
    QMap<int,QString> *map_research;
    QMap<QString,QString> *map_TestTranslate;
    bool add_action;

    void readCommonSettings();
    QTranslator translator;
    QString style;

    QIcon logo;
    QIcon logonull;

    QMovie  *obj_gif;
    QLabel  *label_gif;

public slots:
    void AddClose_Test();
    void Add_Test();
    //void LoadTests_ToSamplesTab();
    //void LoadTest_Tree_
    void LoadTests_Tree(QTreeWidget*, bool);
    void LoadListForSample(QVector<rt_Test*>&);
    void Load_Containers();
    void LoadListForContainer(QVector<rt_Test*>&);
    void CheckState_AddButton();

    virtual void showEvent(QShowEvent * event);

signals:
    void signal_addTest(rt_Test*,int,int,int,int,int,int);      // rt_Test* = pointer of test; int = count of samples; int = doubles; int = K+; int = K-; int = St; int = St_copies
    void signal_addSample(QString, QVector<rt_Test*>*, int,int);         // QString = name of sample; QVector<rt_Test*> = list of tests; int = count of samples

private slots:
    void select_test();
    void change_test(int);
    void change_typeTest(int);
    void change_TabPage(int);
    void change_NameSample();
    void change_NameSample_forContainer();

    void Increment_Sample(QString);

};

#endif // ADD_TESTS_H
