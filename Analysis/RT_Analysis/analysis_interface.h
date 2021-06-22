#ifndef ANALYSIS_INTERFACE_H
#define ANALYSIS_INTERFACE_H

//#include <QWindow>
#include "protocol.h"

class Analysis_Interface
{

public:

    virtual ~Analysis_Interface(){};
    virtual void Destroy() = 0;
    virtual void *Create_Win(void*,void*) = 0;
    virtual void Destroy_Win() = 0;
    virtual void Show() = 0;

    virtual void GetInformation(QVector<QString> *info) = 0;
    virtual void Analyser(rt_Protocol *prot) = 0;
    virtual void Save_Results(char*) = 0;
    virtual void Select_Tube(int pos) = 0;
    virtual void Enable_Tube(QVector<short>*) = 0;
};

#endif // ANALYSIS_INTERFACE_H
