/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "index.h"

#include <qfile.h>
#include <qdir.h>
#include <qstringlist.h>
#include <qapplication.h>
#include <qbytearray.h>
#include <ctype.h>
#include <qtextstream.h>
#include <qalgorithms.h>

struct Term {
    Term() : frequency(-1) {}
    Term( const QString &t, int f, QList<Document> l ) : term( t ), frequency( f ), documents( l ) {}
    QString term;
    int frequency;
    QList<Document>documents;
    bool operator<( const Term &i2 ) const { return frequency < i2.frequency; }
};

QDataStream &operator>>( QDataStream &s, Document &l )
{
    s >> l.docNumber;
    s >> l.frequency;
    return s;
}

QDataStream &operator<<( QDataStream &s, const Document &l )
{
    s << (Q_INT16)l.docNumber;
    s << (Q_INT16)l.frequency;
    return s;
}

Index::Index( const QString &dp, const QString &hp )
    : QObject( 0 ), docPath( dp )
{
    Q_UNUSED(hp);

    alreadyHaveDocList = false;
    lastWindowClosed = false;
    connect( qApp, SIGNAL( lastWindowClosed() ),
             this, SLOT( setLastWinClosed() ) );
}

Index::Index( const QStringList &dl, const QString &hp )
    : QObject( 0 )
{
    Q_UNUSED(hp);
    docList = dl;
    alreadyHaveDocList = true;
    lastWindowClosed = false;
    connect( qApp, SIGNAL( lastWindowClosed() ),
             this, SLOT( setLastWinClosed() ) );
}

void Index::setLastWinClosed()
{
    lastWindowClosed = true;
}

void Index::setDictionaryFile( const QString &f )
{
    dictFile = f;
}

void Index::setDocListFile( const QString &f )
{
    docListFile = f;
}

int Index::makeIndex()
{
    if ( !alreadyHaveDocList )
        setupDocumentList();
    if ( docList.isEmpty() )
        return 1;
    QStringList::Iterator it = docList.begin();
    int steps = docList.count() / 100;
    if ( !steps )
        steps++;
    int prog = 0;
    for ( int i = 0; it != docList.end(); ++it, ++i ) {
        if ( lastWindowClosed ) {
            return -1;
        }
        parseDocument( *it, i );
        if ( i%steps == 0 ) {
            prog++;
            emit indexingProgress( prog );
        }
    }
    return 0;
}

void Index::setupDocumentList()
{
    QDir d( docPath );
    QStringList filters;
    filters.append(QLatin1String("*.html"));
    QStringList lst = d.entryList(filters);
    QStringList::ConstIterator it = lst.begin();
    for ( ; it != lst.end(); ++it )
        docList.append( docPath + QLatin1String("/") + *it );
}

void Index::insertInDict( const QString &str, int docNum )
{
    if ( str == QLatin1String("amp") || str == QLatin1String("nbsp"))
        return;
    Entry *e = 0;
    if ( dict.count() )
        e = dict[ str ];

    if ( e ) {
        if ( e->documents.first().docNumber != docNum )
            e->documents.prepend( Document( docNum, 1 ) );
        else
            e->documents.first().frequency++;
    } else {
        dict.insert( str, new Entry( docNum ) );
    }
}

void Index::parseDocument( const QString &filename, int docNum )
{
    QFile file( filename );
    if ( !file.open( IO_ReadOnly ) ) {
        qWarning( (QLatin1String("can not open file ") + filename).ascii() );
        return;
    }

    QTextStream s( &file );
    s.setEncoding(QTextStream::Latin1);
    QString text = s.read();
    if (text.isNull())
        return;

    bool valid = true;
    const QChar *buf = text.unicode();
    QChar str[64];
    QChar c = buf[0];
    int j = 0;
    int i = 0;
    while ( j < text.length() ) {
        if ( c == QLatin1Char('<') || c == QLatin1Char('&') ) {
            valid = false;
            if ( i > 1 )
                insertInDict( QString(str,i), docNum );
            i = 0;
            c = buf[++j];
            continue;
        }
        if ( ( c == QLatin1Char('>') || c == QLatin1Char(';') ) && !valid ) {
            valid = true;
            c = buf[++j];
            continue;
        }
        if ( !valid ) {
            c = buf[++j];
            continue;
        }
        if ( ( c.isLetterOrNumber() || c == QLatin1Char('_') ) && i < 63 ) {
            str[i] = c.toLower();
            ++i;
        } else {
            if ( i > 1 )
                insertInDict( QString(str,i), docNum );
            i = 0;
        }
        c = buf[++j];
    }
    if ( i > 1 )
        insertInDict( QString(str,i), docNum );
    file.close();
}

