/****************************************************************************
** $Id$
**
** Implementation of the Qt classes dealing with rich text
**
** Created : 990101
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/
#include "qstylesheet.h"
#include "qtextview.h"
#include "qrichtextintern.h"
#include "qformatstuff.h"
#include <qapplication.h>
#include <qlayout.h>
#include <qpainter.h>

#include <stdio.h>
#include <values.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qlayout.h>
#include <qbitmap.h>
#include <qtimer.h>
#include <qimage.h>
#include <qdragobject.h>
#include <qdatetime.h>
#include <qdrawutil.h>

void QtTextOptions::erase( QPainter* p, const QRect& r ) const
{
    if ( !paper )
	return;
    if ( paper->pixmap() )
	p->drawTiledPixmap( r, *paper->pixmap(),
			    QPoint(r.x()+offsetx, r.y()+offsety) );
    else
	p->fillRect(r, *paper );
}



QtTextImage::QtTextImage(const QMap<QString, QString> &attr, const QString& context,
		       const QMimeSourceFactory &factory)
{
    width = height = 0;
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
	if ( width == 0 ) {
	    width = img.width();
	    if ( height != 0 ) {
		width = img.width() * height / img.height();
	    }
	}
	if ( height == 0 ) {
	    height = img.height();
	    if ( width != img.width() ) {
		height = img.height() * width / img.width();
	    }
	}

	if ( img.width() != width || img.height() != height ){
	    img = img.smoothScale(width, height);
	    width = img.width();
	    height = img.height();
	}
	pm.convertFromImage( img );
	qDebug("image conversion");
	if ( pm.mask() ) {
	    qDebug("image with a mask!");
	    QRegion mask( *pm.mask() );
	    QRegion all( 0, 0, pm.width(), pm.height() );
	    reg = new QRegion( all.subtract( mask ) );
	}
    }

    if ( pm.isNull() && (width*height)==0 ) {
	width = height = 50;
    }

    place = PlaceInline;
    if ( attr["align"] == "left" )
	place = PlaceLeft;
    else if ( attr["align"] == "right" )
	place = PlaceRight;

}

QtTextImage::~QtTextImage()
{
}


QtTextCustomItem::Placement QtTextImage::placement()
{
    return place;
}

void QtTextImage::draw(QPainter* p, int x, int y,
		    int ox, int oy, int , int , int , int ,
		       QRegion& backgroundRegion, const QColorGroup& cg, const QtTextOptions& /*to*/)
{
    if ( pm.isNull() ) {
	p->fillRect( x-ox , y-oy, width, height,  cg.dark() );
	return;
    }
    QRect r( x-ox, y-oy, width, height );
    backgroundRegion = backgroundRegion.subtract( r );
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
}


QtTextHorizontalLine::~QtTextHorizontalLine()
{
}


void QtTextHorizontalLine::draw(QPainter* p, int x, int y,
				//int ox, int oy, int cx, int cy, int cw, int ch,
				int ox, int oy, int , int , int , int ,
				QRegion&, const QColorGroup& cg, const QtTextOptions& to )
{
    QRect rm( x-ox, y-oy, width, height);
    //QRect ra( cx-ox, cy-oy, cw,  ch);
    QRect r = rm; // ####.intersect( ra );
    to.erase( p, r );
    qDrawShadeLine( p, r.left()-1, y-oy+4, r.right()+1, y-oy+4, cg, TRUE );
}

//************************************************************************


QtRichText::QtRichText( const QString &doc, const QFont& font,
		      const QString& context,
		      int margin,  const QMimeSourceFactory* factory, const QtStyleSheet* sheet  )
    :QtTextParagraph( 0, new QtTextFormatCollection(),
		      QtTextCharFormat( font, Qt::black ), (base = new QStyleSheetItem(0,"") ) )
{
    contxt = context;

    // for access during parsing only
    factory_ = factory? factory : QMimeSourceFactory::defaultFactory();
    // for access during parsing only
    sheet_ = sheet? sheet : (QtStyleSheet*)QtStyleSheet::defaultSheet();

    int pos = 0;

    //set up base style

    QFont font( format.font() );
    base->setDisplayMode(QStyleSheetItem::DisplayInline);
    base->setFontFamily( font.family() );
    base->setFontItalic( font.italic() );
    base->setFontUnderline( font.underline() );
    base->setFontWeight( font.weight() );
    base->setFontSize( font.pointSize() );
    base->setLogicalFontSize( 3 );
    base->setMargin( QStyleSheetItem::MarginAll, margin );

    keep_going = TRUE;
    init( doc, pos );

    // clear references that are no longer needed
    factory_ = 0;
    sheet_ = 0;
}


// constructor for nested text in text (tables, etc.)
QtRichText::QtRichText( const QMap<QString, QString> &attr, const QString &doc, int& pos,
			const QStyleSheetItem* style, const QtTextCharFormat& fmt,
			const QString& context,
			int margin,  const QMimeSourceFactory* factory, const QtStyleSheet* sheet  )
    :QtTextParagraph( 0, new QtTextFormatCollection(),
	    QtTextCharFormat( fmt ), ( base = new QStyleSheetItem(*style) ), attr )
{
    contxt = context;

    // for access during parsing only
    factory_ = factory? factory : QMimeSourceFactory::defaultFactory();
    // for access during parsing only
    sheet_ = sheet? sheet : (QtStyleSheet*)QtStyleSheet::defaultSheet();

    base->setMargin( QStyleSheetItem::MarginAll, margin );

    keep_going = FALSE;
    init( doc, pos );

     // clear references that are no longer needed
    factory_ = 0;
    sheet_ = 0;
}


void QtRichText::init( const QString& doc, int& pos )
{
    if ( !flow_ )
	flow_ = new QtTextFlow();

    nullstyle = sheet_->item("");

    valid = TRUE;
//     QTime before = QTime::currentTime();
    if ( !keep_going )
	parse(this, style, 0, format, doc, pos);
    else  do {
	parse(this, style, 0, format, doc, pos);
	// missplaced close tags may kick us out of the parser (auto
	// recover failure), simply jump over the tag and continue
	// until the very end
	int oldpos = pos;
	if ( pos < (int) doc.length()-1
	     && (hasPrefix(doc, pos, QChar('<')) ) ) {
	    if ( hasPrefix(doc, pos+1, QChar('/')) )
		(void) parseCloseTag( doc, pos );
	    else {
		QMap<QString, QString> attr;
		bool emptyTag = FALSE;
		(void) parseOpenTag( doc, pos, attr, emptyTag );
	    }
	}
	if ( pos == oldpos )
	    pos++;
    } while ( pos < (int) doc.length()-1 );
    //qDebug("parse time used: %d", ( before.msecsTo( QTime::currentTime() ) ) );
}

QtRichText::~QtRichText()
{
    delete base;
    delete flow_;
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

#define ENSURE_ENDTOKEN     if ( curstyle->displayMode() == QStyleSheetItem::DisplayBlock \
		     || curstyle->displayMode() == QStyleSheetItem::DisplayListItem ){ \
		    (dummy?dummy:current)->text.append( "\n", fmt ); \
		}

#define CLOSE_TAG (void) eatSpace(doc, pos); \
		int recoverPos = pos; \
		valid = (hasPrefix(doc, pos, QChar('<')) \
			 && hasPrefix(doc, pos+1, QChar('/')) \
			 && eatCloseTag(doc, pos, tagname) ); \
		if (!valid) { \
		    pos = recoverPos; \
		    valid = TRUE; \
		    return TRUE; \
		}

