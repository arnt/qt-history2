/****************************************************************************
** $Id: //depot/qt/main/tests/richtextedit/qrichtext.cpp#14 $
**
** Implementation of the Qt classes dealing with rich text
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
#include "qtextview.h"
#include "qrichtextintern.h"
#include "qformatstuff.h"
#include <qapplication.h>
#include <qlayout.h>
#include <qpainter.h>

#include <qstack.h>
#include <stdio.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qlayout.h>
#include <qbitmap.h>
#include <qtimer.h>
#include <qimage.h>
#include <qdragobject.h>




QtTextImage::QtTextImage(const QMap<QString, QString> &attr, const QString& context,
		       const QMimeSourceFactory &factory)
{
    if ( attr.contains("width") )
	width = attr["width"].toInt();
    if ( attr.contains("height") )
	height = attr["height"].toInt();

    reg = 0;
    QImage img;
    QString imageName = attr["source"];

    if (!imageName)
	imageName = attr["src"];

    if ( !imageName.isEmpty() ) {
	const QMimeSource* m =
			factory.data( imageName, context );
	if ( !m ) {
	    qWarning("QtTextImage: no mimesource for %s", imageName.latin1() );
	}
	else {
	    if ( !QImageDrag::decode( m, img ) ) {
		qWarning("QtTextImage: cannot decode %s", imageName.latin1() );
	    }
	}
    }

    if ( !img.isNull() ) {
	if ( width == 0 )
	    width = img.width();
	if ( height == 0 )
	    height = img.height();

	if ( img.width() != width || img.height() != height ){
	    img = img.smoothScale(width, height);
	    width = img.width();
	    height = img.height();
	}
	pm.convertFromImage( img );
	if ( pm.mask() ) {
	    QRegion mask( *pm.mask() );
	    QRegion all( 0, 0, pm.width(), pm.height() );
	    reg = new QRegion( all.subtract( mask ) );
	}
    }

    if ( pm.isNull() && (width*height)==0 ) {
	width = height = 50;
    }
}

QtTextImage::~QtTextImage()
{
}


void QtTextImage::draw(QPainter* p, int x, int y,
		    int ox, int oy, int , int , int , int ,
		       QRegion& backgroundRegion, const QColorGroup& cg, const QtTextOptions& /*to*/)
{
    if ( pm.isNull() ) {
	p->fillRect( x-ox , y-oy, width, height,  cg.dark() );
	return;
    }
    if ( reg ){
	QRegion tmp( *reg );
	tmp.translate( x-ox, y-oy );
	backgroundRegion = backgroundRegion.unite( tmp );
    }
    p->drawPixmap( x-ox , y-oy, pm );
}


QtTextHorizontalLine::QtTextHorizontalLine()
{
    height = 8;
    width = 200;
}


QtTextHorizontalLine::~QtTextHorizontalLine()
{
}


void QtTextHorizontalLine::draw(QPainter* p, int x, int y,
				//int ox, int oy, int cx, int cy, int cw, int ch,
			     int ox, int oy, int , int , int , int ,
			     QRegion&, const QColorGroup&, const QtTextOptions& to)
{
    QRect rm( x-ox, y-oy, width, height);
    //QRect ra( cx-ox, cy-oy, cw,  ch);
    QRect r = rm; // ####.intersect( ra );
    if (to.paper) {
	if ( to.paper->pixmap() )
	    p->drawTiledPixmap( r, *to.paper->pixmap(), QPoint(r.x()+ox, r.y()+oy) );
	else
	    p->fillRect(r, *to.paper );
    }
    QPen pen(p->pen());
    pen.setWidth( 2 );
    p->setPen( pen );
    p->drawLine( r.left()-1, y-oy+4, r.right()+1, y-oy+4) ;
}

//************************************************************************


QtRichText::QtRichText( const QString &doc, const QFont& font,
		      const QString& context,
		      int margin,  const QMimeSourceFactory* factory, const QtStyleSheet* sheet  )
    :QtTextParagraph( 0, new QtTextFormatCollection(),
	    QtTextCharFormat( font, Qt::black ),
	    (base = new QStyleSheetItem( 0, QString::fromLatin1(""))) )
{
    contxt = context;

    // for access during parsing only
    factory_ = factory? factory : QMimeSourceFactory::defaultFactory();
    // for access during parsing only
    sheet_ = sheet? sheet : (QtStyleSheet*)QtStyleSheet::defaultSheet();

    init( doc, font, margin );

    // clear references that are no longer needed
    factory_ = 0;
    sheet_ = 0;
}


void QtRichText::init( const QString& doc, const QFont& font, int margin )
{
    for (int i = 0; i < MAXVIEWS; i++)
	views[i] = 0;
    for (int i = 0; i < MAXVIEWS; i++)
	flows[i] = new QtTextFlow();
    nviews = 0;

    //set up base style
    base->setDisplayMode(QStyleSheetItem::DisplayInline);
    base->setFontFamily( font.family() );
    base->setFontItalic( font.italic() );
    base->setFontUnderline( font.underline() );
    base->setFontWeight( font.weight() );
    base->setFontSize( font.pointSize() );
    base->setLogicalFontSize( 3 );
    base->setMargin( QStyleSheetItem::MarginAll, margin );

    nullstyle = new QStyleSheetItem( (QtStyleSheet*)sheet_, QString::fromLatin1(""));

    valid = TRUE;
    int pos = 0;
    parse(this, base, 0, format, doc, pos);
}

QtRichText::~QtRichText()
{
    for (int i = 0; i < MAXVIEWS; i++)
	delete flows[i];
    delete base;
}


int QtRichText::registerView( QtTextView* v )
{
    //###### make dynamic
    views[nviews++] = v;
    return nviews-1;
}

int QtRichText::viewId( QtTextView* v )
{
    for ( int i = 0; i < MAXVIEWS; i++ ) {
	if (  views[i] == v )
	    return i;
    }
    return -1;
}


QtTextView* QtRichText::view( int id )
{
    return views[id];
}


void QtRichText::updateViews( QtTextParagraph* b, int excludeView )
{
    for ( int i = 0; i < nviews; i++ ) {
	if ( i != excludeView )
	    views[i]->paragraphChanged( b );
    }
}

void QtRichText::dump()
{
}



bool QtRichText::isValid() const
{
    return valid;
}



