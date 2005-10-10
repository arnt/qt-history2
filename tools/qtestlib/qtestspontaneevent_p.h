#ifndef QTESTSPONTANEEVENT_P_H
#define QTESTSPONTANEEVENT_P_H

#include <QtCore/qglobal.h>
#include <QtCore/qcoreevent.h>

#ifndef QTEST_NO_SIZEOF_CHECK
template <int>
class QEventSizeOfChecker
{
private:
    QEventSizeOfChecker() {}
};

template <>
class QEventSizeOfChecker<sizeof(QEvent)>
{
public:
    QEventSizeOfChecker() {}
};
#endif

class QSpontaneKeyEvent
{
public:
    void setSpontaneous() { spont = 1; };
    bool spontaneous() { return spont; };
    virtual void dummyFunc() {  };
    virtual ~QSpontaneKeyEvent() {}

#ifndef QTEST_NO_SIZEOF_CHECK
    inline void ifYouGetCompileErrorHereYouUseWrongQt()
    {
        // this is a static assert in case QEvent changed in Qt
        QEventSizeOfChecker<sizeof(QSpontaneKeyEvent)> dummy;
    }
#endif

protected:
    void *d;
    ushort t;

private:
    ushort posted : 1;
    ushort spont : 1;
    ushort m_accept : 1;
    ushort reserved : 13;
};

#endif

