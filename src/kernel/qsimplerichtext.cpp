/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qsimplerichtext.cpp#5 $
**
** Implementation of the QSimpleRichText class
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

#include "qsimplerichtext.h"
#include "qrichtextintern.cpp"


/*!
  \class QSimpleRichText qsimplerichtext.h
  \brief A small displayable piece of rich text.

  This class encapsulates simple richt text usage where a string is
  interpretted as richt text and can be drawn. This is in particular
  useful if you want to display some rich text in a custom widget.

  Once created, the rich text object can be queried for its width(),
  height() and the actual width used ( see widthUsed() ). And, most
  certainly, it can be drawn on any given QPainter with draw().  By
  using anchor(), it is possible to use QSimpleRichText to implement
  some kind of hypertext or active text facilities as well.

  Changes other than resizing with setWidth() cannot be made. If the
  contents is supposed to change, just throw the rich text object away
  and make a new one with the new contents.

  For large documents, see QTextView or QTextBrowser.
*/

class QSimpleRichTextData
{
public:
    QRichText* doc;
};

/*!
  Constructs a QSimpleRichText from the rich text string  \a text.

  If a font \a fnt is specified, this font will be used as basis for
  the text rendering. When using rich text rendering on a certain
  widget \e w, you would regularily specify the widget's font as shown in
  the following code example:

  \code
  QSimpleRichText* t = new QSimpleRichText( contents, w->font() );
  \endcode

  If not font has been specified, the application's default font is used.

  \a context is the optional context of the document. This becomes
  important if \a text contains relative references, for example
  within image tags. QSimpleRichText always uses the default mime
  source factory (see QMimeSourceFactory::defaultFactory() ) to
  resolve those references. The context will then be used to calculate
  the absolute path. See QMimeSourceFactory::makeAbsolute() for
  details.

  \a s is an optional stylesheet. If it is 0, the default style sheet
  will be used (see QStyleSheet::defaultSheet() ).

*/
QSimpleRichText::QSimpleRichText( const QString& text, const QFont& fnt,
				  const QString& context, const QStyleSheet* s)
{
    d  = new QSimpleRichTextData;
    d->doc = new QRichText( text, fnt, context, 0, 0, s );
}

/*!
  Destructs the document, freeing memory.
*/
QSimpleRichText::~QSimpleRichText()
{
    delete d->doc;
    delete d;
}

/*!
  Sets the width of the document to \a w pixels, recalculating the layout
  as if it were to be drawn with \a p.

  \sa height()
*/
void QSimpleRichText::setWidth( QPainter* p, int w)
{
    d->doc->setWidth( p, w );
}

/*!
  Returns the set width of the document, in pixels.

  \sa widthUsed()
*/
int QSimpleRichText::width() const
{
    return d->doc->width;
}


/*!
  Returns the width in pixels that is actually used by the document.
  This can be smaller or wider than the set width.

  It may be wider, for example, if the text contains images or
  non-breakable words that are already wider than the available
  space. It's smaller when the document only consists of lines that do
  not fill the width completely.

  \sa width()
*/
int QSimpleRichText::widthUsed() const
{
    return d->doc->widthUsed;
}


/*!
  Returns the height of the document, in pixels.
  \sa setWidth()
*/
int QSimpleRichText::height() const
{
    return d->doc->height;
}


/*!
  Draws the formatted text with \a p, at position (\a x,\a y), clipped
  to \a clipRegion.  Colors from the palette \a pal are used as
  needed, and if not 0, *\a paper is used as the background brush.

  Note that the display code is highly optimized to reduce flicker, so
  passing a brush for \a paper is preferrable to simply clearing the area
  to be painted and then calling this without a brush.
*/
void QSimpleRichText::draw( QPainter* p,  int x, int y, const QRegion& clipRegion,
	   const QPalette& pal, const QBrush* paper ) const
{
    draw( p, x, y, clipRegion, pal.normal(), paper );
}

/*!
  Draws the formatted text with \a p, at position (\a x,\a y), clipped
  to \a clipRegion.  Colors from the \a cg are used as
  needed, and if not 0, *\a paper is used as the background brush.

  Note that the display code is highly optimized to reduce flicker, so
  passing a brush for \a paper is preferrable to simply clearing the area
  to be painted and then calling this without a brush.

  This is a convenience function if there's no palette but just a
  cologroup available. If you have a palette, pass this instead of \a cg.
*/
void QSimpleRichText::draw( QPainter* p,  int x, int y, const QRegion& clipRegion,
			      const QColorGroup& cg, const QBrush* paper) const
{
    QRect r = clipRegion.boundingRect();
    QRegion bg = clipRegion;

    d->doc->draw(p, x, y, 0, 0, r.x(), r.y(), r.width(), r.height(), bg, cg, QTextOptions( paper ) );
    if (paper) {
	p->setClipRegion(bg);
	if ( paper->pixmap() )
	    p->drawTiledPixmap( r, *paper->pixmap());
	else
	    p->fillRect(r, *paper);
	p->setClipping( FALSE );
    }
}


/*!
  Returns the context of the rich text document. If no context has been specified
  in the constructor, a null string is returned.
*/
QString QSimpleRichText::context() const
{
    return d->doc->context();
}

/*!
  Returns the anchor at the requested position. The QPainter is needed for font size
  calculations. An empty string is returned if no anchor is specified for this certain
  position.
*/
QString QSimpleRichText::anchor( QPainter* p, const QPoint& pos )
{
    QTextNode* n = d->doc->hitTest( p, 0, 0, pos.x(), pos.y() );
    if ( !n )
      return QString::null;

    const QTextContainer* act = n->parent()->anchor();
    if (act && act->attributes() && act->attributes()->contains("href") )
      return ( act->attributes()->operator[]("href") );

    return QString::null;
}
