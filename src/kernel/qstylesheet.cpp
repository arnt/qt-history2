/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qstylesheet.cpp#10 $
**
** Implementation of the QStyleSheet class
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
#include "qstylesheet.h"
#include "qrichtextintern.cpp"
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

class QStyleSheetItemData
{
public:
    QStyleSheetItem::DisplayMode disp;
    int fontitalic;
    int fontunderline;
    int fontweight;
    int fontsize;
    int fontsizerel;
    QString fontfamily;
    QStyleSheetItem *parentstyle;
    QString stylename;
    int ncolumns;
    QColor col;
    bool anchor;
    int align;
    int margin[4];
    QStyleSheetItem::ListStyle list;
    QStyleSheetItem::WhiteSpaceMode whitespacemode;
    QString contxt;
    bool selfnest;
};

/*!
  \class QStyleSheetItem qstylesheet.h
  \brief The QStyleSheetItem class encapsulates a text format.

  A style consists of a name and a set of font, color, and other
  display properties.  When used in a \link QStyleSheet style
  sheet\endlink, items define the name of a rich text tag, and the
  display property changes associated with it.
*/

/*!
  Constructs a new style named \a name for the stylesheet \a parent.

  All properties in QStyleSheetItem are initially in the "do not change" state,
  except \link QStyleSheetItem::DisplayMode display mode\endlink, which defaults
  to \c DisplayInline.
*/
QStyleSheetItem::QStyleSheetItem( QStyleSheet* parent, const QString& name )
{
    d = new QStyleSheetItemData;
    d->stylename = name.lower();
    init();
    if (parent)
	parent->insert( this );
}


/*!
  Destroys the style.  Note that QStyleSheetItem objects become owned
  by QStyleSheet when they are created.
 */
QStyleSheetItem::~QStyleSheetItem()
{
    delete d;
}


/*!
  \internal
  Internal initialization
 */
void QStyleSheetItem::init()
{
    d->disp = DisplayInline;

    d->fontitalic = Undefined;
    d->fontunderline = Undefined;
    d->fontweight = Undefined;
    d->fontsize = Undefined;
    d->fontsizerel = 100;
    d->ncolumns = Undefined;
    d->col = QColor(); // !isValid()
    d->anchor = FALSE;
    d->align = Undefined;
    d->margin[0] = Undefined;
    d->margin[1] = Undefined;
    d->margin[2] = Undefined;
    d->margin[3] = Undefined;
    d->list = QStyleSheetItem::ListDisc;
    d->whitespacemode = QStyleSheetItem::WhiteSpaceNormal;
    d->selfnest = TRUE;
}

/*!
  Returns the name of style.
*/
QString QStyleSheetItem::name() const
{
    return d->stylename;
}

/*!
  Returns the \link QStyleSheetItem::DisplayMode display mode\endlink of the style.

  \sa setDisplayMode()
 */
QStyleSheetItem::DisplayMode QStyleSheetItem::displayMode() const
{
    return d->disp;
}

/*!
  Sets the display mode of the style to \a m.

  \define QStyleSheetItem::DisplayMode

  The affect of the available values are:
  <ul>
   <li> \c DisplayBlock
		- elements will be displayed as a rectangular block.
		    (eg. &lt;P&gt; ... &lt;/P&gt;)
   <li> \c DisplayInline
		- elements will be displayed in a horizontally flowing sequence.
		    (eg. &lt;EM&gt; ... &lt;/EM&gt;)
   <li> \c DisplayListItem
		- elements will be displayed vertically sequenced.
		    (eg. &lt;EM&gt; ... &lt;/EM&gt;)
   <li> \c DisplayNone
		- elements which are not displayed at all.
  </ul>

  \sa displayMode()
 */
void QStyleSheetItem::setDisplayMode(DisplayMode m)
{
    d->disp=m;
}


/*!
  Returns the alignment of this style. Possible values are AlignLeft,
  AlignRight and AlignCenter.

  \sa setAlignment()
 */
int QStyleSheetItem::alignment() const
{
    return d->align;
}

/*!
  Sets the alignment. This only makes sense for styles with
  \link QStyleSheetItem::DisplayMode display mode\endlink
  DisplayBlock. Possible values are AlignLeft, AlignRight and
  AlignCenter.

  \sa alignment(), displayMode()
 */
