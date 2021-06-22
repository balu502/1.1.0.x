#ifndef ANALYSER_PEAKSAREA_H
#define ANALYSER_PEAKSAREA_H

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
#include <QPointF>
#include <QSlider>
#include <QRadioButton>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QButtonGroup>
#include <QToolButton>
#include <QFileDialog>
#include <QScrollArea>
#include <QProcess>
#include <QColorDialog>

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

#include <qwt_plot_shapeitem.h>
#include <qwt_plot_magnifier.h>
#include <qwt_legend.h>
#include <qwt_plot_renderer.h>

#include "analyser_peaksarea_global.h"
#include "alg_interface_qevent.h"
#include "analysis_interface.h"
#include "algorithm.h"
#include "utility.h"
#include "define_PCR.h"
#include "shapefactory.h"

#include <algorithm>
#include <gsl/gsl_fit.h>
#include <gsl/gsl_statistics.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_spline.h>
#include <gsl/gsl_math.h>

//-----------------------------------------------------------------------------
class Area: public QObject
{
    Q_OBJECT

public:

    QVector<double> s;
    QVector<QPoint> p;
};

//-----------------------------------------------------------------------------
class Fluor_ItemDelegate: public QStyledItemDelegate
{
    Q_OBJECT

public:

    void paint (QPainter *painter,
                const QStyleOptionViewItem &option,
                const QModelIndex &index) const Q_DECL_OVERRIDE;


};
//-----------------------------------------------------------------------------
class ResultsItemDelegate: public QStyledItemDelegate
{
    Q_OBJECT

public:

    void paint (QPainter *painter,
                const QStyleOptionViewItem &option,
                const QModelIndex &index) const Q_DECL_OVERRIDE;

};
//-----------------------------------------------------------------------------

class ANALYSER_PEAKSAREASHARED_EXPORT Analyser_PeaksArea: public QObject, public Analysis_Interface
{
    Q_OBJECT

public:
    Analyser_PeaksArea();

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


    QwtPlotCurve *curve;
    QwtPlotGrid *grid;
    QwtPlotShapeItem *item_first;
    QwtPlotShapeItem *item_second;

    // methods:

    void Create_Curves();
    void Delete_Curves();
    void Fill_TableResults();

    
private:
    rt_Protocol     *Prot;
    QGroupBox       *MainBox;
    QWidget         *main_widget;

    QSplitter       *main_spl;
    QGroupBox       *Box_Control;
    QTableWidget    *Table_Results;
    ResultsItemDelegate *Delegate;
    QwtPlot         *plot_Peaks;
    QComboBox       *Fluor_Box;
    Fluor_ItemDelegate *Fluor_Delegate;
    QLabel          *name_Tube;

    QVector<double> Buf_X;
    QVector<double> Buf_Y;

    QMap<QString, Area*> map_Area;

private slots:
    void Change_Select(int,int,int,int);
    
};

#endif // ANALYSER_PEAKSAREA_H
