/**********************************************************************
**   Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
**   listviews.cpp
**
**   This file is part of Qt Linguist.
**
**   See the file LICENSE included in the distribution for the usage
**   and distribution terms.
**
**   The file is provided AS IS with NO WARRANTY OF ANY KIND,
**   INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR
**   A PARTICULAR PURPOSE.
**
**********************************************************************/

#include <qpainter.h>

#include "listviews.h"

static const int Text0MaxLen = 100;

/*
   LVI implementation
*/

static QString rho( int n )
{
    QString s;
    s.sprintf( "%.10d", n );
    return s;
}

int LVI::count = 0;

LVI::LVI( QListView * parent, QString text )
    : QListViewItem( parent, text )
{
    setText( 0, rho(count++) );
}

LVI::LVI( QListViewItem * parent, QString text )
    : QListViewItem( parent, text )
{
    setText( 0, rho(count++) );
}

/*
    This is a cut-down version of QListViewItem::paintCell() -
    it is needed to produce the effect we want on obsolete items
 */
void LVI::drawObsoleteText( QPainter * p, const QColorGroup & cg, int column,
			    int width, int align )
{
    QListView * lv = listView();
    int marg = lv ? lv->itemMargin() : 1;
    int r = marg;
    p->fillRect( 0, 0, width, height(), cg.brush( QColorGroup::Base ) );
    if ( isSelected() && (column==0 || listView()->allColumnsShowFocus()) )
	p->fillRect( r - marg, 0, width - r + marg, height(),
		     cg.brush( QColorGroup::Highlight ) );
	    
    // Do the ellipsis thingy
    QString t = text( column );
    QString tmp;
    int i  = 0;
    if ( p->fontMetrics().width( t ) > width ) {
	tmp = "...";
	while ( p->fontMetrics().width( tmp + t[i] ) < width )
	    tmp += t[ i++ ];
	tmp.remove( 0, 3 );
	if ( tmp.isEmpty() )
	    tmp = t.left( 1 );
	tmp += "...";
	t = tmp;
    }
    if ( isSelected() )
	p->setPen( lv->palette().disabled().highlightedText() );
    else
	p->setPen( lv->palette().disabled().text() );
    
    if ( !t.isEmpty() ) {
	p->drawText( r, 0, width-marg-r, height(),
		     align | AlignVCenter | SingleLine, t );
    }
	    
}

QString LVI::key( int column, bool /*ascending*/ ) const
{
    // see Section 5, Exercice 4 in The Art of Computer Programming
    QString k = text( column ).lower();
    k.replace( QRegExp("&"), QString("") );
    k += QChar::null;
    k += text( column );
    return k;
}

/*
   MessageLVI implementation
*/
MessageLVI::MessageLVI( QListView *parent,
			const MetaTranslatorMessage & message,
			const QString& text, const QString& comment,
			ContextLVI * c )
    : LVI( parent ), m( message ), tx( text ), com( comment ), ctxt( c )
{
    if ( m.translation().isEmpty() ) {
	QString t = "";
	m.setTranslation( t );
    }
    QString shortened = text.simplifyWhiteSpace();
    if ( (int) shortened.length() > Text0MaxLen ) {
	QString dots = TrWindow::tr( "..." );
	shortened.truncate( Text0MaxLen - dots.length() );
	shortened.append( dots );
    }
    setText( 1, shortened );
    fini = TRUE;
    d = FALSE;

    if( m.type() ==  MetaTranslatorMessage::Unfinished )
 	setFinished( FALSE );
}

void MessageLVI::paintCell( QPainter * p, const QColorGroup & cg, int column, 
			    int width, int align )
{    
    if ( column == 0 ) {
	int x = (width/2) - TrWindow::pxOn->width()/2;
	int y = (height()/2) - TrWindow::pxOn->height()/2;

	int marg = listView() ? listView()->itemMargin() : 1;
	int r = marg;
	
	if ( isSelected() )
	    p->fillRect( r - marg, 0, width - r + marg, height(),
			 cg.brush( QColorGroup::Highlight ) );
	else
	    p->fillRect( 0, 0, width, height(), 
			 cg.brush( QColorGroup::Base ) );
		
  	if ( m.type() == MetaTranslatorMessage::Unfinished && danger() )
  	    p->drawPixmap( x, y, *TrWindow::pxDanger );
  	else if ( m.type() == MetaTranslatorMessage::Finished )
  	    p->drawPixmap( x, y, *TrWindow::pxOn );
  	else if ( m.type() == MetaTranslatorMessage::Unfinished )
  	    p->drawPixmap( x, y, *TrWindow::pxOff );
  	else if ( m.type() == MetaTranslatorMessage::Obsolete )
  	    p->drawPixmap( x, y, *TrWindow::pxObsolete );
    } else {
	if ( m.type() == MetaTranslatorMessage::Obsolete )
	    drawObsoleteText( p, cg, column, width, align );
	else
	    QListViewItem::paintCell( p, cg, column, width, align );
    }
}


