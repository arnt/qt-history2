/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of Qt Linguist.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef PHRASE_H
#define PHRASE_H

#include <qstring.h>
#include <qlist.h>

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

class PhraseBook : public QList<Phrase>
{
public:
    PhraseBook() { }

    bool load( const QString& filename );
    bool save( const QString& filename ) const;
};

#endif
