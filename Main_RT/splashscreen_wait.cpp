#include "splashscreen_wait.h"

SplashScreen_Wait::SplashScreen_Wait(QApplication *aApp, QWidget *parent) :
    QSplashScreen(parent), app(aApp)
{

    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint | Qt::Dialog);
    setCursor(Qt::BusyCursor);

    label_gif = new QLabel(this);
    label_gif->setFixedSize(48,48);
    obj_gif = new QMovie(":/images/flat/wait_1.gif");
    label_gif->setMovie(obj_gif);
    label_gif->setVisible(true);
    obj_gif->start();

    timer = new QTimer();
    connect(timer,SIGNAL(timeout()),this,SLOT(slot_timeout()));
    timer->start(100);

}
//-----------------------------------------------------------------------------
//--- slot_timeout()
//-----------------------------------------------------------------------------
void SplashScreen_Wait::slot_timeout()
{
    repaint();
    update();
    app->processEvents();
}
