#ifndef QACTIVEQT_H
#define QACTIVEQT_H

#include <qwidget.h>

#ifndef NOQT_ACTIVEX
#include <quuid.h>
#ifndef __IID_DEFINED__
#define __IID_DEFINED__
typedef GUID IID;
#endif

#define __IID_DEFINED__

#if defined QT_ACTIVEXIMPL
#define QT_ACTIVEX( Class, IIDClass, IIDInterface, IIDEvents, IIDTypeLib, IIDApp ) \
    extern "C" const IID IID_QAxClass = QUuid(IIDClass); \
    extern "C" const IID IID_QAxInterface = QUuid(IIDInterface); \
    extern "C" const IID IID_QAxEvents = QUuid(IIDEvents); \
    extern "C" const IID IID_QAxTypeLib = QUuid(IIDTypeLib); \
    extern "C" const IID IID_QAxApp = QUuid(IIDApp); \
    QActiveQt *qt_activeqt( QWidget *parent,const char *name,Qt::WFlags f ) { return new Class(parent,name,f); } \
    QMetaObject *qt_activeqt_meta() { return Class::staticMetaObject(); } \

#else
#define QT_ACTIVEX( Class, IIDClass, IIDInterface, IIDEvents, IIDTypeLib, IIDApp ) \
    extern "C" const IID IID_QAxClass; \
    extern "C" const IID IID_QAxInterface; \
    extern "C" const IID IID_QAxEvents; \
    extern "C" const IID IID_QAxTypeLib; \
    extern "C" const IID IID_QAxApp; \

#endif
#endif

class QActiveQtBase;

class QActiveQt : public QWidget
{
    friend class QActiveQtBase;
public:
    QActiveQt( QWidget *parent, const char *name, WFlags f );

protected:
    bool requestPropertyChange( const char *property );
    void propertyChanged( const char *property );

private:
    QActiveQtBase *activex;
};

#endif // QACTIVEQT_H
