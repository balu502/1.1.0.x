#ifndef RUNTIME_H
#define RUNTIME_H

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
#include <QComboBox>
#include <QPalette>
#include <QProgressBar>

#include "protocol.h"
#include "define_PCR.h"
#include "protocol_information.h"

#include "runtime_global.h"

//-----------------------------------------------------------------------------
class RUNTIMESHARED_EXPORT RunTime: public QGroupBox
{
    Q_OBJECT

public:
    RunTime(QWidget *parent = NULL);
    virtual ~RunTime();

    QString XID_state;
    QString TIM_state;
    QDateTime prot_Start;
    QStringList list_Time;

    void readCommonSettings();
    QTranslator translator;
    QString StyleApp;

//private:
    QScrollArea *Scroll_time;
    QGroupBox   *time_Info;
    QGroupBox   *rt_Info;

    // Program name
    QLabel      *program_Name;
    QGroupBox   *box_name;

    // time    
    QComboBox   *time_first;
    QLabel      *label_first;
    QGroupBox   *box_first;

    QComboBox   *time_second;
    QLabel      *label_second;
    QGroupBox   *box_second;

    QLabel      *current_label;
    QLabel      *current_cycle;
    QGroupBox   *box_current;

    QLabel      *end_label;
    QLabel      *end_cycle;
    QGroupBox   *box_end;

    // RT
    Scheme_ProgramAmpl  *scheme_RTProgram;
    QProgressBar        *rt_progress;
    QDateTime           time_Start;

    // methods
    void XID_info(QString,rt_Protocol*);
    void TIM_info(QString,rt_Protocol*);
    void XGS_info(QString,rt_Protocol*);

    void clear();

private slots:
    void change_FirstTime(int);
    void change_SecondTime(int);

};

#endif // RUNTIME_H
