/****************************************************************************
** $Id: //depot/qt/main/util/qtranslator/qtconfig.h#1 $
**
** This is a utility program for translating Qt applications
**
**
** Copyright (C) 1999 by Trolltech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QTCONFIG
#define QTCONFIG

#include <qmap.h>
#include <qstringlist.h>

/****************************************************************************
 *
 * Class: QTConfig
 *
 ****************************************************************************/

class QTConfig
{
public:
    typedef QMap< QString, QString > QTConfigGroup;
    
    QTConfig( const QString &fn );
    ~QTConfig();
    
    void setGroup( const QString &gname );
    void writeEntry( const QString &key, const QString &value );
    void writeEntry( const QString &key, int num );
    void writeEntry( const QString &key, bool b );
    void writeEntry( const QString &key, const QStringList &lst, const QChar &sep );
    
    QString readEntry( const QString &key, const QString &deflt = QString::null );
    int readNumEntry( const QString &key, int deflt = -1 );
    bool readBoolEntry( const QString &key, bool deflt = FALSE );
    QStringList readListEntry( const QString &key, const QChar &sep ); 
    
    void clearGroup();
    
    void write( const QString &fn = QString::null );
    
    static QStringList split( QString line, const QChar &sep );

protected:
    void read();
    void parse( QString line );
    
    QMap< QString, QTConfigGroup > groups;
    QMap< QString, QTConfigGroup >::Iterator git;
    QString filename;

};

#endif
