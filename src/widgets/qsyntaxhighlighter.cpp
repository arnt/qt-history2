/****************************************************************************
** $Id$
**
** Implementation of the QSyntaxHighlighter class
**
** Created : 990101
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
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

#include "qsyntaxhighlighter.h"

#ifndef QT_NO_SYNTAXHIGHLIGHTER
#include "../kernel/qrichtext_p.h"
#include "qtextedit.h"

class QSyntaxHighlighterInternal : public QTextPreProcessor
{
public:
    QSyntaxHighlighterInternal( QSyntaxHighlighter *h ) : highlighter( h ) {}
    void process( QTextDocument *doc, QTextParagraph *p, int, bool invalidate ) {
	if ( p->prev() && p->prev()->endState() == -1 )
	    process( doc, p->prev(), 0, FALSE );

	highlighter->para = p;
	QString text = p->string()->toString();
	int endState = p->prev() ? p->prev()->endState() : -2;
	p->setEndState( highlighter->highlightParagraph( text, endState ) );
	highlighter->para = 0;

	p->setFirstPreProcess( FALSE );
	if ( invalidate && p->next() &&
	     !p->next()->firstPreProcess() && p->next()->endState() != -1 ) {
	    p = p->next();
	    while ( p ) {
		if ( p->endState() == -1 )
		    return;
		p->setEndState( -1 );
		p = p->next();
	    }
	}
    }
    QTextFormat *format( int ) { return 0; }

private:
    QSyntaxHighlighter *highlighter;

};

class QSyntaxHighlighterPrivate
{
};

/*!
    \class QSyntaxHighlighter qsyntaxhighlighter.h

    \brief QSyntaxHighlighter provides a class to implement syntax
    highlighters for QTextEdit

    \ingroup basic
    \ingroup text

    QSyntaxHighlighter provides the API to implement syntax
    highlighers for QTextEdit. A syntax highligher automatically
    highlightes parts of the text while the user enters it into a
    QTextEdit to make the text easier to read. This is very common in
    programming editors.

    To implement a syntax highlighter for QTextEdit you have to
    subclass QSyntaxHighlighter and reimplement
    highlighteParagraph(). In this function you highlighte parts of
    the text.

    To install your syntax highlighter on a QTextEdit, create it and
    pass the QTextEdit in the constructor.
*/

/*! Constructs the QSyntaxHighlighter and installs it on \a
  textEdit. The ownership is transferred to \a textEdit
*/

QSyntaxHighlighter::QSyntaxHighlighter( QTextEdit *textEdit )
    : para( 0 ), edit( textEdit ), d( 0 )
{
    textEdit->document()->setPreProcessor( new QSyntaxHighlighterInternal( this ) );
    textEdit->document()->invalidate();
}

/*! Destructor. Uninstalls this syntax highlighter from the textEdit() */

QSyntaxHighlighter::~QSyntaxHighlighter()
{
    textEdit()->document()->setPreProcessor( 0 );
}

/*! \fn int QSyntaxHighlighterInternal::highlightParagraph( const QString &text, int endStateOfLastPara )

  highlighteParagraph() is called only when necessary by the richtext
  engine on the paragraphs which changed. All you have to do to is to
  reimplement this function and parse the paragraph's text (\a text)
  and highlighte it. You can set colors and fonts on the paragraph
  using setFormat().

  If you implement a syntax highlighter where you need to know the end
  state of the last paragraph, you will need the parameter \a
  endStateOfLastPara and return the last state of the current
  paragraph from this function. An example is, if you write e.g. a C++
  syntax highlighter. You might highlighte strings, but a string can
  run over multiple paragraphs. In this case, you need to know if the
  last paragraph ended within the string, so you know that the current
  paragraph starts with a string and you can highlighte
  appropriately. In this case you also have to return the state with
  which the paragraph ends (String in our example) from this function.

  If \a endStateOfLastPara is -2, you are in the first paragraph of
  the document and you have to start with your initial state of the
  syntax highlighter.

  If your syntax highlighter doesn't need to be able to highlighte
  parts which run over multiple paragraphs, just ignore \a
  endStateOfLastPara and return 0 from this function.
*/

/*!  Sets the format of the currently processed paragraph to \a font
  and \a color starting at index \a start, for the \a len following
  characters.
*/

void QSyntaxHighlighter::setFormat( int start, int len, const QFont &font, const QColor &color )
{
    if ( !para )
	return;
    QTextFormat *f = 0;
    f = para->document()->formatCollection()->format( font, color );
    para->setFormat( start, len, f );
    f->removeRef();
}

/*! \overload */

void QSyntaxHighlighter::setFormat( int start, int len, const QColor &color )
{
    if ( !para )
	return;
    QTextFormat *f = 0;
    QFont fnt = textEdit()->QWidget::font();
    f = para->document()->formatCollection()->format( fnt, color );
    para->setFormat( start, len, f );
    f->removeRef();
}

/*! \overload */

void QSyntaxHighlighter::setFormat( int start, int len, const QFont &font )
{
    if ( !para )
	return;
    QTextFormat *f = 0;
    QColor c = textEdit()->viewport()->paletteForegroundColor();
    f = para->document()->formatCollection()->format( font, c );
    para->setFormat( start, len, f );
    f->removeRef();
}

/*! \fn QTextEdit *QSyntaxHighlighter::textEdit() const
  Returns the QTextEdit on which this syntax highlighter is installed
*/

#endif
