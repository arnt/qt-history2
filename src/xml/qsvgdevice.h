/****************************************************************************
** $Id: //depot/qt/main/src/xml/qsvgdevice.h#14 $
**
** Definition of the QSVGDevice class
**
** Created : 20001024
**
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
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

#include <qmodules.h>

#if !defined(QT_MODULE_XML)
#define QM_EXPORT
#else
#define QM_EXPORT Q_EXPORT
#endif

#ifndef QT_H
#include "qpaintdevice.h"
#include "qrect.h"
#include "qdom.h"
#endif // QT_H

#ifndef QT_NO_SVG

class QPainter;
class QDomNode;
class QDomNamedNodeMap;
class QSVGDevicePrivate;

class Q_EXPORT QSVGDevice : public QPaintDevice
{
public:
    QSVGDevice();
    ~QSVGDevice();

    bool load( const QString& );
    bool play( QPainter *p );

    QString toString() const;
    bool save( const QString& );

    QRect boundingRect() const;

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
    void applyStyle( QDomElement *e ) const;
    void applyTransform( QDomElement *e ) const;

    enum ElementType {
	InvalidElement = 0,
	CommentElement,
	RectElement,
	CircleElement,
	EllipseElement,
	LineElement,
	PolylineElement,
	PolygonElement,
	PathElement,
	TextElement,
	ImageElement,
	GroupElement
    };

    // reading
    QRect brect;	    		// bounding rectangle
    QDomDocument doc;			// document tree
    QDomNode current;
    QPoint curPt;
    QPainter *pt;			// used by play() et al

    // writing
    int imageCount;			// incremental counter for ext. images
    QString svgName;			// name of the SVG document
    bool dirtyTransform, dirtyStyle;

    QSVGDevicePrivate *d;
};

inline QRect QSVGDevice::boundingRect() const
{
    return brect;
}

#endif // QT_NO_SVG

#endif // QSVGDEVICE_H

