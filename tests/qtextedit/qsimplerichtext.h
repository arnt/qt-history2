/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qsimplerichtext.h#8 $
**
** Definition of the QSimpleRichText class
**
** Created : 990101
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.  This file is part of the kernel
** module and therefore may only be used if the kernel module is specified
** as Licensed on the Licensee's License Certificate.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QSIMPLERICHTEXT2_H
#define QSIMPLERICHTEXT2_H

#ifndef QT_H
#include "qnamespace.h"
#include "qstring.h"
#include "qregion.h"
#endif // QT_H

#ifndef QT_NO_RICHTEXT

class QPainter;
class QWidget;
class QStyleSheet;
class QBrush;
class QMimeSourceFactory;
class QSimpleRichTextData;


class Q_EXPORT QSimpleRichText
{
public:
    QSimpleRichText( const QString& text, const QFont& fnt,
		     const QString& context = QString::null, const QStyleSheet* sheet = 0);
    QSimpleRichText( const QString& text, const QFont& fnt,
		     const QString& context,  const QStyleSheet* sheet, 
		     const QMimeSourceFactory* factory, int verticalBreak = -1,
		     const QColor& linkColor = Qt::blue, bool linkUnderline = TRUE );
    ~QSimpleRichText();

    void setWidth( int );
    void setWidth( QPainter*, int );
    int width() const;
    int widthUsed() const;
    int height() const;
    void adjustSize();

    void draw( QPainter*,  int x, int y, const QRegion& clipRegion,
	       const QPalette& pal, const QBrush* paper = 0) const;

    void draw( QPainter*,  int x, int y, const QRegion& clipRegion,
	       const QColorGroup& cg, const QBrush* paper = 0) const;

    QString context() const;
    QString anchorAt( const QPoint& pos ) const;
    QString anchor( QPainter* p, const QPoint& pos ); // remove in 3.0
    
    bool inText( const QPoint& pos ) const;

private:
    QSimpleRichTextData* d;
};

#endif // QT_NO_RICHTEXT

#endif // QSIMPLERICHTEXT_H