void QStyleSheetItem::setAlignment( int f )
{
    d->align = f;
}


/*!
  Returns whether the styles sets an italic or upright font.

 \sa setFontItalic(), definesFontItalic()
 */
bool QStyleSheetItem::fontItalic() const
{
    return d->fontitalic > 0;
}

/*!
  Sets italic or upright shape for the style.

  \sa fontItalic(), definesFontItalic()
 */
void QStyleSheetItem::setFontItalic(bool italic)
{
    d->fontitalic = italic?1:0;
}

/*!
  Returns whether the style defines a font shape.  A style
  does not define any shape until setFontItalic() is called.

  \sa setFontItalic(), fontItalic()
 */
bool QStyleSheetItem::definesFontItalic() const
{
    return d->fontitalic != Undefined;
}

/*!
  Returns whether the styles sets an underlined font.

 \sa setFontUnderline(), definesFontUnderline()
 */
bool QStyleSheetItem::fontUnderline() const
{
    return d->fontunderline > 0;
}

/*!
  Sets underline for the style.

  \sa fontUnderline(), definesFontUnderline()
 */
void QStyleSheetItem::setFontUnderline(bool underline)
{
    d->fontunderline = underline?1:0;
}

/*!
  Returns whether the style defines a setting for the underline
  property of the font.  A style does not define this until
  setFontUnderline() is called.

  \sa setFontUnderline(), fontUnderline() */
bool QStyleSheetItem::definesFontUnderline() const
{
    return d->fontunderline != Undefined;
}


/*!
  Returns the font weight setting of the style. This is either a
  valid QFont::Weight or the value QStyleSheetItem::Undefined.

 \sa setFontWeight, QFont
 */
int QStyleSheetItem::fontWeight() const
{
    return d->fontweight;
}

/*!
  Sets the font weight setting of the style.  Valid values are
  those defined by QFont::Weight.

  \sa QFont, fontWeight()
 */
void QStyleSheetItem::setFontWeight(int w)
{
    d->fontweight = w;
}

/*!
  Returns the font size setting of the style. This is either a valid
  pointsize or QStyleSheetItem::Undefined.

 \sa setFontSize(), QFont::pointSize(), QFont::setPointSize()
 */
int QStyleSheetItem::fontSize() const
{
    return d->fontsize;
}

/*!
  Sets the font size setting of the style, in relative percentage to
  the currently used font. The default value is 100 percent.

 \sa fontSizeRelative(), setFontSize(), QFont::pointSize(), QFont::setPointSize()
 */
void QStyleSheetItem::setFontSizeRelative(int s)
{
    d->fontsizerel = s;
}

/*!
  Returns the font size setting of the style. This is either a valid
  pointsize or QStyleSheetItem::Undefined.

 \sa setFontSizeRelative(), fontSize(), QFont::pointSize(), QFont::setPointSize()
 */
int QStyleSheetItem::fontSizeRelative() const
{
    return d->fontsizerel;
}

/*!
  Sets the font size setting of the style, in point measures.

 \sa fontSize(), QFont::pointSize(), QFont::setPointSize()
 */
void QStyleSheetItem::setFontSize(int s)
{
    d->fontsize = s;
}


/*!
  Returns the font family setting of the style. This is either a valid
  font family or QString::null if no family has been set.

 \sa setFontFamily(), QFont::family(), QFont::setFamily()
 */
QString QStyleSheetItem::fontFamily() const
{
    return d->fontfamily;
}

/*!
  Sets the font family setting of the style.

 \sa fontFamily(), QFont::family(), QFont::setFamily()
 */
void QStyleSheetItem::setFontFamily( const QString& fam)
{
    d->fontfamily = fam;
}


/*!
  Returns the number of columns for this style.

  \sa setNumberOfColumns(), displayMode(), setDisplayMode()

 */
int QStyleSheetItem::numberOfColumns() const
{
    return d->ncolumns;
}


/*!
  Sets the number of columns for this style.  Elements in the style
  are divided into columns.

  This only makes sense
  if the style uses a \link QStyleSheetItem::DisplayMode block display mode\endlink.

  \sa numberOfColumns()
 */
void QStyleSheetItem::setNumberOfColumns(int ncols)
{
    if (ncols > 0)
	d->ncolumns = ncols;
}


