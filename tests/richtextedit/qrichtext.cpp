/****************************************************************************
** $Id: //depot/qt/main/tests/richtextedit/qrichtext.cpp#9 $
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
			     int ox, int oy, int cx, int cy, int cw, int ch,
			     QRegion&, const QColorGroup&, const QtTextOptions& to)
{
    QRect rm( x-ox, y-oy, width, height);
    QRect ra( cx-ox, cy-oy, cw,  ch);
    QRect r = rm.intersect( ra );
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

QtTextRow::QtTextRow()
{
    x_ = y_ = width = height = base = 0;
    fill = 0;
    first = last = 0;
    box = 0;
    text = 0;
    dirty = TRUE;
    next = prev = 0;
}


QtTextRow::QtTextRow( QPainter* p, QtTextRow* row, QFontMetrics & fm,
		      QtBox* b, int w, int& min, int /*align*/)
{
    next = 0;
    prev = row;
    if ( prev )
	prev->next = this;
    
    x_ = y_ = width = height = base = 0;
    fill = 0;
    first = last = 0;
    box = b;
    text = 0;
    dirty = TRUE;

    width = w;

    box->setWidth(p, fm, width );
    width = QMAX( box->widthUsed, width );
    height = box->height;
    base = height;
    min = box->widthUsed;
}


