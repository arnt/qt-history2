/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qsimpletextdocument.cpp#3 $
**
** Implementation of the QSimpleTextDocument class
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

#include "qsimpletextdocument.h"
#include "qrichtextintern.cpp"


/*!
  \class QSimpleTextDocument qsimpletextdocument.h
  \brief A small displayable piece of rich text.

  This class encapsulates simple richt text usage where a string is interpretted
  as richt text and can be drawn.

  For large documents, see QTextView or QTextBrowser.
*/

class QSimpleTextDocumentData
{
public:
    QTextDocument* doc;
};

/*!
  Constructs a QSimpleTextDocument from the richt text  \a contents.

  If \a w is not 0, its properties (font, etc.) are used to set
  default properties for the display. No reference is kept to the
  widget. \a s is an optional stylesheet. If it is 0, the
  QStyleSheet::defaultSheet() will be used.

  Once created, changes cannot be made (just throw it away and make
  a new one with the new contents).
*/
QSimpleTextDocument::QSimpleTextDocument( const QString& contents, const QWidget* w, const QStyleSheet* s)
{
    d  = new QSimpleTextDocumentData;
    d->doc = new QTextDocument( contents, w, 0, 0, s );
}

/*!
  Destructs the document, freeing memory.
*/
QSimpleTextDocument::~QSimpleTextDocument()
{
    delete d->doc;
    delete d;
}

/*!
  Sets the width of the document to \a w pixels, recalculating the layout
  as if it were to be drawn with \a p.  ####### QPaintDevice

  \sa height()
*/
void QSimpleTextDocument::setWidth( QPainter* p, int w)
{
    d->doc->setWidth( p, w );
}

/*!
  Returns the set width of the document, in pixels.
  
  \sa widthUsed()
*/
int QSimpleTextDocument::width() const
{
    return d->doc->width;
}


/*!
  Returns the width in pixels that is actually used by the document.
  This can be smaller or wider than the set width.
  
  \sa width()
 */
int QSimpleTextDocument::widthUsed() const
{
    return d->doc->widthUsed;
}


/*!
  Returns the height of the document, in pixels.
  \sa setWidth()
*/
int QSimpleTextDocument::height() const
{
    return d->doc->height;
}

/*!
  Draws the formatted text with \a p, at position (\a x,\a y), clipped to
  \a clipRegion.  Colors from \a cg are used as needed, and if not 0,
  *\a paper is used as the background brush.

  Note that the display code is highly optimized to reduce flicker, so
  passing a brush for \a paper is preferrable to simply clearing the area
  to be painted and then calling this without a brush.
*/
void QSimpleTextDocument::draw( QPainter* p,  int x, int y, const QRegion& clipRegion,
			      const QColorGroup& cg, const QBrush* paper) const
{
    QRect r = clipRegion.boundingRect();
    QRegion bg = clipRegion;

    d->doc->draw(p, x, y, 0, 0, r.x(), r.y(), r.width(), r.height(), bg, cg, paper);
    if (paper) {
	p->setClipRegion(bg);
	if ( paper->pixmap() )
	    p->drawTiledPixmap( r, *paper->pixmap());
	else
	    p->fillRect(r, *paper);
	p->setClipping( FALSE );
    }
}
