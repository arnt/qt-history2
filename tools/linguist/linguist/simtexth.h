/**********************************************************************
**   Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
**   simtexth.h
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

#ifndef SIMTEXTH_H
#define SIMTEXTH_H

#include <qstring.h>
#include <qvaluelist.h>

struct Candidate {
    QString source;
    QString target;

    Candidate() { }
    Candidate( const QString& source0, const QString& target0 )
	: source( source0 ), target( target0 ) { }
};

inline bool operator==( const Candidate& c, const Candidate& d ) {
    return c.source == d.source && c.target == d.target;
}
inline bool operator!=( const Candidate& c, const Candidate& d ) {
    return !operator==( c, d );
}

typedef QValueList<Candidate> CandidateList;

CandidateList similarTextHeuristicCandidates( const MetaTranslator *tor,
					      const char *text,
					      int maxCandidates );

#endif