QtTextRow::QtTextRow( QPainter* p, QtTextRow* row, QFontMetrics &fm,
		    const QtTextRichString* t, int &index, int w, int& min, int align)
{
    next = 0;
    prev = row;
    if ( prev )
	prev->next = this;
    
    x_ = y_ = width = height = base = 0;
    dirty = TRUE;

    width = w;

    first = last = 0;
    box = 0;
    text = t;


    first = index;

    int tx = 0;
    int rh = 0;
    int rasc = 0;
    int rdesc = 0;

    // do word wrap
    int lastSpace = index;
    int lastHeight = rh;
    int lastWidth = 0;
    int lastBearing = 0;
    int lastAsc = rasc;
    int lastDesc = rdesc;
    bool noSpaceFound = TRUE;

    QtTextCharFormat* fmt = text->formatAt( index );
    int fmt_index = index;
    p->setFont( fmt->font() );
    int minRightBearing = fm.minRightBearing();
    int space_width = fm.width(' ');
    int fm_ascent = fm.ascent();
    int fm_height = fm.height();

    while ( index < text->length() ) {
	int h,a,d;
	
	if ( !text->haveSameFormat( fmt_index, index ) ) {
	    fmt = text->formatAt( index );
	    fmt_index = index;
	    p->setFont( fmt->font() );
	    minRightBearing = fm.minRightBearing();
	    space_width = fm.width(' ');
	    fm_ascent = fm.ascent();
	    fm_height = fm.height();
	}

	QtTextRichString::Item* item = &text->items[index];
	QString c;
	QChar lastc;

	QtTextCustomItem* custom = item->format->customItem();
	if ( !custom ) {
	    c = item->c;
	    lastc = c[c.length()-1];
	    if ( item->width < 0 )
		item->width = fm.width( c );
	    tx += item->width;
	
	    h = fm_height;
	    a = fm_ascent;
	    d = h-a;
	} else {
	    // custom item
	    if ( custom->expandsHorizontally() )
		custom->width = width + fm.minRightBearing() - fm.width(' ');
	    tx += custom->width;
	    h = custom->height;
	    a = h;
	    d = 0;
	    if ( tx > width - space_width + minRightBearing && index > first ) {
		noSpaceFound = TRUE;
		lastc = '\n'; // fake newline
		--index;
		tx -= custom->width;
		h = a = d = 0;
	    }
	}

	if ( tx > width - space_width + minRightBearing && !noSpaceFound )
	    break;

	if ( h > rh )
	    rh = h;
	if ( a > rasc )
	    rasc = a;
	if ( d > rdesc )
	    rdesc = d;
	
	++index;
	
	// word break is possible (a) after a space, (b) after a
	// newline, (c) before expandable item  or (d) at the end of the box.
	if ( noSpaceFound || lastc == ' ' || lastc == '\n' || index == text->length() ){
	    lastSpace = index - 1;
	    lastHeight = rh;
	    lastAsc = rasc;
	    lastDesc = rdesc;
	    lastWidth = tx;
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

    height = QMAX(rh, rasc+rdesc);
    base = rasc;

    if (align == Qt::AlignCenter)
	fill = (width - lastWidth) / 2;
    else if (align == Qt::AlignRight)
	fill = width - lastWidth;
    else
	fill = 0;

    index = lastSpace;

    if ( lastWidth > width ) {
	width = lastWidth;
	fill = 0;
    }

    min = lastWidth - lastBearing;

    ++index;

}



void QtTextRow::move( int nx, int ny )
{
    x_ = nx;
    y_ = ny;
    if ( box ) {
	box->x = nx;
	box->y = ny;
    }
}

QtTextRow::~QtTextRow()
{
    QtTextRow* tmp = next;
    while ( next ) {
	tmp = next;
	next = tmp->next;
	tmp->next = 0;
	delete tmp;
    }
}



void QtTextRow::draw( QPainter* p, int obx, int oby, int ox, int oy, int cx, int cy, int cw, int ch,
		  QRegion& backgroundRegion, const QColorGroup& cg, const QtTextOptions& to,
		  bool onlyDirty, bool onlySelection)
{

    if (!intersects(cx-obx, cy-oby, cw,ch)) {
	dirty = FALSE;
	return;
    }


    if ( box ) {
	//we have to draw the box
	box->draw(p, obx+x_, oby+y_, ox, oy, cx, cy, cw, ch,
		  backgroundRegion, cg, to, dirty?FALSE:onlyDirty, onlySelection);
	dirty = FALSE;
	return;
    }

    QRegion r(x_+obx-ox, y_+oby-oy, width, height);

    backgroundRegion = backgroundRegion.subtract(r);

    if (onlyDirty) {
	if (!dirty)
	    return;
    }

    dirty = FALSE;

    if (TRUE ) { //!onlyDirty && !onlySelection && to.paper) {
	if ( to.paper->pixmap() )
	    p->drawTiledPixmap(x_+obx-ox, y_+oby-oy, width, height, *to.paper->pixmap(), x_+obx, y_+oby);
	else
	    p->fillRect(x_+obx-ox, y_+oby-oy, width, height, *to.paper);
    }

    int tx = x_;

    if ( fill > 0 )
	tx += fill;


    QFontMetrics fm( p->fontMetrics() );
    QString c;
    for ( int index = first; index <= last; ++index ) {
	QtTextCharFormat *format  = text->formatAt( index );
	p->setFont( format->font() );
	p->setPen( format->color() );
	
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
	    format->customItem()->draw(p, tx+obx, y_+oby+base-h, ox, oy,
				       cx, cy, cw, ch, backgroundRegion, cg, to );
	    tx += format->customItem()->width;
	}
	else {
	    c = text->charAt( index );
	    p->drawText(tx+obx-ox, y_+oby-oy+base, c, c.length());
	    tx += fm.width( c );
	}
    }

}

/*!
  \xpos is in the coordinate system of the row's box
 */
void QtTextRow::indexAt(QPainter* p, int xpos, int &index, int& offset )
{
    index = offset = 0;
    if ( !text )
	return;
    QFontMetrics fm( p->fontMetrics() );
    int tx = x_;
    if ( fill > 0 )
	tx += fill;
    int i = first;
    while ( 1 ) {
	QtTextCharFormat *format  = text->formatAt( i );
	p->setFont( format->font() );
	QString c = text->charAt( i );
	if ( format->customItem() )
	    tx += format->customItem()->width;
	else
	    tx += fm.width( c );
	if ( tx > xpos ) {
	    if ( !c.isNull() ) {
		tx -= fm.width( c );
		while ( offset < int(c.length())-1 && tx + fm.width( c, offset+1 ) < xpos )
		    offset++;
	    }
	    break;
	} 
	
	if ( i == last ) {
	    offset = c.length()-1;
	    break;
	}
	++i;
    }
    index = i;
}



void QtTextRow::locate(QPainter* p, int index, int offset, int &lx, int &ly, int &lh)
{
    QFontMetrics fm( p->fontMetrics() );
    QString c;
    int tx = x_;
    if ( fill > 0 )
	tx += fill;
    lx = tx;
    lh = fm.lineSpacing();

    int cursor_ascent = base;
    for ( int i = first; i <= index; ++i ) {
	QtTextCharFormat *format  = text->formatAt( i );
	p->setFont( format->font() );
	
	if ( format->customItem() ) {
	    lh = format->customItem()->height;
	    cursor_ascent = lh;
	    lx = tx;
	    tx += format->customItem()->width;
	    c = QString::null;
	}
	else {
	    lh = fm.lineSpacing();
	    cursor_ascent = fm.ascent();
	    c = text->charAt( i );
	    lx = tx;
	    tx += fm.width( c );
	}
    }
    if ( !c.isNull() && offset > 0 ) {
	lx += fm.width( c, offset );
    }
    ly = y_ + base - cursor_ascent;
}




QtRichText::QtRichText( const QString &doc, const QFont& font,
		      const QString& context,
		      int margin,  const QMimeSourceFactory* factory, const QtStyleSheet* sheet  )
    :QtBox( 0, new QtTextFormatCollection(),
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
    delete base;
}



void QtRichText::dump()
{
}



bool QtRichText::isValid() const
{
    return valid;
}


void QtRichText::setWidth (QPainter* p, int newWidth )
{
    QFontMetrics fm( p->fontMetrics() );
    QtBox::setWidth( p, fm, newWidth );
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
bool QtRichText::parse (QtBox* current, const QStyleSheetItem* curstyle, QtBox* dummy,
			QtTextCharFormat fmt, const QString &doc, int& pos)
{
    bool pre = current->whiteSpaceMode() == QStyleSheetItem::WhiteSpacePre;
    if ( !pre )
	eatSpace(doc, pos);
    while ( valid && pos < int(doc.length() )) {
	int beforePos = pos;
	if (hasPrefix(doc, pos, QChar('<')) ){
	    if (hasPrefix(doc, pos+1, QChar('/'))) {
		return TRUE;
	    }
	    QMap<QString, QString> attr;
	    bool emptyTag = FALSE;
	    QString tagname = parseOpenTag(doc, pos, attr, emptyTag);
	
	    const QStyleSheetItem* nstyle = sheet_->item(tagname);
 	    if ( nstyle && !nstyle->selfNesting() && ( tagname == curstyle->name() ) ) {
 		pos = beforePos;
 		return FALSE;
 	    }

	    if ( nstyle && !nstyle->allowedInContext( curstyle ) ) {
		QString msg;
		msg.sprintf( "QtText Warning: Document not valid ( '%s' not allowed in '%s' #%d)",
			 tagname.ascii(), current->style->name().ascii(), pos);
		sheet_->error( msg );
		pos = beforePos;
		return FALSE;
	    }
	
	    // TODO was wenn wir keinen nstyle haben?
	    if ( !nstyle )
		nstyle = nullstyle;
	
	    QtTextCustomItem* custom = sheet_->tagEx( tagname, attr, contxt, *factory_ , emptyTag );
	    if ( custom ) {
		if ( current->child && !dummy ) {
		    dummy = new QtBox( current, formats, fmt,  nullstyle );
		    QtBox* it = current->child;
		    while ( it->next )
			it = it->next;
		    it->next = dummy;
		    dummy->prev = it;
		}
		(dummy?dummy:current)->text.append( "", fmt.makeTextFormat( nstyle, attr, custom ) );
	    }
	    else if (nstyle->displayMode() == QStyleSheetItem::DisplayBlock
		|| nstyle->displayMode() == QStyleSheetItem::DisplayListItem
		|| nstyle->displayMode() == QStyleSheetItem::DisplayNone
		) {
		if ( !current->text.isEmpty() ) {
		    dummy = new QtBox( current, formats, fmt, nullstyle );
		    dummy->text = current->text;
		    current->text.clear();
		    current->child = dummy;
		}
		QtBox* subbox = new QtBox( current, formats, fmt.makeTextFormat(nstyle,attr), nstyle, attr );
		if ( !current->child )
		    current->child = subbox;
		else {
		    QtBox* it = current->child;
		    while ( it->next )
			it = it->next;
		    it->next = subbox;
		    subbox->prev = it;
		}
		dummy = 0;
		if (parse( subbox, nstyle, 0, fmt.makeTextFormat( nstyle, attr ), doc, pos) ) {
		    if ( !subbox->child ) {
			subbox->text.append( " ", subbox->format );
		    }
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
		dummy = new QtBox( current, formats, fmt,  nullstyle );
		    QtBox* it = current->child;
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

void QtBox::draw(QPainter* p, int obx, int oby, int ox, int oy, int cx, int cy, int cw, int ch,
	  QRegion& backgroundRegion,
	  const QColorGroup& cg, const QtTextOptions& to,
	  bool onlyDirty, bool onlySelection )
{
//     if (onlySelection && !isSelectionDirty)
// 	return;
//     isSelectionDirty = 0;

    if ( !onlySelection && style->displayMode() == QStyleSheetItem::DisplayListItem && rows ) {
	p->setFont( format.font() );
	QFontMetrics fm( p->fontMetrics() );
	int itemsize = fm.width('x');
	
	QtTextRow* row = rows;
	int rowy = row->y();
	int rowh = row->height;
	if ( row->box ) {
	    rowy += row->box->rows->y();
	    rowh = row->box->rows->height;
	}
	QRect r (obx-ox + row->x() - 4*itemsize, oby-oy + rowy, 4*itemsize, rowh); //#### label width
	if ( to.paper ) {
 	    if ( to.paper->pixmap() )
 		p->drawTiledPixmap( r, *to.paper->pixmap(), QPoint(r.x()+ox, r.y()+oy) );
 	    else
		p->fillRect(r, *to.paper );
	}
	
	QtBox* b = parent;
	QStyleSheetItem::ListStyle s = b?b->listStyle():QStyleSheetItem::ListDisc;
	
	
	switch ( s ) {
	case QStyleSheetItem::ListDecimal:
	case QStyleSheetItem::ListLowerAlpha:
	case QStyleSheetItem::ListUpperAlpha:
	    {
		int n = 1;
		if ( b )
		    n = b->numberOfSubBox( this, TRUE );
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
		QRect er( r.right()-itemsize, r.center().y(), itemsize/2, itemsize/2);
		p->fillRect( er , cg.brush( QColorGroup::Foreground ) );
	    }
	    break;
	case QStyleSheetItem::ListCircle:
	    {
		QRect er( r.right()-itemsize, r.center().y(), itemsize/2, itemsize/2);
		p->drawEllipse( er );
	    }
	    break;
	case QStyleSheetItem::ListDisc:
	default:
	    {
		p->setBrush( cg.brush( QColorGroup::Foreground ));
		QRect er( r.right()-itemsize, r.center().y(), itemsize/2, itemsize/2);
		p->drawEllipse( er );
		p->setBrush( Qt::NoBrush );
	    }
	    break;
	}
	
	backgroundRegion = backgroundRegion.subtract( r );
    }

    for (QtTextRow* row = rows; row; row = row->next ) {
	row->draw(p, obx, oby, ox, oy, cx, cy, cw, ch, backgroundRegion, cg, to, onlyDirty, onlySelection);
    }

}

void QtBox::locate( QPainter* p, int index, int offset, int &lx, int &ly, int &lheight )
{
    for (QtTextRow* row = rows; row; row = row->next) {
	if ( index >= row->first && index <= row->last ) {
	    row->locate( p, index, offset, lx, ly, lheight );
	    QtBox* b = this; // TODO qtbox needs mapToParent, mapFromGlobal, etc. #####
	    while ( b ) {
		lx += b->x;
		ly += b->y;
		b = b->parent;
	    }
	    return;
	}
    }
}


void QtBox::setWidth( QPainter* p, QFontMetrics& fm,  int newWidth, bool forceResize )
{
    if (newWidth == width && !forceResize) // no need to resize
	return;

    if (style->displayMode() == QStyleSheetItem::DisplayNone) {
	height = 0;
	return;
    }
    
    // page crap
    int pagesize = 600;
    int pagemargin = 20;
    QtBox* b = this;
    int gx = 0;
    int gy = 0;
    while ( b ) {
	gx += b->x;
	gy += b->y;
	b = b->parent;
    }


    QtTextRow* oldRows = 0;
    if ( newWidth == width ){
	// reduce flicker by storing the old rows.
	oldRows = rows;
	rows = 0;
    }
    else {
	delete rows;
	rows = 0;
    }
	

    width = newWidth;
    widthUsed = 0;
    height = 0;

    p->setFont( format.font() );
    int label_offset = 0;
    if ( style->displayMode() == QStyleSheetItem::DisplayListItem )
	label_offset = 4 * fm.width('x'); // ###hardcoded

    int ncols = 1; // TODO numberOfColumns();
    int colwidth = newWidth / ncols;
    if (colwidth < 10)
	colwidth = 10;

    QtTextRow* row = 0;

    int margintop = margin( QStyleSheetItem::MarginTop );
    int marginbottom = margin( QStyleSheetItem::MarginBottom );
    int marginleft = margin( QStyleSheetItem::MarginLeft );
    int marginright = margin( QStyleSheetItem::MarginRight );
    int marginhorizontal = marginright + marginleft;

    int h = margintop;

    // two different kind of boxes, one has subboxes, the other has text

    if ( child ) {
	int min = 0;
	for ( QtBox* it = child; it; it = it->next ) {
	    it->x = marginleft + label_offset; // TODO
	    it->y = h;
	    row = new QtTextRow( p, row, fm, it,
				 colwidth-marginhorizontal - label_offset, min,
				 alignment() );
	    if ( !rows )
		rows = row;
	    row->move (marginleft + label_offset, h );
	    h += row->height;
	    widthUsed = QMAX( widthUsed , min + marginhorizontal + label_offset);
	}
    } else { // real text
	int index = 0;
	int min;
	while ( index < text.length() ) {
	    row = new QtTextRow( p, row, fm, &text, index,
				 colwidth-marginhorizontal - label_offset, min,
				 alignment() );
	    if ( !rows )
		rows = row;
	    
	    
	    // page crap
	    int yinpage = (gy + h ) % pagesize;
	    if ( yinpage < pagemargin )
		h = ( (gy+h)/pagesize) * pagesize + pagemargin - gy;
	    else  if ( yinpage + row->height  > pagesize - pagemargin )
		h = ( (gy+h)/pagesize) * pagesize + pagesize + pagemargin - gy;
	    
	    row->move (marginleft + label_offset, h );
	    h += row->height;
	    widthUsed = QMAX( widthUsed , min + marginhorizontal + label_offset);
	}
    }

    height = h;

    // adapt colwidth in case some rows didn't fit
    widthUsed *= ncols;
    colwidth = QMAX( width, widthUsed) / ncols;
    if (colwidth < 10)
 	colwidth = 10;

    if ( oldRows || ncols > 1 ) {
	// do multi columns if required. Also check with the old rows to
	// optimize the refresh

	row = rows;
	QtTextRow* old = oldRows;
	height = 0;
	h /= ncols;
	for (int col = 0; col < ncols; col++) {
	    int colheight = margintop;
	    for (; row && colheight < h; row = row->next ) {
		row->move ( col  * colwidth + marginleft + label_offset,
			    colheight );
		
		colheight += row->height;
		
		if ( old) {
		    if ( row->box ) {
			// do not check a height change of box rows!
			if ( old->first == row->first &&
			     old->last == row->last &&
			     old->width == old->width &&
			     old->x() == row->x() &&
			     old->y() == row->y() )
			    row->dirty = old->dirty;
		    } else if ( old->first == row->first &&
				old->last == row->last &&
				old->height == row->height &&
				old->width == old->width &&
				old->x() == row->x() &&
				old->y() == row->y() )
			row->dirty = old->dirty;
		    old = old->next;
		}
	    }
	    height = QMAX( height, colheight );
	}
    }

    delete oldRows;
    
    // collapse the bottom margin
    if ( !next && parent ) {
	// ignore bottom margin
    } else if ( next  ) {
	// collapse
	height += QMAX( next->style->margin( QStyleSheetItem::MarginTop), marginbottom );
    } else {
	// nothing to collapse
	height += marginbottom;
    }
}


QStyleSheetItem::ListStyle QtBox::listStyle()
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


int QtBox::numberOfSubBox( QtBox* subbox, bool onlyListItems)
{
    QtBox* it = child;
    int i = 1;
    while ( it && it != subbox ) {
	if ( !onlyListItems || it->style->displayMode() == QStyleSheetItem::DisplayListItem )
	    ++i;
	it = it->next;
    }
    return i;
}

QtBox::QtBox( QtBox* p, QtTextFormatCollection* formatCol, const QtTextCharFormat& fmt,
	      const QStyleSheetItem *stl, const QMap<QString, QString> &attr )
    : parent( p ), formats( formatCol ), format( fmt ), text( formatCol ), style ( stl ), attributes_( attr )
{
    formats->registerFormat( format );
    child = next = prev = 0;
    rows = 0;
    width = widthUsed = height = x = y = 0;
};


QtBox::QtBox( QtBox* p, QtTextFormatCollection* formatCol, const QtTextCharFormat& fmt,
	      const QStyleSheetItem *stl )
    : parent( p ), formats( formatCol ), format( fmt ), text( formats ), style ( stl )
{
    formats->registerFormat( format );
    child = next = prev = 0;
    rows = 0;
    width = widthUsed = height = x = y = 0;
};

QtBox::~QtBox()
{
    formats->unregisterFormat( format );
    QtBox* tmp = child;
    while ( child ) {
	tmp = child;
	child = child->next;
	delete tmp;
    }
    delete rows;
}

QtBox* QtBox::nextInDocument()
{
    if ( next ) {
	QtBox* b = next;
	while ( b->child )
	    b = b->child;
	return b;
    }
    if ( parent ) {
	return parent->nextInDocument();
    }
    return 0;
}

QtBox* QtBox::prevInDocument()
{
    if ( prev ){
	QtBox* b = prev;
	while ( b->child )
	    b = b->child;
	return b;
    }
    if ( parent ) {
	return parent->prevInDocument();
    }
    return 0;
}


QtTextCursor::QtTextCursor(QtRichText& doc)
{
    document = &doc;
    box = document;
    while ( box->child )
	box = box->child;
    index = 0;
    offset = 0;

    x = y = height = 0;
    
    xline = 0;
    yline = 0;
    ylineOffsetClean = FALSE;
}

QtTextCursor::~QtTextCursor()
{
}

void QtTextCursor::right(QPainter* p, bool select )
{
    qDebug("cursor right ");
    QString s;
    if ( index < int(box->text.length()) )
	s =  box->text.charAt( index );
    if ( offset  < int(s.length()) - 1 ) {
	offset++;
	qDebug("cursor right in item ");
    }
    else if ( index < box->text.length() - 1 ) {
	index++;
	offset = 0;
	qDebug("cursor right in box ");
    } else if ( box->nextInDocument() ) {
	index = 0;
	offset = 0;
	qDebug("cursor next  box ");
	box = box->nextInDocument();
    }
    calculatePosition( p );
}

void QtTextCursor::left(QPainter* p, bool select )
{
    qDebug("cursor left ");
    QString s;
    if ( index > 0 && index < int(box->text.length()) )
	s =  box->text.charAt( index );
    if ( offset  > 0  ) {
	offset--;
	qDebug("cursor left ");
    }
    else if ( index > 0 ) {
	index--;
	s =  box->text.charAt( index );
	offset = s.length()-1;
	qDebug("cursor left in box ");
    } else if ( box->prevInDocument() ) {
	box = box->prevInDocument();
	index = box->text.length()-1;
	s =  box->text.charAt( index );
	offset = s.length()-1;
	qDebug("cursor prev  box ");
    }
    calculatePosition( p );
}

void QtTextCursor::draw(QPainter* p,  int ox, int oy, int cx, int cy, int cw, int ch)
{
    if ( QMAX( x, cx ) <= QMIN( x+width(), cx+cw ) &&
	 QMAX( y, cy ) <= QMIN( y+height, cy+ch ) ) {
	p->drawLine(x-ox, y-oy, x-ox, y-oy+height-1);
	// warwick says two-pixels cursor are ugly
	//	p->drawLine(x+1-ox, y-oy, x+1-ox, y-oy+height-1);
    }
}



void QtTextCursor::calculatePosition( QPainter* p )
{
    box->locate(p, index, offset, x, y, height );
    xline = x;
    yline = y;
    ylineOffsetClean = FALSE;
    qDebug("cursor position %d %d  ( %p, %d, %d)", x, y, box, index, offset );
}


void QtTextCursor::goTo(QPainter* p, int xarg, int yarg, bool select )
{
    if ( !document->rows )
	return;
    QtTextRow* row = document->rows;
    int obx = document->x;
    int oby = document->y;
    QtBox* b = document;
    do {
	while ( row && !row->intersects( xarg-obx, yarg-oby, 0, 0 ) ) 
	    row = row->next;
	if ( !row )
	    break;
	if ( row->box ) {
	    b = row->box;
	    obx += b->x;
	    oby += b->y;
	    row = b->rows;
	} else {
	    box = b;
	    row->indexAt( p, xarg - obx, index, offset );
	    break ;
	}
    } while ( 1 );
	
    calculatePosition( p );
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
    delete [] items;
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
	for (int i = len+1; i > index; --i)
	    items[i] = items[i-1];
    }

    items[index].c = c;
    items[index].format = f;
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
