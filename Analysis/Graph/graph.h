#ifndef GRAPH_H
#define GRAPH_H

#include <QLabel>
#include <QVector>
#include <QEvent>
#include <QMouseEvent>
#include <QTimer>
#include <qmath.h>
#include <QDebug>
#include <QCursor>
#include <QFont>
#include <QFontMetrics>
#include <QApplication>
#include <QComboBox>
#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>
#include <QPainter>
#include <QBoxLayout>
#include <QPointF>
#include <QAction>
#include <QMenu>
#include <QIcon>
#include <QPixmap>
#include <QFileDialog>
#include <QClipboard>
#include <QFile>
#include <QTextStream>
#include <QTranslator>
#include <QSettings>
#include <QGroupBox>
#include <QToolBox>
#include <QPushButton>
#include <QToolButton>
#include <QButtonGroup>
#include <QtOpenGL/QtOpenGL>

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

#include "Graph_global.h"

#include "protocol.h"
#include "utility.h"
#include "define_PCR.h"



class TypeData_ItemDelegate: public QStyledItemDelegate
{
    Q_OBJECT

public:

    void paint (QPainter *painter,
                const QStyleOptionViewItem &option,
                const QModelIndex &index) const Q_DECL_OVERRIDE;
};
//-----------------------------------------------------------------------------
class RejimeData_ItemDelegate: public QStyledItemDelegate
{
    Q_OBJECT

public:

    void paint (QPainter *painter,
                const QStyleOptionViewItem &option,
                const QModelIndex &index) const Q_DECL_OVERRIDE;
};
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

        //QPixmap pixmap(":/images/fam.png");
        //p.drawPixmap(0, 0, pixmap);


    }
};
//-----------------------------------------------------------------------------
class Select_Tube: public QEvent
{
public:
    Select_Tube(): QEvent((Type)1110)
    {
    }
    int pos_tube;
    QString cell_grid;
};
//-----------------------------------------------------------------------------
class ReAnalysis: public QEvent
{
public:
    ReAnalysis(): QEvent((Type)1120)
    {
    }
    int param;  //for example: 0x02 - only fill table
};

//-----------------------------------------------------------------------------
class GRAPHSHARED_EXPORT GraphPlot: public QwtPlot
{
    Q_OBJECT      //widget's signal or slot

public:
    GraphPlot(QWidget *parent = NULL);
    virtual ~GraphPlot();

public slots:

    void Hide_LabelCurve(){label_curve->setVisible(false);}


public:

    QLabel *label_curve;
    QwtPlotGrid *grid;
    QPoint pos_previous;
    QWidget *main_widget;
    Select_Tube select_tube;
    ReAnalysis reanalysis;
    int id_wcurve;
    QBrush bg_color;

    int current_ch;
    int current_rejime;
    int current_type;

    double value_MeltingMarker;

    //...
    Fluor_ComboBox *Fluor_Box;
    QComboBox *RejimeData_Box;
    QComboBox *Type_Box;
    QLabel    *Coordinates_XY;

    QGroupBox *Group_FluorButton;
    QToolButton *fam_button;
    QToolButton *hex_button;
    QToolButton *rox_button;
    QToolButton *cy5_button;
    QToolButton *cy55_button;
    QButtonGroup *Fluor_Buttons;

    QToolButton *Norm_1000;
    QVector<double> *PCR_1000;   // pcr data for norm_1000
    QVector<double> *MC_1000;    // mc data for norm_1000

    QGroupBox *Group_FluorSingle;
    QToolButton *fam_single;
    QToolButton *hex_single;
    QToolButton *rox_single;
    QToolButton *cy5_single;
    QToolButton *cy55_single;
    QButtonGroup *Fluor_Single;


    //...

    QPoint p;                                   //Plate.PlateSize

    QwtPlotCurve *curve;    
    int size_symbol;
    int thickness_line;
    QVector<QwtPlotCurve*> list_Curve;
    QVector<QwtPlotCurve*> list_Threshold;
    QwtPlotCurve curve_threshold;
    QwtPlotCurve curve_melting;
    QLabel *label_threshold;
    QLabel *label_melting;
    QLabel *label_XY;
    QFont font_plot;

    //QVector<double> CoeffNorm_PCR;
    //QVector<double> CoeffNorm_MC;

    rt_Protocol *prot;

    bool rejime_Ct;
    bool rejime_Lg;
    bool rejime_Normalization;
    bool rejime_Melting;

    bool curve_edit;
    QwtPlotCurve *d_selectedCurve;
    int d_selectedPoint;

    //... Actions ...
    QAction *copy_to_clipboard;
    QAction *copy_to_excelfile;
    QAction *save_as_PNG;
    QAction *copy_to_clipboard_as_image;
    QAction *view_as_buttons;

    //... qwtplot copy image ...
    QGroupBox *box_image;
    QwtPlot *plot_image;
    QwtPlotGrid *grid_image;
    QwtPlotCurve *curve_image;
    QVector<QwtPlotCurve*> listCurve_image;
    //...

    void Draw_Chart(rt_Protocol *prot, int rejime, int channel, int type = 1);

    void Create_Curve(rt_Protocol*);
    void Delete_Curve();
    void Clear_Graph();

    void Change_SizeMarker();
    void Change_ThicknessLine();
    void Change_Enable(rt_Protocol*, QVector<short>*);
    //void Calculate_NormCoefficients(rt_Protocol*);

    void SetAutoScale() {setAxisAutoScale(0); setAxisAutoScale(2);}
    void select_GraphCurve(const QPoint &);
    double Calculate_closestDistance(QPoint pos, QPoint pos_before, QPoint pos_after);    
    void Select_Curve(int pos);

    void select_GraphMarker(const QPoint &);    // using curve_edit mode
    void select_Marker(bool);
    void move_GraphMarker(const QPoint &);

    void helpChart();
    bool Ct_Chart(bool);
    bool Norm_Chart(bool);
    bool Lg_Chart(bool);
    bool Melt_Chart(bool);

    void readCommonSettings();

//public slots:
    void Increase_Scale();
    void Decrease_Scale();
    void Auto_Scale();

private slots:
    void change_Fluor(int type);
    void change_RejimeData(int type);
    void change_TypeData(int type);
    void Repaint_Threshold(double,bool);
    void Repaint_Melting(double);
    //void Repaint_XY();

    void contextMenu_Graph();
    void contextMenu_Fluor();

    QString Graph_ToDigits();
    void to_ClipBoard();
    void to_Excel();
    void to_Image();
    void to_ImageClipBoard();

    void slot_FluorButton(int);
    void slot_Norm1000();

    void slot_View_as_Buttons();
    void slot_ChangeFluor(int);

protected:
    virtual bool eventFilter(QObject *, QEvent *);

signals:
    void rePaint_Threshold(double,bool);
    void rePaint_Melting(double);
    void rePaint_XY();
    void reAnaysis();
    void clear_rejimes(int);

private:
    Fluor_ItemDelegate *Fluor_Delegate;
    TypeData_ItemDelegate *Type_Delegate;
    RejimeData_ItemDelegate *Rejime_Delegate;

    QSettings *CommonSettings;
    QTranslator translator;
    QString StyleApp;

};

GRAPHSHARED_EXPORT GraphPlot* Create_Graph(QWidget*);       // create Graph
GRAPHSHARED_EXPORT void Delete_Graph(GraphPlot*);           // delete Graph

#endif // GRAPH_H
