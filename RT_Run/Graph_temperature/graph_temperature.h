#ifndef GRAPH_TEMPERATURE_H
#define GRAPH_TEMPERATURE_H

#define HISTORY 300

#include <QLabel>
#include <QVector>
#include <QList>
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
#include <QMap>
#include <QPixmap>
#include <QSettings>
#include <QTranslator>

#include "qwt_plot.h"
#include "qwt_plot_curve.h"
#include "qwt_symbol.h"
#include "qwt_plot_grid.h"
#include "qwt_plot_zoomer.h"
#include "qwt_picker.h"
#include "qwt_plot_canvas.h"
#include "qwt_curve_fitter.h"
#include "qwt_date_scale_draw.h"
#include "qwt_date_scale_engine.h"
#include "qwt_date.h"
#include "qwt_scale_widget.h"

#include "Graph_Temperature_global.h"
#include "../Run/request_dev.h"



class TemperatureCurve: public QwtPlotCurve
{

public:
    TemperatureCurve(QWidget *parent = NULL){}
    virtual ~TemperatureCurve() {}
    QList<QColor> color_XY;

    virtual void drawLines( QPainter *painter,
                            const QwtScaleMap &xMap, const QwtScaleMap &yMap,
                            const QRectF &canvasRect, int from, int to ) const
    {
        int i,j;

        for(i=from; i< to; i++)
        {            
            painter->setPen(color_XY.at(i));
            QwtPlotCurve::drawLines( painter, xMap, yMap, canvasRect, i, i+1);
        }
    }

};
//-----------------------------------------------------------------------------
class TimeScaleDraw:public QwtScaleDraw
{
public:
    TimeScaleDraw(const QTime & base)
        :baseTime(base)
    {
    }
virtual QwtText label(double v)const
{
    QTime upTime = baseTime.addSecs((int)v);
    return upTime.toString("h:mm:ss");
}
private:
    QTime baseTime;
};

//-----------------------------------------------------------------------------
class GRAPHTEMPERATURESHARED_EXPORT GraphTemperaturePlot: public QwtPlot
{
    Q_OBJECT      //widget's signal or slot

public:
    GraphTemperaturePlot(QWidget *parent = NULL);
    virtual ~GraphTemperaturePlot();

public:

    QwtPlotGrid     *grid;
    TemperatureCurve    *curve;
    QList<QPointF> XY;
    //QList<QColor> color_XY;
    double x0;
    TimeScaleDraw *scaleDraw;
    QTime base;

    QTimer          timer_warning;
    QTimer          timer_measuring;
    QLabel          *label_caption;
    QLabel          *current_Temperature;
    QLabel          *warning_State;
    QLabel          *label_measuring;
    QLabel          *label_SaveImageRejime;
    bool            flag_SaveImageRejime;

    QWidget         *main_widget;

public slots:
    void slotRefreshData(QMap<QString,QString>*);
    void clearPlot();

    void slot_ChangeRejime(int);
    void slot_WarningRejime();
    void slot_MeasuringRejime();

    void set_Enable(bool);

private:
    QTranslator translator;
    void readCommonSettings();

};

#endif // GRAPH_TEMPERATURE_H
