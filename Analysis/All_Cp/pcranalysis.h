#ifndef PCRANALYSIS_H
#define PCRANALYSIS_H

#include <QApplication>
#include <QObject>
#include <QWidget>
#include <QGroupBox>
#include <QBoxLayout>
#include <QSplitter>
#include <QComboBox>
#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>
#include <QPainter>
#include <QTableWidget>
#include <QEvent>
#include <QHeaderView>
#include <QFont>
#include <QStringList>
#include <QVector>
#include <QLabel>
#include <QFrame>
#include <QPointF>
#include <QResizeEvent>
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


#include "protocol.h"
#include "define_PCR.h"
#include "point_takeoff.h"
#include "algorithm.h"
#include "utility.h"

class SinglePCRCurve: public QwtPlotCurve
{
 public:
    SinglePCRCurve(QWidget *parent = NULL){}
    virtual ~SinglePCRCurve() {}

    QVector<short> points_Toff;

    virtual void drawSymbols(QPainter *  painter,
                             const QwtSymbol &  symbol,
                             const QwtScaleMap &  xMap,
                             const QwtScaleMap &  yMap,
                             const QRectF &  canvasRect,
                             int  from,
                             int  to) const
    {
        int i;
        int h = symbol.size().height();
        int w = symbol.size().width();
        QwtSymbol sym_1;
        sym_1.setStyle(QwtSymbol::Ellipse);
        sym_1.setBrush(QBrush(Qt::yellow));
        sym_1.setPen(Qt::black, 1);
        sym_1.setSize(5,5);
        QwtSymbol sym_2;
        sym_2.setStyle(QwtSymbol::Ellipse);
        sym_2.setBrush(QBrush(Qt::green));
        sym_2.setPen(Qt::black, 1);
        sym_2.setSize(6,6);

        for(i=from; i< to; i++)
        {
            switch(points_Toff.at(i))
            {
            default:
            case 0:     QwtPlotCurve::drawSymbols(painter, symbol, xMap, yMap, canvasRect, i, i);
                        break;
            case 1:     QwtPlotCurve::drawSymbols(painter, sym_1, xMap, yMap, canvasRect, i, i);
                        break;
            case 2:     QwtPlotCurve::drawSymbols(painter, sym_2, xMap, yMap, canvasRect, i, i);
                        break;
            }
        }
    }
};

//-----------------------------------------------------------------------------
class SinglePCRPlot: public QwtPlot
{
    Q_OBJECT

public:
    SinglePCRPlot(QWidget *parent = NULL);
    virtual ~SinglePCRPlot();

    SinglePCRCurve *curve;
    QwtPlotGrid *grid;

    QLabel *label_caption;

};
//-----------------------------------------------------------------------------
class SigmoidPCRCurve: public QwtPlotCurve
{

public:
    SigmoidPCRCurve(QWidget *parent = NULL){}
    virtual ~SigmoidPCRCurve() {}

    short bounds_Sigmoid;

    virtual void drawLines( QPainter *painter,
                            const QwtScaleMap &xMap, const QwtScaleMap &yMap,
                            const QRectF &canvasRect, int from, int to ) const
    {
        int i,j;

        for(i=from; i< to; i++)
        {
            if(i < bounds_Sigmoid) painter->setPen(Qt::red);
            else painter->setPen(Qt::blue);
            QwtPlotCurve::drawLines(painter, xMap, yMap, canvasRect, i, i+1);
        }
    }

};

//-----------------------------------------------------------------------------
class SigmoidPlot: public QwtPlot
{
    Q_OBJECT

public:
    SigmoidPlot(QWidget *parent = NULL);
    virtual ~SigmoidPlot();

    QwtPlotCurve *curve_real;
    QwtPlotCurve *curve_spline;
    SigmoidPCRCurve *curve_sigmoid;
    QwtPlotCurve *curveXY_Cp;

    QwtPlotGrid *grid;

    QLabel *label_caption;

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
class Fluor_ComboBox: public QComboBox
{
    Q_OBJECT

protected:

    virtual void paintEvent(QPaintEvent* e)
    {
        QComboBox::paintEvent(e);
        if(currentIndex() < 0) return;
        QPainter p(this);
    }
};

//-----------------------------------------------------------------------------
class TableWidget : public QTableWidget
{
    Q_OBJECT

public:
        TableWidget(int rows, int cols, QWidget* parent = 0)
                : QTableWidget(rows, cols, parent)
        {
            selected = 0;
        }

        int selected;

protected:
        //void mousePressEvent(QMouseEvent *event);        
        //void paintEvent(QPaintEvent *e);
        //void resizeEvent(QResizeEvent *e);

};

//-----------------------------------------------------------------------------
class ItemDelegate: public QStyledItemDelegate
{
public:
    ItemDelegate()
    {
    }

    //rt_Protocol *p_prot;

    //void get_prot(rt_Protocol *p) {p_prot = p;}
    void paint (QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

};

class TableTOFF_ItemDelegate: public QStyledItemDelegate
{
    Q_OBJECT

public:

    Sigmoid *sigmoid;
    void get_sigmoid(Sigmoid *s) {sigmoid = s;}

    void paint (QPainter *painter,
                const QStyleOptionViewItem &option,
                const QModelIndex &index) const Q_DECL_OVERRIDE;
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

class PCRAnalysis: public QGroupBox
{
        Q_OBJECT

public:
    PCRAnalysis(QWidget *parent = NULL);
    virtual ~PCRAnalysis();

    rt_Protocol *prot;

    QSplitter *main_spl;            // splitter between ChartBox and GridBox
    QSplitter *chart_spl;           // splitter between Charts
    QGroupBox *ChartBox;            // Charts
    QGroupBox *GridBox;             // Grids

    void Fill_PCRAnalysis(rt_Protocol*);
    void Draw_SinglePlot(int pos, int ch);
    void Draw_SigmoidPlot(int pos, int ch, int num_ptoff);
    void Fill_TablePOFF();
    void Fill_GridResults(rt_Protocol*, int pos = 0);

private:
    Fluor_ItemDelegate *fluor_delegate;
    Fluor_ComboBox *fluor_select;
    QLabel *index_tube;

    TableWidget *Select_Grid;       // Select Table with color results
    ItemDelegate *Delegate;

    QTableWidget *Table_Points;     // Table with result for TakeOff Points
    TableTOFF_ItemDelegate *toff_Delegate;

    SinglePCRPlot *Single_plot;
    SigmoidPlot   *Sigmoid_plot;

    Sigmoid *sigmoid;

private slots:
    void Select_Tube(int row, int col);
    void Select_Fluor(int ch);
    void Select_PointsTOFF();
};

#endif // PCRANALYSIS_H
