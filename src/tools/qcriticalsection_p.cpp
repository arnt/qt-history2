#if defined(QT_THREAD_SUPPORT)

#include "qt_windows.h"

#include <private/qcriticalsection_p.h>

class QCriticalSectionPrivate 
{
public:
    QCriticalSectionPrivate() {}

    CRITICAL_SECTION section;
};


QCriticalSection::QCriticalSection()
{
    d = new QCriticalSectionPrivate;
    InitializeCriticalSection( &d->section );
}

QCriticalSection::~QCriticalSection()
{
    DeleteCriticalSection( &d->section );
    delete d;
}

void QCriticalSection::enter()
{
    EnterCriticalSection( &d->section );
}

void QCriticalSection::leave()
{
    LeaveCriticalSection( &d->section );
}

#endif
