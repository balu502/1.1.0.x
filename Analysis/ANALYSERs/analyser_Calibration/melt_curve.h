#ifndef MELT_CURVE_H
#define MELT_CURVE_H

#include <QObject>
#include <QWidget>
#include <QApplication>
#include <QGroupBox>
#include <QBoxLayout>
#include <QDir>
#include <QProgressBar>
#include <QLabel>
#include <QPainter>
#include <QMap>
#include <QImage>
#include <QVector>
#include <QTableWidget>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <QImage>
#include <QVector>
#include <QVector2D>
#include <QtMath>
#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>
#include <QHeaderView>
#include <QProcess>
#include <QTemporaryFile>

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
#include "qwt_scale_engine.h"
#include "qwt_legend.h"

#include <stdlib.h>
#include <stdio.h>
#include <windows.h>

//#include <gsl/gsl_rng.h>
//#include <gsl/gsl_randist.h>
//#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_multifit_nlin.h>
#include "gsl/gsl_statistics_double.h"

#include "protocol.h"
#include "utility.h"
#include "algorithm.h"


#define fluor_NAME  {"fam","hex","rox","cy5","cy5.5"}
#define count_CH 5

#define ORDER 3
#define MAX_FIT_DATA 1024

#define COUNT_RESULT_PARAM 3

typedef void (__stdcall *Interface_3D)(std::string, bool&);

typedef struct
{
    size_t n;
    double *y;
    double *t;
    double K_coef;
}Data;


int expb_f (const gsl_vector*, void*, gsl_vector*);
double weib(double, double, double, double, double);
double weibc(double, double, double, double, double);

//-----------------------------------------------------------------------------
class MeltCurvePlot: public QwtPlot
{
    Q_OBJECT

public:
    MeltCurvePlot(QWidget *parent = NULL);
    virtual ~MeltCurvePlot();

    QwtPlotCurve    *Measure_curve;
    QwtPlotCurve    *Fit_curve;
    QwtLegend       *legend;

    QLabel          *peak_Label;
    QLabel          *chi_Label;
    QLabel          *ampl_Label;

    QwtPlotGrid *grid;
    QLabel *label_caption;

};
//-----------------------------------------------------------------------------
class FluorMC_ItemDelegate: public QStyledItemDelegate
{
Q_OBJECT

public:

    void paint (QPainter *painter,
                const QStyleOptionViewItem &option,
                const QModelIndex &index) const Q_DECL_OVERRIDE;

};
//-----------------------------------------------------------------------------
class RowColMC_ItemDelegate: public QStyledItemDelegate
{
Q_OBJECT

public:

    void paint (QPainter *painter,
                const QStyleOptionViewItem &option,
                const QModelIndex &index) const Q_DECL_OVERRIDE;

};
//-----------------------------------------------------------------------------
class ResultsMCItemDelegate: public QStyledItemDelegate
{
    Q_OBJECT

public:

    void paint (QPainter *painter,
                const QStyleOptionViewItem &option,
                const QModelIndex &index) const Q_DECL_OVERRIDE;

    int count_ActiveCh;
};
//-----------------------------------------------------------------------------

class ResultsSpectrumItemDelegate: public QStyledItemDelegate
{
    Q_OBJECT

public:

    void paint (QPainter *painter,
                const QStyleOptionViewItem &option,
                const QModelIndex &index) const Q_DECL_OVERRIDE;

    int count_ActiveCh;
};

//-----------------------------------------------------------------------------

class TableColorItemDelegate: public QStyledItemDelegate
{
    Q_OBJECT

public:

    double min_Value;
    double max_Value;
    bool view_value;
    int type_view;
    int mode_view;

    void paint (QPainter *painter,
                const QStyleOptionViewItem &option,
                const QModelIndex &index) const Q_DECL_OVERRIDE;
};

//-----------------------------------------------------------------------------

class Melt_Curve : public QGroupBox
{
    Q_OBJECT
public:
    Melt_Curve(QWidget *parent = 0);
    ~Melt_Curve();

    rt_Protocol *Prot;
    QTemporaryFile file_color;

