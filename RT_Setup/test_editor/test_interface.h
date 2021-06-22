#ifndef TEST_INTERFACE_H
#define TEST_INTERFACE_H

#include <QWindow>

class Test_Interface
{

public:

    virtual ~Test_Interface(){};
    virtual void Destroy() = 0;
    virtual void *Create_Win(void*,void*) = 0;
    virtual void Destroy_Win() = 0;
    virtual void Show() = 0;

    //virtual void GetInformation(std::vector<std::string> *info) = 0;
    virtual void GetInformation(QVector<QString> *info) = 0;
    virtual void Set_Test(std::string xml, std::string name) = 0;
    virtual void Get_Test(std::string *xml, std::string *name) = 0;

    virtual void Disable_Action() = 0;

    virtual void Reserve(void*) = 0;

};


#endif // TEST_INTERFACE_H