/*!
  Returns the context of the rich text document. If no context has been specified
  in the constructor, a null string is returned.
*/
QString QtRichText::context() const
{
    return contxt;
}

static int qt_link_count = 0;

#define ENSURE_ENDTOKEN     if ( curstyle->displayMode() == QStyleSheetItem::DisplayBlock \
		     || curstyle->displayMode() == QStyleSheetItem::DisplayListItem ){ \
		    (dummy?dummy:current)->text.append( "*", fmt ); \
		} \


bool QtRichText::parse (QtTextParagraph* current, const QStyleSheetItem* curstyle, QtTextParagraph* dummy,
			QtTextCharFormat fmt, const QString &doc, int& pos)
{
    bool pre = current->whiteSpaceMode() == QStyleSheetItem::WhiteSpacePre;
    if ( !pre )
	eatSpace(doc, pos);
    while ( valid && pos < int(doc.length() )) {
	int beforePos = pos;
	if (hasPrefix(doc, pos, QChar('<')) ){
	    if (hasPrefix(doc, pos+1, QChar('/'))) {
		ENSURE_ENDTOKEN;
		return TRUE;
	    }
	    QMap<QString, QString> attr;
	    bool emptyTag = FALSE;
	    QString tagname = parseOpenTag(doc, pos, attr, emptyTag);
	
	    const QStyleSheetItem* nstyle = sheet_->item(tagname);
 	    if ( nstyle && !nstyle->selfNesting() && ( tagname == curstyle->name() ) ) {
 		pos = beforePos;
		ENSURE_ENDTOKEN;
 		return FALSE;
 	    }

	    if ( nstyle && !nstyle->allowedInContext( curstyle ) ) {
		QString msg;
		msg.sprintf( "QtText Warning: Document not valid ( '%s' not allowed in '%s' #%d)",
			 tagname.ascii(), current->style->name().ascii(), pos);
		sheet_->error( msg );
		pos = beforePos;
		ENSURE_ENDTOKEN;
		return FALSE;
	    }
	
	    // TODO was wenn wir keinen nstyle haben?
	    if ( !nstyle )
		nstyle = nullstyle;
	
	    QtTextCustomItem* custom = sheet_->tagEx( tagname, attr, contxt, *factory_ , emptyTag );
	    if ( custom || tagname == "br") {
		if ( current->child && !dummy ) {
		    dummy = new QtTextParagraph( current, formats, fmt,  nullstyle );
		    QtTextParagraph* it = current->child;
		    while ( it->next )
			it = it->next;
		    it->next = dummy;
		    dummy->prev = it;
		}
		if ( custom )
		    (dummy?dummy:current)->text.append( "", fmt.makeTextFormat( nstyle, attr, custom ) );
		else // br
		    (dummy?dummy:current)->text.append( "\n", fmt ) ;
		
	    }
	    else if (nstyle->displayMode() == QStyleSheetItem::DisplayBlock
		|| nstyle->displayMode() == QStyleSheetItem::DisplayListItem
		|| nstyle->displayMode() == QStyleSheetItem::DisplayNone
		) {
		if ( !current->text.isEmpty() ) {
		    dummy = new QtTextParagraph( current, formats, fmt, nullstyle );
		    dummy->text = current->text;
		    dummy->text.append( "*", fmt );
		    current->text.clear();
		    current->child = dummy;
		}
		QtTextParagraph* subparagraph = new QtTextParagraph( current, formats, fmt.makeTextFormat(nstyle,attr), nstyle, attr );
		
		if ( !current->child )
		    current->child = subparagraph;
		else {
		    QtTextParagraph* it = current->child;
		    while ( it->next )
			it = it->next;
		    it->next = subparagraph;
		    subparagraph->prev = it;
		}
		dummy = 0;
		if (parse( subparagraph, nstyle, 0, fmt.makeTextFormat( nstyle, attr ), doc, pos) ) {
		    (void) eatSpace(doc, pos);
		    int recoverPos = pos;
		    valid = (hasPrefix(doc, pos, QChar('<'))
			     && hasPrefix(doc, pos+1, QChar('/'))
			     && eatCloseTag(doc, pos, tagname) );
		    // sloppy mode, warning was done in eatCloseTag
		    if (!valid) {
			pos = recoverPos;
			valid = TRUE;
			return TRUE;
		    }
		    if (!valid)
			return TRUE;
		    }

		(void) eatSpace(doc, pos);
	    }
	    else { // containers and empty tags
		// TODO: check empty tags and custom tags in stylesheet
		if ( nstyle->isAnchor() ) {
		    qt_link_count++;
		}
		// for now: assume container
		if ( parse( current, nstyle, dummy, fmt.makeTextFormat( nstyle, attr ), doc, pos ) ) {
		    (void) eatSpace(doc, pos);
		    int recoverPos = pos;
		    valid = (hasPrefix(doc, pos, QChar('<'))
			     && hasPrefix(doc, pos+1, QChar('/'))
			     && eatCloseTag(doc, pos, tagname) );
		    // sloppy mode, warning was done in eatCloseTag
		    if (!valid) {
			pos = recoverPos;
			valid = TRUE;
			return TRUE;
		    }
		    if (!valid)
			return TRUE;
		}
	    }
	}
	else { // plain text
	    if ( current->child && !dummy ) {
		dummy = new QtTextParagraph( current, formats, fmt,  nullstyle );
		    QtTextParagraph* it = current->child;
		    while ( it->next )
			it = it->next;
		    it->next = dummy;
		    dummy->prev = it;
	    }
 	    QString word = parsePlainText( doc, pos, pre, TRUE );
 	    if (valid){
		(dummy?dummy:current)->text.append( word, fmt );
		if (!pre && doc[pos] == '<')
		    (void) eatSpace(doc, pos);
	    }
	}
    }
    return TRUE;
}

bool QtRichText::eatSpace(const QString& doc, int& pos, bool includeNbsp )
{
    int old_pos = pos;
    while (pos < int(doc.length()) && doc[pos].isSpace() && ( includeNbsp || doc[pos] != QChar(0x00a0U) ) )
	pos++;
    return old_pos < pos;
}

bool QtRichText::eat(const QString& doc, int& pos, QChar c)
{
    valid = valid && (bool) (doc[pos] == c);
    if (valid)
	pos++;
    return valid;
}

bool QtRichText::lookAhead(const QString& doc, int& pos, QChar c)
{
    return (doc[pos] == c);
}


