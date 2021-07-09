#include "rdml.h"

extern "C" RDML_EXPORT bool __stdcall RDML_2_RT(char* p_str)
{
    QString text(p_str);

    qDebug() << "rdml.dll: " << text;

    return(true);
}

RDML::RDML()
{
}
