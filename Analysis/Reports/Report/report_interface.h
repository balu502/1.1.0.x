#ifndef REPORT_INTERFACE_H
#define REPORT_INTERFACE_H

//#define FLUORS {"fam","hex","rox","cy5","cy5.5"}

#include "protocol.h"

class Report_Interface
{

public:

    virtual ~Report_Interface(){};

    virtual void* Create_Report(rt_Protocol *prot) = 0;
    virtual void Destroy_Report() = 0;
    virtual void Type_Report(int*, QString*) = 0;
    virtual void Set_SamplesEnable(QVector<short>*);
};

#endif // REPORT_INTERFACE_H
