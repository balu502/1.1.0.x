#include "incr_temperature.h"

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Incr_Temperature::Incr_Temperature(QWidget *parent) : QDialog(parent)
{

    setFont(qApp->font());
    setStyleSheet(
                "QGroupBox {margin-top: 1ex; background: #FAFAFA; border: 1px solid #ddd; border-radius: 0px;}"
                "QGroupBox#Transparent {border: 1px solid transparent;}"
                "QDialog {background-color: #FAFAFA; border: 1px solid #ddd;}"
                "QComboBox {background-color: #ffffff; selection-background-color: #d7d7ff; selection-color: black;}"
                //"QComboBox {border: 1px solid #ddd; border-radius: 0px;}"
                "QPushButton::hover {background-color: #fff;}"
                "QPushButton::disabled {background-color: #ccc;}"
                "QPushButton::enabled {background-color: #FAFAFA;}"
                "QPushButton::pressed {background-color: #ccc;}"
                "QPushButton {text-align: center;}"
                "QPushButton {min-width: 4em; margin:0; padding:5;}"
                "QPushButton {border: 1px solid #aaa; border-radius: 0px;}"
                "QSpinBox {selection-background-color: #d7d7ff; selection-color: black;}"
                "QDoubleSpinBox {selection-background-color: #d7d7ff; selection-color: black;}"
                "QTableWidget {selection-background-color: #FAFAFA;}"
                "QToolTip {background: #FFFFFF; border: 1px solid #ddd;}"
                "QLabel {color : red; }"
                //"QDoubleSpinBox {border: 1px solid #aaa; border-radius: 0px;}"
    );

    QVBoxLayout *layout = new QVBoxLayout();
    setLayout(layout);

    Main_Box = new QGroupBox(this);
    QVBoxLayout *layout_param = new QVBoxLayout();
    Main_Box->setLayout(layout_param);

    TempStart = new Param(true,tr("Initial temperature"), 20, 1, QPointF(1,98.9), " °C", 1, this);
    TempFinish = new Param(false, tr("Final temperature"), 90, 1, QPointF(1,98.9), " °C", 1, this);
    IncrTemp = new Param(true, tr("Temperature increment"), 1, 1, QPointF(-5,+5), " °C", 0.1, this);
    CountCycle = new Param(true, tr("Count of cycles"), 40, 0, QPointF(1,1000), "", 1, this);
    params.append(TempStart);
    params.append(TempFinish);
    params.append(IncrTemp);
    params.append(CountCycle);

    layout_param->addWidget(TempStart);
    layout_param->addWidget(TempFinish);
    layout_param->addWidget(IncrTemp);
    layout_param->addWidget(CountCycle);

    apply = new QPushButton(tr("Apply"), this);
    apply->setFocusPolicy(Qt::NoFocus);
    cancel = new QPushButton(tr("Cancel"), this);
    cancel->setFocusPolicy(Qt::NoFocus);
    attention = new QLabel("", this);
    attention->adjustSize();
    timer = new QTimer(this);
    QHBoxLayout *layout_control = new QHBoxLayout();
    layout_control->addWidget(attention, 1, Qt::AlignLeft);
    layout_control->addWidget(apply, 1, Qt::AlignRight);
    layout_control->addWidget(cancel, 0, Qt::AlignRight);

    layout->addWidget(Main_Box);
    layout->addSpacing(20);
    layout->addLayout(layout_control);

    connect(cancel, SIGNAL(clicked(bool)), this, SLOT(reject()));
    connect(apply, SIGNAL(clicked(bool)), this, SLOT(accept()));
    connect(timer, SIGNAL(timeout()), this, SLOT(Timer_On()));

    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowTitle(tr("Temperatures Increment"));
    setWindowIcon(QIcon(":/images/DTm.ico"));
    //setWindowIcon(QIcon(":/images/RT.ico"));
    resize(450,250);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Incr_Temperature::~Incr_Temperature()
{
    delete TempStart;
    delete TempFinish;
    delete IncrTemp;
    delete CountCycle;
    params.clear();

    delete Main_Box;
    delete apply;
    delete cancel;
    timer->stop();
    delete timer;
  }
//-----------------------------------------------------------------------------
//--- showEvent
//-----------------------------------------------------------------------------
void Incr_Temperature::showEvent(QShowEvent *e)
{
    ReCalculate(true);



    e->accept();
}
//-----------------------------------------------------------------------------
//--- Apply()
//-----------------------------------------------------------------------------
void Incr_Temperature::Apply()
{
    int i;

    incrTemp_Info.T_s = TempStart->value_param->value();
    incrTemp_Info.N = CountCycle->value_param->value();
    incrTemp_Info.dt = IncrTemp->value_param->value();

    //setResult(QDialog::Accepted);
    //close();
}
//-----------------------------------------------------------------------------
//--- ReCalculate()
//-----------------------------------------------------------------------------
void Incr_Temperature::ReCalculate(bool init)
{
    Param *item;
    bool state;
    int i = 0;
    int id = 0;
    int j = 0;
    int id_unchecked;

    //qDebug() << "Recalc...";

    if(init)
    {
        //qDebug() << "incrTemp_Info: " << incrTemp_Info.T_s << incrTemp_Info.T_f << incrTemp_Info.dt << incrTemp_Info.N;

        foreach(item, params)
        {
            item->blockSignals(true);
            switch(j)
            {
            case 0: item->value_param->setValue(incrTemp_Info.T_s); break;
            case 1: item->value_param->setValue(incrTemp_Info.T_f); break;
            case 2: item->value_param->setValue(incrTemp_Info.dt); break;
            case 3: item->value_param->setValue(incrTemp_Info.N); break;
            default:    break;
            }
            item->blockSignals(false);
            j++;
        }
    }


    double T_s = params.at(0)->value_param->value();
    double T_f = params.at(1)->value_param->value();
    double dt = params.at(2)->value_param->value();
    int N = (int)params.at(3)->value_param->value();

    foreach(item, params)
    {
         state = item->check_param->isChecked();
         item->value_param->setEnabled(state);
         if(state) id++;
         if(!state) id_unchecked = i;
         i++;
    }

    if(id == 3)
    {
        state = true;
        attention->setText("");

        params.at(id_unchecked)->value_param->blockSignals(true);

        switch(id_unchecked)
        {
        case 0:     // disable TempStart
                T_s = T_f - N*dt;
                T_s = roundTo(T_s, 1);
                params.at(0)->value_param->setValue(T_s);
                if(T_s != params.at(0)->value_param->value()) state = false;
                break;

        case 1:     // disable TempFinish
                T_f = T_s + N*dt;
                T_f = roundTo(T_f, 1);
                params.at(1)->value_param->setValue(T_f);
                if(T_f != params.at(1)->value_param->value()) state = false;
                break;

        case 2:     // disable dt
                if(N == 0) {state = false; break;}
                dt = (T_f - T_s)/N;
                dt = roundTo(dt, 1);
                params.at(2)->value_param->setValue(dt);
                if(dt != params.at(2)->value_param->value()) state = false;
                break;

        case 3:     // disable N
                if(dt < 0.09 && dt > -0.09) {state = false; break;}
                N = (T_f - T_s)/dt;
                params.at(3)->value_param->setValue(N);
                if(N != params.at(3)->value_param->value()) state = false;
                break;

        default:    break;
        }
        params.at(id_unchecked)->value_param->blockSignals(false);

        if(!state) attention->setText(tr("Invalid VALUES of parameters"));

    }
    else
    {
        attention->setText(tr("Invalid NUMBER of parameters"));
    }

    state = attention->text().isEmpty();
    apply->setEnabled(state);

    if(!state) timer->start(500);
    else
    {
        Apply();
    }


}
//-----------------------------------------------------------------------------
//--- Timer_On()
//-----------------------------------------------------------------------------
void Incr_Temperature::Timer_On()
{
    if(attention->text().isEmpty())
    {
        timer->stop();
        return;
    }
    else
    {
        attention->setVisible(!attention->isVisible());
    }
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Param::Param(bool check, QString name, double value, int prec, QPointF MinMax, QString suffix, double step, QWidget *parent): QGroupBox(parent)
{
    Parent = (Incr_Temperature*)parent;
    setObjectName("Transparent");

    QHBoxLayout *layout = new QHBoxLayout;
    setLayout(layout);
    layout->setMargin(2);
    layout->setSpacing(2);

    check_param = new QCheckBox(tr("Initial temperature"), this);
    value_param = new QDoubleSpinBox(this);

    value_param->setFont(QFont("Times New Roman", 8, QFont::Bold));
    value_param->setFixedWidth(80);

    layout->addWidget(check_param, 0, Qt::AlignLeft);
    layout->addWidget(value_param, 1, Qt::AlignRight);

    check_param->setChecked(check);
    check_param->setText(name);
    value_param->setValue(value);
    value_param->setDecimals(prec);
    value_param->setMinimum(MinMax.x());
    value_param->setMaximum(MinMax.y());
    value_param->setEnabled(check);
    value_param->setSuffix(suffix);
    value_param->setSingleStep(step);

    connect(check_param, SIGNAL(clicked(bool)), this, SLOT(change_value()));
    connect(value_param, SIGNAL(valueChanged(double)), this, SLOT(change_value()));
    connect(this, SIGNAL(change()), Parent, SLOT(ReCalculate()));
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Param::~Param()
{
    delete check_param;
    delete value_param;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Param::change_value()
{
    emit change();
}
