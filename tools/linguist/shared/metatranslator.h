/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of Qt Linguist.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef METATRANSLATOR_H
#define METATRANSLATOR_H

#include <qmap.h>
#include <qstring.h>
#include <qcstring.h>
#include <qtranslator.h>
#include <qvaluelist.h>

class QTextCodec;

class MetaTranslatorMessage : public QTranslatorMessage
{
public:
    enum Type { Unfinished, Finished, Obsolete };

    MetaTranslatorMessage();
    MetaTranslatorMessage( const char *context, const char *sourceText,
			   const char *comment,
			   const QString& translation = QString::null,
			   bool utf8 = FALSE, Type type = Unfinished );
    MetaTranslatorMessage( const MetaTranslatorMessage& m );

    MetaTranslatorMessage& operator=( const MetaTranslatorMessage& m );

    void setType( Type nt ) { ty = nt; }
    Type type() const { return ty; }
    bool utf8() const { return utfeight; }

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
    bool utfeight;
    Type ty;
};

class MetaTranslator
{
public:
    MetaTranslator();
    MetaTranslator( const MetaTranslator& tor );

    MetaTranslator& operator=( const MetaTranslator& tor );

    void clear();
    bool load( const QString& filename );
    bool save( const QString& filename ) const;
    bool release( const QString& filename, bool verbose = FALSE,
		  bool ignoreUnfinished = FALSE,
		  QTranslator::SaveMode mode = QTranslator::Stripped ) const;

    bool contains( const char *context, const char *sourceText,
		   const char *comment ) const;
    void insert( const MetaTranslatorMessage& m );

    void stripObsoleteMessages();
    void stripEmptyContexts();

    void setCodec( const char *name );
    QString toUnicode( const char *str, bool utf8 ) const;

    QValueList<MetaTranslatorMessage> messages() const;
    QValueList<MetaTranslatorMessage> translatedMessages() const;

private:
    typedef QMap<MetaTranslatorMessage, int> TMM;
    typedef QMap<int, MetaTranslatorMessage> TMMInv;

    TMM mm;
    QCString codecName;
    QTextCodec *codec;
};

/*
  This is a quick hack. The proper way to handle this would be
  to extend MetaTranslator's interface.
*/
#define ContextComment "QT_LINGUIST_INTERNAL_CONTEXT_COMMENT"

#endif
