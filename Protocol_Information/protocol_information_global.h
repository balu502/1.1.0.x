#ifndef PROTOCOL_INFORMATION_GLOBAL_H
#define PROTOCOL_INFORMATION_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(PROTOCOL_INFORMATION_LIBRARY)
#  define PROTOCOL_INFORMATIONSHARED_EXPORT Q_DECL_EXPORT
#else
#  define PROTOCOL_INFORMATIONSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // PROTOCOL_INFORMATION_GLOBAL_H
