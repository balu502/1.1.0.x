#ifndef RDML_GLOBAL_H
#define RDML_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(RDML_LIBRARY)
#  define RDML_EXPORT Q_DECL_EXPORT
#else
#  define RDML_EXPORT Q_DECL_IMPORT
#endif

#endif // RDML_GLOBAL_H
