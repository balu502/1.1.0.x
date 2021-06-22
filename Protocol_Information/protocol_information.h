#ifndef PROTOCOL_INFORMATION_H
#define PROTOCOL_INFORMATION_H

#include <QtCore/QObject>
#include <QApplication>
#include <QWidget>
#include <QTabWidget>
#include <QCheckBox>
#include <QGroupBox>
#include <QVector>
#include <QTableWidget>
#include <QHeaderView>
#include <QFont>
#include <QStyledItemDelegate>
#include <QPalette>
#include <QStyleOptionViewItem>
#include <QPainter>
#include <QLabel>
#include <QBoxLayout>
#include <QFont>
#include <QTreeWidget>
#include <QScrollArea>
#include <QDateTime>
#include <QPolygon>
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

#include "protocol.h"
#include "define_PCR.h"

#include "windows.h"
#include "protocol_information_global.h"


class ColorCurve: public QwtPlotCurve
{

public:
    ColorCurve(QWidget *parent = NULL){}
    virtual ~ColorCurve() {}
    QVector<QColor> color_lev;

    virtual void drawLines( QPainter *painter,
                            const QwtScaleMap &xMap, const QwtScaleMap &yMap,
                            const QRectF &canvasRect, int from, int to ) const
    {
        int i,j;

        for(i=from; i< to; i++)
        {
            j = div(i,2).quot;
            if(div(i,2).rem) painter->setPen(Qt::darkGreen);
            else painter->setPen(color_lev.at(j));

            QwtPlotCurve::drawLines( painter, xMap, yMap, canvasRect, i, i+1);
        }
    }

};

//-----------------------------------------------------------------------------

class PROTOCOL_INFORMATIONSHARED_EXPORT Plot_ProgramAmpl: public QwtPlot
{
    Q_OBJECT

public:
    Plot_ProgramAmpl(QWidget *parent = NULL);
    virtual ~Plot_ProgramAmpl();

    //QwtPlotCurve *curve;
    ColorCurve      *curve;
    QwtPlotGrid     *grid;
    QLabel          *name_program;
    QLabel          *param_program;
    QLabel          *volume_program;

    void readCommonSettings();
    QTranslator translator;
    QString style;

public slots:
    void clear_Program(void);
    void draw_Program(rt_Protocol*);
};

//-----------------------------------------------------------------------------

class PROTOCOL_INFORMATIONSHARED_EXPORT Plate_Protocol: public QTableWidget
{
    Q_OBJECT

public:
    Plate_Protocol(QWidget *parent = NULL);
    virtual ~Plate_Protocol();


public slots:
    void clear_Plate(void);
    void draw_Plate(rt_Protocol*);
};

//-----------------------------------------------------------------------------

class ItemDelegate_info: public QStyledItemDelegate
{
public:
    ItemDelegate_info()
    {
    }

    rt_Protocol *p_prot;

    void get_prot(rt_Protocol *p) {p_prot = p;}
    void paint (QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;


};

//-----------------------------------------------------------------------------

class ItemDelegate_Information: public QStyledItemDelegate
{
public:
    ItemDelegate_Information()
    {
    }

    QString style;

    void paint (QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

};

//-----------------------------------------------------------------------------

class PROTOCOL_INFORMATIONSHARED_EXPORT Info_Protocol: public QTreeWidget
{
    Q_OBJECT

public:
    Info_Protocol(QWidget *parent = NULL);
    virtual ~Info_Protocol();

    ItemDelegate_Information *Delegate_Info;

    void readCommonSettings();
    QString style;

public slots:
    void clear_Info(void);
    void fill_Info(rt_Protocol*);
};

//-----------------------------------------------------------------------------
enum block_Type {bUnknown, bCycle, bMelt, bPause, bHold};

class Block_Pro
{

public:
    QVector<QString>    Level;
    QVector<short>      Measure;
    int                 Num_Cycle;
    block_Type          Type_Block;
    QVector<short>      Incr_Temp;
    QVector<short>      Incr_Time;
    QVector<short>      Gradient;

    Block_Pro()
    {
        Level.reserve(16);
        Measure.reserve(16);
        Num_Cycle = 0;
        Type_Block = bCycle;
        Incr_Temp.reserve(16);
        Incr_Time.reserve(16);
        Gradient.reserve(16);
    }
    ~Block_Pro()
    {
        Level.clear();
        Measure.clear();
        Incr_Temp.clear();
        Incr_Time.clear();
        Gradient.clear();
    }
};
//-----------------------------------------------------------------------------
class PROTOCOL_INFORMATIONSHARED_EXPORT Scheme_ProgramAmpl: public QScrollArea
{
    Q_OBJECT

public:
    Scheme_ProgramAmpl(QColor = Qt::transparent, QWidget *parent = NULL);

    //virtual ~Scheme_ProgramAmpl();
    void readCommonSettings();
    QString style;

    QLabel label_Program;


    void draw_Program(rt_Protocol*, int draw_Level = -1);
    void draw_Program(QVector<std::string>*, int draw_Level = -1);

    void clear_scheme();

};

#endif // PROTOCOL_INFORMATION_H
