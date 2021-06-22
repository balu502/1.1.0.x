#ifndef ANALYSER_QUALITY_H
#define ANALYSER_QUALITY_H

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

#include <QDomDocument>

#include <QDebug>

#include "analyser_quality_global.h"
#include "alg_interface_qevent.h"
#include "analysis_interface.h"
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

};
//-----------------------------------------------------------------------------
class sample_Quality
{
public:
    sample_Quality() {}
    ~sample_Quality()
    {
        Cp.clear();
        Type.clear();
    }

    QString unique_name;
    QVector<double> Cp;
    QVector<short> Type;    // 0 -specific  1 - IC
    QString result;

    int pos;

    rt_Test *ptest;
};

//-----------------------------------------------------------------------------

class Quality_TableWidget: public QTableWidget
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

class ANALYSER_QUALITYSHARED_EXPORT Analyser_Quality: public QObject, public Analysis_Interface
{
    Q_OBJECT

public:
    Analyser_Quality();

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
    Quality_TableWidget    *Results_Table;
    ResultsItemDelegate *results_Delegate;

    QVector<sample_Quality*> Samples_Quality;
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
};

#endif // ANALYSER_QUALITY_H
