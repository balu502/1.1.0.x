#ifndef SPLASHSCREEN_H
#define SPLASHSCREEN_H

#include <QSplashScreen>
#include <QTimer>
#include <QApplication>
#include <QStyleOptionProgressBarV2>
#include <QProgressBar>
#include <QGroupBox>

#include <QWidget>
#include <QDialog>
#include <QSettings>
#include <QTranslator>
#include <QPushButton>
#include <QBoxLayout>
#include <QTabWidget>
#include <QGroupBox>
#include <QLabel>
#include <QComboBox>
#include <QIcon>
#include <QFont>
#include <QShowEvent>
#include <QDebug>
#include <QFontDialog>
#include <QTranslator>
#include <QFileInfo>
#include <QDir>
#include <QDateTime>

#include "windows.h"
#include <stdio.h>
#include <winver.h>
#include <Strsafe.h>


class SplashScreen : public QSplashScreen
{
    Q_OBJECT
public:
    SplashScreen(QApplication *app, QWidget *parent = 0, int param = 0);
    //virtual ~SplashScreen() {}

    int m_progress;
    QApplication *app;
    QProgressBar *progress;
    QGroupBox *box;
    QGroupBox *box_web;
    QLabel    *name;
    QLabel    *icon;
    QLabel    *web_address;
    QLabel    *limited;
    QLabel    *version;
    QLabel    *last_mod;
    QLabel    *logo;

    QLabel    *web_info;
    QLabel    *factory;

    QLabel    *load_Tests;

    void readCommonSettings();
    QString lang;
    int type_reagent;

    QString GetVersion();

    bool restart;
    QTimer *timer;

public slots:
    void setProgress()
    {
      m_progress += 5;
      if (m_progress > 100)
      {
          if(restart) m_progress = 0;
          else m_progress = 100;
      }
      if (m_progress < 0)
        m_progress = 0;
      update();
      app->processEvents();
      //qDebug() << "m_progress: " << m_progress;
    }

    void Get_SplashPercent(QString str);
    /*{
        //qDebug() << "get percent: " << str;
        load_Tests->setText(QString("%1: %2").arg(tr("load tests")).arg(str));
    }*/


protected:
    //void drawContents(QPainter *painter);

};

#endif // SPLASHSCREEN_H

