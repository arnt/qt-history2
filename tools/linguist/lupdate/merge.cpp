/**********************************************************************
**   Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
**   merge.cpp
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

#include <metatranslator.h>

// defined in numberh.cpp
extern void applyNumberHeuristic( MetaTranslator *tor );
// defined in sametexth.cpp
extern void applySameTextHeuristic( MetaTranslator *tor );

typedef QValueList<MetaTranslatorMessage> TML;

/*
  Merges two MetaTranslator objects into the first one.  The first one is a set
  of source texts and translations for a previous version of the
  internationalized program; the second one is a set of fresh source text newly
  extracted from the source code, without any translation yet.
*/

void merge( MetaTranslator *tor, const MetaTranslator *virginTor  )
{
    TML all = tor->messages();
    TML::Iterator it;

    /*
      The types of all the messages from the vernacular translator are updated
      according to the virgin translator.
    */
    for ( it = all.begin(); it != all.end(); ++it ) {
	MetaTranslatorMessage::Type newType;
	MetaTranslatorMessage m = *it;

	if ( !virginTor->contains((*it).context(), (*it).sourceText(),
				  (*it).comment()) )
	    newType = MetaTranslatorMessage::Obsolete;
	else if ( m.type() == MetaTranslatorMessage::Finished )
	    newType = MetaTranslatorMessage::Finished;
	else
	    newType = MetaTranslatorMessage::Unfinished;

	if ( newType != m.type() ) {
	    m.setType( newType );
	    tor->insert( m );
	}
    }

    /*
      Messages found only in the virgin translator are added to the vernacular
      translator.
    */
    all = virginTor->messages();

    for ( it = all.begin(); it != all.end(); ++it ) {
	if ( !tor->contains((*it).context(), (*it).sourceText(),
			    (*it).comment()) )
	    tor->insert( *it );
    }

    /*
      The same-text heuristic handles cases where a message has an obsolete
      counterpart with a different context or comment.
    */
    applySameTextHeuristic( tor );

    /*
      The number heuristic handles cases where a message has an obsolete
      counterpart with mostly numbers differing in the source text.
    */
    applyNumberHeuristic( tor );
}
