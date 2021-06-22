#ifndef VIDEO_GLOBAL_H
#define VIDEO_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(VIDEO_LIBRARY)
#  define VIDEOSHARED_EXPORT Q_DECL_EXPORT
#else
#  define VIDEOSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // VIDEO_GLOBAL_H
