#ifndef SAVER_H
#define SAVER_H

#include "saver_global.h"

#include <QtWidgets>
#include <QWidget>
#include <QMainWindow>
#include <QGroupBox>
#include <QPushButton>
#include <QBoxLayout>
#include <QPainter>
#include <QTabWidget>
#include <QTextBrowser>
#include <QTextEdit>
#include <QFile>
#include <QEvent>
#include <QTextStream>
#include <QLabel>
#include <QSettings>
#include <QTranslator>
#include <QUrl>


class SAVERSHARED_EXPORT Saver : public QMainWindow
{
    Q_OBJECT

public:
    Saver(QWidget *parent = 0);
    ~Saver();

    void readCommonSettings();
    QTranslator translator;

private:

    QGroupBox   *MainGroupBox;        // Main Box
    QTabWidget  *TabHelp;    
    QLabel      *main_Idea;
    QTextBrowser *Browser;


    void Load_HTML();

protected:

    bool event(QEvent *e);
};

#endif // SAVER_H