/*!
  Returns the text color of this style, or
  \link QColor::QColor() an invalid color\endlink
  if no color has been set yet.

  \sa setColor()
 */
QColor QStyleSheetItem::color() const
{
    return d->col;
}

/*!
  Sets the text color of this style.

  \sa color()
 */
void QStyleSheetItem::setColor( const QColor &c)
{
    d->col = c;
}

/*!
  Returns whether this style is an anchor.

  \sa setAnchor()
 */
bool QStyleSheetItem::isAnchor() const
{
    return d->anchor;
}

/*!
  Sets whether the style is an anchor (link).  Elements in this style
  have connections to other documents or anchors.

  \sa isAnchor()
 */
void QStyleSheetItem::setAnchor(bool anc)
{
    d->anchor = anc;
}


/*!
  Returns  the white space mode.

  \sa setWhiteSpaceMode()
 */
QStyleSheetItem::WhiteSpaceMode QStyleSheetItem::whiteSpaceMode() const
{
    return d->whitespacemode;
}

/*!
  Sets the white space mode to \a m. Possible values are
  <ul>
   <li> \c WhiteSpaceNormal
	- white spaces in the document only serve as seperators.
	Multiple spaces or indentation therefore is ignored.
   <li> \c WhiteSpacePre
          - white spaces are preserved. This is particulary useful to
	  display programming code.
  </ul>
 */
void QStyleSheetItem::setWhiteSpaceMode(WhiteSpaceMode m)
{
    d->whitespacemode = m;
}


/*!
  Returns the width of margin \a m in pixel.

  The margin determinator \a m can be \c MarginLeft, \c MarginRight,
  \c MarginTop, \c MarginBottom, \c MarginAll, \c MarginVertical or \c
  MarginHorizontal.

  \sa setMargin()
 */
int QStyleSheetItem::margin(Margin m) const
{
    return d->margin[m];
}


/*!
  Sets the width of margin \a m to \a v  pixels.

  The margin determinator \a m can be \c MarginLeft, \c MarginRight,
  \c MarginTop, \c MarginBottom, \c MarginAll, \c MarginVertical or \c
  MarginHorizontal.  The value \a v must be >= 0.

  \sa border()
 */
void QStyleSheetItem::setMargin(Margin m, int v)
{
    if (m == MarginAll ) {
	d->margin[0] = v;
	d->margin[1] = v;
	d->margin[2] = v;
	d->margin[3] = v;
    }
    else if (m == MarginVertical ) {
	d->margin[MarginTop] = v;
	d->margin[MarginBottom] = v;
    }
    else if (m == MarginHorizontal ) {
	d->margin[MarginLeft] = v;
	d->margin[MarginRight] = v;
    }
    else
	d->margin[m] = v;
}


/*!
  Returns the list style of the style.

  \sa setListStyle()
 */
QStyleSheetItem::ListStyle QStyleSheetItem::listStyle() const
{
    return d->list;
}

/*!
  Sets the list style of the style

  This is used by nested elements which have a
  \link QStyleSheetItem::DisplayMode display mode\endlink of DisplayListItem.

  \define QStyleSheetItem::ListStyle

  The affect of the available values are:
  <ul>
   <li> \c ListDisc
		- a filled circle
   <li> \c ListCircle
		- an unfilled circle
   <li> \c ListSquare
		- a filled circle
   <li> \c ListDecimal
		- an integer in base 10: \e 1, \e 2, \e 3, ...
   <li> \c ListLowerAlpha
		- a lowercase letter: \e a, \e b, \e c, ...
   <li> \c ListUpperAlpha
		- an uppercase letter: \e A, \e B, \e C, ...
  </ul>

  \sa listStyle()
 */
void QStyleSheetItem::setListStyle(ListStyle s)
{
    d->list=s;
}


/*!
  Returns a space separated list of names of styles that may contain
  elements of this style. As default, contexs() returns an empty
  string, which indicates that this style can be nested everywhere.

  \sa setContexts()
 */
QString QStyleSheetItem::contexts() const
{
    return d->contxt;
}

/*!
  Sets a space separated list of names of styles that may contain
  elements of this style. If \a c is empty, the style can be nested
  everywhere.

  \sa contexts()
 */
void QStyleSheetItem::setContexts( const QString& c)
{
    d->contxt = QChar(' ') + c + QChar(' ');
}

