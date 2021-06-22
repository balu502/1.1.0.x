#include "worker_wait.h"

//-----------------------------------------------------------------------------
//--- Constructor
//-----------------------------------------------------------------------------
Worker_wait::Worker_wait(Wait_state w_st, QString label, QObject *parent) : QObject(parent)
{
    Label = label;
    count = 0;
    state = w_st;
}
//-----------------------------------------------------------------------------
//--- Destructor
//-----------------------------------------------------------------------------
Worker_wait::~Worker_wait()
{
    //qDebug() << "~Worker_wait()";
}

//-----------------------------------------------------------------------------
//--- start_process()
//-----------------------------------------------------------------------------
void Worker_wait::start_process()
{    
    emit cursor_wait(true);
}
//-----------------------------------------------------------------------------
//--- stop_process()
//-----------------------------------------------------------------------------
void Worker_wait::stop_process()
{
    emit cursor_wait(false);
    emit finished();   
    emit display_process(0, "");
}
//-----------------------------------------------------------------------------
//--- slot_timeout()
//-----------------------------------------------------------------------------
void Worker_wait::slot_timeout()
{
    switch (state)
    {
    case wRun:
    case wStop:
    case wOpen:
    case wClose:
    case wPreRun:
    case wPic:
                    count += 5;
                    if(count > 100) count = 0;
                    emit display_process(count, Label);

                    break;

    case wMeasure:
    case wCmd:
                    break;
    default:
                    break;
    }


}


