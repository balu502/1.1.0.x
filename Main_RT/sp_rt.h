#ifndef SPLASHSCREEN_RT_H
#define SPLASHSCREEN_RT_H

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
#include <QLabel>
#include <QMovie>

#include "windows.h"
#include <stdio.h>
#include <winver.h>
#include <Strsafe.h>

class SplashScreen_RT: public QSplashScreen
{
    Q_OBJECT

public:
    SplashScreen_RT(QApplication *app, QWidget *parent = 0, int param = 0);

    QApplication *app;
    int m_progress;

    void readCommonSettings();
    QString lang;

    QLabel  *name;
    QLabel  *animation;
    QMovie  *gif;
    QLabel  *PC;
    QLabel  *User;
    QLabel  *factory;
    QLabel  *email;
    QLabel  *www;
    QLabel  *version;
    QLabel  *logo;



    QString GetVersion();




};

#endif // SPLASHSCREEN_RT_H