void Index::writeDict()
{
    QFile f( dictFile );
    if ( !f.open( IO_WriteOnly ) )
        return;
    QDataStream s( &f );
    for(QHash<QString, Entry *>::Iterator it = dict.begin(); it != dict.end(); ++it) {
        s << it.key();
        s << it.value()->documents;
    }
    f.close();
    writeDocumentList();
}

void Index::writeDocumentList()
{
    QFile f( docListFile );
    if ( !f.open( IO_WriteOnly ) )
        return;
    QDataStream s( &f );
    s << docList;
}

void Index::readDict()
{
    QFile f( dictFile );
    if ( !f.open( IO_ReadOnly ) )
        return;

    dict.clear();
    QDataStream s( &f );
    QString key;
    QList<Document> docs;
    while ( !s.atEnd() ) {
        s >> key;
        s >> docs;
        dict.insert( key, new Entry( docs ) );
    }
    f.close();
    readDocumentList();
}

void Index::readDocumentList()
{
    QFile f( docListFile );
    if ( !f.open( IO_ReadOnly ) )
        return;
    QDataStream s( &f );
    s >> docList;
}

QStringList Index::query( const QStringList &terms, const QStringList &termSeq, const QStringList &seqWords )
{
    QList<Term> termList;
    for (QStringList::ConstIterator it = terms.begin(); it != terms.end(); ++it ) {
        Entry *e = 0;
        if ( (*it).contains(QLatin1Char('*')) ) {
            QList<Document> wcts = setupDummyTerm( getWildcardTerms( *it ) );
            termList.append( Term(QLatin1String("dummy"), wcts.count(), wcts ) );
        } else if ( dict[ *it ] ) {
            e = dict[ *it ];
            termList.append( Term( *it, e->documents.count(), e->documents ) );
        } else {
            return QStringList();
        }
    }
    if ( !termList.count() )
        return QStringList();
    qHeapSort(termList);

    QList<Document> minDocs = termList.takeFirst().documents;
    for(QList<Term>::Iterator it = termList.begin(); it != termList.end(); ++it) {
        Term *t = &(*it);
        QList<Document> docs = t->documents;
        for(QList<Document>::Iterator minDoc_it = minDocs.begin(); minDoc_it != minDocs.end(); ) {
            bool found = false;
            for (QList<Document>::ConstIterator doc_it = docs.begin(); doc_it != docs.end(); ++doc_it ) {
                if ( (*minDoc_it).docNumber == (*doc_it).docNumber ) {
                    (*minDoc_it).frequency += (*doc_it).frequency;
                    found = true;
                    break;
                }
            }
            if ( !found )
                minDoc_it = minDocs.erase( minDoc_it );
            else
                ++minDoc_it;
        }
    }

    QStringList results;
    qHeapSort( minDocs );
    if ( termSeq.isEmpty() ) {
        for(QList<Document>::Iterator it = minDocs.begin(); it != minDocs.end(); ++it)
            results << docList[ (int)(*it).docNumber ];
        return results;
    }

    QString fileName;
    for(QList<Document>::Iterator it = minDocs.begin(); it != minDocs.end(); ++it) {
        fileName =  docList[ (int)(*it).docNumber ];
        if ( searchForPattern( termSeq, seqWords, fileName ) )
            results << fileName;
    }
    return results;
}

QString Index::getDocumentTitle( const QString &fileName )
{
    QFile file( fileName );
    if ( !file.open( IO_ReadOnly ) ) {
        qWarning( (QLatin1String("cannot open file ") + fileName).ascii() );
        return fileName;
    }
    QTextStream s( &file );
    QString text = s.read();

    int start = text.indexOf(QLatin1String("<title>"), 0, Qt::CaseInsensitive) + 7;
    int end = text.indexOf(QLatin1String("</title>"), 0, Qt::CaseInsensitive);

    QString title = ( end - start <= 0 ? tr("Untitled") : text.mid( start, end - start ) );
    return title;
}

QStringList Index::getWildcardTerms( const QString &term )
{
    QStringList lst;
    QStringList terms = split( term );
    QStringList::Iterator iter;

    for(QHash<QString, Entry*>::Iterator it = dict.begin(); it != dict.end(); ++it) {
        int index = 0;
        bool found = false;
        QString text( it.key() );
        for ( iter = terms.begin(); iter != terms.end(); ++iter ) {
            if ( *iter == QLatin1String("*") ) {
                found = true;
                continue;
            }
            if ( iter == terms.begin() && (*iter)[0] != text[0] ) {
                found = false;
                break;
            }
            index = text.indexOf( *iter, index );
            if ( *iter == terms.last() && index != (int)text.length()-1 ) {
                index = text.lastIndexOf( *iter );
                if ( index != (int)text.length() - (int)(*iter).length() ) {
                    found = false;
                    break;
                }
            }
            if ( index != -1 ) {
                found = true;
                index += (*iter).length();
                continue;
            } else {
                found = false;
                break;
            }
        }
        if ( found )
            lst << text;
    }

    return lst;
}

