/****************************************************************************
**
**
** Definition of QSettings class
**
** Created : 2000.06.26
**
** Copyright (C) 2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
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
#include <qmap.h>


class QSettingsNode;


class QSettings : public QObject
{
    Q_OBJECT

public:
    QSettings(bool writable = FALSE, QSettings *override = 0,
	      QObject *parent = 0, const char *name = 0);
    ~QSettings();

    void setWritable(bool writable);
    bool writable() const;

    void write();

    void setOverride(QSettings *settings);
    const QSettings *override() const;

    void setPath(int system, const QString &path);
    const QString &path(int system) const;

    // "key" is of the form /path/to/key, e.g. /trolltech/designer/palette/active/foreground
    void writeEntry(const QString &key, const QVariant &value);
    QVariant readEntry(const QString &key);
    
    // this works like a remove directory... if it is a path instead of an entry, it will remove
    // all entries and all children
    void removeEntry(const QString &key);

    enum System { Unix, Windows };


protected:
    void cleanup();


private:
    // TODO: make this work
    // QSettingsPrivate *d;

    // Override settings in this object with ones from the override object
    QSettings *override_p;

    // Where do we save these settings on the system?
    QMap<int, QString> pathMap_p;

    // Are changes to the settings able to be saved (written to disk/registry)?
    bool writable_p;

    // Settings values
    QSettingsNode *node, *tree;


#if defined(Q_DISABLE_COPY)
    QSettings(const QSettings &);
    QSettings &operator=(const QSettings &);
#endif

};


#endif // QSETTINGS_H
