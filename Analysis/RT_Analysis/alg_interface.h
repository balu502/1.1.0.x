#ifndef ALG_INTERFACE
#define ALG_INTERFACE
/*
#include <QtCore/QObject>

class Select_Plot: public QEvent
{
public:
    Select_Plot(): QEvent((Type)1111)
    {
    }
    int pos_plot;
};
*/
class Alg_Interface
{
    //Q_OBJECT

public:

    virtual ~Alg_Interface(){};
    virtual void Destroy() = 0;
    virtual void *Create_Win(void*,void*,void*) = 0;
    virtual void Destroy_Win() = 0;
    virtual void Show() = 0;

    virtual void Analyser(rt_Protocol *prot, int param = 0x00) = 0;
    virtual void Select_Tube(int pos) = 0;
    virtual void Enable_Tube(QVector<short>*) = 0;

    virtual void *ReferenceForMap() = 0;

};

//Q_DECLARE_INTERFACE(Alg_Interface, "fr.inria.tstPlugin")

#endif // ALG_INTERFACE

