#ifndef ANALYSER_RELATIVE_H
#define ANALYSER_RELATIVE_H

#include <QtCore/QObject>
#include <QtCore/qmath.h>
#include <QObject>
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
#include <QMouseEvent>
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
#include <QClipboard>
#include <QFileDialog>
#include <QFile>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QMouseEvent>
#include <QTextCodec>
#include <QTableWidgetSelectionRange>

#include <QDomDocument>

#include <QDebug>

#include "analyser_relative_global.h"
#include "alg_interface_qevent.h"
#include "analysis_interface.h"
#include "algorithm.h"
#include "utility.h"
#include "define_PCR.h"

#include <algorithm>
#include <gsl/gsl_fit.h>


//-----------------------------------------------------------------------------

class ResultsItemDelegate: public QStyledItemDelegate
{
    Q_OBJECT

public:

    void paint (QPainter *painter,
                const QStyleOptionViewItem &option,
                const QModelIndex &index) const Q_DECL_OVERRIDE;

    QVector<short> *background;
    double max_value;
};
//-----------------------------------------------------------------------------
class sample_Relative
{
public:
    sample_Relative() {valid = true;}
    ~sample_Relative()
    {
        Cp.clear();
        Type.clear();
        DDCT.clear();        
        DCT_tar.clear();
    }

    QString unique_name;
    QVector<double> Cp;
    QVector<double> DDCT;    
    QVector<double> DCT_tar;
    QVector<short> Type;    // 0 - Target gen  1 - Reference gen
    int type_Sample;        // 0 - Target sample   1 - Control sample
    QString result;
    bool valid;

    int pos;

    rt_Test *ptest;
    rt_Sample *sample;
};

//-----------------------------------------------------------------------------

class Relative_TableWidget: public QTableWidget
{
    Q_OBJECT

public:

    virtual void mouseMoveEvent(QMouseEvent* e)
    {
        if(e->buttons() & Qt::LeftButton) e->ignore();
        else e->accept();
    }

};

//-----------------------------------------------------------------------------

class ANALYSER_RELATIVESHARED_EXPORT Analyser_Relative: public QObject, public Analysis_Interface
{
    Q_OBJECT

public:
    Analyser_Relative();

    void *Create_Win(void *, void *);
    void Destroy_Win();
    void Show();
    void Destroy();

    void GetInformation(QVector<QString> *info);
    void Analyser(rt_Protocol *prot);
    void Save_Results(char*);
    void Select_Tube(int pos);
    void Enable_Tube(QVector<short>*);

    void readCommonSettings();
    QTranslator translator;
    QString StyleApp;
    bool ru_Lang;

private:
    rt_Protocol     *Prot;
    QGroupBox       *MainBox;
    Relative_TableWidget    *Results_Table;
    ResultsItemDelegate *results_Delegate;

    QVector<sample_Relative*> Samples_Relative;
    QVector<short> Samples_Background;

    void Fill_ResultsTable();
    void Select_Curve(int);

    QAction         *copy_to_clipboard;
    QAction         *copy_to_excelfile;
    QAction         *copy_to_clipboard_on_column;

    Select_Plot event_plot;
    QWidget *main_widget;

private slots:
    void ResultsTable_ChangedCurrentCell(int,int,int,int);
    void contextMenu_ResultsTable();
    void to_ClipBoard();
    void to_ClipBoard_ByColumn();
    void to_Excel();
    QString ResultsTable_ToDigits();
    QString ResultsTable_ToDigits_ByColumn();
    void SelectNewCell();
};

#endif // ANALYSER_RELATIVE_H
