/**********************************************************************
**   Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
**   metatranslator.h
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

#ifndef METATRANSLATOR_H
#define METATRANSLATOR_H

#include <qmap.h>
#include <qstring.h>
#include <qtranslator.h>
#include <qvaluelist.h>

class MetaTranslatorMessage : public QTranslatorMessage
{
public:
    enum Type { Unfinished, Finished, Obsolete };

    MetaTranslatorMessage() : ty( Unfinished ) { }
    MetaTranslatorMessage( const char *context, const char *sourceText,
			   const char *comment,
			   const QString& translation = QString::null,
			   Type type = Unfinished )
	: QTranslatorMessage( context, sourceText, comment, translation ),
	  ty( type ) { }
    MetaTranslatorMessage( const MetaTranslatorMessage& m )
	: QTranslatorMessage( m ), ty( m.ty ) { }

    MetaTranslatorMessage& operator=( const MetaTranslatorMessage& m );

    void setType( Type nt ) { ty = nt; }
    Type type() const { return ty; }

    bool operator==( const MetaTranslatorMessage& m ) const;
    bool operator!=( const MetaTranslatorMessage& m ) const
    { return !operator==( m ); }
    bool operator<( const MetaTranslatorMessage& m ) const;
    bool operator<=( const MetaTranslatorMessage& m )
    { return !operator>( m ); }
    bool operator>( const MetaTranslatorMessage& m ) const
    { return this->operator<( m ); }
    bool operator>=( const MetaTranslatorMessage& m ) const
    { return !operator<( m ); }

private:
    Type ty;
};

class MetaTranslator
{
public:
    MetaTranslator();
    MetaTranslator( const MetaTranslator& tor )
	: mm( tor.mm ), codec( tor.codec ) { }

    MetaTranslator& operator=( const MetaTranslator& tor );

    bool load( const QString& filename );
    bool save( const QString& filename ) const;
    bool release( const QString& filename ) const;

    bool contains( const char *context, const char *sourceText,
		   const char *comment ) const;
    void insert( const MetaTranslatorMessage& m );

    void setCodec( const char *name ) { codec = name; }
    QString toUnicode( const char *str ) const;

    QValueList<MetaTranslatorMessage> messages() const;

private:
    typedef QMap<MetaTranslatorMessage, int> TMM;
    typedef QMap<int, MetaTranslatorMessage> TMMInv;

    TMM mm;
    QCString codec;
};

#endif
