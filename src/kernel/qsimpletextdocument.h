/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qsimpletextdocument.h#2 $
**
** Definition of the QSimpleTextDocument class
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

#ifndef QSIMPLETEXTDOCUMENT_H
#define QSIMPLETEXTDOCUMENT_H

#include "qnamespace.h"
#include "qstring.h"
#include "qregion.h"
class QPainter;
class QWidget;
class QStyleSheet;
class QBrush;


class QSimpleTextDocumentData;

class Q_EXPORT QSimpleTextDocument
{
public:
    QSimpleTextDocument( const QString& contents, const QWidget* w = 0, const QStyleSheet* s = 0);
    ~QSimpleTextDocument();

    void setWidth( QPainter*, int );
    int width() const;
    
    int widthUsed() const;
    
    int height() const;

    void draw( QPainter*,  int x, int y, const QRegion& clipRegion,
	       const QColorGroup& cg, const QBrush* paper = 0) const;

private:
    QSimpleTextDocumentData* d;
};


#endif
