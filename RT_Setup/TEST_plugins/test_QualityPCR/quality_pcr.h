#ifndef QUANTITY_PCR_H
#define QUANTITY_PCR_H

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

#include <QDebug>

#include "quality_pcr_global.h"
#include "test_editor.h"
#include "test_interface.h"
#include "protocol_information.h"
#include "utility.h"
#include "define_PCR.h"

#include "common.h"
#include "header_test.h"

#define  Method "Quality_PCR"

typedef void (__stdcall *Interface_Editor)(std::string, bool&, std::string&);

//-----------------------------------------------------------------------------
class QualityItemDelegate: public QStyledItemDelegate
{
    Q_OBJECT

public:

    QualityItemDelegate(QObject *parent=0) : QStyledItemDelegate(parent)
    {
        connect(this, SIGNAL(showComboPopup(QComboBox *)), this, SLOT(openComboPopup(QComboBox *)), Qt::QueuedConnection);
    }

    void paint (QPainter *painter,
                const QStyleOptionViewItem &option,
                const QModelIndex &index) const Q_DECL_OVERRIDE;

    QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;

signals:
   void showComboPopup(QComboBox *) const;

public slots:
    void Close_Editor(int);
    void openComboPopup(QComboBox *);

};

//-----------------------------------------------------------------------------
class QUALITY_PCRSHARED_EXPORT Quality_PCR: public QObject, public Test_Interface
{
    Q_OBJECT

public:
    Quality_PCR();

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

    QMainWindow *program_group;
    QSplitter   *program_spl;       // Program
    Plot_ProgramAmpl    *plot_Program;      //...
    Scheme_ProgramAmpl  *scheme_Program;    //...
    QToolBar    *program_tool;

    Header_Test     *header_Test;   // Header
    Common          *common_Tree;   // Common

    QTableWidget    *Table_Quality; // Table
    QualityItemDelegate *quality_Delegate;

    QAction     *open_program;
    QAction     *edit_program;

    QString     User_Folder;

    void create_HeaderTest(QWidget*);
    void create_Program(QWidget*);
    void create_Common(QWidget*);
    void create_TableQuality(QWidget*);
    void fill_Test(rt_Test*);

private slots:

    void Open_Program(QString fn = "");
    void Edit_Program();

    void Change_TabIndex(int);
    void Get_ActiveChannels(int act_ch, QMap<int,QString> *map = NULL);
};


#endif // QUALITY_PCR_H
