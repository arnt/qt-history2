/****************************************************************************
**
** Definition of QSettings class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSETTINGS_H
#define QSETTINGS_H

#ifndef QT_H
#include "qdatetime.h"
#include "qstringlist.h"
#endif // QT_H

#ifndef QT_NO_SETTINGS

class QSettingsPrivate;


class Q_CORE_EXPORT QSettings
{
public:
    enum Format {
	Native = 0,
	Ini
    };
    enum System {
	Unix = 0,
	Windows,
	Mac
    };
    enum Scope {
	User,
	Global
    };

    QSettings();
    QSettings( Format format );

    ~QSettings();

#if !defined(Q_NO_BOOL_TYPE)
    bool	writeEntry( const QString &, bool );
#endif
    bool	writeEntry( const QString &, double );
    bool	writeEntry( const QString &, int );
    bool	writeEntry( const QString &, const char * );
    bool	writeEntry( const QString &, const QString & );
    bool	writeEntry( const QString &, const QStringList & );
    bool	writeEntry( const QString &, const QStringList &, const QChar& sep );

    QStringList entryList(const QString &) const;
    QStringList subkeyList(const QString &) const;

    //### remove non const versions in 4.0
    QStringList readListEntry( const QString &, bool * = 0 );
    QStringList readListEntry( const QString &, const QChar& sep, bool * = 0 );
    QString	readEntry( const QString &, const QString &def = QString::null, bool * = 0 );
    int		readNumEntry( const QString &, int def = 0, bool * = 0 );
    double	readDoubleEntry( const QString &, double def = 0, bool * = 0 );
    bool	readBoolEntry( const QString &, bool def = FALSE, bool * = 0 );

    //### make those non-inlined in 4.0
    QStringList readListEntry( const QString &key, bool *ok = 0 ) const
    {
	QSettings *that = (QSettings*)this;
	return that->readListEntry( key, ok );
    }
    QStringList readListEntry( const QString &key, const QChar& sep, bool *ok = 0 ) const
    {
	QSettings *that = (QSettings*)this;
	return that->readListEntry( key, sep, ok );
    }
    QString	readEntry( const QString &key, const QString &def = QString::null,
			   bool *ok = 0 ) const
    {
	QSettings *that = (QSettings*)this;
	return that->readEntry( key, def, ok );
    }
    int		readNumEntry( const QString &key, int def = 0, bool *ok = 0 ) const
    {
	QSettings *that = (QSettings*)this;
	return that->readNumEntry( key, def, ok );
    }

    double	readDoubleEntry( const QString &key, double def = 0, bool *ok = 0 ) const
    {
	QSettings *that = (QSettings*)this;
	return that->readDoubleEntry( key, def, ok );
    }
    bool	readBoolEntry( const QString &key, bool def = FALSE, bool *ok = 0 ) const
    {
	QSettings *that = (QSettings*)this;
	return that->readBoolEntry( key, def, ok );
    }

    bool	removeEntry( const QString & );

    void insertSearchPath( System, const QString & );
    void removeSearchPath( System, const QString & );

    void setPath( const QString &domain, const QString &product, Scope = Global );

    void beginGroup( const QString &group );
    void endGroup();
    void resetGroup();
    QString group() const;

    bool sync();

private:
    QSettingsPrivate *d;

#if defined(Q_DISABLE_COPY)
    QSettings(const QSettings &);
    QSettings &operator=(const QSettings &);
#endif

    QDateTime lastModificationTime( const QString & );

    friend class QApplication;
};

#endif // QT_NO_SETTINGS
#endif // QSETTINGS_H