static QMap<QCString, QChar> *html_map = 0;
static void qt_cleanup_html_map()
{
    delete html_map;
    html_map = 0;
}

QMap<QCString, QChar> *htmlMap()
{
    if ( !html_map ){
	html_map = new QMap<QCString, QChar>;
	qAddPostRoutine( qt_cleanup_html_map );
  	html_map->insert("lt", '<');
  	html_map->insert("gt", '>');
  	html_map->insert("amp", '&');
  	html_map->insert("nbsp", 0x00a0U);
  	html_map->insert("aring", 'å');
  	html_map->insert("oslash", 'ø');
  	html_map->insert("ouml", 'ö');
  	html_map->insert("auml", 'ä');
  	html_map->insert("uuml", 'ü');
  	html_map->insert("Ouml", 'Ö');
  	html_map->insert("Auml", 'Ä');
  	html_map->insert("Uuml", 'Ü');
  	html_map->insert("szlig", 'ß');
  	html_map->insert("copy", '©');
  	html_map->insert("deg", '°');
  	html_map->insert("micro", 'µ');
  	html_map->insert("plusmn", '±');
    }
    return html_map;
}

QChar QtRichText::parseHTMLSpecialChar(const QString& doc, int& pos)
{
    QCString s;
    pos++;
    int recoverpos = pos;
    while ( pos < int(doc.length()) && doc[pos] != ';' && !doc[pos].isSpace() && pos < recoverpos + 6) {
	s += doc[pos];
	pos++;
    }
    if (doc[pos] != ';' && !doc[pos].isSpace() ) {
	pos = recoverpos;
	return '&';
    }
    pos++;

    if ( s.length() > 1 && s[0] == '#') {
	return s.mid(1).toInt();
    }

    QMap<QCString, QChar>::Iterator it = htmlMap()->find(s);
    if ( it != htmlMap()->end() ) {
	return *it;
    }

    pos = recoverpos;
    return '&';
}

QString QtRichText::parseWord(const QString& doc, int& pos, bool insideTag, bool lower)
{
    QString s;

    if (doc[pos] == '"') {
	pos++;
	while ( pos < int(doc.length()) && doc[pos] != '"' ) {
	    s += doc[pos];
	    pos++;
	}
	eat(doc, pos, '"');
    }
    else {
	static QString term = QString::fromLatin1("/>");
	while( pos < int(doc.length()) &&
	       ( !insideTag || (doc[pos] != '>' && !hasPrefix( doc, pos, term)) )
	       && doc[pos] != '<'
	       && doc[pos] != '='
	       && !doc[pos].isSpace())
	{
	    if ( doc[pos] == '&')
		s += parseHTMLSpecialChar( doc, pos );
	    else {
		s += doc[pos];
		pos++;
	    }
	}
	if (lower)
	    s = s.lower();
    }
    valid = valid && pos <= int(doc.length());

    return s;
}

QString QtRichText::parsePlainText(const QString& doc, int& pos, bool pre, bool justOneWord)
{
    QString s;
    while( pos < int(doc.length()) &&
	   doc[pos] != '<' ) {
	if (doc[pos].isSpace() && doc[pos] != QChar(0x00a0U) ){
	
	    if ( pre ) {
		s += doc[pos];
	    }
	    else { // non-pre mode: collapse whitespace except nbsp
		while ( pos+1 < int(doc.length() ) &&
			doc[pos+1].isSpace()  && doc[pos+1] != QChar(0x00a0U) )
		    pos++;
		
		s += ' ';
	    }
	    pos++;
	    if ( justOneWord )
		return s;
	}
	else if ( doc[pos] == '&')
		s += parseHTMLSpecialChar( doc, pos );
	else {
	    s += doc[pos];
	    pos++;
	}
    }
    valid = valid && pos <= int(doc.length());
    return s;
}


bool QtRichText::hasPrefix(const QString& doc, int pos, QChar c)
{
    return valid && doc[pos] ==c;
}

bool QtRichText::hasPrefix(const QString& doc, int pos, const QString& s)
{
    if ( pos + s.length() >= doc.length() )
	return FALSE;
    for (int i = 0; i < int(s.length()); i++) {
	if (doc[pos+i] != s[i])
	    return FALSE;
    }
    return TRUE;
}

QString QtRichText::parseOpenTag(const QString& doc, int& pos,
				  QMap<QString, QString> &attr, bool& emptyTag)
{
    emptyTag = FALSE;
    pos++;
    QString tag = parseWord(doc, pos, TRUE, TRUE);
    eatSpace(doc, pos, TRUE);

    if (tag[0] == '!') {
	if (tag.left(3) == QString::fromLatin1("!--")) {
	    // eat comments
	    QString pref = QString::fromLatin1("-->");
	    while ( valid && !hasPrefix(doc, pos, pref )
			&& pos < int(doc.length()) )
		pos++;
	    if ( valid && hasPrefix(doc, pos, pref ) ) {
		pos += 4;
		eatSpace(doc, pos, TRUE);
	    }
	    else
		valid = FALSE;
	    return QString::null;
	}
	else {
	    // eat strange internal tags
	    while ( valid && !hasPrefix(doc, pos, QChar('>')) && pos < int(doc.length()) )
		pos++;
	    if ( valid && hasPrefix(doc, pos, QChar('>')) ) {
		pos++;
		eatSpace(doc, pos, TRUE);
	    }
	    else
		valid = FALSE;
	    return QString::null;
	}
    }

    static QString term = QString::fromLatin1("/>");
    static QString s_true = QString::fromLatin1("true");

    while (valid && !lookAhead(doc, pos, '>')
	    && ! (emptyTag = hasPrefix(doc, pos, term) ))
    {
	QString key = parseWord(doc, pos, TRUE, TRUE);
	eatSpace(doc, pos, TRUE);
	if ( key.isEmpty()) {
	    // error recovery
	    while ( pos < int(doc.length()) && !lookAhead(doc, pos, '>'))
		pos++;
	    break;
	}
	QString value;
	if (hasPrefix(doc, pos, QChar('=')) ){
	    pos++;
	    eatSpace(doc, pos);
	    value = parseWord(doc, pos, TRUE, FALSE);
	}
	else
	    value = s_true;
	attr.insert(key, value );
	eatSpace(doc, pos, TRUE);
    }

    if (emptyTag) {
	eat(doc, pos, '/');
	eat(doc, pos, '>');
    }
    else
	eat(doc, pos, '>');

    return tag;
}