#define PROVIDE_DUMMY	if ( current->child && !dummy ) { \
		    dummy = new QtTextParagraph( current, formats, fmt,  nullstyle ); \
		    QtTextParagraph* it = current->child; \
		    while ( it->next ) \
			it = it->next; \
		    it->next = dummy; \
		    dummy->prev = it; \
		}

void QtRichText::append( const QString& txt, const QMimeSourceFactory* factory, const QtStyleSheet* sheet  )
{
    // for access during parsing only
    factory_ = factory? factory : QMimeSourceFactory::defaultFactory();
    // for access during parsing only
    sheet_ = sheet? sheet : (QtStyleSheet*)QtStyleSheet::defaultSheet();
    int pos = 0;
    lastChild()->invalidateLayout(); // fix bottom border
    parse( this, style, 0, format, txt, pos );
    // clear references that are no longer needed
    factory_ = 0;
    sheet_ = 0;
}


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
		if ( curstyle->isAnchor() ) {
		    PROVIDE_DUMMY
		    (dummy?dummy:current)->text.append( '\0', fmt );
		}
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
		PROVIDE_DUMMY
		if ( custom )
		    (dummy?dummy:current)->text.append( "", fmt.makeTextFormat( nstyle, attr, custom ) );
		else {// br
		    (dummy?dummy:current)->text.append( "\n", fmt ) ;
		    if ( !pre )
			eatSpace(doc, pos);
		}
	    }
	    else if ( tagname == "table" ) {
		QtTextCharFormat nfmt( fmt.makeTextFormat( nstyle, attr ) );
		custom = parseTable( attr, nfmt, doc, pos );
		if ( custom ) {
		    PROVIDE_DUMMY
		    (dummy?dummy:current)->text.append( "", fmt.makeTextFormat( nstyle, attr, custom )  );
		    (dummy?dummy:current)->text.append( " ", fmt );
		}
		CLOSE_TAG
	    }
	    else if (nstyle->displayMode() == QStyleSheetItem::DisplayBlock
		|| nstyle->displayMode() == QStyleSheetItem::DisplayListItem
		|| nstyle->displayMode() == QStyleSheetItem::DisplayNone
		) {
		QtTextParagraph* subparagraph = new QtTextParagraph( current, formats, fmt.makeTextFormat(nstyle,attr), nstyle, attr );

		if ( current == this && !child && text.isEmpty() )
		    attributes_ = attr; // propagate attributes

		if ( !current->text.isEmpty() ){
		    dummy = new QtTextParagraph( current, formats, fmt, nullstyle );
		    dummy->text = current->text;
		    dummy->text.append( "\n", fmt );
		    current->text.clear();
		    current->child = dummy;
		}

		bool recover = FALSE;
		if (parse( subparagraph, nstyle, 0, subparagraph->format, doc, pos) ) {
		    (void) eatSpace(doc, pos);
		    int recoverPos = pos;
		    valid = (hasPrefix(doc, pos, QChar('<'))
			     && hasPrefix(doc, pos+1, QChar('/'))
			     && eatCloseTag(doc, pos, tagname) );
		    // sloppy mode, warning was done in eatCloseTag
		    if (!valid) {
			pos = recoverPos;
			valid = TRUE;
			recover = TRUE;
		    }
		}
		if ( subparagraph->style->displayMode() == QStyleSheetItem::DisplayNone ) {
		    // delete invisible paragraphs
		    delete subparagraph;
		} else {
		    // connect visible paragraphs
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
		}

		if ( recover )
		    return TRUE; // sloppy, we could return FALSE to abort
		(void) eatSpace(doc, pos);
	    }
	    else { // containers and empty tags
		// TODO: check empty tags and custom tags in stylesheet

		if ( parse( current, nstyle, dummy, fmt.makeTextFormat(nstyle, attr), doc, pos ) ) {
		    CLOSE_TAG
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
		if (!pre && (doc.unicode())[pos] == '<')
		    (void) eatSpace(doc, pos);
	    }
	}
    }
    return TRUE;
}

static bool qt_is_cell_in_use( QPtrList<QtTextTableCell>& cells, int row, int col )
{
    for ( QtTextTableCell* c = cells.first(); c; c = cells.next() ) {
	if ( row >= c->row() && row < c->row() + c->rowspan()
	     && col >= c->column() && col < c->column() + c->colspan() )
	    return TRUE;
    }
    return FALSE;
}

QtTextCustomItem* QtRichText::parseTable( const QMap<QString, QString> &attr, const QtTextCharFormat &fmt, const QString &doc, int& pos )
{

    QtTextTable* table = new QtTextTable( attr );
    int row = -1;
    int col = -1;

    QString rowbgcolor;
    QString rowalign;
    QString tablebgcolor = attr["bgcolor"];

    QPtrList<QtTextTableCell> multicells;

    QString tagname;
    (void) eatSpace(doc, pos);
    while ( valid && pos < int(doc.length() )) {
	int beforePos = pos;
	if (hasPrefix(doc, pos, QChar('<')) ){
	    if (hasPrefix(doc, pos+1, QChar('/'))) {
		tagname = parseCloseTag( doc, pos );
		if ( tagname == "table" ) {
		    pos = beforePos;
		    return table;
		}
	    } else {
		QMap<QString, QString> attr2;
		bool emptyTag = FALSE;
		tagname = parseOpenTag( doc, pos, attr2, emptyTag );
		if ( tagname == "tr" ) {
		    rowbgcolor = attr2["bgcolor"];
		    rowalign = attr2["align"];
		    row++;
		    col = -1;
		}
		else if ( tagname == "td" || tagname == "th" ) {
		    col++;
		    while ( qt_is_cell_in_use( multicells, row, col ) ) {
			col++;
		    }

		    if ( row >= 0 && col >= 0 ) {
			const QStyleSheetItem* style = sheet_->item(tagname);
			if ( !attr2.contains("bgcolor") ) {
			    if (!rowbgcolor.isEmpty() )
				attr2["bgcolor"] = rowbgcolor;
			    else if (!tablebgcolor.isEmpty() )
				attr2["bgcolor"] = tablebgcolor;
			}
			if ( !attr2.contains("align") ) {
			    if (!rowalign.isEmpty() )
				attr2["align"] = rowalign;
			}
			QtTextTableCell* cell  = new QtTextTableCell( table, row, col,
			      attr2, style,
			      fmt.makeTextFormat( style, attr2, 0 ),
			      contxt, *factory_, sheet_, doc, pos );
			if ( cell->colspan() > 1 || cell->rowspan() > 1 )
			    multicells.append( cell );
// 			qDebug("add cell %d %d (%d %d )", row, col, cell->rowspan(), cell->colspan() );
			col += cell->colspan()-1;
		    }
		}
	    }

	} else {
	    ++pos;
	}
    }
    return table;
}




bool QtRichText::eatSpace(const QString& doc, int& pos, bool includeNbsp )
{
    int old_pos = pos;
    while (pos < int(doc.length()) && (doc.unicode())[pos].isSpace() && ( includeNbsp || (doc.unicode())[pos] != QChar(0x00a0U) ) )
	pos++;
    return old_pos < pos;
}

bool QtRichText::eat(const QString& doc, int& pos, QChar c)
{
    valid = valid && (bool) ((doc.unicode())[pos] == c);
    if (valid)
	pos++;
    return valid;
}

bool QtRichText::lookAhead(const QString& doc, int& pos, QChar c)
{
    return ((doc.unicode())[pos] == c);
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
  	html_map->insert("middot", '*');
    }
    return html_map;
}

