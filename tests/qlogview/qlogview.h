/****************************************************************************
** $Id: $
**
** Definition of the QLogView class
**
** Created : 011113
**
** Copyright (C) 1992-2001 Trolltech AS.  All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
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
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
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
**********************************************************************/

#ifndef QLOGVIEW_H
#define QLOGVIEW_H

#ifndef QT_H
#include "qscrollview.h"
#include "qstring.h"
#include "qregexp.h"
#endif // QT_H

#ifndef QT_NO_LOGVIEW
class QLogViewPrivate;

class QLogView : public QScrollView
{
    Q_OBJECT
public:
    QLogView( const QString & txt, QWidget * parent = 0,
	      const char * name = 0 );
    QLogView( QWidget * parent = 0, const char * name = 0 );
    virtual ~QLogView();
    
    QString text() const;
    virtual void setText( const QString & txt );
    int     length();
    virtual void append( const QString & str );
    virtual int  find( const QRegExp & reg );
    
protected:
    void fontChange( const QFont & oldFont );
    void drawContents( QPainter * p, int clipx, int clipy, int clipw,
		       int cliph );    
private:
    void 	init();
    QStringList lineRange( int startLine, int numLines ) const;
    QColor 	lineColor( QString & str ) const;
    
    QLogViewPrivate * d;
};

#endif // QT_NO_LOGVIEW
#endif // QLOGVIEW_H
