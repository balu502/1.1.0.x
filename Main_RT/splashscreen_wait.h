#ifndef SPLASHSCREEN_WAIT_H
#define SPLASHSCREEN_WAIT_H

#include <QObject>
#include <QWidget>
#include <QSplashScreen>
#include <QTimer>
#include <QApplication>
#include <QMovie>
#include <QLabel>

class SplashScreen_Wait: public QSplashScreen
{
    Q_OBJECT
public:
    SplashScreen_Wait(QApplication *app, QWidget *parent = 0);

    QApplication *app;
    QLabel *label_gif;
    QMovie *obj_gif;

    QTimer *timer;

public slots:
    void slot_timeout();
};

#endif // SPLASHSCREEN_WAIT_H
