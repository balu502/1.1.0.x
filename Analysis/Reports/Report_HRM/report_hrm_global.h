#ifndef REPORT_HRM_GLOBAL_H
#define REPORT_HRM_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(REPORT_HRM_LIBRARY)
#  define REPORT_HRMSHARED_EXPORT Q_DECL_EXPORT
#else
#  define REPORT_HRMSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // REPORT_HRM_GLOBAL_H
