/****************************************************************************
** $Id: //depot/qt/main/src/xml/qsvgdevice_p.h#4 $
**
** Definition of the QSvgDevice class
**
** Created : 20001024
**
** Copyright (C) 2000-2001 Trolltech AS.  All rights reserved.
**
** This file is part of the XML module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition licenses may use this
** file in accordance with the Qt Commercial License Agreement provided
** with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
*****************************************************************************/

#ifndef QSVGDEVICE_H
#define QSVGDEVICE_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of the QPicture class. This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//
//

#ifndef QT_H
#include "qpaintdevice.h"
#include "qrect.h"
#include "qdom.h"
#endif // QT_H

#include "qfeatures.h"

#if !defined(QT_MODULE_XML)
#define QM_EXPORT
#else
#define QM_EXPORT Q_EXPORT
#endif

#ifndef QT_NO_SVG

class QPainter;
class QDomNode;
class QDomNamedNodeMap;

class Q_EXPORT QSvgDevice : public QPaintDevice
{
public:
    QSvgDevice();
    ~QSvgDevice();

    bool play( QPainter *p );

    QString toString() const;

    bool load( const QString &fileName );
    bool save( const QString &fileName );

    QRect boundingRect() const;
    void setBoundingRect( const QRect &r );

protected:
    virtual bool cmd ( int, QPainter*, QPDevCmdParam* );
    virtual int	 metric( int ) const;

private:
    // reading
    bool play( const QDomNode &node );
    QColor parseColor( const QString &col );
    double parseLen( const QString &str, bool *ok=0 ) const;
    int lenToInt( const QDomNamedNodeMap &map, const QString &attr,
		  int def=0 ) const;
    void setStyle( const QString &s );
    void setTransform( const QString &tr );
    void drawPath( const QString &data );

    // writing
    void applyStyle( QDomElement *e, int c ) const;
    void applyTransform( QDomElement *e ) const;

    // reading
    QRect brect;			// bounding rectangle
    QDomDocument doc;			// document tree
    QDomNode current;
    QPoint curPt;
    QPainter *pt;			// used by play() et al

    // writing
    bool dirtyTransform, dirtyStyle;

    class Private;
    Private *d;
};

inline QRect QSvgDevice::boundingRect() const
{
    return brect;
}

#endif // QT_NO_SVG

#endif // QSVGDEVICE_H
