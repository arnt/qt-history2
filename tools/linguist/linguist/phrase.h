/**********************************************************************
**   Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
**   phrase.h
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

#ifndef PHRASE_H
#define PHRASE_H

#include <qstring.h>
#include <qvaluelist.h>

class Phrase
{
public:
    Phrase() { }
    Phrase( const QString& source, const QString& target,
	    const QString& definition );

    QString source() const { return s; }
    void setSource( const QString& ns ) { s = ns; }
    QString target() const { return t; }
    void setTarget( const QString& nt ) { t = nt; }
    QString definition() const { return d; }
    void setDefinition ( const QString& nd ) { d = nd; }

private:
    QString s;
    QString t;
    QString d;
};

bool operator==( const Phrase& p, const Phrase& q );
inline bool operator!=( const Phrase& p, const Phrase& q ) {
    return !( p == q );
}

class PhraseBook : public QValueList<Phrase>
{
public:
    PhraseBook() { }

    bool load( const QString& filename );
    bool save( const QString& filename ) const;
};

#endif
