#include "wait_process.h"

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Wait_Process::Wait_Process(QWidget *parent)
    : QWidget(parent)
{

}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Wait_Process::~Wait_Process()
{

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Wait_Process::wait(void *pobj, callback_WaitProcess *cb_fun, QString text)
{
    if(!cb_fun) return;

    int percent = 0;

    while(cb_fun(pobj, percent, text))
    {
        QCoreApplication::processEvents();
        QThread::msleep(100);
        percent += 5;

        if(percent > 100) percent = 0;
    }
    cb_fun(pobj, 0, "");
}