bool QtRichText::eatCloseTag(const QString& doc, int& pos, const QString& open)
{
    pos++;
    pos++;
    QString tag = parseWord(doc, pos, TRUE, TRUE);
    eatSpace(doc, pos, TRUE);
    eat(doc, pos, '>');
    if (!valid) {
	QString msg;
	msg.sprintf( "QtText Warning: Document not valid ( '%s' not closing #%d)", open.ascii(), pos);
	sheet_->error( msg );
	valid = TRUE;
    }
    valid = valid && tag == open;
    if (!valid) {
	QString msg;
	msg.sprintf( "QtText Warning: Document not valid ( '%s' not closed before '%s' #%d)",
		     open.ascii(), tag.ascii(), pos);
	sheet_->error( msg );
    }
    return valid;
}



QtStyleSheet::QtStyleSheet( QObject *parent=0, const char *name=0 )
    : QStyleSheet( parent, name )
{
}

QtStyleSheet::~QtStyleSheet()
{
}

QtTextCustomItem* QtStyleSheet::tagEx( const QString& name,
				     const QMap<QString, QString> &attr,
				     const QString& context,
				     const QMimeSourceFactory& factory,
				       bool /* emptyTag */) const
{
    static QString s_img = QString::fromLatin1("img");
    static QString s_hr = QString::fromLatin1("hr");
    static QString s_br = QString::fromLatin1("br");
    static QString s_multicol = QString::fromLatin1("multicol");
    static QString s_font = QString::fromLatin1("font");

    const QStyleSheetItem* style = item( name );
    // first some known  tags
    if ( !style )
	return 0;
    if ( style->name() == s_img )
	return new QtTextImage(attr, context, factory);
    if ( style->name() == s_hr )
	return new QtTextHorizontalLine();
   return 0; //TODO
}


void QtTextParagraph::invalidateLayout( int view )
{
    dirty[view] = TRUE;	
    QtTextParagraph* b = child;
    while ( b ) {
	b->dirty[view] = TRUE;
	b->invalidateLayout( view );
	b = b->next;
    }
}


QStyleSheetItem::ListStyle QtTextParagraph::listStyle()
{
    QString s =  attributes()["type"];
	
    if ( !s )
	return style->listStyle();
    else {
	QCString sl = s.latin1();
	if ( sl == "1" )
	    return QStyleSheetItem::ListDecimal;
	else if ( sl == "a" )
	    return QStyleSheetItem::ListLowerAlpha;
	else if ( sl == "A" )
	    return QStyleSheetItem::ListUpperAlpha;
	sl = sl.lower();
	if ( sl == "square" )
	    return QStyleSheetItem::ListSquare;
	else if ( sl == "disc" )
		return QStyleSheetItem::ListDisc;
	else if ( sl == "circle" )
	    return QStyleSheetItem::ListCircle;
    }
    return style->listStyle();
}


int QtTextParagraph::numberOfSubParagraph( QtTextParagraph* subparagraph, bool onlyListItems)
{
    QtTextParagraph* it = child;
    int i = 1;
    while ( it && it != subparagraph ) {
	if ( !onlyListItems || it->style->displayMode() == QStyleSheetItem::DisplayListItem )
	    ++i;
	it = it->next;
    }
    return i;
}

QtTextParagraph::QtTextParagraph( QtTextParagraph* p, QtTextFormatCollection* formatCol, const QtTextCharFormat& fmt,
	      const QStyleSheetItem *stl, const QMap<QString, QString> &attr )
    : parent( p ), formats( formatCol ), format( fmt ), text( formatCol ), style ( stl ), attributes_( attr )
{
    init();
};


QtTextParagraph::QtTextParagraph( QtTextParagraph* p, QtTextFormatCollection* formatCol, const QtTextCharFormat& fmt,
	      const QStyleSheetItem *stl )
    : parent( p ), formats( formatCol ), format( fmt ), text( formats ), style ( stl )
{
    init();
};


void QtTextParagraph::init()
{
    formats->registerFormat( format );
    bool multicol = style->name() == "multicol";

    child = next = prev = 0;
    for (int i = 0; i < MAXVIEWS; i++) {
	widthUsed[i] = height[i] = y[i] = 0;
	dirty[i] = TRUE;
	if ( multicol ) {
	    flows[i] = new QtTextFlow( parent?parent->flow(i):0, 2 );
	}
	else
	    flows[i] = 0;
    }
}

QtTextParagraph::~QtTextParagraph()
{
    formats->unregisterFormat( format );
    QtTextParagraph* tmp = child;
    while ( child ) {
	tmp = child;
	child = child->next;
	delete tmp;
    }
}

QtTextParagraph* QtTextParagraph::nextInDocument()
{
    if ( next  ) {
	QtTextParagraph* b = next;
 	while ( b->child )
 	    b = b->child;
	return b;
    }
    if ( parent ) {
	return parent->nextInDocument();
    }
    return 0;
}

QtTextParagraph* QtTextParagraph::prevInDocument()
{
    if ( prev ){
	QtTextParagraph* b = prev;
	while ( b->child ) {
	    b = b->child;
	    while ( b->next )
		b = b->next;
	}
	return b;
    }
    if ( parent ) {
	return parent->prevInDocument();
    }
    return 0;
}


//####TODO slow
QtTextFlow* QtTextParagraph::flow( int view )
{
    if ( flows[ view ] )
	return flows[view];
//     else if ( prev )
// 	return prev->flow( view );
    else if ( parent )
	return parent->flow( view );
    return 0; // should not happen
}



QtTextRichString::QtTextRichString( QtTextFormatCollection* fmt )
    : format( fmt )
{
    items = 0;
    len = 0;
    store = 0;
}

QtTextRichString::~QtTextRichString()
{
    for (int i = 0; i < len; ++i )
	format->unregisterFormat( *items[i].format );
}


void QtTextRichString::clear()
{
    for (int i = 0; i < len; ++i )
	format->unregisterFormat( *items[i].format );
    if ( items )
	delete [] items;
    items = 0;
    len = 0;
    store = 0;
}


