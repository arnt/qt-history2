/****************************************************************************
** $Id$
**
** Definition of QSettings class
**
** Created: 2000.06.26
**
** Copyright (C) 2000-2001 Trolltech AS.  All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QSETTINGS_H
#define QSETTINGS_H

#ifndef QT_H
#include <qdatetime.h>
#include <qstringlist.h>
#endif // QT_H

#ifndef QT_NO_SETTINGS

class QSettingsPrivate;


class Q_EXPORT QSettings
{
public:
    QSettings();
    ~QSettings();

    enum System {
	Unix = 0,
	Windows,
	Mac
    };

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

    QStringList readListEntry( const QString &, bool * = 0 );
    QStringList readListEntry( const QString &, const QChar& sep, bool * = 0 );
    QString	readEntry( const QString &, const QString &def = QString::null,
			   bool * = 0 );
    int		readNumEntry( const QString &, int def = 0, bool * = 0 );
    double	readDoubleEntry( const QString &, double def = 0, bool * = 0 );
    bool	readBoolEntry( const QString &, bool def = 0, bool * = 0 );

    bool	removeEntry( const QString & );

    void insertSearchPath( System, const QString & );
    void removeSearchPath( System, const QString & );

private:
    QSettingsPrivate *d;

#if defined(Q_DISABLE_COPY)
    QSettings(const QSettings &);
    QSettings &operator=(const QSettings &);
#endif

    QDateTime lastModficationTime( const QString & );
    bool sync();

    friend class QApplication;
};

#endif // QT_NO_SETTINGS
#endif // QSETTINGS_H
