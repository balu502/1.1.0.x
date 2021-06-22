#ifndef RUN_GLOBAL_H
#define RUN_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(RUN_LIBRARY)
#  define RUNSHARED_EXPORT Q_DECL_EXPORT
#else
#  define RUNSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // RUN_GLOBAL_H
