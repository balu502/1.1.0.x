#ifndef QUANTITY_PCR_GLOBAL_H
#define QUANTITY_PCR_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(QUANTITY_PCR_LIBRARY)
#  define QUANTITY_PCRSHARED_EXPORT Q_DECL_EXPORT
#else
#  define QUANTITY_PCRSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // QUANTITY_PCR_GLOBAL_H
