/****************************************************************************
**
** Copyright (C) 2001-2003 Trolltech AS. All rights reserved.
**
** This file is part of the Active Qt integration.
** EDITIONS: ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QAXWIDGET_H
#define QAXWIDGET_H

#include "qaxbase.h"
#include <qwidget.h>

class QAxHostWindow;

class QAxWidget : public QWidget, public QAxBase
{
public:
    const QMetaObject *metaObject() const;
    void* qt_metacast( const char* ) const;
    int qt_metacall(QMetaObject::Call, int, void **);
    QObject* qObject() const { return (QWidget*)this; }
    const char *className() const;

    QAxWidget( QWidget* parent = 0, const char* name = 0, WFlags f = 0 );
    QAxWidget( const QString &c, QWidget *parent = 0, const char *name = 0, WFlags f = 0 );
    QAxWidget( IUnknown *iface, QWidget *parent = 0, const char *name = 0, WFlags f = 0 );
    ~QAxWidget();

    void clear();

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

protected:
    bool initialize( IUnknown** );
    virtual bool createHostWindow( bool );

    void enabledChange( bool old );
    void paletteChange( const QPalette &old );
    void fontChange( const QFont &old );
    void windowActivationChange( bool old );

    void resizeEvent( QResizeEvent * );
    virtual bool translateKeyEvent(int message, int keycode) const;
private:
    friend class QAxHostWindow;

    const QMetaObject *parentMetaObject() const;

    QAxHostWindow *container;
};

#endif // QAXWIDGET_H
