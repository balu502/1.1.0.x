#ifndef REPORT_PARAMETERIZATION_H
#define REPORT_PARAMETERIZATION_H

#include "report_parameterization_global.h"

#include <QWindow>
#include <QApplication>
#include <QObject>
#include <QDialog>
#include <QtWidgets>
#include <QWidget>
#include <QTabWidget>
#include <QTableWidget>
#include <QTreeWidget>
#include <QBoxLayout>
#include <QSplitter>
#include <QDebug>
#include <QList>
#include <QSettings>
#include <QTranslator>
#include <QPrinter>
#include <QAction>
#include <QMainWindow>
#include <QDockWidget>
#include <QGroupBox>
#include <QComboBox>
#include <QCheckBox>
#include <QDate>
#include <QToolBar>
#include <QCheckBox>
#include <QRadioButton>
#include <QMessageBox>
#include <QFileDialog>

/*
#include <QtCharts>
#include <QtCharts/QLineSeries>
#include <QtCharts/QChartView>
QT_CHARTS_USE_NAMESPACE
*/
#include "qwt_plot.h"
#include "qwt_plot_curve.h"
#include "qwt_symbol.h"
#include "qwt_plot_grid.h"
#include "qwt_plot_zoomer.h"
#include "qwt_picker.h"
#include "qwt_plot_canvas.h"
#include "qwt_curve_fitter.h"
#include "qwt_plot_directpainter.h"
#include "qwt_scale_engine.h"

#include <ncreport.h>
#include <ncreportoutput.h>
#include <ncreportpreviewoutput.h>
#include <ncreportpreviewwindow.h>

#include "protocol.h"
#include "utility.h"
#include "define_PCR.h"


//-----------------------------------------------------------------------------

class TreeWidget_Chart: public QTreeWidget
{
    Q_OBJECT

public:
    TreeWidget_Chart(QWidget *parent = 0)
    {
    }
    //void mousePressEvent(QMouseEvent *event);
    //void Recheck_TreeItems();

private slots:
    //void click_item(QTreeWidgetItem* item, int col);
};

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
class ReportPreview : public NCReportPreviewWindow
{
    Q_OBJECT

public:
    ReportPreview(QWidget *parent = 0);
    ~ReportPreview();    
};
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
class REPORT_PARAMETERIZATIONSHARED_EXPORT Report_Parameterization: public QDialog
{
    Q_OBJECT

public:
    Report_Parameterization(rt_Protocol*, QString user_folder = "", QWidget *parent = 0);
    ~Report_Parameterization();

    rt_Protocol *prot;
    NCReport *report;
    QTranslator translator;
    QTranslator translator_report;
    void readCommonSettings();
    QString lang;
    QString User_Folder;
    QPushButton *ApplyChange;
    QProgressBar *main_progress;

    ReportPreview *pv;

    QToolBar *client_toolbar;
    QGroupBox *client_box;
    TreeWidget_Chart *charts_tree;
    QGroupBox *type_box;
    QRadioButton *pcr_type;
    QRadioButton *mc_type;
    QGroupBox *rejime_box;
    QRadioButton *rawdata_rejime;
    QRadioButton *filtered_rejime;
    QRadioButton *analysis_rejime;
    QTreeWidget *samples_tree;
    QTreeWidget *tests_tree;

    QAction *nc_toPNG;
    QAction *nc_toPDF;

    QGroupBox *box_graph;
    QwtPlot *graph;
    QwtPlotGrid *grid;
    QwtPlotCurve *curve;
    QVector<QwtPlotCurve*> list_Curve;

    bool ignore_slot;
    QVector<short> Sample_PosEnable;
    QMap<QString,QString> Map_TestTranslate;

    // methods
    void Fill_PCRResults(QString&);

    void Fill_ChartsInfo(QString&);
    void Create_Curve();
    void Clear_Graph();

    void Generate_Report();
    void Create_MapTranslate();

public slots:
    void edit_param();
    void edit_param_Graph();
    void apply_change();
    void change_ignore();

    void SaveTo_Image();
    void SaveTo_PDF();

};

#endif // REPORT_PARAMETERIZATION_H
