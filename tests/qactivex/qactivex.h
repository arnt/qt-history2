#ifndef QACTIVEX_H
#define QACTIVEX_H

#include <qwidget.h>
#include "qcomobject.h"

class QClientSite;

class QCOM_EXPORT QActiveX : public QWidget, public QComBase
{
    friend class QClientSite;
public:
    QMetaObject *metaObject() const;
    const char *className() const;
    void* qt_cast( const char* );
    bool qt_invoke( int, QUObject* );
    bool qt_emit( int, QUObject* );
    bool qt_property( int, int, QVariant* );
    QObject* qObject() { return (QObject*)this; }

    QActiveX( QWidget* parent = 0, const char* name = 0 );
    QActiveX( const QString &c, QWidget *parent = 0, const char *name = 0);
    ~QActiveX();

    void clear();

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

    //void reparent( QWidget *parent, WFlags f, const QPoint &, bool showIt = FALSE );
    //void setAcceptDrops( bool on );
    //bool customWhatsThis() const;
    void setUpdatesEnabled( bool );

protected:
    void enabledChange( bool old );
    void paletteChange( const QPalette &old );
    void fontChange( const QFont &old );
    void windowActivationChange( bool old );

private:
    bool initialize( IUnknown** );
    QMetaObject *parentMetaObject() const;

    QClientSite *clientsite;
    QSize extent;
};

#endif //QACTIVEX_H
