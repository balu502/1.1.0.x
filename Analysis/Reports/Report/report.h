#ifndef REPORT_H
#define REPORT_H

#include "report_global.h"

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
#include "report_interface.h"

typedef Report_Interface* (__stdcall *Report_factory)();

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
class ReportPreview_ALL : public NCReportPreviewWindow
{
    Q_OBJECT

public:
    ReportPreview_ALL(QWidget *parent = 0);
    ~ReportPreview_ALL();
};
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
class REPORT_EXPORT Report: public QDialog
{
    Q_OBJECT

public:
    Report(rt_Protocol*, QString user_folder = "", QString report_folder = "", QWidget *parent = 0);
    ~Report();

    rt_Protocol *prot;
    NCReport *report;
    QTranslator translator;
    QTranslator translator_report;
    void readCommonSettings();
    QString lang;
    QPushButton *ApplyChange;
    QProgressBar *main_progress;

    ReportPreview_ALL *pv;

    QToolBar *client_toolbar;
    QGroupBox *client_box;    
    QTreeWidget *samples_tree;

    QAction *nc_toPNG;
    QAction *nc_toPDF;

    QString User_Folder;

    HINSTANCE report_handle;
    Report_Interface *report_obj;
    Report_Interface *current_ReportObj;

    bool ignore_slot;
    QVector<short> Sample_PosEnable;
    QMap<int,Report_Interface*> Map_Reports;
    QMap<int,QString> Map_Description;
    QVector<HINSTANCE> List_reportHandle;
    QVector<short> samples_Enable;

    QComboBox *box_Report;

    // methods
    void Generate_Report();
    void Load_Reports(QString);
    void PreView_Report(Report_Interface*);
    void Samples_ForTest(Report_Interface*);

public slots:
    void edit_param(QTreeWidgetItem*,int);

    void change_Report(int);
    void apply_change();
    void change_ignore();

    void SaveTo_Image();
    void SaveTo_PDF();


};

#endif // REPORT_H
