/****************************************************************************
**
**
** Definition of QSettings class
**
** Created : 2000.06.26
**
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QSETTINGS_H
#define QSETTINGS_H

#include <qobject.h>
#include <qstring.h>


class QSettingsPrivate;


class QSettings
{
public:
    enum System { Unix, Windows };

    QSettings(bool = FALSE, QSettings * = 0);
    ~QSettings();

    void setWritable(bool);
    bool writable() const;

    bool write();

    void setFallback(QSettings *);
    const QSettings *fallback() const;

    void setPath(System, const QString &);
    const QString &path(System) const;

    bool writeEntry(const QString &, const QVariant &);
    QVariant readEntry(const QString &);

    bool removeEntry(const QString &);


protected:


private:
    QSettingsPrivate *d;

#if defined(Q_DISABLE_COPY)
    QSettings(const QSettings &);
    QSettings &operator=(const QSettings &);
#endif

};


#endif // QSETTINGS_H