void QtTextRichString::remove( int index, int len )
{
    for (int i = 0; i < len; ++i )
	format->unregisterFormat( *formatAt( index + i ) );

    int olen = length();
    if ( index + len >= olen ) {		// range problems
	if ( index < olen ) {			// index ok
	    setLength( index );
	}
    } else if ( len != 0 ) {
// 	memmove( items+index, items+index+len,
// 		 sizeof(Item)*(olen-index-len) );
	for (int i = index; i < olen-len; ++i)
	    items[i] = items[i+len];
	setLength( olen-len );
    }
}

void QtTextRichString::insert( int index, const QString& c, const QtTextCharFormat& form )
{
    QtTextCharFormat* f = format->registerFormat( form );

    if ( index >= len ) {			// insert after end
	setLength( index+1 );
    } else {					// normal insert
//	int olen = len;
	int nlen = len + 1;
	setLength( nlen );
// 	memmove( items+index+1, items+index,
// 		 sizeof(Item)*(olen-index) );
	for (int i = len-1; i > index; --i)
	    items[i] = items[i-1];
    }

    items[index].c = c;
    items[index].format = f;
    items[index].width = -1;
}


QString& QtTextRichString::getCharAt( int index )
{
    items[index].width = -1;
    return items[index].c;
}

void QtTextRichString::setLength( int l )
{
    if ( l <= store ) {
	len = l; // TODO shrinking
	return;
    } else {
	store = l*2;
	Item* newitems = new Item[store]; // TODO speedup using new char(....)
// 	if ( items )
// 	    memcpy( newitems, items, sizeof(Item)*len );
	for (int i = 0; i < len; ++i )
	    newitems[i] = items[i];
	if ( items )
	    delete [] items;
	items = newitems;
	len = l;
    }
}

QtTextRichString::QtTextRichString( const QtTextRichString &other )
{
    format = other.format;
    len = other.len;
    items = 0;
    store = 0;
    if ( len ) {
	items = new Item[ len ];
	store = len;
// 	memcpy( items, other.items, sizeof(Item)*len );
	for (int i = 0; i < len; ++i ) {
	    items[i] = other.items[i];
	    items[i].format->addRef();
	}
    }
}

QtTextRichString& QtTextRichString::operator=( const QtTextRichString &other )
{
    clear();
    format = other.format;
    len = other.len;
    items = 0;
    store = 0;
    if ( len ) {
	items = new Item[ len ];
	store = len;
// 	memcpy( items, other.items, sizeof(Item)*len );
	for (int i = 0; i < len; ++i ) {
	    items[i] = other.items[i];
	    items[i].format->addRef();
	}
    }
    return *this;
}


QtTextCursor::QtTextCursor(QtRichText& document, int view)
{
    viewId = view;
    doc = &document;
    paragraph = doc;
    first = y_ = width = widthUsed = height = base = fill = 0;
    last = first - 1;
    current = currentx = currentoffset = currentoffsetx = 0;
    currentasc  = currentdesc = 0;
    xline_current = 0;
    xline = 0;
    xline_paragraph = 0;
    adjustFlowMode = TRUE;
}
QtTextCursor::~QtTextCursor()
{
}



void QtTextCursor::initFlow( QtTextFlow* frm, int w )
{
    flow = frm;
    y_ = 0;
    flow->initialize( w );
    //flow->initializeMe( this );
}

/*!
  Like gotoParagraph() but also initializes the paragraph
  (i.e. setting the cached values in the paragraph to the cursor's
  settings and not vice versa.

 */
void QtTextCursor::initParagraph( QPainter* p, QtTextParagraph* b )
{
	b->y[viewId] = y_;
	b->height[viewId] = 0;
	//	b->flows[viewId] = flow;
	while ( b->child ) {
	    b->child->y[viewId] = b->y[viewId];
	    b = b->child;
	}
	gotoParagraph( p, b );
}

void QtTextCursor::gotoParagraph( QPainter* p, QtTextParagraph* b )
{
    if ( !b )
	return;
    while ( b->child ) {
	b = b->child;
    }

    paragraph = b;
    flow = paragraph->flow( viewId );
    if ( paragraph->text.isEmpty() )
	paragraph->text.append( " ", paragraph->format );
	
    first = y_ = width = widthUsed = height = base = fill = 0;
    last = first - 1;

//     QtTextParagraph* bp = b->parent;
//     while ( bp ) {
// 	x_ += bp->margin( QStyleSheetItem::MarginLeft );
// 	bp = bp->parent;
//     }

    y_ =  b->y[viewId];
    int m =  b->margin( QStyleSheetItem::MarginTop ); // todo: if first par, think about daddy!

    if ( adjustFlowMode )
	flow->adjustFlow( y_, m ) ;
    else
	flow->countFlow( y_, m );

    y_ += m;

    current = 0;

    currentx = 0;
    currentoffset = 0;
    currentoffsetx = 0;
    QFontMetrics fm( p->fontMetrics() );
    updateCharFormat( p, fm );

    //qDebug("goto paragraph. Width = %d", width );
    widthUsed = 0;
}

void QtTextCursor::update( QPainter* p )
{
    int i = current;
    int io = currentoffset;
    QFontMetrics fm( p->fontMetrics() );
    gotoParagraph( p, paragraph );
    makeLineLayout( p, fm );
    gotoLineStart( p, fm );
    while ( current < i || currentoffset < io )
	right( p );
}


bool QtTextCursor::gotoNextLine( QPainter* p, const QFontMetrics& fm )
{
    current = last;
    if ( atEnd() ) {
	current++;
	y_ += height + 1; // first pixel below us
	int m = paragraph->margin( QStyleSheetItem::MarginBottom );
	if ( adjustFlowMode )
	    flow->adjustFlow( y_, m ) ;
	else
	    flow->countFlow( y_, m );
	y_ += m;
	paragraph->height[viewId] = y() - paragraph->y[viewId]; //####
	paragraph->dirty[viewId] = FALSE;
	return FALSE;
    }
    current++;
    currentx = 0;
    y_ += height;

    height = 0;
    updateCharFormat( p, fm );
    return TRUE;
}

void QtTextCursor::gotoLineStart( QPainter* p, const QFontMetrics& fm )
{
    current = first;
    currentoffset = currentoffsetx = 0;
    currentx = fill;
    updateCharFormat( p, fm );
}


