/****************************************************************************
** $Id: //depot/qt/main/tests/richtextedit/qtextbrowser.cpp#2 $
**
** Implementation of the QtTextView class
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

#include "qtextbrowser.h"
#include "qrichtextintern.h"

#include "qapplication.h"
#include "qlayout.h"
#include "qpainter.h"

#include "qstack.h"
#include "stdio.h"
#include "qfile.h"
#include "qtextstream.h"
#include "qlayout.h"
#include "qbitmap.h"
#include "qtimer.h"
#include "qimage.h"
#include "qsimplerichtext.h"
#include "qdragobject.h"


/*!
  \class QtTextBrowser qtextbrowser.h
  \brief A rich text  browser with simple navigation.
  \ingroup realwidgets

  This class is the same as the QtTextView it inherits, with the
  addition that it provides basic navigation features to follow links
  in hypertext documents that link to other rich text documents. While
  QtTextView only allows to set its contents with setText(),
  QtTextBrowser has an additional function setSource(), that makes it
  possible to set documents by name. These names are looked up in the
  text view's mime source factory. If a document name ends with an
  anchor, for example "\c #anchor", the text browser will
  automatically scroll accordingly ( using scrollToAnchor() ). When
  the user clicks on a hyperlink, the browser will call setSource()
  itself, with the link's \c href value as argument.

  QtTextBrowser doesn't provide actual Back and Forward buttons, but it
  has backward() and forward() slots that implement the
  functionality. The home() slots brings it back to its very first
  document displayed.

  By using QtTextView::setMimeSourceFactory(), you can provide your own
  subclass of QMimeSourceFactory. This makes it possible to access
  data from anywhere you need to, may it be the network or a
  database. See QMimeSourceFactory::data() for details.

  If you intend to use the mime factory to read the data directly from
  the file system, you may have to specify the encoding for the file
  extension you are using. For example
  \code
  mimeSourceFactory()->setExtensionType("qml", "text/utf8");
  \endcode
  Otherwise, the factory will not be able to resolve the document names.

  For simpler richt text use, see QLabel, QtTextView or QSimpleRichText.
*/

class QtTextBrowserData
{
public:
    QString searchPath;
    QString buttonDown;
    QString highlight;
    QPoint lastClick;
    QStack<QString> stack;
    QStack<QString> forwardStack;
    QString home;
    QString curmain;
};


/*!
  Constructs an empty QtTextBrowser.
*/
QtTextBrowser::QtTextBrowser(QWidget *parent, const char *name)
    : QtTextView( parent, name )
{
    d = new QtTextBrowserData;

    viewport()->setMouseTracking( TRUE );
}

/*!
  Destructs the browser.
*/
QtTextBrowser::~QtTextBrowser()
{
    delete d;
}


/*!
  Sets the text document with the given \a name to be displayed.  The
  name is looked up in the mimeSourceFactory() of the browser.

  In addition to the factory lookup, this functions also checks for
  optional anchors and scrolls the document accordingly.

  If the first tag in the document is \c &lt;qt \c type=detail&gt;, it is
  displayed as a popup rather than as new document in the browser
  window itself. Otherwise, the document is set normally via
  setText(), with \a name as new context.

  If you are using the filesystem access capabilities of the mime
  source factory, you have to ensure that the factory knows about the
  encoding of specified text files, otherwise no data will be
  available. The default factory handles a couple of common file
  extensions such as \c *.html and \c *.txt with reasonable defaults. See
  QMimeSourceFactory::data() for details.

*/
void QtTextBrowser::setSource(const QString& name)
{
    qDebug("setSource %s", name.latin1() );
    if ( isVisible() )
	qApp->setOverrideCursor( waitCursor );
    QString source = name;
    QString mark;
    int hash = name.find('#');
    if ( hash != -1) {
	source  = name.left( hash );
	mark = name.mid( hash+1 );
    }

    if ( source.left(5) == "file:" )
	source = source.mid(6);

    QString url = mimeSourceFactory()->makeAbsolute( source, context() );

    if ( !source.isEmpty() && url != d->curmain ) {
	const QMimeSource* m =
		    mimeSourceFactory()->data( source, context() );
	QString txt;
	if ( !m ){
	    qWarning("QtTextBrowser: no mimesource for %s", source.latin1() );
	}
	else {
	    if ( !QTextDrag::decode( m, txt ) ) {
		qWarning("QtTextBrowser: cannot decode %s", source.latin1() );
	    }
	}

	//#### TODO
// 	if ( isVisible() ) {
// 	    QString firstTag = txt.left( txt.find('>' )+1 );
// 	    QRichText tmp( firstTag );
// 	    static QString s_type = QString::fromLatin1("type");
// 	    static QString s_detail = QString::fromLatin1("detail");
// 	    if (tmp.attributes() && tmp.attributes()->contains(s_type)
// 		&& (*tmp.attributes())[s_type] == s_detail ) {
// 		popupDetail( txt, d->lastClick );
// 		qApp->restoreOverrideCursor();
// 		return;
// 	    }
// 	}

	d->curmain = url;
	setText( txt, url );
    }

    if ( !mark.isEmpty() ) {
	url += "#";
	url += mark;
    }

    if ( !d->home )
	d->home = url;

    if ( d->stack.isEmpty() || *d->stack.top() != url) {
	emit backwardAvailable( !d->stack.isEmpty() );
	d->stack.push(new QString( url ) );
    }

    if ( !mark.isEmpty() )
	scrollToAnchor( mark );
    else
	setContentsPos( contentsX(), 0 );

    if ( isVisible() )
	qApp->restoreOverrideCursor();
}

