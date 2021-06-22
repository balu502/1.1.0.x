#ifndef GRAPH_FLUOR_H
#define GRAPH_FLUOR_H

#include <QLabel>
#include <QVector>
#include <QEvent>
#include <QMouseEvent>
#include <QTimer>
#include <qmath.h>
#include <QDebug>
#include <QCursor>
#include <QFont>
#include <QApplication>
#include <QComboBox>
#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>
#include <QPainter>
#include <QBoxLayout>
#include <QPointF>
#include <QIcon>
#include <QSettings>
#include <QTranslator>

#include <protocol.h>
#include <utility.h>

#include "qwt_plot.h"
#include "qwt_plot_curve.h"
#include "qwt_symbol.h"
#include "qwt_plot_grid.h"
#include "qwt_plot_zoomer.h"
#include "qwt_picker.h"
#include "qwt_plot_canvas.h"
#include "qwt_curve_fitter.h"

#include "Graph_Fluor_global.h"

#include "../../Analysis/RT_Analysis/define_PCR.h"
#include "../../Analysis/Protocol/protocol.h"



class Fluor_ItemDelegate: public QStyledItemDelegate
{
    Q_OBJECT

public:

    void paint (QPainter *painter,
                const QStyleOptionViewItem &option,
                const QModelIndex &index) const Q_DECL_OVERRIDE;
    QString style;
};

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

class GRAPHFLUORSHARED_EXPORT GraphFluorPlot: public QwtPlot
{
    Q_OBJECT      //widget's signal or slot

public:
    GraphFluorPlot(QWidget *parent = NULL);
    virtual ~GraphFluorPlot();

    void Create_Curve(rt_Protocol*);
    void Delete_Curve();
    void Clear_Graph();
    void Draw_Chart(rt_Protocol*, int channel);
    void Change_Enable(rt_Protocol*, QVector<short>*);
    void Change_SizeMarker();

public:

    rt_Protocol             *prot;
    QwtPlotCurve            *curve;
    QVector<QwtPlotCurve*>  list_Curve;
    int size_symbol;

    QVector<double> X;
    QVector<double> Y;
    int current_fn;

    QwtPlotGrid     *grid;
    QLabel          *label_caption;
    QLabel          *label_fn;

    QWidget         *main_widget;

    QLabel *label_curve;
    void select_GraphCurve(const QPoint &);
    double Calculate_closestDistance(QPoint pos, QPoint pos_before, QPoint pos_after);

public slots:
    void slot_NewProtocol(rt_Protocol*);
    void change_Fluor(int type);
    void slot_RefreshFluor(void);
    void Hide_LabelCurve(){label_curve->setVisible(false);}

private:
    Fluor_ItemDelegate *fluor_delegate;
    Fluor_ComboBox *fluor_select;

    QTranslator translator;
    void readCommonSettings();
    QString StyleApp;

protected:
    virtual bool eventFilter(QObject *, QEvent *);
};

#endif // GRAPH_FLUOR_H