/*!
  Returns whether this style can be nested into an element
  of style \a s .

  \sa contxts(), setContexts()
 */
bool QStyleSheetItem::allowedInContext( const QStyleSheetItem* s) const
{
    if ( d->contxt.isEmpty() )
	return TRUE;
    return d->contxt.find( QChar(' ')+s->name()+QChar(' ')) != -1;
}


/*!
  Returns whether this style has self nesting enabled.

  \sa setSelfNesting()
 */
bool QStyleSheetItem::selfNesting() const
{
    return d->selfnest;
}

/*!
  Sets the self nesting property for this style to \a nesting.

  Usually, all styles are self nesting, i.e. they can legally be
  nested recursively.  A paragraph, for example, might contain other
  paragraphs as subparagraphs.

  You may nevertheles want to disable self nesting for common HTML
  tags such as \c &lt;p&gt; or \c &lt;li&gt;, if you want to parse and
  display rich text documents based on "dirty" HTML.  However, we
  recommend fixing the documents instead to close all non-empty tags,
  if that is possible.

 */
void QStyleSheetItem::setSelfNesting( bool nesting )
{
    d->selfnest = nesting;
}


//************************************************************************




//************************************************************************


/*!
  \class QStyleSheet qstylesheet.h
  \brief A collection of styles for rich text rendering and a generator of tags.

  By creating QStyleSheetItem objects for a style sheet, you build a
  definition of a set of tags.  This definition will be used by the
  internal rich text rendering system to parse and display text
  documents to which the style sheet applies. Rich text is normally
  visualized in a QTextView or a QTextBrowser. But also QLabel,
  QWhatsThis and QMessageBox support it for now, with others likely to
  follow. With QSimpleRichText it is possible to use the rich text
  renderer for custom widgets as well.

  The default QStyleSheet object has the following style bindings:

  <ul>
    <li>\c &lt;qt&gt;...&lt;/qt&gt;
	- A Qt rich text document. It understands the following attributes
	<ul>
	<li> \c title
	- the caption of the document. This attribute is easily accessible with
	QTextView::documentTitle()
	<li> \c type
	- The type of the document. The default type is \c page . It indicates that
	the document is displayed in a page of its own. Another style is \c detail.
	It can be used to explain certain expressions more detailed in a few
	sentences. The QTextBrowser will then keep the current page and display the
	new document in a small popup similar to QWhatsThis. Note that links
	will not work in documents with \c &lt;qt \c type="detail" \c &gt;...&lt;/qt&gt;
	<li> \c bgcolor
	- The background color, for example \c bgcolor="yellow" or \c bgcolor="#0000FF"
	<li> \c bgpixmap
	- The background pixmap, for example \c bgpixmap="granit.xpm". The pixmap name
	will be resolved by a QMimeSourceFactory().
	<li> \c text
	- The default text color, for example \c text="red"
	</ul>

    <li>\c &lt;a&gt;...&lt;/a&gt;
	- An anchor or link. The reference target is defined in the
	\c href attribute of the tag as in \c&lt;a \c href="target.qml"&gt;...&lt;/a&gt;.
	You can also specify an additional anchor within the specified target document, for
	example \c &lt;a \c href="target.qml#123"&gt;...&lt;/a&gt;.  If
	\c a is meant to be an anchor, the reference source is given in
	the \c name attribute.

    <li>\c &lt;em&gt;...&lt;/em&gt;
	- Emphasized. As default, this is the same as \c &lt;i&gt;...&lt;/i&gt; (Italic)

    <li>\c &lt;strong&gt;...&lt;/strong&gt;
	- Strong. As default, this is the same as \c &lt;bold&gt;...&lt;/bold&gt; (bold)

    <li>\c &lt;large&gt;...&lt;/large&gt;
	- A larger font size.

    <li>\c &lt;small&gt;...&lt;/small&gt;
	- A smaller font size.
	
    <li>\c &lt;code&gt;...&lt;/code&gt;
	- Indicates Code. As default, this is the same as \c &lt;tt&gt;...&lt;/tt&gt; (typewriter)

    <li>\c &lt;pre&gt;...&lt;/pre&gt;
	- For larger junks of code. Whitespaces in the contents are preserved.

    <li>\c &lt;large&gt;...&lt;/large&gt;
	- Large font size.

    <li>\c &lt;b&gt;...&lt;/b&gt;
    - style->setFontWeight( QFont::Bold);
	Bold font style.

    <li>\c &lt;h1&gt;...&lt;/h1&gt;
	- A top-level heading.

    <li>\c &lt;h2&gt;...&lt;/h2&gt;
	- A sub-level heading.

    <li>\c &lt;h3&gt;...&lt;/h3&gt;
	- A sub-sub-level heading.

    <li>\c &lt;p&gt;...&lt;/p&gt;
	- A paragraph.

    <li>\c &lt;center&gt;...&lt;/center&gt;
	- A centered paragraph.

    <li>\c &lt;blockquote&gt;...&lt;/blockquote&gt;
	- An indented paragraph, useful for quotes.

    <li>\c &lt;multicolumn \c cols=\a n \c &gt;...&lt;/multicolumn&gt;
	- Multicolumn display with \a n columns

    <li>\c &lt;twocolumn&gt;...&lt;/twocolumn&gt;
	- Two-column display.

    <li>\c &lt;ul&gt;...&lt;/ul&gt;
	- An un-ordered list. You can also pass a type argument to
	define the bullet style. The default is \c type=disc,  other
	types are \c circle and \c square.

    <li>\c &lt;ol&gt;...&lt;/ol&gt;
	- An ordered list. You can also pass a type argument to define
	the enumeration label style. The default is \c type="1", other
	types are \c "a" and \c "A".

    <li>\c &lt;li&gt;...&lt;/li&gt;
	- A list item.

    <li>\c &lt;img/&gt;
	- An image. The image name for the mime source
	factory  is given in the source attribute, for example
	\c &lt;img \c source="qt.xpm"/&gt; The image tag also
	understands the attributes \c width and \c height that determine
	the size of the image. If the pixmap does not fit to the specified
	size, it will be scaled automatically ( by using QImage::smoothScale() ).

    <li>\c &lt;br/&gt;
	- A line break
	
    <li>\c &lt;hr/&gt;
	- A horizonal line
  </ul>
*/

