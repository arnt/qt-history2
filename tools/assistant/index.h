/**********************************************************************
** Copyright (C) 2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qt Assistant.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef INDEX_H
#define INDEX_H

#include <qstringlist.h>
#include <qasciidict.h>
#include <qdatastream.h>
#include <qobject.h>

struct Document {
    Document( int d, int f ) : docNumber( d ), frequency( f ) {}
    Document() : docNumber( -1 ), frequency( 0 ) {}
    bool operator==( const Document &doc ) const {
	return docNumber == doc.docNumber;
    }
    bool operator<( const Document &doc ) const {
	return frequency > doc.frequency;
    }
    bool operator<=( const Document &doc ) const {
	return frequency >= doc.frequency;
    }
    bool operator>( const Document &doc ) const {
	return frequency < doc.frequency;
    }
    Q_INT16 docNumber;
    Q_INT16 frequency;
};

QDataStream &operator>>( QDataStream &s, Document &l );
QDataStream &operator<<( QDataStream &s, const Document &l );

class Index : public QObject
{
    Q_OBJECT
public:
    struct Entry {
	Entry( int d ) { documents.append( Document( d, 1 ) ); }
	Entry( QValueList<Document> l ) : documents( l ) {}
	QValueList<Document> documents;
    };
    struct PosEntry {
	PosEntry( int p ) { positions.append( p ); }
	QValueList<uint> positions;
    };

    Index( const QString &dp, const QString &hp );
    Index( const QStringList &dl, const QString &hp );
    void writeDict();
    void readDict();
    void makeIndex();
    QStringList query( const QStringList&, const QStringList&, const QStringList& );
    QString getDocumentTitle( const QString& );
    void setDictionaryFile( const QString& );
    void setDocListFile( const QString& );

signals:
    void indexingProgress( int );

private:
    void setupDocumentList();
    void parseDocument( const QString&, int );
    void insertInDict( const char*, int );
    void writeDocumentList();
    void readDocumentList();
    QStringList getWildcardTerms( const QString& );
    QValueList<QCString> split( const QString& );
    QValueList<Document> setupDummyTerm( const QStringList& );
    bool searchForPattern( const QStringList&, const QStringList&, const QString& );
    void buildMiniDict( const char* );
    QStringList docList;
    QAsciiDict<Entry> dict;
    QAsciiDict<PosEntry> miniDict;
    uint wordNum;
    QString docPath, homePath;
    QString dictFile, docListFile;
    bool alreadyHaveDocList;
};

struct Term {
    Term( const char *t, int f, QValueList<Document> l )
	: term( t ), frequency( f ), documents( l ) {}
    const char* term;
    int frequency;
    QValueList<Document>documents;
};

class TermList : public QPtrList<Term>
{
public:
    TermList() : QPtrList<Term>() {}
    int compareItems( QPtrCollection::Item i1, QPtrCollection::Item i2 );
};

#endif