/*!
  Returns the source of the currently display document. If no document is displayed or
  the source is unknown, a null string is returned.

  \sa setSource()
 */
QString QtTextBrowser::source() const
{
    if ( d->stack.isEmpty() )
	return QString::null;
    else
	return *d->stack.top();
}


/*!
  Sets the contents of the browser to \a text, and emits the
  textChanged() signal.
*/
void QtTextBrowser::setText( const QString& text, const QString& context )
{
    QtTextView::setText( text, context );
    emit textChanged();
}

/*!
  \fn void QtTextBrowser::backwardAvailable(bool available)
  This signal is emitted when the availability of the backward()
  changes.  It becomes available when the user navigates forward,
  and unavailable when the user is at the home().
*/

/*!
  \fn void QtTextBrowser::forwardAvailable(bool available)
  This signal is emitted when the availability of the forward()
  changes.  It becomes available after backward() is activated,
  and unavailable when the user navigates or goes forward() to
  the last navigated document.
*/

/*!
  \fn void QtTextBrowser::highlighted (const QString &href)
  This signal is emitted when the user has selected but not activated
  a link in the document.  \a href is the value of the href tag
  in the link.
*/

/*!
  \fn void QtTextBrowser::textChanged()
  This signal is emitted whenever the setText() changes the
  contents (eg. because the user clicked on a link).
*/

/*!
  Changes the document displayed to be the previous document
  in the list of documents build by navigating links.

  \sa forward(), backwardAvailable()
*/
void QtTextBrowser::backward()
{
    if ( d->stack.count() <= 1)
	return;
    d->forwardStack.push( d->stack.pop() );
    QString* ps = d->stack.pop();
    setSource( *ps );
    delete ps;
    emit forwardAvailable( TRUE );
}

/*!
  Changes the document displayed to be the next document
  in the list of documents build by navigating links.

  \sa backward(), forwardAvailable()
*/
void QtTextBrowser::forward()
{
    if ( d->forwardStack.isEmpty() )
	return;
    QString* ps = d->forwardStack.pop();
    setSource( *ps );
    delete ps;
    emit forwardAvailable( !d->forwardStack.isEmpty() );
}

/*!
  Changes the document displayed to be the first document the
  browser displayed.
*/
void QtTextBrowser::home()
{
    if (!d->home.isNull() )
	setSource( d->home );
}

/*!
  Add Backward and Forward on ALT-Left and ALT-Right respectively.
*/
void QtTextBrowser::keyPressEvent( QKeyEvent * e )
{
    if ( e->state() & AltButton ) {
	switch (e->key()) {
	case Key_Right:
	    forward();
	    return;
	case Key_Left:
	    backward();
	    return;
	case Key_Up:
	    home();
	    return;
	}
    }
    QtTextView::keyPressEvent(e);
}

/*!
  \e override to press anchors.
*/
void QtTextBrowser::viewportMousePressEvent( QMouseEvent* e )
{
    if ( e->button() == LeftButton ) {
	d->buttonDown = formatAt( e->pos() ).anchorHref();
	d->lastClick = e->globalPos();
    }
}

/*!
  \e override to activate anchors.
*/
void QtTextBrowser::viewportMouseReleaseEvent( QMouseEvent* e )
{
    if ( e->button() == LeftButton ) {
	if ( !d->buttonDown.isEmpty() && formatAt( e->pos() ).anchorHref() == d->buttonDown ) {
	    setSource( d->buttonDown );
	}
    }
    d->buttonDown = QString::null;
}

/*!
  Activate to emit highlighted().
*/
void QtTextBrowser::viewportMouseMoveEvent( QMouseEvent* e)
{
    QString act = formatAt( e->pos() ).anchorHref();
    if (d->highlight != act) {
	if ( !act.isEmpty() ){
	    emit highlighted( act );
	    d->highlight = act;
	}
	else if ( !d->highlight.isEmpty() ) {
	    emit highlighted( QString::null );
	    d->highlight = QString::null;
	}
	viewport()->setCursor( d->highlight.isEmpty()?arrowCursor:pointingHandCursor );
    }
}