QChar QtRichText::parseHTMLSpecialChar(const QString& doc, int& pos)
{
    QCString s;
    pos++;
    int recoverpos = pos;
    while ( pos < int(doc.length()) && (doc.unicode())[pos] != ';' && !(doc.unicode())[pos].isSpace() && pos < recoverpos + 6) {
	s += (doc.unicode())[pos];
	pos++;
    }
    if ((doc.unicode())[pos] != ';' && !(doc.unicode())[pos].isSpace() ) {
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

    if ((doc.unicode())[pos] == '"') {
	pos++;
	while ( pos < int(doc.length()) && (doc.unicode())[pos] != '"' ) {
	    s += (doc.unicode())[pos];
	    pos++;
	}
	eat(doc, pos, '"');
    }
    else {
	static QString term = QString::fromLatin1("/>");
	while( pos < int(doc.length()) &&
	       ( !insideTag || ((doc.unicode())[pos] != '>' && !hasPrefix( doc, pos, term)) )
	       && (doc.unicode())[pos] != '<'
	       && (doc.unicode())[pos] != '='
	       && !(doc.unicode())[pos].isSpace())
	{
	    if ( (doc.unicode())[pos] == '&')
		s += parseHTMLSpecialChar( doc, pos );
	    else {
		s += (doc.unicode())[pos];
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
	   (doc.unicode())[pos] != '<' ) {
	if ((doc.unicode())[pos].isSpace() && (doc.unicode())[pos] != QChar(0x00a0U) ){

	    if ( pre ) {
		if ( (doc.unicode())[pos] == ' ' )
		    s += QChar(0x00a0U);
		else
		    s += (doc.unicode())[pos];
	    }
	    else { // non-pre mode: collapse whitespace except nbsp
		while ( pos+1 < int(doc.length() ) &&
			(doc.unicode())[pos+1].isSpace()  && (doc.unicode())[pos+1] != QChar(0x00a0U) )
		    pos++;

		s += ' ';
	    }
	    pos++;
	    if ( justOneWord )
		return s;
	}
	else if ( (doc.unicode())[pos] == '&')
		s += parseHTMLSpecialChar( doc, pos );
	else {
	    s += (doc.unicode())[pos];
	    pos++;
	}
    }
    valid = valid && pos <= int(doc.length());
    return s;
}


bool QtRichText::hasPrefix(const QString& doc, int pos, QChar c)
{
    return valid && (doc.unicode())[pos] ==c;
}

bool QtRichText::hasPrefix(const QString& doc, int pos, const QString& s)
{
    if ( pos + s.length() >= doc.length() )
	return FALSE;
    for (int i = 0; i < int(s.length()); i++) {
	if ((doc.unicode())[pos+i] != s[i])
	    return FALSE;
    }
    return TRUE;
}

QString QtRichText::parseOpenTag(const QString& doc, int& pos,
				  QMap<QString, QString> &attr, bool& emptyTag)
{
    emptyTag = FALSE;
    pos++;
    if ( hasPrefix(doc, pos, '!') ) {
	if ( hasPrefix( doc, pos+1, "--")) {
	    pos += 3;
	    // eat comments
	    QString pref = QString::fromLatin1("-->");
	    while ( valid && !hasPrefix(doc, pos, pref )
			&& pos < int(doc.length()) )
		pos++;
	    if ( valid && hasPrefix(doc, pos, pref ) ) {
		pos += 3;
		eatSpace(doc, pos, TRUE);
	    }
	    else
		valid = FALSE;
	    emptyTag = TRUE;
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

    QString tag = parseWord(doc, pos, TRUE, TRUE);
    eatSpace(doc, pos, TRUE);
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

QString QtRichText::parseCloseTag( const QString& doc, int& pos )
{
    pos++;
    pos++;
    QString tag = parseWord(doc, pos, TRUE, TRUE);
    eatSpace(doc, pos, TRUE);
    eat(doc, pos, '>');
    return tag;
}

bool QtRichText::eatCloseTag(const QString& doc, int& pos, const QString& open)
{
    QString tag = parseCloseTag( doc, pos );
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



// convenience function
void QtRichText::draw(QPainter* p, int x, int y,
		      int ox, int oy, int cx, int cy, int cw, int ch,
		      QRegion& backgroundRegion, const QColorGroup& cg, const QtTextOptions& to )
{
    QtTextCursor tc( *this );
    QtTextParagraph* b = this;
    QFontMetrics fm( p->fontMetrics() );
    while ( b ) {
	tc.gotoParagraph( p, b );
	do {
	    tc.makeLineLayout( p, fm );
	    QRect geom( tc.lineGeometry() );
	    if ( geom.bottom()+y > cy && geom.top()+y < cy+ch )
		tc.drawLine( p, ox-x, oy-y, cx-x, cy-y, cw, ch, backgroundRegion, cg, to );
	}
	while ( tc.gotoNextLine( p, fm ) );
	b = tc.paragraph->nextInDocument();
    }
    flow()->drawFloatingItems( p, ox-x, oy-y, cx-x, cy-y, cw, ch, backgroundRegion, cg, to );
}


// convenience function
void QtRichText::doLayout( QPainter* p, int nwidth ) {
    QtTextCursor fc( *this );
    invalidateLayout();
    flow()->initialize( nwidth );
    fc.initParagraph( p, this );
    fc.updateLayout( p );
}

// convenience function
QString QtRichText::anchorAt( QPainter* p, int x, int y) const
{
    QFontMetrics fm( p->fontMetrics() );
    QtTextCursor tc( *((QtRichText*)this) );
    tc.gotoParagraph( p, (QtRichText*)this );
    QtTextParagraph* b = tc.paragraph;
    while ( b && tc.y() < y ) {
	if ( b && b->dirty ){
	    tc.initParagraph( p, b );
	    tc.updateLayout( p, y + 1 );
	}

	tc.gotoParagraph( p, b );

	if ( tc.y() + tc.paragraph->height > y ) {
	    do {
		tc.makeLineLayout( p, fm );
		QRect geom( tc.lineGeometry() );
		if ( geom.contains( QPoint(x,y) ) ) {
		    tc.gotoLineStart( p, fm );
		    while ( !tc.atEndOfLine() && geom.left() + tc.currentx + tc.currentoffsetx < x ) {
			tc.right( p );
		    }
		    if ( geom.left() + tc.currentx + tc.currentoffsetx > x )
			tc.left( p );
		    QtTextCharFormat format( *tc.currentFormat() );
		    if ( format.customItem() ) {
			// custom items may have anchors as well
			int h = format.customItem()->height;
			int nx = x - geom.x() - tc.currentx;
			int ny = y - geom.y() - tc.base + h;
			QString anchor = format.customItem()->anchorAt( p, nx, ny );
			if ( !anchor.isNull() )
			    return anchor;
		    }
		    return format.anchorHref();
		}
	    }
	    while ( tc.gotoNextLine( p, fm ) );
	}
	b = b->nextInDocument();
    }

    return QString::null;
}

bool QtRichText::clearSelection()
{
    bool result = FALSE;
    QtTextParagraph* b = this;
    while ( b->child )
	b = b->child;
    while ( b ) {
	if ( b->text.isSelected() ) {
	    result = TRUE;
	    b->text.clearSelection();
	    flow()->invalidateRect( QRect( 0, b->y, flow()->width, b->height ) );
	}
	b = b->nextInDocument();
    }
    return result;
}

QString QtRichText::selectedText()
{
    QString result;
    QtTextParagraph* b = this;
    while ( b->child )
	b = b->child;
    while ( b ) {
	int column = 0;
	if ( b->text.isSelected() ) {
	    for (int i = 0; i < b->text.length(); i++ ) {
		if ( b->text.isSelected( i ) ) {
		    QString& s = b->text.getCharAt( i );
		    if ( column + s.length() > 79 ) {
			result += '\n';
			column = 0;
		    }
		    result += s;
		    column += s.length();
		}
	    }
	    result += '\n';
	}
	b = b->nextInDocument();
    }
    return result;
}


QtStyleSheet::QtStyleSheet( QObject *parent, const char *name )
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


void QtTextParagraph::invalidateLayout()
{
    dirty = TRUE;
    QtTextParagraph* b = child;
    while ( b ) {
	b->dirty = TRUE;
	b->invalidateLayout();
	b = b->next;
    }
    if ( next && !next->dirty ) {
	next->invalidateLayout();
    }


    if ( parent ) {
	QtTextParagraph* p = parent;
	while ( p && !p->next && p->parent )
	    p = p->parent;
	if ( p->next && !p->next->dirty)
	    p->next->invalidateLayout();
    }
}


QtTextParagraph* QtTextParagraph::realParagraph()
{
    // to work around dummy paragraphs
    if ( parent &&style->name().isEmpty() )
	return parent->realParagraph();
    return this;
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

    child = next = prev = 0;
    height = y = 0;
    dirty = TRUE;
    selected = FALSE;
    flow_ = 0;
    if ( parent )
	flow_ = parent->flow();

    align = QStyleSheetItem::Undefined;

    if ( attributes_.contains("align") ) {
 	QString s = attributes_["align"].lower();
 	if ( s  == "center" )
 	    align = Qt::AlignCenter;
 	else if ( s == "right" )
 	    align = Qt::AlignRight;
	else
	    align = Qt::AlignLeft;
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


QtTextParagraph* QtTextParagraph::lastChild()
{
    if ( !child )
	return this;
    QtTextParagraph* b = child;
    while ( b->next )
	b = b->next;
    return b->lastChild();
}


QtTextFlow* QtTextParagraph::flow()
{
    if ( !flow_ ) {
	qDebug("AUTSCH!!!!!!! no flow");
    }
    return flow_;
}



QtTextRichString::QtTextRichString( QtTextFormatCollection* fmt )
    : formats( fmt )
{
    items = 0;
    len = 0;
    store = 0;
    selection = 0;
}

QtTextRichString::~QtTextRichString()
{
    for (int i = 0; i < len; ++i )
	formats->unregisterFormat( *items[i].format );
}

void QtTextRichString::clearSelection()
{
    for (int i = 0; i < len; ++i ) {
	items[i].selected = 0;
    }
    selection = FALSE;
}

bool QtTextRichString::isSelected() const
{
    return selection;
}


void QtTextRichString::clear()
{
    for (int i = 0; i < len; ++i )
	formats->unregisterFormat( *items[i].format );
    if ( items )
	delete [] items;
    items = 0;
    len = 0;
    store = 0;
}


void QtTextRichString::remove( int index, int len )
{
    for (int i = 0; i < len; ++i )
	formats->unregisterFormat( *formatAt( index + i ) );

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
    QtTextCharFormat* f = formats->registerFormat( form );

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

void QtTextRichString::setSelected( int index, bool selected )
{
    items[index].selected = selected;
    if ( selected )
	selection = TRUE;
}

bool QtTextRichString::isSelected( int index ) const
{
    return items[index].selected;
}


void QtTextRichString::setBold( int index, bool b )
{
     if ( bold( index ) == b )
 	return ;
     QtTextCharFormat* oldfmt = items[index].format;
     QFont f = oldfmt->font();
     QColor c = oldfmt->color();
     f.setBold( b );
     QtTextCharFormat newfmt( f, c );
     formats->unregisterFormat( *oldfmt );
     items[index].format =formats->registerFormat( newfmt );
     items[index].width = -1;
}

bool QtTextRichString::bold( int index ) const
{
    return items[index].format->font().bold();
}

void QtTextRichString::setLength( int l )
{
    if ( l <= store ) {
	len = l; // TODO shrinking
	return;
    } else {
	store = QMAX( l*2, 40 );
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
    formats = other.formats;
    len = other.len;
    items = 0;
    store = 0;
    if ( len ) {
	store = QMAX( len, 40 );
	items = new Item[ store ];
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
    formats = other.formats;
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


QtTextCursor::QtTextCursor(QtRichText& document )
{
    doc = &document;
    paragraph = doc;
    first = y_ = width = widthUsed = height = base = fill = 0;
    last = first - 1;
    current = currentx = currentoffset = currentoffsetx = 0;
    lmargin = rmargin = 0;
    static_lmargin = static_rmargin = 0;
    currentasc  = currentdesc = 0;
    xline_current = 0;
    xline = 0;
    xline_paragraph = 0;
    formatinuse = 0;
}
QtTextCursor::~QtTextCursor()
{
}


/*!
  Like gotoParagraph() but also initializes the paragraph
  (i.e. setting the cached values in the paragraph to the cursor's
  settings and not vice versa.

 */
void QtTextCursor::initParagraph( QPainter* p, QtTextParagraph* b )
{
	b->y = y_;
	b->height = 0;
	while ( b->child  ) {
	    b->child->y = b->y;
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
    flow = paragraph->flow();
    if ( paragraph->text.isEmpty() )
	paragraph->text.append( " ", paragraph->format );

    first = y_ = width = widthUsed = height = base = fill = 0;
    last = first - 1;


    y_ =  b->y;
    int m =  b->topMargin();
    flow->adjustFlow( y_, widthUsed, m ) ;

    y_ += m;

    static_lmargin = paragraph->totalMargin( QStyleSheetItem::MarginLeft );
    static_rmargin = paragraph->totalMargin( QStyleSheetItem::MarginRight );
    static_labelmargin = paragraph->totalLabelMargin();

    width = flow->width;
    lmargin = flow->adjustLMargin( y_, static_lmargin );
    lmargin += static_labelmargin;
    rmargin = flow->adjustRMargin( y_, static_rmargin );

    current = 0;

    currentx = lmargin;
    currentoffset = 0;
    currentoffsetx = 0;
    QFontMetrics fm( p->fontMetrics() );
    updateCharFormat( p, fm );

}

void QtTextCursor::update( QPainter* p )
{
    int i = current;
    int io = currentoffset;
    QFontMetrics fm( p->fontMetrics() );
    gotoParagraph( p, paragraph );
    makeLineLayout( p, fm );
    gotoLineStart( p, fm );
    while ( current < i )
	rightOneItem( p );
    while ( currentoffset < io )
	right( p );
}


bool QtTextCursor::gotoNextLine( QPainter* p, const QFontMetrics& fm )
{
    current = last;
    if ( atEnd() ) {
	current++;
	y_ += height + 1; // first pixel below us
	int m = paragraph->bottomMargin();
	QtTextParagraph* nid = paragraph->nextInDocument();
	if ( nid ) {
	    m -= nid->topMargin();
	}
	if ( m > 0 ) {
	    flow->adjustFlow( y_, widthUsed, m ) ;
	    y_ += m;
	}
	width = flow->width;
	lmargin = flow->adjustLMargin( y_, static_lmargin );
	lmargin += static_labelmargin;
	rmargin = flow->adjustRMargin( y_, static_rmargin );
	paragraph->height = y() - paragraph->y; //####
	paragraph->dirty = FALSE;
	return FALSE;
    }
    current++;
    currentx = lmargin;
    y_ += height;
    width = flow->width;
    lmargin = flow->adjustLMargin( y_, static_lmargin );
    lmargin += static_labelmargin;
    rmargin = flow->adjustRMargin( y_, static_rmargin );

    height = 0;
    updateCharFormat( p, fm );
    return TRUE;
}

void QtTextCursor::gotoLineStart( QPainter* p, const QFontMetrics& fm )
{
    current = first;
    currentoffset = currentoffsetx = 0;
    currentx = lmargin + fill;
    updateCharFormat( p, fm );
}


void QtTextCursor::updateCharFormat( QPainter* p, const QFontMetrics& fm )
{
    if ( pastEnd() )
	return;
    QtTextCharFormat* fmt = currentFormat();
    p->setFont( fmt->font() );
    p->setPen( fmt->color() );
    currentasc = fm.ascent();
    currentdesc = fm.descent();
    QtTextCustomItem* custom = fmt->customItem();
    if ( custom ) {
	if ( custom->width < 0 ) {
	    custom->realize( p );
	}
	if ( width >= 0 && custom->expandsHorizontally() ) {
	    custom->resize( p, width - lmargin - rmargin - fm.width(' ' ) );
	}
	if ( custom->placeInline() )
	    currentasc = custom->height;
    }
    formatinuse = fmt;
}


void QtTextCursor::drawLabel( QPainter* p, QtTextParagraph* par, int x, int y, int w, int h, int ox, int oy,
			      QRegion& backgroundRegion,
			      const QColorGroup& cg, const QtTextOptions& to )

{
    if ( !par->parent )
	return;
    if ( par->style->displayMode() != QStyleSheetItem::DisplayListItem )
	return;

    QRect r (x - ox, y-oy, w, h ); ///#### label width?
    to.erase( p, r );
    backgroundRegion = backgroundRegion.subtract( r );
    QStyleSheetItem::ListStyle s = par->parent->listStyle();

    QFont font = p->font();
    p->setFont( par->parent->format.font() );
    int size = p->fontMetrics().lineSpacing() / 3;
    if ( size > 12 )
	size = 12;

    switch ( s ) {
    case QStyleSheetItem::ListDecimal:
    case QStyleSheetItem::ListLowerAlpha:
    case QStyleSheetItem::ListUpperAlpha:
	{
	    int n = 1;
	    n = par->parent->numberOfSubParagraph( par, TRUE );
	    QString l;
	    switch ( s ) {
	    case QStyleSheetItem::ListLowerAlpha:
		if ( n < 27 ) {
		    l = QChar( ('a' + (char) (n-1)));
		    break;
		}
	    case QStyleSheetItem::ListUpperAlpha:
		if ( n < 27 ) {
		    l = QChar( ('A' + (char) (n-1)));
		    break;
		}
		break;
	    default:  //QStyleSheetItem::ListDecimal:
		l.setNum( n );
		break;
	    }
	    l += QString::fromLatin1(". ");
	    p->drawText( r, Qt::AlignRight|Qt::AlignVCenter, l);
	}
	break;
    case QStyleSheetItem::ListSquare:
	{
	    QRect er( r.right()-size*2, r.center().y()-1, size, size);
	    p->fillRect( er , cg.brush( QColorGroup::Foreground ) );
	}
	break;
    case QStyleSheetItem::ListCircle:
	{
	    QRect er( r.right()-size*2, r.center().y()-1, size, size);
	    p->drawEllipse( er );
	}
	break;
    case QStyleSheetItem::ListDisc:
    default:
	{
	    p->setBrush( cg.brush( QColorGroup::Foreground ));
	    QRect er( r.right()-size*2, r.center().y()-1, size, size );
	    p->drawEllipse( er );
	    p->setBrush( Qt::NoBrush );
	}
	break;
    }

    p->setFont( font );
}

void QtTextCursor::drawLine( QPainter* p, int ox, int oy,
			     int cx, int cy, int cw, int ch,
			     QRegion& backgroundRegion,
			     const QColorGroup& cg, const QtTextOptions& to )

{
    QFontMetrics fm( p->fontMetrics() );
    gotoLineStart( p, fm );

    if ( pastEndOfLine() ) {
	qDebug("try to draw empty line!!");
	return;
    }

    int gx = 0;
    int gy = y();

    int realWidth = QMAX( width, widthUsed );
    QRect r(gx-ox+lmargin, gy-oy, realWidth-lmargin-rmargin, height);

    bool clipMode = currentFormat()->customItem() && currentFormat()->customItem()->noErase();

    if (!clipMode )
	to.erase( p, r );

    if ( first == 0 ) {
	//#### TODO cache existence of label
	QtTextParagraph*it = paragraph;
	int m = 0;
	while ( it &&
		( it->style->displayMode() == QStyleSheetItem::DisplayListItem
		  || it->prev == 0 ) ) {
	    int itm = it->labelMargin();
	    m += itm;
	    drawLabel( p, it, gx+lmargin-m, gy, itm, height, ox, oy, backgroundRegion, cg, to );
	    m += it->margin( QStyleSheetItem::MarginLeft );
	    if ( it->style->displayMode() == QStyleSheetItem::DisplayListItem && it->prev )
		it = 0;
	    else
		it = it->parent;
	}
    }

    QString c;
    while ( !atEndOfLine() ) {
	QtTextCharFormat *format  = currentFormat();
	if ( !format->anchorHref().isEmpty() ) {
	    p->setPen( to.linkColor );
	    if ( to.linkUnderline ) {
		QFont f = format->font();
		f.setUnderline( TRUE );
		p->setFont( f );
	    }
	}
	bool selected = paragraph->text.isSelected(current);
	if ( selected ) {
	    int w = paragraph->text.items[current].width;
	    p->fillRect( gx-ox+currentx, gy-oy, w, height, cg.highlight() );
	    p->setPen( cg.highlightedText() );
	}

	QtTextCustomItem* custom = format->customItem();
	if ( custom ) {
	    int h = custom->height;
	    if ( custom->placeInline() )
		custom->draw(p, gx+currentx, gy+base-h, ox, oy,
			     cx, cy, cw, ch, backgroundRegion, cg, to );
	}
	else {
	    c = paragraph->text.charAt( current );
	    p->drawText(gx+currentx-ox, gy-oy+base, c, c.length());
	}
	if ( selected )
	    p->setPen( format->color() );
	gotoNextItem( p, fm );
	gy = y();
    }

    if (clipMode ) {
	p->setClipRegion( backgroundRegion );
	to.erase( p, r);
	p->setClipping( FALSE );
    }
    backgroundRegion = backgroundRegion.subtract(r);
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

bool QtTextCursor::inLastLine() const
{
    return last > paragraph->text.length()-2;
}

bool QtTextCursor::rightOneItem( QPainter* p )
{
    QFontMetrics fm( p->fontMetrics() );
    if ( atEnd() ) {
	QtTextParagraph* next = paragraph->nextInDocument();
	if ( next ) {
	    if ( next->dirty ) {
		updateLayout( p );
	    }
	    gotoParagraph( p, next );
	    makeLineLayout( p, fm );
	    gotoLineStart( p, fm );
	} else {
	    return FALSE;
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
    return TRUE;
}

void QtTextCursor::right( QPainter* p )
{
    if ( !pastEnd() && !pastEndOfLine() ) {
	QString c =  paragraph->text.charAt( current );
	if ( currentoffset  < int(c.length()) - 1 ) {
	    QtTextCharFormat* fmt = currentFormat();
	    p->setFont( fmt->font() );
	    QFontMetrics fm( p->fontMetrics() );
	    currentoffset++;
	    currentoffsetx = fm.width( c, currentoffset );
	    return;
	}
    }
    (void) rightOneItem( p );
}

void QtTextCursor::left( QPainter* p )
{
    QFontMetrics fm( p->fontMetrics() );
    if ( currentoffset > 0 ) {
	QString c =  paragraph->text.charAt( current );
	QtTextCharFormat* fmt = currentFormat();
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
		(void) rightOneItem( p );
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
	    (void) rightOneItem( p );
	QString c =  paragraph->text.charAt( current );
	if ( c.length() > 1 ) {
	    currentoffset = c.length() - 1;
	    QtTextCharFormat* fmt = currentFormat();
	    p->setFont( fmt->font() );
	    QFontMetrics fm( p->fontMetrics() );
	    currentoffsetx = fm.width( c, currentoffset );
	}
    }
}


void QtTextCursor::up( QPainter* p )
{
    if ( xline_paragraph != paragraph || xline_current != current )
	xline = x();
    QFontMetrics fm( p->fontMetrics() );

    gotoLineStart( p, fm );
    left( p );
    gotoLineStart( p, fm );
    while ( !atEndOfLine() && x()  < xline ) {
	right( p );
    }
    xline_paragraph = paragraph;
    xline_current = current;
}

void QtTextCursor::down( QPainter* p )
{
    if ( xline_paragraph != paragraph || xline_current != current )
	xline = x();
    while ( current < last )
	(void) rightOneItem( p );
    (void) rightOneItem( p );
    while ( !atEndOfLine() && x() < xline ) {
	right( p );
    }

    xline_paragraph = paragraph;
    xline_current = current;
}


void QtTextCursor::goTo( QPainter* p, int xpos, int ypos )
{
    QFontMetrics fm( p->fontMetrics() );
    gotoParagraph( p, doc );
    QtTextParagraph* b = paragraph;
    while ( b ) {
	gotoParagraph( p, b );
	b = paragraph;
	b = b->nextInDocument();
	//??? update dirty stuff here?
	if ( !b || y() + paragraph->height  > ypos ) {
	    do {
		makeLineLayout( p, fm );
		QRect geom( lineGeometry() );
		if ( ypos <= geom.bottom() || inLastLine() ) {
		    gotoLineStart( p, fm );
 		    while ( !atEndOfLine() && geom.left() + x() < xpos ) {
 			right( p );
 		    }
  		    if ( geom.left() + x() > xpos )
  			left( p );
		    return;
		}
	    }
	    while ( gotoNextLine( p, fm ) );
	}
    }
}


void QtTextCursor::setSelected( bool selected )
{
    if ( current < paragraph->text.length() ) {
	if ( selected )
	    paragraph->selected = TRUE;
	paragraph->text.setSelected( current, selected );
    }
}

bool QtTextCursor::isSelected() const
{
    if ( current < paragraph->text.length() )
	return ( paragraph->text.isSelected( current ) );
    return FALSE;
}

void QtTextCursor::insert( QPainter* p, const QString& text )
{
    QFontMetrics fm( p->fontMetrics() );
    if ( paragraph->text.isCustomItem( current ) ) {
	paragraph->text.insert( current, text, currentFormat()->formatWithoutCustom() );
	current++;
	currentoffset = 0;
    }
    else {
	QString& sref = paragraph->text.getCharAt( current );
	for ( uint i = 0; i < text.length(); i++ ) {
	    sref.insert( currentoffset, text[i] );
	    if ( text[i] == ' ' && currentoffset < int(sref.length())-1 ) { // not isSpace() to ignore nbsp
		paragraph->text.insert( current+1, sref.mid( currentoffset+1 ),
					*currentFormat() );
		sref.truncate( currentoffset+1 );
		current++;
		currentoffset = 0;
	    }
	    else {
		currentoffset++;
	    }
	}
    }

    updateParagraph( p );
}

bool QtTextCursor::split()
{
    QString s = paragraph->text.charAt( current );
    if ( currentoffset == 0 || currentoffset >= int(s.length()) ) {
	return FALSE;
    }
    bool sel = isSelected();
    paragraph->text.insert( current+1, s.mid( currentoffset ),
			    *currentFormat() );
    paragraph->text.getCharAt( current ).truncate( currentoffset );
    current++;
    currentoffset = 0;
    last++;
    setSelected( sel );
    return TRUE;
}

void QtTextCursor::updateParagraph( QPainter* p )
{
     int ph = paragraph->height;

     int oldy = y_;
     int oldfirst = first;
     int oldlast = last;
     int oldcurrent = current;
     int prevliney = oldy;

     QtTextCursor store ( *this );
     QFontMetrics fm( p->fontMetrics() );
     gotoParagraph( p, paragraph );
     do {
	 makeLineLayout( p, fm );
	 if ( last < oldcurrent )
	     prevliney = y_;
     } while ( gotoNextLine( p, fm ) );
     *this = store;

     update( p );

     int uy = QMIN( oldy, y_ );
     if ( current == first )
	 uy = QMIN( uy, prevliney );

     if ( ph != paragraph->height ) {
	 qDebug("height different ");
	 if ( paragraph->nextInDocument() )
	     paragraph->nextInDocument()->invalidateLayout();
	 flow->invalidateRect( QRect( QPoint(0, uy), QPoint(width, MAXINT-1000) ) );
     } else if ( first == oldfirst && last == oldlast && current != first ) {
	 flow->invalidateRect( QRect( 0, uy, width, height ) );
     }
     else {
	 flow->invalidateRect( QRect( 0, uy, width, paragraph->height - (uy - paragraph->y ) ) );
     }
}


void QtTextCursor::gotoNextItem( QPainter* p, const QFontMetrics& fm )
{
    if ( pastEnd() )
	return;
    // tabulators belong here
    QtTextRichString::Item* item = &paragraph->text.items[current];
    QtTextCustomItem* custom = item->format->customItem();
    if ( currentFormat() != formatinuse ){ // somebody may have changed the document
	updateCharFormat( p, fm );
    }
    if ( custom ) {
	if ( custom->placeInline() )
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

    if ( current < paragraph->text.length() && !paragraph->text.haveSameFormat( current-1, current ) ) {
 	updateCharFormat( p, fm );
    }
}

void QtTextCursor::makeLineLayout( QPainter* p, const QFontMetrics& fm  )
{
    first = current;

    if ( pastEnd() )
	return;

    last = first;
    int rh = 0;
    int rasc = 0;
    int rdesc = 0;

    // do word wrap
    int lastSpace =current;
    int lastHeight = rh;
    int lastWidth = 0;
    int lastAsc = rasc;
    int lastDesc = rdesc;
    bool noSpaceFound = TRUE;

    QtTextCharFormat* fmt = currentFormat();
    int fmt_current = current;
    p->setFont( fmt->font() );
    int space_width = fm.width(' ');
    int fm_ascent = fm.ascent();
    int fm_height = fm.height();

    widthUsed = 0;

    QPtrList<QtTextCustomItem> floatingItems;

    while ( !pastEnd() ) {

	if ( !paragraph->text.haveSameFormat( fmt_current, current ) ) {
	    fmt = currentFormat();
	    fmt_current = current;
	    p->setFont( fmt->font() );
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

	if ( custom && !custom->placeInline() ) {
	    floatingItems.append( custom );
	}

	bool custombreak = custom && custom->ownLine();

	if ( custombreak && current > first ) {
	    // break _before_ a custom expander
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
	    if ( custombreak ) {
		// also break _behind_ a custom expander
		++current;
		lastc = '\n';
	    }
	}
	// if a wordbreak is possible and required, do it. Unless we
	// have a newline, of course. In that case we break after the
	// newline to avoid empty lines.
	if ( currentx > width - rmargin - space_width
	     && !noSpaceFound && lastc != '\n' )
	    break;

	// word break is possible (a) after a space, (b) after a
	// newline, (c) before expandable item  or (d) at the end of the paragraph.
	if ( noSpaceFound || lastc == ' ' || lastc == '\n' || current == paragraph->text.length() ){
	    lastSpace = current - 1;
	    lastHeight = rh;
	    lastAsc = rasc;
	    lastDesc = rdesc;
	    lastWidth = currentx;
 	    if ( lastc == ' ' )
 		noSpaceFound = FALSE;
	}
	if ( lastc == '\n' )
	    break;
    }

    last = lastSpace;
    if ( last == paragraph->text.length() ) { // can happen with break behind custom expanders
	last--;
    }

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
	fill = (width - rmargin - lastWidth) / 2;
	break;
    case Qt::AlignRight:
	fill = width - rmargin - lastWidth;
	break;
    }

    current = lastSpace;//###

    int min = lastWidth ;
    if ( min + rmargin > widthUsed )
	widthUsed = min + rmargin;
    if ( widthUsed > width )
	fill = 0; // fall back to left alignment if there isn't sufficient space

    flow->adjustFlow( y_, widthUsed, height ) ;


    int fl = lmargin;
    int fr = width - rmargin;
    for ( QtTextCustomItem* item = floatingItems.first(); item; item = floatingItems.next() ) {
	item->y = y_ + height;
	flow->adjustFlow( item->y, item->width, item->height );
	if ( item->placement() == QtTextCustomItem::PlaceRight ) {
	    fr -= item->width;
	    item->x = fr;
	} else {
	    item->x = fl;
	    fl += item->width;
	}
	flow->registerFloatingItem( item, item->placement() == QtTextCustomItem::PlaceRight );
    }
}


/*\internal

  Note: The cursor's paragraph needs to be initialized
 */
bool QtTextCursor::updateLayout( QPainter* p, int ymax )
{
    QFontMetrics fm( p->fontMetrics() );
    QtTextParagraph* b = paragraph;
    gotoParagraph( p, b );
    while ( b && ( ymax < 0 || y_ <= ymax ) ) {

	if ( !b->dirty )
	    y_ = b->y + b->height;
	else do {
		makeLineLayout( p, fm );
	} while ( gotoNextLine( p, fm ) );

	b = b->nextInDocument();
	if ( b ) {
	    if ( b->dirty ) {
		initParagraph( p, b );
	    } else {
		gotoParagraph( p, b );
	    }
	}
    }
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
    return QRect( x(), y()+base-currentasc, 1, currentasc + currentdesc + 1 );
}
QRect QtTextCursor::lineGeometry() const
{
    int realWidth = QMAX( width, widthUsed );
    return QRect( 0, y(), realWidth, height );
}


QtTextFlow::QtTextFlow()
{
    width = widthUsed = height = 0;
}

QtTextFlow::~QtTextFlow()
{
}


void QtTextFlow::initialize( int w)
{
    height = 0;
    width = w;
    widthUsed = 0;

    leftItems.clear();
    rightItems.clear();
}


int QtTextFlow::adjustLMargin( int yp, int margin )
{
    QtTextCustomItem* item = 0;
    for ( item = leftItems.first(); item; item = leftItems.next() ) {
	if ( yp >= item->y && yp < item->y + item->height )
	    margin = QMAX( margin, item->x + item->width + 4 );
    }
    return margin;
}

int QtTextFlow::adjustRMargin( int yp, int margin )
{
    QtTextCustomItem* item = 0;
    for ( item = rightItems.first(); item; item = rightItems.next() ) {
	if ( yp >= item->y && yp < item->y + item->height )
	    margin = QMAX( margin, width - item->x - 4 );
    }
    return margin;
}


const int pagesize = 100000;

void QtTextFlow::adjustFlow( int  &yp, int w, int h, bool pages )
{
    if ( w > widthUsed )
	widthUsed = w;


    if ( FALSE && pages ) { // check pages
	int ty = yp;
	int yinpage = ty % pagesize;
 	if ( yinpage < 2 )
 	    yp += 2 - yinpage;
 	else
	    if ( yinpage + h > pagesize - 2 )
	    yp += ( pagesize - yinpage ) + 2;
    }

    if ( yp + h > height )
	height = yp + h;

}

void QtTextFlow::registerFloatingItem( QtTextCustomItem* item, bool right   )
{
    if ( right ) {
	if ( !rightItems.contains( item ) )
	    rightItems.append( item );
    }
    else if ( !leftItems.contains( item ) )
	leftItems.append( item );
}

void QtTextFlow::drawFloatingItems(QPainter* p,
				   int ox, int oy, int cx, int cy, int cw, int ch,
				   QRegion& backgroundRegion, const QColorGroup& cg, const QtTextOptions& to )
{
    QtTextCustomItem* item = 0;
    for ( item = leftItems.first(); item; item = leftItems.next() ) {
	item->draw( p, item->x, item->y, ox, oy, cx, cy, cw, ch, backgroundRegion, cg, to );
    }

    for ( item = rightItems.first(); item; item = rightItems.next() ) {
	item->draw( p, item->x, item->y, ox, oy, cx, cy, cw, ch, backgroundRegion, cg, to );
    }
}




QtTextTable::QtTextTable(const QMap<QString, QString> & attr  )
{
    cells.setAutoDelete( TRUE );

    cellspacing = 2;
    if ( attr.contains("cellspacing") )
	cellspacing = attr["cellspacing"].toInt();
    cellpadding = 1;
    if ( attr.contains("cellpadding") )
	cellpadding = attr["cellpadding"].toInt();
    border = 0;
    if ( attr.contains("border" ) ) {
	QString s( attr["border"] );
	if ( s == "true" )
	    border = 1;
	else
	    border = attr["border"].toInt();
    }

    if ( border )
	cellspacing += 2;
    outerborder = cellspacing + border;
    layout = new QGridLayout( 1, 1, cellspacing );

    fixwidth = 0;
    stretch = 0;
    if ( attr.contains("width") ) {
	bool b;
	QString s( attr["width"] );
	int w = s.toInt( &b );
	if ( b ) {
	    fixwidth = w;
	} else {
 	    s = s.stripWhiteSpace();
 	    if ( s.length() > 1 && s[ s.length()-1 ] == '%' )
		stretch = s.left( s.length()-1).toInt();
	}
    }

    cachewidth = 0;
}

QtTextTable::~QtTextTable()
{
    delete layout;
}

void QtTextTable::realize( QPainter* p)
{
    painter = p;
    for (QtTextTableCell* cell = cells.first(); cell; cell = cells.next() )
	cell->realize();

    width = 0;
}

void QtTextTable::draw(QPainter* p, int x, int y,
		       int ox, int oy, int cx, int cy, int cw, int ch,
		       QRegion& backgroundRegion, const QColorGroup& cg, const QtTextOptions& to )
{
    painter = p;
    for (QtTextTableCell* cell = cells.first(); cell; cell = cells.next() ) {
	if ( y + cell->geometry().top() < cy+ch && y + 2*outerborder + cell->geometry().bottom() > cy ) {
	    cell->draw( x+outerborder, y+outerborder, ox, oy, cx, cy, cw, ch, backgroundRegion, cg, to );
	    if ( border ) {
		const int w = 1;
		QRect r( x+outerborder+cell->geometry().x()-w-ox,
			 y+outerborder+cell->geometry().y()-w-oy,
			 cell->geometry().width()+2*w,
			 cell->geometry().height()+2*w);
		int s = cellspacing;
		p->fillRect( r.left()-s, r.top(), s, r.height(), cg.button() );
		p->fillRect( r.right(), r.top(), s, r.height(), cg.button() );
		p->fillRect( r.left()-s, r.top()-s, r.width()+2*s, s, cg.button() );
		p->fillRect( r.left()-s, r.bottom(), r.width()+2*s, s, cg.button() );
		qDrawShadePanel( p, r, cg, TRUE );
//  		QRect r2(x+outerborder-ox+cell->geometry().x()-w, y+outerborder-oy+cell->geometry().y()-w,
//  			cell->geometry().width()+2*w, cell->geometry().height()+2*w );
//  		backgroundRegion = backgroundRegion.subtract( r2 );

	    }
	}
    }
    if ( border ) {
	QRect r ( x-ox, y-oy, width, height );
	int s = border;
	p->fillRect( r.left(), r.top(), s, r.height(), cg.button() );
	p->fillRect( r.right()-s, r.top(), s, r.height(), cg.button() );
	p->fillRect( r.left(), r.top(), r.width(), s, cg.button() );
	p->fillRect( r.left(), r.bottom()-s, r.width(), s, cg.button() );
	qDrawShadePanel( p, r, cg, FALSE, border );
	backgroundRegion = backgroundRegion.subtract( r );
    }
}

void QtTextTable::resize( QPainter* p, int nwidth )
{
    if ( nwidth == cachewidth )
	return;
    cachewidth = nwidth;
    painter = p;

    if ( stretch )
	nwidth = nwidth * stretch / 100;

    width = nwidth + 2*outerborder;
    layout->invalidate();
    int shw = layout->sizeHint().width() + 2*outerborder;
    int mw = layout->minimumSize().width() + 2*outerborder;
    if ( stretch )
	width = QMAX( mw, nwidth );
    else
	width = QMAX( mw, QMIN( nwidth, shw ) );

    if ( fixwidth )
	width = fixwidth;

    // play it again, Sam, to fix the stretches
    layout->invalidate();
    mw = layout->minimumSize().width() + 2*outerborder;
    width = QMAX( width, mw );

    int h = layout->heightForWidth( width-2*outerborder );
    layout->setGeometry( QRect(0, 0, width-2*outerborder, h)  );
    height = layout->geometry().height()+2*outerborder;
};

QString QtTextTable::anchorAt( QPainter* p, int x, int y )
{
    painter = p;
    for (QtTextTableCell* cell = cells.first(); cell; cell = cells.next() ) {
	if ( cell->geometry().contains( QPoint(x,y) ) )
	    return cell->anchorAt( x+outerborder, y+outerborder );
    }
    return QString::null;
}



void QtTextTable::addCell( QtTextTableCell* cell )
{
    cells.append( cell );
    layout->addMultiCell( cell, cell->row(), cell->row() + cell->rowspan()-1,
			  cell->column(), cell->column() + cell->colspan()-1 );
//     if ( cell->stretch() ) {
// 	qDebug("colstrech %d to %d", cell->column(), cell->stretch() );
// 	//### other columns when colspan
// 	layout->setColStretch( cell->column(), cell->stretch() );
//     }
}

QtTextTableCell::QtTextTableCell(QtTextTable* table,
       int row, int column,
       const QMap<QString, QString> &attr,
       const QStyleSheetItem* style,
       const QtTextCharFormat& fmt, const QString& context,
       const QMimeSourceFactory &factory, const QtStyleSheet *sheet, const QString& doc, int& pos )
{
    maxw = QWIDGETSIZE_MAX;
    minw = 0;

    parent = table;
    row_ = row;
    col_ = column;
    stretch_ = 0;
    richtext = new QtRichText( attr, doc, pos, style,
 			       fmt, context, parent->cellpadding, &factory, sheet );
    rowspan_ = 1;
    colspan_ = 1;
    if ( attr.contains("colspan") )
	colspan_ = attr["colspan"].toInt();
    if ( attr.contains("rowspan") )
	rowspan_ = attr["rowspan"].toInt();

    background = 0;
    if ( attr.contains("bgcolor") ) {
	background = new QBrush(QColor( attr["bgcolor"] ));
    }

    hasFixedWidth = FALSE;
    if ( attr.contains("width") ) {
	bool b;
	QString s( attr["width"] );
	int w = s.toInt( &b );
	if ( b ) {
	    maxw = w;
	    minw = maxw;
	    hasFixedWidth = TRUE;
	} else {
 	    s = s.stripWhiteSpace();
 	    if ( s.length() > 1 && s[ s.length()-1 ] == '%' )
		stretch_ = s.left( s.length()-1).toInt();
	}
    }

    parent->addCell( this );
}

QtTextTableCell::~QtTextTableCell()
{
    delete richtext;
}

QSize QtTextTableCell::sizeHint() const
{
    //### see QLabel::sizeHint()
    return QSize(maxw,0).expandedTo( minimumSize() );
}

QSize QtTextTableCell::minimumSize() const
{
    if ( stretch_ )
	return QSize( QMAX( minw, parent->width * stretch_ / 100 - 2*parent->cellspacing), 0);
    return QSize(minw,0);
}

QSize QtTextTableCell::maximumSize() const
{
    return QSize( QWIDGETSIZE_MAX, QWIDGETSIZE_MAX );
}

QSizePolicy::ExpandData QtTextTableCell::expanding() const
{
    return QSizePolicy::BothDirections;
}

bool QtTextTableCell::isEmpty() const
{
    return FALSE;
}
void QtTextTableCell::setGeometry( const QRect& r)
{
    if ( r.width() != richtext->flow()->width ) {
	richtext->doLayout( painter(), r.width() );
    }
    geom = r;
}
QRect QtTextTableCell::geometry() const
{
    return geom;
}

bool QtTextTableCell::hasHeightForWidth() const
{
    return TRUE;
}

int QtTextTableCell::heightForWidth( int w ) const
{
    w = QMAX( minw, w ); //####PAUL SHOULD DO THAT

    if ( richtext->flow()->width != w ) {
	QtTextTableCell* that = (QtTextTableCell*) this;
	that->richtext->doLayout(painter(), w );
    }
    return richtext->flow()->height;
}

void QtTextTableCell::realize()
{

    if ( hasFixedWidth )
	return;

    richtext->doLayout(painter(), QWIDGETSIZE_MAX );
    maxw = richtext->flow()->widthUsed + 6;
    richtext->doLayout(painter(), 0 );
    minw = richtext->flow()->widthUsed;
}

QPainter* QtTextTableCell::painter() const
{
    return parent->painter;
}

void QtTextTableCell::draw(int x, int y,
			int ox, int oy, int cx, int cy, int cw, int ch,
			QRegion& backgroundRegion, const QColorGroup& cg, const QtTextOptions& to )
{
    if ( richtext->flow()->width != geom.width() )
	richtext->doLayout(painter(), geom.width() );

    QtTextOptions o(to);
    if ( background )
	o.paper = background;

    QRect r(x-ox+geom.x(), y-oy+geom.y(), geom.width(), geom.height() );
    richtext->draw(painter(), x+geom.x(), y+geom.y(), ox, oy, cx, cy, cw, ch, backgroundRegion, cg, o );

    painter()->setClipRegion( backgroundRegion );
    o.erase( painter(), r );
    backgroundRegion = backgroundRegion.subtract( r );
}

QString QtTextTableCell::anchorAt( int x, int y ) const
{
    return richtext->anchorAt( painter(), x - geometry().x(), y - geometry().y() );
}