void QtTextCursor::updateCharFormat( QPainter* p, const QFontMetrics& fm )
{
    if ( pastEnd() )
	return;
    QtTextCharFormat* fmt = paragraph->text.formatAt( current );
    p->setFont( fmt->font() );
    p->setPen( fmt->color() );
    currentasc = fm.ascent();
    currentdesc = fm.descent();
    QtTextCustomItem* custom = fmt->customItem();
    if ( custom ) {
	currentasc = custom->height;
    }

}


void QtTextCursor::drawLine( QPainter* p, int ox, int oy,
		    QRegion& backgroundRegion,
		    const QColorGroup& cg, const QtTextOptions& to )
{
    QFontMetrics fm( p->fontMetrics() );
    gotoLineStart( p, fm );

    if ( pastEndOfLine() ) {
	qDebug("try to draw empty line!!");
	return;
    }

    int gx, gy;
    flow->mapToView( y_, gx, gy );
	
    // todo ask flow to get position
    QRegion r(gx-ox, gy-oy, width, height);
    p->setClipRegion( r );
    p->setClipping( TRUE );

    backgroundRegion = backgroundRegion.subtract(r);

    if (TRUE ) { //!onlyDirty && !onlySelection && to.paper) {
	if ( to.paper->pixmap() )
	    p->drawTiledPixmap(gx-ox, gy-oy, width, height, *to.paper->pixmap(), gx, gy);
	else
	    p->fillRect(gx-ox, gy-oy, width, height, *to.paper);
    }


    QString c;
    while ( !atEndOfLine() ) {
	QtTextCharFormat *format  = paragraph->text.formatAt( current );
	if ( !format->anchorHref().isEmpty() ) {
	    p->setPen( to.linkColor );
	    if ( to.linkUnderline ) {
		QFont f = format->font();
		f.setUnderline( TRUE );
		p->setFont( f );
	    }
	}
	if ( format->customItem() ) {
	    int h = format->customItem()->height;
	    format->customItem()->draw(p, gx+currentx, gy+base-h, ox, oy,
				       0, 0, 0, 0, backgroundRegion, cg, to );
	}
	else {
	    c = paragraph->text.charAt( current );
	    p->drawText(gx+currentx-ox, gy-oy+base, c, c.length());
	}
	gotoNextItem( p, fm );
	flow->mapToView( y_, gx, gy );
    }
    p->setClipping( FALSE );
}

bool QtTextCursor::atEnd() const
{
    return current > paragraph->text.length()-2;
}


bool QtTextCursor::pastEnd() const
{
    return current > paragraph->text.length() - 1;
}

bool QtTextCursor::pastEndOfLine() const
{
    return current > last;
}

bool QtTextCursor::atEndOfLine() const
{
    return current > last || (current == last && currentoffset >= int(paragraph->text.charAt( current ).length())-1);
}



void QtTextCursor::rightOneItem( QPainter* p )
{
    QFontMetrics fm( p->fontMetrics() );
    if ( atEnd() ) {
	if ( paragraph->nextInDocument() ) {
	    gotoParagraph( p, paragraph->nextInDocument() );
	    makeLineLayout( p, fm );
	    gotoLineStart( p, fm );
	}
    }
    else if ( current >= last ) {
	(void) gotoNextLine( p, fm );
	makeLineLayout( p, fm );
	gotoLineStart( p, fm );
    }
    else {
	gotoNextItem( p, fm );
    }
}

void QtTextCursor::right( QPainter* p )
{
    if ( !pastEnd() && !pastEndOfLine() ) {
	QString c =  paragraph->text.charAt( current );
	if ( currentoffset  < int(c.length()) - 1 ) {
	    QtTextCharFormat* fmt = paragraph->text.formatAt( current );
	    p->setFont( fmt->font() );
	    QFontMetrics fm( p->fontMetrics() );
	    currentoffset++;
	    currentoffsetx = fm.width( c, currentoffset );
	    return;
	}
    }
    rightOneItem( p );
}

void QtTextCursor::left( QPainter* p )
{
    QFontMetrics fm( p->fontMetrics() );
    if ( currentoffset > 0 ) {
	QString c =  paragraph->text.charAt( current );
	QtTextCharFormat* fmt = paragraph->text.formatAt( current );
	p->setFont( fmt->font() );
	currentoffset--;
	currentoffsetx = fm.width( c, currentoffset );
    }
    else if ( current == 0 ) {
	if ( paragraph->prevInDocument() ) {
	    gotoParagraph( p, paragraph->prevInDocument() );
	    makeLineLayout( p, fm );
	    gotoLineStart( p, fm );
	    while ( !atEnd() )
		rightOneItem( p );
	}
    }
    else {
	int i = current;
	if ( current == first ) {
	    gotoParagraph( p, paragraph );
	    makeLineLayout( p, fm );
	}
	gotoLineStart( p, fm );
	while ( current <  i - 1 )
	    rightOneItem( p );
	QString c =  paragraph->text.charAt( current );
	if ( c.length() > 1 ) {
	    currentoffset = c.length() - 1;
	    QtTextCharFormat* fmt = paragraph->text.formatAt( current );
	    p->setFont( fmt->font() );
	    QFontMetrics fm( p->fontMetrics() );
	    currentoffsetx = fm.width( c, currentoffset );
	}
    }
}


void QtTextCursor::up( QPainter* p )
{
    if ( xline_paragraph != paragraph || xline_current != current )
	xline = currentx + currentoffsetx;
    QFontMetrics fm( p->fontMetrics() );

    gotoLineStart( p, fm );
    left( p );
    gotoLineStart( p, fm );
    while ( !atEndOfLine() && currentx + currentoffsetx  < xline ) {
	right( p );
    }
    xline_paragraph = paragraph;
    xline_current = current;
}

void QtTextCursor::down( QPainter* p )
{
    if ( xline_paragraph != paragraph || xline_current != current )
	xline = currentx + currentoffsetx;
    while ( current < last )
	rightOneItem( p );
    rightOneItem( p );
    while ( !atEndOfLine() && currentx + currentoffsetx < xline ) {
	right( p );
    }

    xline_paragraph = paragraph;
    xline_current = current;
}

