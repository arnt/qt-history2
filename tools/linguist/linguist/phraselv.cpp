/**********************************************************************
**   Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
**   phraselv.cpp
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

/*  TRANSLATOR PhraseLV

  The phrase list in the right panel of the main window (with Source phrase,
  Target phrase, and Definition in its header) is a PhraseLV object.
*/

#include <qregexp.h>
#include <qwhatsthis.h>
#include <qheader.h>

#include "phraselv.h"

class WhatPhrase : public QWhatsThis
{
public:
    WhatPhrase( PhraseLV *w );

    virtual QString text( const QPoint& p );

private:
    PhraseLV *parent;
};

WhatPhrase::WhatPhrase( PhraseLV *w )
    : QWhatsThis( w )
{
    parent = w;
}

QString WhatPhrase::text( const QPoint& p )
{
    QListViewItem *item = parent->itemAt( p );
    if ( item == 0 )
	return PhraseLV::tr( "This is a list of phrase entries relevant to the"
		  " source text.  Each phrase is supplemented with a suggested"
		  " translation and a definition." );
    else
	return QString( PhraseLV::tr("<p><u>%1:</u>&nbsp;&nbsp;%2</p>"
				     "<p><u>%3:</u>&nbsp;&nbsp;%4</p>"
				     "<p><u>%5:</u>&nbsp;&nbsp;%6</p>") )
	       .arg( parent->columnText(PhraseLVI::SourceTextShown) )
	       .arg( item->text(PhraseLVI::SourceTextShown) )
	       .arg( parent->columnText(PhraseLVI::TargetTextShown) )
	       .arg( item->text(PhraseLVI::TargetTextShown) )
	       .arg( parent->columnText(PhraseLVI::DefinitionText) )
	       .arg( item->text(PhraseLVI::DefinitionText) );
}

PhraseLVI::PhraseLVI( PhraseLV *parent, const Phrase& phrase )
    : QListViewItem( parent )
{
    setPhrase( phrase );
}

QString PhraseLVI::key( int column, bool ascending ) const
{
    if ( text( column ) == NewPhrase ) {
	// always first
	return ascending ? QString( "" ) : QString::null;
    } else {
	// see Section 5, Exercise 4 in The Art of Computer Programming
	QString k = text( column ).lower();
	k.replace( QRegExp(QString("&")), QString("") );
	k += QChar::null;
	k += text( column );
	return k;
    }
}

void PhraseLVI::setPhrase( const Phrase& phrase )
{
    setText( SourceTextShown, phrase.source().simplifyWhiteSpace() );
    setText( TargetTextShown, phrase.target().simplifyWhiteSpace() );
    setText( DefinitionText, phrase.definition() );
    setText( SourceTextOriginal, phrase.source() );
    setText( TargetTextOriginal, phrase.target() );
}

Phrase PhraseLVI::phrase() const
{
    return Phrase( text(SourceTextOriginal), text(TargetTextOriginal), text(DefinitionText) );
}

PhraseLV::PhraseLV( QWidget *parent, const char *name )
    : QListView( parent, name )
{
    setAllColumnsShowFocus( TRUE );
    setShowSortIndicator( TRUE );
    for ( int i = 0; i < 3; i++ )
	addColumn( QString::null, 120 );
    setColumnText( PhraseLVI::SourceTextShown, tr("Source phrase") );
    setColumnText( PhraseLVI::TargetTextShown, tr("Translation") );
    setColumnText( PhraseLVI::DefinitionText, tr("Definition") );
    header()->setStretchEnabled( TRUE, -1 );
    what = new WhatPhrase( this );
}

PhraseLV::~PhraseLV()
{
//    delete what;
}

QSize PhraseLV::sizeHint() const
{
    return QSize( QListView::sizeHint().width(), 50 );
}