QtTextCharFormat QtTextBrowser::formatAt(const QPoint& pos)
{
    QtTextCharFormat format;
    QPoint realPos( contentsX() + pos.x(), contentsY() + pos.y() );
    QPainter p( viewport() );
    QFontMetrics fm( p.fontMetrics() );
    int viewId = richText().viewId( this );
    QtTextCursor tc( richText(), viewId );
    tc.gotoParagraph( &p, &richText() );
    QtTextParagraph* b = tc.paragraph;
    while ( b && tc.referenceTop() <= contentsY() + visibleWidth() ) {
	if ( b && b->dirty[viewId] ) {
	    tc.initParagraph( &p, b );
	    tc.doLayout( &p, tc.referenceBottom() );
	}

	tc.gotoParagraph( &p, b );

	if ( tc.referenceBottom() > contentsY() ) {
	    do {
		tc.makeLineLayout( &p, fm );
		QRect geom( tc.lineGeometry() );
		if ( geom.contains( realPos ) ) {
		    tc.gotoLineStart( &p, fm );
		    while ( !tc.atEndOfLine() && geom.left() + tc.currentx < realPos.x() )
			tc.right( &p );
		    if ( geom.left() + tc.currentx > realPos.x() )
			tc.left( &p );
		    return *(tc.currentFormat());
		}
	    }
	    while ( tc.gotoNextLine( &p, fm ) );
	}
	b = b->nextInDocument();
    };
    return format;
}


class QTextDetailPopup : public QWidget
{
public:
    QTextDetailPopup()
	: QWidget ( 0, "automatic QText detail widget", WType_Popup | WDestructiveClose )
	{
	}

protected:

    void mousePressEvent( QMouseEvent*)
	{
	close();
	}
};


void QtTextBrowser::popupDetail( const QString& contents, const QPoint& pos )
{

    const int shadowWidth = 6;   // also used as '5' and '6' and even '8' below
    const int vMargin = 8;
    const int hMargin = 12;

    QWidget* popup = new QTextDetailPopup;
    popup->setBackgroundMode( QWidget::NoBackground );

    QPainter p( popup );
    QSimpleRichText* qmlDoc = new QSimpleRichText( contents, popup->font() );
    qmlDoc->adjustSize( &p );
    QRect r( 0, 0, qmlDoc->width(), qmlDoc->height() );

    int w = r.width() + 2*hMargin;
    int h = r.height() + 2*vMargin;

    popup->resize( w + shadowWidth, h + shadowWidth );

    // okay, now to find a suitable location
    //###### we need a global fancy popup positioning somewhere
    popup->move(pos - popup->rect().center());
    if (popup->geometry().right() > QApplication::desktop()->width())
	popup->move( QApplication::desktop()->width() - popup->width(),
		     popup->y() );
    if (popup->geometry().bottom() > QApplication::desktop()->height())
	popup->move( popup->x(),
		     QApplication::desktop()->height() - popup->height() );
    if ( popup->x() < 0 )
	popup->move( 0, popup->y() );
    if ( popup->y() < 0 )
	popup->move( popup->x(), 0 );


    popup->show();

    // now for super-clever shadow stuff.  super-clever mostly in
    // how many window system problems it skirts around.

    p.setPen( QApplication::palette().normal().foreground() );
    p.drawRect( 0, 0, w, h );
    p.setPen( QApplication::palette().normal().mid() );
    p.setBrush( QColor( 255, 255, 240 ) );
    p.drawRect( 1, 1, w-2, h-2 );
    p.setPen( black );

    qmlDoc->draw( &p, hMargin, vMargin, r, popup->colorGroup(), 0 );
    delete qmlDoc;

    p.drawPoint( w + 5, 6 );
    p.drawLine( w + 3, 6,
		w + 5, 8 );
    p.drawLine( w + 1, 6,
		w + 5, 10 );
    int i;
    for( i=7; i < h; i += 2 )
	p.drawLine( w, i,
		    w + 5, i + 5 );
    for( i = w - i + h; i > 6; i -= 2 )
	p.drawLine( i, h,
		    i + 5, h + 5 );
    for( ; i > 0 ; i -= 2 )
	p.drawLine( 6, h + 6 - i,
		    i + 5, h + 5 );
    p.end();

}



/*!
  Scrolls the browser so that the part of the document named
  \a name is at the top of the view (or as close to the top
  as the size of the document allows).
*/
void QtTextBrowser::scrollToAnchor(const QString& name)
{
    qDebug("scroll to anchor %s", name.latin1() );
    QPainter p( viewport() );
    QFontMetrics fm( p.fontMetrics() );
    int viewId = richText().viewId( this );
    QtTextCursor tc( richText(), viewId );
    tc.gotoParagraph( &p, &richText() );
    tc.makeLineLayout( &p, fm );
    tc.gotoLineStart( &p, fm );
    do {
	if ( !tc.currentFormat()->anchorName().isEmpty() )
	if ( tc.currentFormat()->anchorName() == name ) {
	    resizeContents( viewport()->width(), richText().flow( viewId)->height );
	    setContentsPos( contentsX(), tc.lineGeometry().top() );
	    return;
	}
    } while ( tc.rightOneItem( &p ) );
}