void QtTextCursor::insert( QPainter* p, const QString& text )
{
    QFontMetrics fm( p->fontMetrics() );
    if ( paragraph->text.isCustomItem( current ) ) {
	paragraph->text.insert( current, text,
				paragraph->text.formatAt( current )->formatWithoutCustom() );
	current++;
	currentoffset = 0;
    }
    else {
	QString& sref = paragraph->text.getCharAt( current );
	for ( uint i = 0; i < text.length(); i++ ) {
	    sref.insert( currentoffset, text[i] );
	    if ( text[i] == ' ' && currentoffset < int(sref.length())-1 ) { // not isSpace() to ignore nbsp
		paragraph->text.insert( current+1, sref.mid( currentoffset+1 ),
					*paragraph->text.formatAt( current ) );
		sref.truncate( currentoffset+1 );
		current++;
		currentoffset = 0;
	    }
	    else {
		currentoffset++;
	    }
	}
    }
    //update( p );
    updateParagraph( p );
    doc->updateViews( paragraph, viewId );
}

void QtTextCursor::updateParagraph( QPainter* p )
{
     int ph = paragraph->height[viewId];

     QtTextCursor store ( *this );
     QFontMetrics fm( p->fontMetrics() );
     gotoParagraph( p, paragraph );
     do {
	 makeLineLayout( p, fm );
     } while ( gotoNextLine( p, fm ) );
     *this = store;



     if ( TRUE && ph != paragraph->height[viewId] ) { //## TODO optimize again
	 // not sufficient.
	 if ( paragraph->nextInDocument() )
	     paragraph->nextInDocument()->invalidateLayout( viewId );
     }

    p->end();

    //##### bad design, use signal slots etc.
    doc->view(viewId)->repaintContents(doc->view(viewId)->contentsX(),
				       doc->view(viewId)->contentsY(),
				       doc->view(viewId)->visibleWidth(),
				       doc->view(viewId)->visibleHeight() , FALSE);

    return;
    if ( ph == paragraph->height[viewId] )
	doc->view(viewId)->repaintContents( 0,
				   paragraph->y[viewId], doc->view(viewId)->contentsWidth(),
				   paragraph->height[viewId], FALSE );
    else
	doc->view(viewId)->repaintContents( 0,
				   paragraph->y[viewId], doc->view(viewId)->contentsWidth(),
				   doc->view(viewId)->viewport()->height(), FALSE );

}


void QtTextCursor::gotoNextItem( QPainter* p, const QFontMetrics& fm )
{
    if ( pastEnd() )
	return;
    // tabulators belong here
    QtTextRichString::Item* item = &paragraph->text.items[current];
    QtTextCustomItem* custom = item->format->customItem();
    updateCharFormat( p, fm ); // optimize again
    if ( custom ) {
	    if ( width >= 0 && custom->expandsHorizontally() )
		custom->width = width + fm.minRightBearing() - fm.width(' ');
	    currentx += custom->width;
    }
    else {
	QString c = item->c;
	if ( item->width < 0 ) {
	    item->width = fm.width( c );
	}
	currentx += item->width;
    }
    current++;
    currentoffset = currentoffsetx = 0;

    updateCharFormat( p, fm ); // optimize again
    if ( current < paragraph->text.length() && !paragraph->text.haveSameFormat( current-1, current ) ) {
 	updateCharFormat( p, fm );
    }
    //     else if ( paragraph->nextInDocument() ) {
// 	gotoParagraph( p, paragraph->nextInDocument() );
//     }
}

void QtTextCursor::makeLineLayout( QPainter* p, const QFontMetrics& fm  )
{
    first = current;

    if ( pastEnd() )
	return;

    width = flow->availableWidth(y_ ); // TODO #### margins
		

    last = first;
    int rh = 0;
    int rasc = 0;
    int rdesc = 0;

    // do word wrap
    int lastSpace =current;
    int lastHeight = rh;
    int lastWidth = 0;
    int lastBearing = 0;
    int lastAsc = rasc;
    int lastDesc = rdesc;
    bool noSpaceFound = TRUE;

    QtTextCharFormat* fmt = paragraph->text.formatAt( current );
    int fmt_current = current;
    p->setFont( fmt->font() );
    int minRightBearing = fm.minRightBearing();
    int space_width = fm.width(' ');
    int fm_ascent = fm.ascent();
    int fm_height = fm.height();

    widthUsed = 0;

    while ( !pastEnd() ) {
	
	if ( !paragraph->text.haveSameFormat( fmt_current, current ) ) {
	    fmt = paragraph->text.formatAt( current );
	    fmt_current = current;
	    p->setFont( fmt->font() );
	    minRightBearing = fm.minRightBearing();
	    space_width = fm.width(' ');
	    fm_ascent = fm.ascent();
	    fm_height = fm.height();
	}

	QtTextRichString::Item* item = &paragraph->text.items[current];

	QChar lastc;

	QtTextCustomItem* custom = item->format->customItem();
	if ( !custom && !item->c.isEmpty() ) {
	    lastc = item->c[ item->c.length()-1];
	}
	
	if ( custom && current > first && custom->expandsHorizontally() ){
	    noSpaceFound = TRUE;
	    lastc = '\n'; // fake newline
	} else {
	    if ( currentasc + currentdesc > rh )
		rh = currentasc + currentdesc;
	    if ( currentasc > rasc )
		rasc = currentasc;
	    if ( currentdesc > rdesc )
		rdesc = currentdesc;

	    gotoNextItem( p, fm );
	}
	// TODO fake newlines with custom items that expand horizontally!
	if ( currentx > width - space_width + minRightBearing && !noSpaceFound )
	    break;
	
	// word break is possible (a) after a space, (b) after a
	// newline, (c) before expandable item  or (d) at the end of the paragraph.
	if ( noSpaceFound || lastc == ' ' || lastc == '\n' || current == paragraph->text.length() ){
	    lastSpace = current - 1;
	    lastHeight = rh;
	    lastAsc = rasc;
	    lastDesc = rdesc;
	    lastWidth = currentx;
	    lastBearing = minRightBearing;
 	    if ( lastc == ' ' )
 		noSpaceFound = FALSE;
	}
	if ( lastc == '\n' )
	    break;
    }

    last = lastSpace;

    rh = lastHeight;
    rasc = lastAsc;
    rdesc = lastDesc;

    height = QMAX(rh, rasc+rdesc+1);
    base = rasc;

    fill = 0;
    switch ( paragraph->alignment() ) {
    case Qt::AlignLeft:
	fill = 0;
	break;
    case Qt::AlignCenter:
	fill = (width - lastWidth) / 2;
	break;
    case Qt::AlignRight:
	fill = width - lastWidth;
	break;
    }

    current = lastSpace;//###

    int min = lastWidth - lastBearing;
    if ( min > widthUsed )
	widthUsed = min;
    if ( widthUsed > width )
	fill = 0;

    if ( adjustFlowMode )
	flow->adjustFlow( y_, height ) ;
    else
	flow->countFlow( y_, height );
}

