#ifndef IEWIDGET_H
#define IEWIDGET_H

#include <qwidget.h>

struct IUnknown;
class QAxEventSink;

class QActiveXBase : public QWidget
{
    Q_OBJECT
    Q_PROPERTY( QString control READ control WRITE setControl )

    friend class QAxEventSink;
public:
    QActiveXBase( QWidget *parent = 0, const char *name = 0 );
    ~QActiveXBase();

    QString control() const;

public slots:
    virtual void clear();
    void invoke( const QString& );
    void setControl( const QString& );

signals:
    void signal( const QString &, int argc, void *argv );

protected:
    IUnknown *ptr;
    QAxEventSink *eventSink;

private:
    void initialize();
    
    QString ctrl;
};

class QMetaObject;

class QActiveX : public QActiveXBase
{
public:
    QMetaObject *metaObject() const;
    static QMetaObject *staticMetaObject();
    const char *className() const;
    void* qt_cast( const char* );
    bool qt_invoke( int, QUObject* );
    bool qt_emit( int, QUObject* );
    bool qt_property( int, int, QVariant* );
    QObject* qObject() { return (QObject*)this; }

    QActiveX( QWidget* parent = 0, const char* name = 0 );
    QActiveX( const QString &c, QWidget *parent = 0, const char *name = 0 );
    ~QActiveX();

    void clear();

private:
    QMetaObject *metaObj;
};

#endif //IEWIDGET_H
