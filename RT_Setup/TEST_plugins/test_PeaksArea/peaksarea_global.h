#ifndef PEAKSAREA_GLOBAL_H
#define PEAKSAREA_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(PEAKSAREA_LIBRARY)
#  define PEAKSAREASHARED_EXPORT Q_DECL_EXPORT
#else
#  define PEAKSAREASHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // PEAKSAREA_GLOBAL_H