void MessageLVI::setTranslation( const QString& translation )
{
    m.setTranslation( translation );
}

void MessageLVI::setFinished( bool finished )
{
    if ( !fini && finished ) {
	m.setType( MetaTranslatorMessage::Finished );
	repaint();
	ctxt->decrementUnfinishedCount();
    } else if ( fini && !finished ) {
	m.setType( MetaTranslatorMessage::Unfinished );
	repaint();
	ctxt->incrementUnfinishedCount();
    }
    fini = finished;
}

void MessageLVI::setDanger( bool danger )
{
    if ( !d && danger ) {
	ctxt->incrementDangerCount();
	repaint();
    } else if ( d && !danger ) {
	ctxt->decrementDangerCount();
	repaint();
    }
    d = danger;
}

QString MessageLVI::context() const
{
    return QString( m.context() );
}

MetaTranslatorMessage MessageLVI::message() const
{
    return m;
}

/*
   ContextLVI implementation
*/
ContextLVI::ContextLVI( QListView *lv, const QString& context  )
    : LVI( lv ), com( "" )
{
    messageItems.setAutoDelete( TRUE );

    unfinishedCount = 0;
    dangerCount   = 0;
    obsoleteCount = 0;
    itemCount = 0;
    setText( 1, context );
}

void ContextLVI::instantiateMessageItem( QListView * lv, MessageLVI * i )
{ 
    itemCount++;
    appendMessageItem( lv, i );
}

void ContextLVI::appendMessageItem( QListView * lv, MessageLVI * i )
{ 
    lv->takeItem( i );
    messageItems.append( i );
}

void ContextLVI::updateStatus()
{
    QString s;
    s.sprintf( "%d/%d", itemCount - unfinishedCount - obsoleteCount, 
	       itemCount - obsoleteCount );
    setText( 2, s );
}

void ContextLVI::paintCell( QPainter * p, const QColorGroup & cg, int column, 
		    int width, int align )
{
    if ( column == 0 ) {
	int x = (width/2) - TrWindow::pxOn->width()/2;
	int y = (height()/2) - TrWindow::pxOn->height()/2;

	int marg = listView() ? listView()->itemMargin() : 1;
	int r = marg;
	
	if ( isSelected() )
	    p->fillRect( r - marg, 0, width - r + marg, height(),
			 cg.brush( QColorGroup::Highlight ) );
	else
	    p->fillRect( 0, 0, width, height(),
			 cg.brush( QColorGroup::Base ) );
		
	if ( isContextObsolete() )
  	    p->drawPixmap( x, y, *TrWindow::pxObsolete );
  	else if ( unfinishedCount == 0 )
  	    p->drawPixmap( x, y, *TrWindow::pxOn );
  	else
  	    p->drawPixmap( x, y, *TrWindow::pxOff );

    } else {
	if ( isContextObsolete() )
	    drawObsoleteText( p, cg, column, width, align );
	else
	    QListViewItem::paintCell( p, cg, column, width, align );
    }
}

void ContextLVI::appendToComment( const QString& x )
{
    if ( !com.isEmpty() )
	com += QString( "\n\n" );
    com += x;
}

void ContextLVI::incrementUnfinishedCount()
{
    if ( unfinishedCount++ == 0 )
	repaint();
}

void ContextLVI::decrementUnfinishedCount()
{
    if ( --unfinishedCount == 0 )
	repaint();
}

void ContextLVI::incrementDangerCount()
{
    if ( dangerCount++ == 0 )
	repaint();
}

void ContextLVI::decrementDangerCount()
{
    if ( --dangerCount == 0 )
	repaint();
}

void ContextLVI::incrementObsoleteCount()
{
    if ( obsoleteCount++ == 0 )
	repaint();
}

bool ContextLVI::isContextObsolete()
{
    return (obsoleteCount == itemCount);
}

QString ContextLVI::fullContext() const
{
    return comment().stripWhiteSpace();
}