/*!
  Create a style sheet.  Like any QObject, the created object will be
  deleted when its parent destructs (if the child still exists then).

  By default, the style sheet has the tag definitions defined above.
*/
QStyleSheet::QStyleSheet( QObject *parent, const char *name )
    : QObject( parent, name )
{
    init();
}

/*!
  Destroys the style sheet.  All styles inserted into the style sheet
  will be deleted.
*/
QStyleSheet::~QStyleSheet()
{
}

/*!
  \internal
  Initialized the style sheet to the basic Qt style.
*/
void QStyleSheet::init()
{
    styles.setAutoDelete( TRUE );

    nullstyle  = new QStyleSheetItem( this,
	QString::fromLatin1("") );

    QStyleSheetItem*  style;

    style = new QStyleSheetItem( this, "qml" ); // compatibility
    style->setDisplayMode( QStyleSheetItem::DisplayBlock );

    style = new QStyleSheetItem( this, QString::fromLatin1("qt") );
    style->setDisplayMode( QStyleSheetItem::DisplayBlock );
    //style->setMargin( QStyleSheetItem::MarginAll, 4 );

    style = new QStyleSheetItem( this, QString::fromLatin1("a") );
    style->setColor( Qt::blue );
    style->setFontUnderline( TRUE );
    style->setAnchor( TRUE );

    style = new QStyleSheetItem( this, QString::fromLatin1("em") );
    style->setFontItalic( TRUE );

    style = new QStyleSheetItem( this, QString::fromLatin1("i") );
    style->setFontItalic( TRUE );

    style = new QStyleSheetItem( this, QString::fromLatin1("big") );
    style->setFontSizeRelative( 120 );

    style = new QStyleSheetItem( this, QString::fromLatin1("small") );
    style->setFontSizeRelative( 80 );

    style = new QStyleSheetItem( this, QString::fromLatin1("strong") );
    style->setFontWeight( QFont::Bold);

    style = new QStyleSheetItem( this, QString::fromLatin1("b") );
    style->setFontWeight( QFont::Bold);

    style = new QStyleSheetItem( this, QString::fromLatin1("h1") );
    style->setFontWeight( QFont::Bold);
    style->setFontSize(24);
    style->setDisplayMode(QStyleSheetItem::DisplayBlock);
    style-> setMargin(QStyleSheetItem::MarginVertical, 12);

    style = new QStyleSheetItem( this, QString::fromLatin1("h2") );
    style->setFontWeight( QFont::Bold);
    style->setFontSize(16);
    style->setDisplayMode(QStyleSheetItem::DisplayBlock);
    style-> setMargin(QStyleSheetItem::MarginVertical, 10);

    style = new QStyleSheetItem( this, QString::fromLatin1("h3") );
    style->setFontWeight( QFont::Bold);
    style->setFontSize(14);
    style->setDisplayMode(QStyleSheetItem::DisplayBlock);
    style-> setMargin(QStyleSheetItem::MarginVertical, 8);

    style = new QStyleSheetItem( this, QString::fromLatin1("p") );
    style->setDisplayMode(QStyleSheetItem::DisplayBlock);
    style-> setMargin(QStyleSheetItem::MarginVertical, 4);

    style = new QStyleSheetItem( this, QString::fromLatin1("center") );
    style->setDisplayMode(QStyleSheetItem::DisplayBlock);
    style->setAlignment( AlignCenter );

    style = new QStyleSheetItem( this, QString::fromLatin1("twocolumn") );
    style->setNumberOfColumns( 2 );

    /*style =*/ new QStyleSheetItem( this, QString::fromLatin1("multicol") );
    // ##### NOT USED?

    style = new QStyleSheetItem( this, QString::fromLatin1("ul") );
    style->setDisplayMode(QStyleSheetItem::DisplayBlock);

    style = new QStyleSheetItem( this, QString::fromLatin1("ol") );
    style->setDisplayMode(QStyleSheetItem::DisplayBlock);
    style->setListStyle( QStyleSheetItem::ListDecimal );

    style = new QStyleSheetItem( this, QString::fromLatin1("li") );
    style->setDisplayMode(QStyleSheetItem::DisplayListItem);
    style->setContexts(QString::fromLatin1("ol ul"));
    //    style-> setMargin(QStyleSheetItem::MarginVertical, 4);

    style = new QStyleSheetItem( this, QString::fromLatin1("code") );
    style->setFontFamily( QString::fromLatin1("courier") );

    style = new QStyleSheetItem( this, QString::fromLatin1("tt") );
    style->setFontFamily( QString::fromLatin1("courier") );

    new QStyleSheetItem(this, QString::fromLatin1("img"));
    new QStyleSheetItem(this, QString::fromLatin1("br"));
    new QStyleSheetItem(this, QString::fromLatin1("hr"));

    style = new QStyleSheetItem( this, QString::fromLatin1("table") );
    style->setDisplayMode(QStyleSheetItem::DisplayBlock);
    style = new QStyleSheetItem( this, QString::fromLatin1("pre") );
    style->setFontFamily( QString::fromLatin1("courier") );
    style->setDisplayMode(QStyleSheetItem::DisplayBlock);
    style->setWhiteSpaceMode(QStyleSheetItem::WhiteSpacePre);
    style = new QStyleSheetItem( this, QString::fromLatin1("blockquote") );
    style->setDisplayMode(QStyleSheetItem::DisplayBlock);
    style->setMargin(QStyleSheetItem::MarginAll, 8 );
}



