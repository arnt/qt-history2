/****************************************************************************
** $Id: $
**
** Declaration of the QAxWidget class
**
** Copyright (C) 2001-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Active Qt integration.
**
** Licensees holding valid Qt Enterprise Edition
** licenses for Windows may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
**
** This file is not available for use under any other license without
** express written permission from the copyright holder.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QAXWIDGET_H
#define QAXWIDGET_H

#include "qaxbase.h"
#include <qwidget.h>

struct IOleClientSite;

class QAX_EXPORT QAxWidget : public QWidget, public QAxBase
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

    QAxWidget( QWidget* parent = 0, const char* name = 0, WFlags f = 0 );
    QAxWidget( const QString &c, QWidget *parent = 0, const char *name = 0, WFlags f = 0 );
    QAxWidget( IUnknown *iface, QWidget *parent = 0, const char *name = 0, WFlags f = 0 );
    ~QAxWidget();

    void clear();

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

    void reparent( QWidget *parent, WFlags f, const QPoint &, bool showIt = FALSE );
    //void setAcceptDrops( bool on );
    //bool customWhatsThis() const;
    void setUpdatesEnabled( bool );
    bool event( QEvent * );

protected:
    bool initialize( IUnknown** );

    void enabledChange( bool old );
    void paletteChange( const QPalette &old );
    void fontChange( const QFont &old );
    void windowActivationChange( bool old );
    
    void resizeEvent( QResizeEvent * );

private:
    QMetaObject *parentMetaObject() const;
    void initContainer();
    QWidget *container;
    IOleClientSite *clientsite;
    IUnknown *host;
    QSize extent;
};

#endif // QAXWIDGET_H
