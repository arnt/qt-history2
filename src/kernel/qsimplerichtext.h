/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qsimplerichtext.h#8 $
**
** Definition of the QSimpleRichText class
**
** Created : 990101
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QSIMPLERICHTEXT_H
#define QSIMPLERICHTEXT_H

#ifndef QT_H
#include "qnamespace.h"
#include "qapplication.h"
#include "qstring.h"
#include "qregion.h"
#endif // QT_H
class QPainter;
class QWidget;
class QStyleSheet;
class QBrush;


class QSimpleRichTextData;

class Q_EXPORT QSimpleRichText
{
public:
    QSimpleRichText( const QString& text, const QFont& fnt,
		     const QString& context = QString::null, const QStyleSheet* s = 0);
    ~QSimpleRichText();

    void setWidth( QPainter*, int );
    int width() const;

    int widthUsed() const;

    int height() const;

    void adjustSize( QPainter* p );

    void draw( QPainter*,  int x, int y, const QRegion& clipRegion,
	       const QPalette& pal, const QBrush* paper = 0) const;

    void draw( QPainter*,  int x, int y, const QRegion& clipRegion,
	       const QColorGroup& cg, const QBrush* paper = 0) const;

    QString context() const;
    QString anchor( QPainter* p, const QPoint& pos );

private:
    QSimpleRichTextData* d;
};


#endif
