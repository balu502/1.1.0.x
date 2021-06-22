#ifndef INCR_TEMPERATURE_H
#define INCR_TEMPERATURE_H

#include <QObject>
#include <QWidget>
#include <QDialog>
#include <QBoxLayout>
#include <QGroupBox>
#include <QCheckBox>
#include <QLabel>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QApplication>
#include <QPushButton>
#include <QPointF>
#include <QTimer>
#include <QShowEvent>
#include <QDebug>

#include "algorithm.h"

class Incr_Temperature;

//-----------------------------------------------------------------------------
typedef struct
{
    double T_s;
    double T_f;
    double dt;
    int N;
    int unchecked;

}IncrTemperature_INFO;

//-----------------------------------------------------------------------------
class Param: public QGroupBox
{
Q_OBJECT

public:
    Param(bool check, QString name, double value, int prec, QPointF MinMax, QString suffix, double step, QWidget *parent = 0);
    ~Param();

    QCheckBox   *check_param;    
    QDoubleSpinBox *value_param;

    Incr_Temperature     *Parent;

signals:
    void change();

public slots:
    void change_value();

};
//-----------------------------------------------------------------------------

class Incr_Temperature : public QDialog
{
    Q_OBJECT
public:
    explicit Incr_Temperature(QWidget *parent = 0);
    ~Incr_Temperature();

    QGroupBox   *Main_Box;
    Param   *TempStart;
    Param   *TempFinish;
    Param   *IncrTemp;
    Param   *CountCycle;

    QVector<Param*> params;

    QPushButton *apply;
    QPushButton *cancel;
    QLabel  *attention;
    QTimer  *timer;

    IncrTemperature_INFO incrTemp_Info;

signals:

public slots:
     void Apply();
     void ReCalculate(bool init = false);
     void Timer_On();

protected:
    void showEvent(QShowEvent *e) Q_DECL_OVERRIDE;
};

#endif // INCR_TEMPERATURE_H
