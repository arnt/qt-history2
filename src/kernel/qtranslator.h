/****************************************************************************
**
** Definition of the translator class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/


#ifndef QTRANSLATOR_H
#define QTRANSLATOR_H

#ifndef QT_H
#include "qobject.h"
#include "qbytearray.h"
#endif // QT_H

#ifndef QT_NO_TRANSLATION

class QTranslatorPrivate;
template <typename T> class QList;

class Q_CORE_EXPORT QTranslatorMessage
{
public:
    QTranslatorMessage();
    QTranslatorMessage( const char * context,
			const char * sourceText,
			const char * comment,
			const QString& translation = QString() );
    QTranslatorMessage( QDataStream & );
    QTranslatorMessage( const QTranslatorMessage & m );

    QTranslatorMessage & operator=( const QTranslatorMessage & m );

    uint hash() const { return h; }
    const char *context() const { return cx; }
    const char *sourceText() const { return st; }
    const char *comment() const { return cm; }

    void setTranslation( const QString & translation ) { tn = translation; }
    QString translation() const { return tn; }

    enum Prefix { NoPrefix, Hash, HashContext, HashContextSourceText,
		  HashContextSourceTextComment };
    void write( QDataStream & s, bool strip = FALSE,
		Prefix prefix = HashContextSourceTextComment ) const;
    Prefix commonPrefix( const QTranslatorMessage& ) const;

    bool operator==( const QTranslatorMessage& m ) const;
    bool operator!=( const QTranslatorMessage& m ) const
    { return !operator==( m ); }
    bool operator<( const QTranslatorMessage& m ) const;
    bool operator<=( const QTranslatorMessage& m ) const
    { return !m.operator<( *this ); }
    bool operator>( const QTranslatorMessage& m ) const
    { return m.operator<( *this ); }
    bool operator>=( const QTranslatorMessage& m ) const
    { return !operator<( m ); }

private:
    uint h;
    QByteArray cx;
    QByteArray st;
    QByteArray cm;
    QString tn;

    enum Tag { Tag_End = 1, Tag_SourceText16, Tag_Translation, Tag_Context16,
	       Tag_Hash, Tag_SourceText, Tag_Context, Tag_Comment,
	       Tag_Obsolete1 };
};


class Q_CORE_EXPORT QTranslator : public QObject
{
    Q_OBJECT
    Q_DECL_PRIVATE(QTranslator);
public:
    QTranslator( QObject * parent, const char * name);
    QTranslator(QObject *parent = 0);
    ~QTranslator();

    virtual QTranslatorMessage findMessage( const char *, const char *,
					    const char * = 0 ) const;

    bool load( const QString & filename,
	       const QString & directory = QString::null,
	       const QString & search_delimiters = QString::null,
	       const QString & suffix = QString::null );
    bool load( const uchar *data, int len ) {
	clear();
	return do_load( data, len );
    }

    void clear();

#ifndef QT_NO_TRANSLATION_BUILDER
    enum SaveMode { Everything, Stripped };

    bool save( const QString & filename, SaveMode mode = Everything );

    void insert( const QTranslatorMessage& );
    void insert( const char *context, const char *sourceText, const QString &translation ) {
	insert( QTranslatorMessage(context, sourceText, "", translation) );
    }
    void remove( const QTranslatorMessage& );
    void remove( const char *context, const char *sourceText ) {
	remove( QTranslatorMessage(context, sourceText, "") );
    }
    bool contains( const char *, const char *, const char * comment = 0 ) const;

    void squeeze( SaveMode = Everything );
    void unsqueeze();

    QList<QTranslatorMessage> messages() const;
#endif

    bool isEmpty() const;

private:
#if defined(Q_DISABLE_COPY)
    QTranslator( const QTranslator & );
    QTranslator &operator=( const QTranslator & );
#endif

    bool do_load( const uchar *data, int len );
#ifdef QT_COMPAT
public:
    QT_COMPAT QString find( const char *context, const char *sourceText, const char * comment = 0 ) const {
	return findMessage( context, sourceText, comment ).translation();
    }
#endif
};

#endif // QT_NO_TRANSLATION

#endif
