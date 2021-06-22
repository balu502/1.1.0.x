#ifndef TEST_HRM_GLOBAL_H
#define TEST_HRM_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(TEST_HRM_LIBRARY)
#  define TEST_HRMSHARED_EXPORT Q_DECL_EXPORT
#else
#  define TEST_HRMSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // TEST_HRM_GLOBAL_H