QStringList Index::split( const QString &str )
{
    QStringList lst;
    int j = 0;
    int i = str.indexOf(QLatin1Char('*'), j );

    while ( i != -1 ) {
        if ( i > j && i <= (int)str.length() ) {
            lst << str.mid( j, i - j );
            lst << QLatin1String("*");
        }
        j = i + 1;
        i = str.indexOf(QLatin1Char('*'), j );
    }

    int l = str.length() - 1;
    if ( str.mid( j, l - j + 1 ).length() > 0 )
        lst << str.mid( j, l - j + 1 );

    return lst;
}

QList<Document> Index::setupDummyTerm( const QStringList &terms )
{
    QList<Term> termList;
    for (QStringList::ConstIterator it = terms.begin(); it != terms.end(); ++it) {
        Entry *e = 0;
        if ( dict[ *it ] ) {
            e = dict[ *it ];
            termList.append( Term( *it, e->documents.count(), e->documents ) );
        }
    }
    QList<Document> maxList;
    if ( !termList.count() )
        return maxList;
    qHeapSort(termList);

    maxList = termList.takeLast().documents;
    for(QList<Term>::Iterator it = termList.begin(); it != termList.end(); ++it) {
        Term *t = &(*it);
        QList<Document> docs = t->documents;
        for (QList<Document>::iterator docIt = docs.begin(); docIt != docs.end(); ++docIt ) {
            if ( maxList.indexOf( *docIt ) == -1 )
                maxList.append( *docIt );
        }
    }
    return maxList;
}

void Index::buildMiniDict( const QString &str )
{
    if ( miniDict[ str ] )
        miniDict[ str ]->positions.append( wordNum );
    ++wordNum;
}

bool Index::searchForPattern( const QStringList &patterns, const QStringList &words, const QString &fileName )
{
    QFile file( fileName );
    if ( !file.open( IO_ReadOnly ) ) {
        qWarning( (QLatin1String("cannot open file ") + fileName).ascii() );
        return false;
    }

    wordNum = 3;
    miniDict.clear();
    QStringList::ConstIterator cIt = words.begin();
    for ( ; cIt != words.end(); ++cIt )
        miniDict.insert( *cIt, new PosEntry( 0 ) );

    QTextStream s( &file );
    QString text = s.read();
    bool valid = true;
    const QChar *buf = text.unicode();
    QChar str[64];
    QChar c = buf[0];
    int j = 0;
    int i = 0;
    while ( j < text.length() ) {
        if ( c == QLatin1Char('<') || c == QLatin1Char('&') ) {
            valid = false;
            if ( i > 1 )
                buildMiniDict( QString(str,i) );
            i = 0;
            c = buf[++j];
            continue;
        }
        if ( ( c == QLatin1Char('>') || c == QLatin1Char(';') ) && !valid ) {
            valid = true;
            c = buf[++j];
            continue;
        }
        if ( !valid ) {
            c = buf[++j];
            continue;
        }
        if ( ( c.isLetterOrNumber() || c == QLatin1Char('_') ) && i < 63 ) {
            str[i] = c.toLower();
            ++i;
        } else {
            if ( i > 1 )
                buildMiniDict( QString(str,i) );
            i = 0;
        }
        c = buf[++j];
    }
    if ( i > 1 )
        buildMiniDict( QString(str,i) );
    file.close();

    QStringList::ConstIterator patIt = patterns.begin();
    QStringList wordLst;
    QList<uint> a, b;
    QList<uint>::iterator aIt;
    for ( ; patIt != patterns.end(); ++patIt ) {
        wordLst = (*patIt).split(QLatin1Char(' '));
        a = miniDict[ wordLst[0] ]->positions;
        for ( int j = 1; j < (int)wordLst.count(); ++j ) {
            b = miniDict[ wordLst[j] ]->positions;
            aIt = a.begin();
            while ( aIt != a.end() ) {
                if ( b.contains( *aIt + 1 )) {
                    (*aIt)++;
                    ++aIt;
                } else {
                    aIt = a.erase( aIt );
                }
            }
        }
    }
    if ( a.count() )
        return true;
    return false;
}