    QGroupBox       *Control_Group;
    QGroupBox       *Chart_Group;
    QGroupBox       *Result_Group;

    MeltCurvePlot   *mc_chart;
    QComboBox            *Fluor_Box;
    FluorMC_ItemDelegate *fluor_delegate;
    QComboBox            *row_Box;
    QComboBox            *col_Box;
    RowColMC_ItemDelegate *delegate;
    QComboBox            *type_Box;

    QGroupBox           *Color_Group;
    QGroupBox           *Color_Control;
    QComboBox           *color_FluorBox;
    QComboBox           *color_TypeBox;
    QComboBox           *color_PlateBox;
    QCheckBox           *color_ViewValue;
    QPushButton         *color_3D;
    QLabel              *color_min;
    QLabel              *color_max;
    QLabel              *color_mean;
    QLabel              *color_sigma;
    QLabel              *color_peakTopeak;

    QTabWidget          *Result_Tab;
    QTableWidget        *Table_Result;    
    ResultsMCItemDelegate *results_Delegate;
    QTableWidget        *Table_Color;
    TableColorItemDelegate *color_Delegate;
    QTableWidget        *Table_ResultSpectrum;
    ResultsSpectrumItemDelegate *spectrum_Delegate;

    QProgressBar    *main_progress;
    QLabel          *PrBar_status;

    QVector<double> X_MeltCurve;
    QMap<int, QVector<int>*>    *map_RawData;
    QMap<int, QVector<double>*> map_RawMC;
    QMap<int, QVector<double>*> map_RawdFdT;
    QVector<QVector<QVector2D>*> list_FitMC;
    QVector<QVector<QVector2D>*> list_FitdFdT;

    QMap<QString, QString> map_ATTENTION_Temp;
    QMap<QString, QString> map_ATTENTION_Optic;
    bool TemperatureValid_status;
    bool OpticValid_status;

    QMap<int, QVector<double>*> map_Param;
    QVector<double> Peaks_3D;

    QVector<QVector2D> Meas,Fit;

    QVector<double> Pick_2_Pick_Block;
    QVector<int>    Num_Mistakes;
    QVector<double> Deviation_amplitude;
    QVector<double> Mean_temperature;
    QVector<double> Std_temperature;
    QVector<double> Std_amplitude;

    QVector<double> FluorCorrection;
    QVector<double> Amplitude;
    QVector<double> TPeaks;
    double Par[4];
    double Qu[3];
    QVector<double> Mean_Amplitude;
    //double kCoef,lCoef,sCoef;
    //double chi,terr,aerr;
    double kA,kK,kL,kS;
    //int npf;
    QString *Lang;

    // borders:
    double border_AbsDeviation;
    double border_IrregularityPlate;
    double border_PlateDeviation;
    double border_AbsDeviationOptic;
    double border_IrregularityPlateOptic;
    double border_PlateDeviationOptic;

    bool Overflow_T;
    bool Overflow_A;

    // methods:
    int Analyser(rt_Protocol*);
    void Clear_AllMaps();
    void LoadFluor(rt_Protocol *);
    void Load_RowCol(rt_Protocol*);
    void Display_ProgressBar(int percent, QString text);
    void Draw_Curves();
    void Fill_ResultsTable();
    void Fill_ResultsTableSpectrum();
    void Calculate_Results(rt_Protocol*);

    QVector<QVector2D> Calculate_Weibull(double*, QVector<QVector2D>*);
    QVector<QVector2D> Calculate_WeibullC(double*, QVector<QVector2D>*);

    QVector<QVector2D> calcDerivate(QVector<QVector2D>);
    int calcFitting(double*, QVector<QVector2D>, QVector<double>*);

    void Convert_To_ThermoBloks(QVector<double>*, int, int*, int*);

    int Validate_Data();
    int Validate_AFF(int, QVector<QVector2D>*);

signals:

public slots:
    void Change_Tube();
    void Change_CurrentIndex(int);
    void Fill_ResultsColor();
    void View_3D();
};



#endif // MELT_CURVE_H