static QStyleSheet* defaultsheet = 0;
void qt_cleanup_defaultsheet()
{
    delete defaultsheet;
}

/*!
  Returns the application-wide default style sheet.This style sheet is
  used by rich text rendering classes such as QSimpleRichText,
  QWhatsThis and also QMessageBox to define the rendering style and
  available tags within rich text documents. It serves also as initial
  style sheet for the more complex render widgets QTextView and
  QTextBrowser.

  \sa setDefaultSheet()
*/
QStyleSheet* QStyleSheet::defaultSheet()
{
    if (!defaultsheet) {
	defaultsheet = new QStyleSheet();
	qAddPostRoutine( qt_cleanup_defaultsheet );
    }
    return defaultsheet;
}

/*!
  Sets the application-wide default style sheet, deleting any style
  sheet previously set. The ownership is transferred.

  \sa defaultSheet()
*/
void QStyleSheet::setDefaultSheet( QStyleSheet* sheet)
{
    if ( defaultsheet != sheet )
	delete defaultsheet;
    defaultsheet = sheet;
}

/*!\internal
  Inserts \a style.  Any tags generated after this time will be
  bound to this style.  Note that \a style becomes owned by the
  style sheet and will be deleted when the style sheet destructs.
*/
void QStyleSheet::insert( QStyleSheetItem* style )
{
    styles.insert(style->name(), style);
}


