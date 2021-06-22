#ifndef ALG_INTERFACE_QEVENT_H
#define ALG_INTERFACE_QEVENT_H

#include <QtCore/QObject>

//-------------------------------------
class Select_Plot: public QEvent
{
public:
    Select_Plot(): QEvent((Type)1111)
    {
    }
    int pos_plot;
};

//-------------------------------------
class Open_TechReport: public QEvent
{
public:
    Open_TechReport(): QEvent((Type)1112)
    {
    }
    QString report_dll;     // filename (dll)
};

#endif // ALG_INTERFACE_QEVENT_H

