#ifndef ANALYSER_QUANTITY_H
#define ANALYSER_QUANTITY_H

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
#include <QTextCodec>

#include <QDomDocument>

#include <QDebug>

#include "qwt_plot.h"
#include "qwt_plot_curve.h"
#include "qwt_symbol.h"
#include "qwt_plot_grid.h"
#include "qwt_plot_zoomer.h"
#include "qwt_picker.h"
#include "qwt_plot_canvas.h"
#include "qwt_curve_fitter.h"
#include "qwt_plot_directpainter.h"
#include "qwt_legend.h"

#include "analyser_quantity_global.h"
#include "alg_interface_qevent.h"
#include "analysis_interface.h"
#include "utility.h"
#include "define_PCR.h"

#include <algorithm>
#include <gsl/gsl_fit.h>

#define BG_COLOR "{0xD8E2F1,0xD8E9D8,0xFEF0CB,0xFFE6FF,0xFFE6FF}"

#define ID_METHOD "0x0001"
#define NAME_METHOD "Quantity_PCR"

class sample_Quantity;

//-----------------------------------------------------------------------------
class Fluor_ItemDelegate: public QStyledItemDelegate
{
    Q_OBJECT

public:

    void paint (QPainter *painter,
                const QStyleOptionViewItem &option,
                const QModelIndex &index) const Q_DECL_OVERRIDE;

    QString style;
};
//-----------------------------------------------------------------------------
class Fluor_ComboBox: public QComboBox
{
    Q_OBJECT

protected:

    virtual void paintEvent(QPaintEvent* e)
    {
        QComboBox::paintEvent(e);
        if(currentIndex() < 0) return;
        QPainter p(this);
        p.setFont(qApp->font());
    }
};
//-----------------------------------------------------------------------------
class StdCurvePlot: public QwtPlot
{
    Q_OBJECT

public:
    StdCurvePlot(QWidget *parent = NULL);
    virtual ~StdCurvePlot();

    QwtPlotCurve    *LinFit_curve;
    QwtPlotCurve    *Standarts_curve;
    QwtPlotCurve    *Samples_curve;
    QwtLegend       *legend;

    QLabel          *eff_Label;
    QLabel          *sigma_Label;
    QLabel          *r2_Label;

    QwtPlotGrid *grid;
    QLabel *label_caption;
    QVector<sample_Quantity*> vec_St;
    QVector<sample_Quantity*> vec_Sample;
    rt_Protocol     *Prot;

    QLabel *Sample_Info;
    void select_GraphMarker(const QPoint &);

public slots:
    void Hide_LabelInfo(){Sample_Info->setVisible(false);}

protected:
    virtual bool eventFilter(QObject *, QEvent *);

};

//-----------------------------------------------------------------------------
class AddStdCurvePlot: public QwtPlot
{
    Q_OBJECT

public:
    AddStdCurvePlot(QWidget *parent = NULL);
    virtual ~AddStdCurvePlot();

    QwtPlotCurve            *curve;
    QVector<QwtPlotCurve*>  list_Curve;

    QwtPlotGrid     *grid;
};

//-----------------------------------------------------------------------------
class StdCurve_Result
{
public:
    StdCurve_Result()
    {
        c0 = 0.;
        c1 = 0.;
        Eff = 0.;
        R_2 = 0.;
        Sigma = 0.;
    }

    QString id_Test;
    int id_channel;
    double c0,c1;           // Y = c0 + c1*X
    double Eff,R_2,Sigma;
};

//-----------------------------------------------------------------------------
enum SAMPLE_TYPE{mSample, mStandart, mAddStandart};

class sample_Quantity
{
public:
    sample_Quantity()
    {
        type = mSample;
        active = true;
    }
    ~sample_Quantity()
    {
        Cp.clear();        
        conc.clear();
        conc_calc.clear();
        Analysis_Curves.clear();
    }

    QString unique_name;
    QVector<double> Cp;    
    QVector<double> conc;
    QVector<double> conc_calc;

    QVector<QString> Analysis_Curves;
    int count_measure;

    SAMPLE_TYPE type;
    int pos;
    bool active;

    rt_Test *ptest;
    rt_Sample *sample_Plate;
};
//-----------------------------------------------------------------------------

class StandartsItemDelegate: public QStyledItemDelegate
{
    Q_OBJECT

public:

    void paint (QPainter *painter,
                const QStyleOptionViewItem &option,
                const QModelIndex &index) const Q_DECL_OVERRIDE;

    QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;
    //void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
    //void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    //QStringList *list;
    //int *active_ch;
};
//-----------------------------------------------------------------------------

class ResultsItemDelegate: public QStyledItemDelegate
{
    Q_OBJECT

public:

    void paint (QPainter *painter,
                const QStyleOptionViewItem &option,
                const QModelIndex &index) const Q_DECL_OVERRIDE;

    QVector<double> *max_Conc;
    QVector<QColor> *color_Ch;
    QVector<short> *background;
};

//-----------------------------------------------------------------------------

class FileName_Label: public QLabel
{
    Q_OBJECT

public:
    FileName_Label(QWidget *parent = 0)
        : QLabel(parent) {}

signals:
    void Resize(void);

protected:
    virtual void resizeEvent(QResizeEvent *e);
};

//-----------------------------------------------------------------------------

class Quantity_TableWidget: public QTableWidget
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
class ANALYSER_QUANTITYSHARED_EXPORT Analyser_Quantity: public QObject, public Analysis_Interface
{
    Q_OBJECT

public:
    Analyser_Quantity();

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

    bool Calc_StValue;
    QPushButton *calc_St;

private:
    rt_Protocol     *Prot;

    QGroupBox       *MainBox;
    QSplitter       *main_spl;
    QSplitter       *control_spl;
    QTabWidget      *Tab_Result;
    Quantity_TableWidget    *Results_Table;
    ResultsItemDelegate *results_Delegate;
    QGroupBox       *Control_Box;
    QGroupBox       *Std_Curve;
    QTabWidget      *Tab_Control;
    Fluor_ComboBox  *Fluor;
    Fluor_ItemDelegate *fluor_delegate;
    QTableWidget    *Standart_Table;
    StandartsItemDelegate *standarts_Delegate;
    QComboBox       *Tests_Box;
    QLabel          *Tests_Label;
    QGroupBox       *Group_AdditionalSt;
    FileName_Label  *FilePath_Label;
    QPushButton     *Open_addProtocol;
    QLabel          *StNotDetected_Label;

    AddStdCurvePlot *AddStdCurve_plot;
    StdCurvePlot    *StdCurve_plot;
    QAction         *save_as_PNG;
    QAction         *copy_to_clipboard;
    QAction         *copy_to_excelfile;
    QAction         *copy_to_clipboard_on_column;


    QVector<sample_Quantity*> Samples_Quantity;                 // all list samples with type analysis == 0x0001
    QMap<QString,QVector<sample_Quantity*>*>    Map_Samples;    // map samples for tests
    QMap<QString,QString>   Map_NamesTests;                     // map Names-Tests    
    QMap<QString,QVector<StdCurve_Result*>*> Map_TestsResults;  // map results Std_Curve for every test

    QMap<QString,QVector<double>*> Max_Concentration;           // map max.Conc(for all channels) for every test
    QMap<QString,QVector<QColor>*> Color_Channels;              // map Color for Channels
    QString FileName_AdditionalSt;                              // file name of the protocol with add.standards
    rt_Test *current_Test;
    QVector<sample_Quantity*> Samples_AddStandards;             // list additional standards for current test

    QVector<short> Samples_Background;

    Select_Plot event_plot;
    QWidget *main_widget;


    // methods
    void Fill_StandartsTable(QString,int);
    void Draw_StdCurve(QString,int);
    void StdCurve_Calculate(QString,int,void*);
    double Linear_Fit(QVector<double> &X, QVector<double> &Y, double &c0, double &c1);
    void Fill_ResultsTable();
    void SaveResults_Quantity();
    void AddResult(vector<string>&, QString, QString);

    QString MinimizeFileName(QWidget*, QString);
    bool Load_AddStandards(QString fn, rt_Test*);
    void Draw_AddStdCurves();
    void Clear_AddStdCurves();

    void Select_Curve(int);

private slots:
    void Fill_StdCurveForTest(int);
    void Fill_StdCurveForFluor(int);
    void Changed_StandartsParameters(int,int);
    void contextMenu_Graph();
    void to_Image();
    void contextMenu_ResultsTable();
    void to_ClipBoard();
    void to_ClipBoard_ByColumn();
    void to_Excel();
    QString ResultsTable_ToDigits();
    QString ResultsTable_ToDigits_ByColumn();
    void Open_AdditionalProtocol();    
    void Resize_FilePath();
    void ResultsTable_ChangedCurrentCell(int,int,int,int);
    void Calculate_StandartValue();
};

#endif // ANALYSER_QUANTITY_H
