/**********************************************************************
**   Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
**   sametexth.cpp
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

#include <qcstring.h>
#include <qmap.h>

#include <metatranslator.h>

typedef QMap<QCString, MetaTranslatorMessage> TMM;
typedef QValueList<MetaTranslatorMessage> TML;

static QCString sameTextKey( const MetaTranslatorMessage& m )
{
    return QCString( m.context() ) + QCString( "::" ) +
	   QCString( m.sourceText() );
}

/*
  Augments a MetaTranslator with trivially derived translations.

  For example, if "Enabled:" is consistendly translated as "Eingeschaltet:" no
  matter the context or the comment, "Eingeschaltet:" is added as the
  translation of any untranslated "Enabled:" text and is marked Unfinished.
*/

void applySameTextHeuristic( MetaTranslator *tor )
{
    TMM translated, avoid;
    TMM::Iterator t;
    TML untranslated;
    TML::Iterator u;
    TML all = tor->messages();
    TML::Iterator it;

    for ( it = all.begin(); it != all.end(); ++it ) {
	if ( (*it).type() == MetaTranslatorMessage::Unfinished ) {
	    untranslated.append( *it );
	} else {
	    QCString key = sameTextKey( *it );
	    t = translated.find( key );
	    if ( t != translated.end() ) {
		if ( (*t).translation() != (*it).translation() ) {
		    translated.remove( key );
		    avoid.insert( key, *it );
		}
	    } else if ( !avoid.contains(key) ) {
		translated.insert( key, *it );
	    }
	}
    }

    for ( u = untranslated.begin(); u != untranslated.end(); ++u ) {
	t = translated.find( sameTextKey(*u) );
	if ( t != translated.end() ) {
	    MetaTranslatorMessage m( *u );
	    m.setTranslation( (*t).translation() );
	    tor->insert( m );
	}
    }
}
