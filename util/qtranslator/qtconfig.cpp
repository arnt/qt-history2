/****************************************************************************
** $Id: //depot/qt/main/util/qtranslator/qtconfig.cpp#2 $
**
** This is a utility program for translating Qt applications
**
**
** Copyright (C) 1999 by Trolltech AS.  All rights reserved.
**
*****************************************************************************/

#include "qtconfig.h"

#include <qfile.h>
#include <qfileinfo.h>
#include <qtextstream.h>

#include <stdlib.h>

/****************************************************************************
 *
 * Class: QTConfig
 *
 ****************************************************************************/

QTConfig::QTConfig( const QString &fn )
    : filename( fn )
{
    git = groups.end();
    read();
}

QTConfig::~QTConfig()
{
    write();
}

void QTConfig::setGroup( const QString &gname )
{
    QMap< QString, QTConfigGroup>::Iterator it = groups.find( gname );
    if ( it == groups.end() ) {
        QTConfigGroup *grp = new QTConfigGroup;
        git = groups.insert( gname, *grp );
        return;
    }
    git = it;
}

void QTConfig::writeEntry( const QString &key, const QString &value )
{
    if ( git == groups.end() ) {
        qWarning( "no group set" );
        return;
    }
    ( *git ).insert( key, value );
}

void QTConfig::writeEntry( const QString &key, int num )
{
    QString s;
    s.setNum( num );
    writeEntry( key, s );
}

void QTConfig::writeEntry( const QString &key, bool b )
{
    QString s;
    s.setNum( ( int )b );
    writeEntry( key, s );
}

void QTConfig::writeEntry( const QString &key, const QStringList &lst, const QChar &sep )
{
    QString s;
    QStringList::ConstIterator it = lst.begin();
    for ( ; it != lst.end(); ++it )
        s += *it + sep;
    writeEntry( key, s );
}

QString QTConfig::readEntry( const QString &key, const QString &deflt )
{
    if ( git == groups.end() ) {
        qWarning( "no group set" );
        return deflt;
    }
    QTConfigGroup::Iterator it = ( *git ).find( key );
    if ( it != ( *git ).end() )
        return *it;
    else
        return deflt;
}

int QTConfig::readNumEntry( const QString &key, int deflt )
{
    QString s = readEntry( key );
    if ( s.isEmpty() )
        return deflt;
    else
        return atoi( s );
}

bool QTConfig::readBoolEntry( const QString &key, bool deflt )
{
    QString s = readEntry( key );
    if ( s.isEmpty() )
        return deflt;
    else
        return ( bool )atoi( s );
}

QStringList QTConfig::readListEntry( const QString &key, const QChar &sep )
{
    QString s = readEntry( key );
    if ( s.isEmpty() )
        return QStringList();
    else
        return QTConfig::split( s, sep );
}

void QTConfig::clearGroup()
{
    if ( git == groups.end() ) {
        qWarning( "no group set" );
        return;
    }
    ( *git ).clear();
}

void QTConfig::write( const QString &fn = QString::null )
{
    if ( !fn.isEmpty() )
        filename = fn;

    QFile f( filename );
    if ( !f.open( IO_WriteOnly ) ) {
        qWarning( "could not open for writing `%s'", filename.latin1() );
        git = groups.end();
        return;
    }

    QTextStream s( &f );
    QMap< QString, QTConfigGroup >::Iterator g_it = groups.begin();
    for ( ; g_it != groups.end(); ++g_it ) {
        s << "[" << g_it.key() << "]" << "\n";
        QTConfigGroup::Iterator e_it = ( *g_it ).begin();
        for ( ; e_it != ( *g_it ).end(); ++e_it )
            s << e_it.key() << " = " << *e_it << "\n";
    }

    f.close();
}

void QTConfig::read()
{
    if ( !QFileInfo( filename ).exists() ) {
        qWarning( "`%s' doesn't exist", filename.latin1() );
        git = groups.end();
        return;
    }

    QFile f( filename );
    if ( !f.open( IO_ReadOnly ) ) {
        qWarning( "could not open for reading `%s'", filename.latin1() );
        git = groups.end();
        return;
    }

    QTextStream s( &f );

    QString line;
    while ( !s.atEnd() ) {
        line = s.readLine();
        parse( line );
    }

    f.close();
}

void QTConfig::parse( QString line )
{
    line = line.stripWhiteSpace();
    if ( line[ 0 ] == QChar( '[' ) ) {
        QString gname = line;
        gname = gname.remove( 0, 1 );
        if ( gname[ gname.length() - 1 ] == QChar( ']' ) )
            gname = gname.remove( gname.length() - 1, 1 );
        QTConfigGroup *grp = new QTConfigGroup;
        git = groups.insert( gname, *grp );
    } else {
        if ( git == groups.end() ) {
            qWarning( "line `%s' out of group", line.latin1() );
            return;
        }
        QStringList lst = split( line, QChar( '=' ) );
        if ( lst.count() != 2 && line.find( '=' ) == -1 ) {
            qWarning( "corrupted line `%s' in group `%s'",
                      line.latin1(), git.key().latin1() );
            return;
        }
        ( *git ).insert( lst[ 0 ].stripWhiteSpace(), lst[ 1 ].stripWhiteSpace() );
    }
}

QStringList QTConfig::split( QString line, const QChar &sep )
{
    int i = line.find( sep, 0 );
    QStringList lst;

    while ( i != -1 ) {
        if ( line.left( i ).length() > 0 )
            lst.append( line.left( i ) );
        line.remove( 0, i + 1 );
        i = line.find( sep, 0 );
    }

    if ( !line.simplifyWhiteSpace().isEmpty() )
        lst.append( line.simplifyWhiteSpace() );

    return lst;
}