/*!
  Returns the style with name \a name or 0 if there is no such style.
 */
QStyleSheetItem* QStyleSheet::item( const QString& name)
{
    return styles[name];
}

/*!
  Returns the style with name \a name or 0 if there is no such style (const version)
 */
const QStyleSheetItem* QStyleSheet::item( const QString& name) const
{
    return styles[name];
}


/*!
  Generates an internal object for tag named \a name, given the
  attributes \a attr, and using additional information provided
  by the mime source factory \a factory .

  This function should not (yet) be used in application code.
*/
QTextNode* QStyleSheet::tag( const QString& name,
			     const QMap<QString, QString> &attr,
			     const QString& context,
			     const QMimeSourceFactory& factory,
			     bool emptyTag ) const
{
    QStyleSheetItem* style = styles[name];
    if ( !style ) {
	qWarning( "QStyleSheet Warning: unknown tag '%s'", name.ascii() );
	style = nullstyle;
    }

    static QString s_img = QString::fromLatin1("img");
    static QString s_hr = QString::fromLatin1("hr");
    static QString s_br = QString::fromLatin1("br");
    static QString s_multicol = QString::fromLatin1("multicol");

    // first some known  tags
    if (style->name() == s_img)
	return new QTextImage(attr, context, factory);
    else if (style->name() == s_hr)
	return new QTextHorizontalLine(attr, factory);
    else if (style->name() == s_br ) {
	QTextNode* result = new QTextNode;
	result->text = '\n';
	return result;
    }
    else if (style->name() == s_multicol)
	return new QTextMulticol( style, attr );

    // empty tags
    if ( emptyTag ) { // w know nothing about that, make a null node
	return new QTextNode;
    }

    // process containers
    switch ( style->displayMode() ) {
    case QStyleSheetItem::DisplayBlock:
    case QStyleSheetItem::DisplayNone:
    case QStyleSheetItem::DisplayListItem:
	return new QTextBox( style, attr );
    default: // inline, none
	return new QTextContainer( style, attr );
    }
}


/*!
  Auxiliary function. Converts the plain text string \a plain to a
  rich text formatted string while preserving its look.
 */
QString QStyleSheet::convertFromPlainText( const QString& plain)
{
    int col = 0;
    QString rich;
    rich += "<p>";
    for ( int i = 0; i < int(plain.length()); ++i ) {
	if ( plain[i] == '\n' ){
	    if ( col == 1 )
		rich += "<p></p>";
	    else
		rich += "<br>";
	    col = 0;
	}
	else if ( plain[i] == '\t' ){
	    rich += 0x00a0U;
	    while ( col / 4.0 != int( col/4 ) ) {
		rich += 0x00a0U;
		++col;
	    }
	}
	else if ( plain[i].isSpace() )
	    rich += 0x00a0U;
	else if ( plain[i] == '<' )
	    rich +="&lt;";
	else if ( plain[i] == '>' )
	    rich +="&gt;";
	else
	    rich += plain[i];
	++col;
    }
    rich += "</p>";
    return rich;
}

/*!
  Returns whether the string \a text is likely to be rich text
  formatted.

  Note: The function uses a fast and therefore simple heuristic. It
  mainly checks whether there is something that looks like a tag
  before the first line break. While the result may be correct for
  most common cases, there is no guarantee.
*/
bool QStyleSheet::mightBeRichText( const QString& text)
{
    if ( text.isEmpty() )
	return FALSE;
    int open = 0;
    while ( open < int(text.length()) && text[open] != '<' && text[open] != '\n' )
	++open;
    if ( text[open] == '<' ) {
	int close = text.find('>', open);
	if ( close > -1 ) {
	    bool hasTag = FALSE;
	    for (int i = open+1; i < close; ++i) {
		if ( text[i].isDigit() || text[i].isLetter() )
		    hasTag = TRUE;
		else if ( hasTag && text[i].isSpace() )
		    return TRUE;
		else if ( !text[i].isSpace() && (hasTag || text[i] != '!' ) )
		    return FALSE; // that's not a tag
	    }
	    return TRUE;
	}
    }
    return FALSE;
}
