#include "runtime.h"

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
RunTime::RunTime(QWidget *parent):
                 QGroupBox(parent)
{
    int i;

    readCommonSettings();
    //setFont(qApp->font());
    setObjectName("Transparent");

    XID_state = "";
    TIM_state = "";
    prot_Start = QDateTime::currentDateTime();
    for(i=0; i<5; i++) list_Time.append("00:00:00");

    rt_Info = new QGroupBox(this);
    rt_Info->setObjectName("Transparent");
    time_Info = new QGroupBox(this);
    Scroll_time = new QScrollArea(this);
    Scroll_time->setWidget(time_Info);
    Scroll_time->setWidgetResizable(true);

    QHBoxLayout *layout_rt = new QHBoxLayout;
    setLayout(layout_rt);
    layout_rt->setSpacing(2);
    layout_rt->setMargin(2);
    layout_rt->addWidget(rt_Info, 1);
    layout_rt->addWidget(Scroll_time);

    // time
    QVBoxLayout *layout_time = new QVBoxLayout;
    layout_time->setSpacing(0);
    layout_time->setMargin(0);
    time_Info->setLayout(layout_time);
    QStringList list;
    list << tr("From start") << tr("Till end") << tr("Time of start") << tr("End time") << tr("Current time");

    QFont label_font("Times New Roman", 14, QFont::Bold);
    QFont time_font("Times New Roman", 10, QFont::Bold);

    // program name
    program_Name = new QLabel("", this);
    program_Name->setStyleSheet("color: darkblue");
    program_Name->setFont(label_font);
    box_name = new QGroupBox(this);
    box_name->setStyleSheet("QGroupBox {border: none;}");
    QHBoxLayout *layout_name = new QHBoxLayout;
    box_name->setLayout(layout_name);
    layout_name ->addWidget(program_Name, 0, Qt::AlignHCenter);

    // time_first
    time_first = new QComboBox(this);
    //time_first->setFont(time_font);
    time_first->addItems(list);    
    time_first->setCurrentIndex(4);
    for(i=0; i < time_first->count(); i++) time_first->setItemData(i, Qt::AlignCenter, Qt::TextAlignmentRole);
    time_first->setStyleSheet("background-color: white; selection-background-color: #d7d7ff; selection-color: black;");
    label_first = new QLabel("00:00:00", this);
    label_first->setStyleSheet("color: darkblue");
    label_first->setFont(label_font);

    box_first = new QGroupBox(this);
    box_first->setStyleSheet("QGroupBox {border: none;}");
    QHBoxLayout *layout_first = new QHBoxLayout;
    layout_first->setMargin(2);
    box_first->setLayout(layout_first);
    layout_first->addSpacing(10);
    layout_first->addWidget(time_first,0,Qt::AlignLeft);
    layout_first->addWidget(label_first,0,Qt::AlignRight);
    layout_first->addSpacing(10);

    connect(time_first, SIGNAL(currentIndexChanged(int)), this, SLOT(change_FirstTime(int)));


    // time_second
    time_second = new QComboBox(this);
    //time_second->setFont(time_font);
    time_second->addItems(list);
    time_second->setCurrentIndex(1);    
    for(i=0; i < time_second->count(); i++) time_second->setItemData(i, Qt::AlignCenter, Qt::TextAlignmentRole);
    time_second->setStyleSheet("background-color: white; selection-background-color: #d7d7ff; selection-color: black;");
    label_second = new QLabel("00:00:00", this);
    label_second->setStyleSheet("color: darkblue");
    label_second->setFont(label_font);

    box_second = new QGroupBox(this);
    box_second->setStyleSheet("QGroupBox {border: none;}");
    QHBoxLayout *layout_second = new QHBoxLayout;
    layout_second->setMargin(2);
    box_second->setLayout(layout_second);
    layout_second->addSpacing(10);
    layout_second->addWidget(time_second,0,Qt::AlignLeft);
    layout_second->addWidget(label_second,0,Qt::AlignRight);
    layout_second->addSpacing(10);

    connect(time_second, SIGNAL(currentIndexChanged(int)), this, SLOT(change_SecondTime(int)));

    // time_current
    current_label = new QLabel(tr("Current cycle:"), this);
    //current_label->setFont(time_font);
    current_cycle = new QLabel("0", this);
    current_cycle->setStyleSheet("color: darkblue");
    current_cycle->setFont(label_font);
    box_current = new QGroupBox(this);
    box_current->setStyleSheet("QGroupBox {border: none;}");
    QHBoxLayout *layout_current = new QHBoxLayout;
    layout_current->setMargin(2);
    box_current->setLayout(layout_current);
    layout_current->addSpacing(10);
    layout_current->addWidget(current_label,0,Qt::AlignLeft);
    layout_current->addWidget(current_cycle,0,Qt::AlignRight);
    layout_current->addSpacing(10);

    // time_end
    end_label = new QLabel(tr("Remaining cycles:"), this);
    //end_label->setFont(time_font);
    end_cycle = new QLabel("0", this);
    end_cycle->setStyleSheet("color: darkblue");
    end_cycle->setFont(label_font);
    box_end = new QGroupBox(this);
    box_end->setStyleSheet("QGroupBox {border: none;}");
    QHBoxLayout *layout_end = new QHBoxLayout;
    layout_end->setMargin(2);
    box_end->setLayout(layout_end);
    layout_end->addSpacing(10);
    layout_end->addWidget(end_label,0,Qt::AlignLeft);
    layout_end->addWidget(end_cycle,0,Qt::AlignRight);
    layout_end->addSpacing(10);

    layout_time->addWidget(box_name,0,Qt::AlignTop);
    layout_time->addWidget(box_first,0,Qt::AlignTop);
    layout_time->addWidget(box_second,1,Qt::AlignTop);
    layout_time->addStretch(1);
    layout_time->addWidget(box_current,1,Qt::AlignBottom);
    layout_time->addWidget(box_end,0,Qt::AlignBottom);


    // RT_Scheme
    scheme_RTProgram = new Scheme_ProgramAmpl(QColor(0,0,0,0), this);
    rt_progress = new QProgressBar(this);
    rt_progress->setStyleSheet("QProgressBar:horizontal {border: 1px solid gray; border-radius: 2px; background: #E8E8E8; padding: 2px;}"
               "QProgressBar::chunk:horizontal {background: qlineargradient(x1: 0, y1: 0.1, x2: 1, y2: 0.1, stop: 0 #FFC4C4, stop: 1 red);}");
    rt_progress->setFixedHeight(12);
    rt_progress->setTextVisible(false);
    rt_progress->setVisible(true);

    QVBoxLayout *layout_scheme = new QVBoxLayout;
    layout_scheme->setSpacing(4);
    layout_scheme->setMargin(0);
    rt_Info->setLayout(layout_scheme);
    layout_scheme->addWidget(scheme_RTProgram);
    layout_scheme->addWidget(rt_progress);

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
RunTime::~RunTime()
{
    delete program_Name;
    delete box_name;

    delete time_first;
    delete label_first;
    delete box_first;

    delete time_second;
    delete label_second;
    delete box_second;

    delete current_label;
    delete current_cycle;
    delete box_current;

    delete end_label;
    delete end_cycle;
    delete box_end;

    delete scheme_RTProgram;
    delete rt_progress;

    delete time_Info;
    delete Scroll_time;
    delete rt_Info;
}
//-----------------------------------------------------------------------------
//--- readCommonSettings()
//-----------------------------------------------------------------------------
void RunTime::readCommonSettings()
{
    QString text;
    QString dir_SysTranslate;
    QString dir_path = qApp->applicationDirPath();
    QSettings *CommonSettings = new QSettings(dir_path + "/tools/ini/preference.ini", QSettings::IniFormat);

    CommonSettings->beginGroup("Common");

        //... Language ...
        text = CommonSettings->value("language","ru").toString();
        if(translator.load(":/translations/runtime_" + text + ".qm"))
        {
            qApp->installTranslator(&translator);
        }

        //... Style ...
        text = CommonSettings->value("style","xp").toString();
        StyleApp = text;

    CommonSettings->endGroup();
    delete CommonSettings;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void RunTime::XID_info(QString text, rt_Protocol* prot)
{
    QStringList list;
    int value;
    bool ok;
    qint64 diff_sec;
    int xid = 0;
    int sts = 0;
    int level = 0;
    int block = 0;

    sscanf(text.toStdString().c_str(),"%d %d %d", &block, &level, &sts);
    if(text != XID_state)
    {
        XID_state = text;
        xid = (block << 16) | (level << 8) | sts;
        scheme_RTProgram->draw_Program(prot, xid);

        if(block == 0) sts = false;

        if(sts)
        {
            rt_progress->setValue(0);
            //rt_progress->setVisible(true);
            time_Start = QDateTime::currentDateTime();

            if(block-1 >= prot->PrAmpl_timesInBlocks.size())
            {
                rt_progress->setValue(0);
                return;
            }

            text = QString::fromStdString(prot->PrAmpl_timesInBlocks.at(block-1));
            list = text.split(" ");

            if(level >= list.size()) {rt_progress->setValue(0); return;}
            text = list.at(level);
            value = text.toInt(&ok);
            if(ok) rt_progress->setMaximum(value);
            else rt_progress->setMaximum(100);
        }
        //else rt_progress->setVisible(false);
        else rt_progress->setValue(0);
    }
    else
    {
        if(sts)
        {
            diff_sec = time_Start.secsTo(QDateTime::currentDateTime());
            if(diff_sec >= 0)
            {
                if(diff_sec > rt_progress->maximum()) rt_progress->setValue(rt_progress->maximum());
                else rt_progress->setValue((int)diff_sec);
            }
        }
    }
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void RunTime::TIM_info(QString text, rt_Protocol* prot)
{
    QDateTime tim;
    QTime     tau;
    int tim_0 = 0;
    int tim_1 = 0;
    int tim_2 = 0;

    qint64 msec;

    sscanf(text.toStdString().c_str(),"%d %d %d", &tim_0, &tim_1, &tim_2);
    if(text != TIM_state)
    {
        TIM_state = text;
        tim = QDateTime::currentDateTime();

        // From start:
        msec = prot_Start.msecsTo(tim);
        tau = QTime::fromMSecsSinceStartOfDay(msec);
        list_Time.replace(0, tau.toString("HH:mm:ss"));

        // Till end
        tau = QTime::fromMSecsSinceStartOfDay(tim_0*1000);
        list_Time.replace(1, tau.toString("HH:mm:ss"));

        // Time of start
        list_Time.replace(2, prot_Start.toString("HH:mm:ss"));

        // End time        
        list_Time.replace(3, tim.addSecs(tim_0).toString("HH:mm:ss"));

        // Current time
        list_Time.replace(4, tim.toString("HH:mm:ss"));


        change_FirstTime(time_first->currentIndex());
        change_SecondTime(time_second->currentIndex());
    }

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void RunTime::XGS_info(QString text, rt_Protocol* prot)
{
    int i;

    int dev_status = 0;
    int current_block = 0;
    int current_repeat = 0;
    int current_vector = 0;

    QVector<int> vec_LS;
    int value;
    quint16 count_levels, count_cycles;
    int count = 0;
    int cur_cycle = 0;

    sscanf(text.toStdString().c_str(),"%d %d %d %d", &dev_status, &current_block, &current_repeat, &current_vector);

    if(dev_status == 0 || dev_status == 3)  // nothing, hold
    {
        current_cycle->setText("0");
        end_cycle->setText("0");
        return;
    }

    vec_LS = QVector<int>::fromStdVector(prot->PrAmpl_countLevelsCycles);
    for(i=0; i<vec_LS.size(); i++)
    {
        value = vec_LS.at(i);
        count_cycles = (short)(value & 0xff);
        count_levels = (short)((value & 0xff00) >> 16);

        if((current_block - 1) == i) cur_cycle = count + current_repeat + 1;

        if(count_cycles > 1) count += count_cycles;
    }

    current_cycle->setText(QString::number(cur_cycle));
    end_cycle->setText(QString::number(count - cur_cycle + 1));
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void RunTime::clear()
{

}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void RunTime::change_FirstTime(int id)
{
    label_first->setText(list_Time.at(id));
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void RunTime::change_SecondTime(int id)
{
    label_second->setText(list_Time.at(id));
}