bool QtTextCursor::doLayout( QPainter* p, int ymax, QtTextFlow* backFlow )
{
    QFontMetrics fm( p->fontMetrics() );
    QtTextParagraph* b = paragraph;
    gotoParagraph( p, b );
    QtTextFlow* oldFlow = flow;
    while ( b && b->flow( viewId ) != backFlow && ( ymax < 0 || y_ < ymax ) ) {
	do {
	    makeLineLayout( p, fm );
	}
	while ( gotoNextLine( p, fm ) );
	b = b->nextInDocument();
	if ( b && b->flow( viewId ) != backFlow ) {
	    initParagraph( p, b );
	    while ( b && b->flow( viewId ) != backFlow && oldFlow != flow ) { // a new flow, do it completely
		{
		    flow->initialize( oldFlow->availableWidth( y_ ) );
		    flow->x = 0;
		    flow->y = y_;
		    QtTextCursor other( *this );
		    other.adjustFlowMode = FALSE;
		    other.y_ = 0;
		    other.initParagraph(p, b );
		    other.doLayout( p, -1, oldFlow );
		
		    other.adjustFlowMode = TRUE;
		    other.gotoParagraph(p, b );
		    other.doLayout( p, -1, oldFlow );

		    if ( adjustFlowMode )
			oldFlow->adjustFlow( flow->y, flow->height, FALSE );
		    else
			oldFlow->countFlow( flow->y, flow->height, FALSE );
		
		    if ( flow->y != y_ ) {
			// adjust possible page breaks
			other.gotoParagraph(p, b );
			other.doLayout( p, -1, oldFlow );
		    }
		    y_ = flow->y + flow->height + 1;
		    b = other.paragraph->nextInDocument();
		}
		if ( b && b->flow( viewId ) != backFlow)
		    initParagraph( p, b );
	    }
	    paragraph->dirty[viewId] = FALSE;
	} 
    };
    return b == 0;
}



void QtTextCursor::draw(QPainter* p,  int ox, int oy, int cx, int cy, int cw, int ch)
{
    QRect r( caretGeometry() );
    if ( QMAX( r.left(), cx ) <= QMIN( r.right(), cx+cw ) &&
	 QMAX( r.top(), cy ) <= QMIN( r.bottom(), cy+ch ) ) {
	p->drawLine(r.left()-ox, r.top()-oy, r.left()-ox, r.bottom()-oy );
    }
}

QRect QtTextCursor::caretGeometry() const
{
    int gx, gy;
    flow->mapToView( y_, gx, gy );
    return QRect( gx+currentx + currentoffsetx, gy+base-currentasc, 1, currentasc + currentdesc + 1 );
}
QRect QtTextCursor::lineGeometry() const
{
    int gx, gy;
    flow->mapToView( y_, gx, gy );
    return QRect( gx, gy, width, height );
}


int QtTextCursor::referenceTop()
{
    if ( !flow->parent )
	return lineGeometry().top();
    else {
	QtTextFlow* f = flow;
	while ( f->parent->parent ) {
	    f = f->parent;
	}
	return f->parent->y + f->y;
    }
}

int QtTextCursor::referenceBottom()
{
    if ( !flow->parent )
	return lineGeometry().top() + paragraph->height[viewId];
    else {
	QtTextFlow* f = flow;
	while ( f->parent->parent ) {
	    f = f->parent;
	}
	return f->parent->y + f->y + f->height;
    }
}


QtTextFlow::QtTextFlow( QtTextFlow* parentFlow, int ncolumns )
{
    parent = parentFlow;
    ncols = ncolumns;
    totalheight = colheight = 0;
    x = y = width = height = 0;
}

QtTextFlow::~QtTextFlow()
{
}


void QtTextFlow::initialize( int w)
{
    height = 0;
    totalheight = colheight = 0;
    y = 0;
    width = w;
}

void QtTextFlow::mapToView( int yp, int& gx, int& gy )
{
    if ( parent )
	parent->mapToView( y, gx, gy );
    else {
	gx = 0;
	gy = y;
    }

    gx += x;

    if ( ncols == 1 || colheight == 0) {
	gx += 0;
	gy += yp;
	return;
    }

    int col = yp / colheight ;
    gx += col  * (width/ncols);
    gy += yp % colheight;

    if ( col >= ncols ) {
	gx -= (col-ncols+1) * (width/ncols);
	gy += (col-ncols+1) * colheight;
    }

    //TODO pages for printing

}

int QtTextFlow::availableWidth( int yp )
{
    if ( ncols == 1) {
	return width;
    }
    yp = 0; // shut up, compiler
    return width/ncols - 5;
}


const int pagesize = 500;

void QtTextFlow::adjustFlow( int  &yp, int h, bool pages )
{
    if ( totalheight ) {
	colheight = totalheight / ncols;
	totalheight = 0;
    }

    if ( ncols > 1 ) {
  	if ( yp+h < colheight * ncols && yp % colheight  + h > colheight )
  	    yp = (yp/colheight)*colheight + colheight;
	if ( colheight  ) {
	    if ( yp - ncols * colheight + colheight > height)
		height = yp -  ncols * colheight + colheight;
	}
    } else {
	if ( yp + h > height )
	    height = yp + h;
    }


    if ( pages ) { // check pages
	int tx, ty;
	mapToView( yp, tx, ty );
	int yinpage = ty % pagesize;
 	if ( yinpage < 2 )
 	    yp += 2 - yinpage;
 	else
	    if ( yinpage + h > pagesize - 2 )
	    yp += ( pagesize - yinpage ) + 2;
    }

    height = QMAX( colheight, height );

    //TODO pages for printing

}

void QtTextFlow::countFlow( int yp, int h, bool pages )
{
    totalheight = yp + h;

    if ( pages ) {
 	if ( totalheight % pagesize + h > pagesize - 2  )
 	    totalheight += pagesize - (totalheight % pagesize) + 2 ;
     }

    //totalheight += h;
}

