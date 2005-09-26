#ifndef QTESTSPONTANEEVENT_P_H
#define QTESTSPONTANEEVENT_P_H

#include <QtCore/qglobal.h>
#include <QtCore/qcoreevent.h>

#if defined Q_CC_HPACC
// aCC 3.37 complains about the template syntax in this
// file. It doesn't complain in Qt itself, though. (?)
#define QTEST_NO_SIZEOF_CHECK
#endif

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

    inline void ifYouGetCompileErrorHereYouUseWrongQt()
    {
        // this is a static assert in case QEvent changed in Qt
        QEventSizeOfChecker<sizeof(QSpontaneKeyEvent)> dummy;
    }

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

