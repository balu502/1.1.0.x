#ifndef INFO_H
#define INFO_H

#include "info_global.h"

#include <QWindow>
#include <QApplication>
#include <QDialog>
#include <QtWidgets>
#include <QWidget>
#include <QTabWidget>
#include <QBoxLayout>
#include <QSplitter>
#include <QDebug>
#include <QList>
#include <QSettings>
#include <QTranslator>
#include <QTextEdit>

#include <protocol_information.h>

class INFOSHARED_EXPORT Info: public QDialog
{
    Q_OBJECT

public:
    Info(QWidget *parent = 0);
    ~Info();

    void refresh_Info(rt_Protocol*);

    QTabWidget  *TabInfo;

    Info_Protocol   *info_pro;
    QSplitter       *program_spl;
    Plot_ProgramAmpl *plot_Program;
    Scheme_ProgramAmpl  *scheme_Program;
    QTextEdit       *comments;

private:
    QTranslator translator;
    void readCommonSettings();

protected:

    void showEvent(QShowEvent *e);

};

#endif // INFO_H
