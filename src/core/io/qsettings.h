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

#ifndef QSETTINGS_H
#define QSETTINGS_H

#include "QtCore/qobject.h"
#include "QtCore/qcorevariant.h"
#include "QtCore/qstring.h"
#ifdef QT_COMPAT
#include "QtCore/qstringlist.h"
#endif

#include <ctype.h>

#ifdef Status // ### we seem to pick up a macro Status --> int somewhere
#undef Status
#endif

class QSettingsPrivate;

#ifndef QT_NO_QOBJECT
class Q_CORE_EXPORT QSettings : public QObject
#else
class Q_CORE_EXPORT QSettings 
#endif
{
#ifndef QT_NO_QOBJECT
    Q_OBJECT
#else
    QSettingsPrivate *d_ptr;
#endif
    Q_DECLARE_PRIVATE(QSettings)

public:
    enum Status { 
        NoError = 0, 
        AccessError, 
        FormatError 
    };
    
    enum Format {
        NativeFormat,
        IniFormat
    };

    enum Scope {
        UserScope,
        SystemScope
#ifdef QT_COMPAT
        ,
        User = UserScope,
        Global = SystemScope
#endif
    };


#ifndef QT_NO_QOBJECT
    QSettings(const QString &organization, const QString &application = QString(),
              QObject *parent = 0);
    QSettings(Scope scope, const QString &organization,
              const QString &application = QString(), QObject *parent = 0);
    QSettings(Format format, Scope scope,
              const QString &organization, const QString &application = QString(),
              QObject *parent = 0);
    QSettings(const QString &fileName, Format format, QObject *parent = 0);
    QSettings(QObject *parent = 0);
#else
    QSettings(const QString &organization, const QString &application = QString());
    QSettings(Scope scope, const QString &organization,
              const QString &application = QString());
    QSettings(Format format, Scope scope,
              const QString &organization, const QString &application = QString());
    QSettings(const QString &fileName, Format format);
#endif
    ~QSettings();

    void clear();
    void sync();
    Status status() const;

    void beginGroup(const QString &prefix);
    void endGroup();
    QString group() const;

    int beginReadArray(const QString &prefix);
    void beginWriteArray(const QString &prefix, int size = -1);
    void endArray();
    void setArrayIndex(int i);

    QStringList allKeys() const;
    QStringList childKeys() const;
    QStringList childGroups() const;
    bool isWritable() const;

    void setValue(const QString &key, const QCoreVariant &value);
    QCoreVariant value(const QString &key, const QCoreVariant &defaultValue = QCoreVariant()) const;

    void remove(const QString &key);
    bool contains(const QString &key) const;

    void setFallbacksEnabled(bool b);
    bool fallbacksEnabled() const;

    QString fileName() const;
    
    static void setSystemIniPath(const QString &dir);
    static void setUserIniPath(const QString &dir);

#ifdef QT_COMPAT
    inline QT_COMPAT bool writeEntry(const QString &key, bool value)
    { setValue(key, value); return isWritable(); }
    inline QT_COMPAT bool writeEntry(const QString &key, double value)
    { setValue(key, value); return isWritable(); }
    inline QT_COMPAT bool writeEntry(const QString &key, int value)
    { setValue(key, value); return isWritable(); }
    inline QT_COMPAT bool writeEntry(const QString &key, const char *value)
    { setValue(key, value); return isWritable(); }
    inline QT_COMPAT bool writeEntry(const QString &key, const QString &value)
    { setValue(key, value); return isWritable(); }
    inline QT_COMPAT bool writeEntry(const QString &key, const QStringList &value)
    { setValue(key, value); return isWritable(); }
    inline QT_COMPAT bool writeEntry(const QString &key, const QStringList &value, QChar separator)
    { setValue(key, value.join(QString(separator))); return isWritable(); }
    inline QT_COMPAT QStringList readListEntry(const QString &key, bool *ok = 0)
    {
        if (ok)
            *ok = contains(key);
        return value(key).toStringList();
    }
    inline QT_COMPAT QStringList readListEntry(const QString &key, QChar separator, bool *ok = 0)
    {
        if (ok)
            *ok = contains(key);
        QString str = value(key).toString();
        if (str.isEmpty())
            return QStringList();
        return str.split(separator);
    }
    inline QT_COMPAT QString readEntry(const QString &key, const QString &defaultValue = QString(),
                                       bool *ok = 0)
    {
        if (ok)
            *ok = contains(key);
        return value(key, defaultValue).toString();
    }
    inline QT_COMPAT int readNumEntry(const QString &key, int defaultValue = 0, bool *ok = 0)
    {
        if (ok)
            *ok = contains(key);
        return value(key, defaultValue).toInt();
    }
     inline QT_COMPAT double readDoubleEntry(const QString &key, double defaultValue = 0,
                                            bool *ok = 0)
    {
        if (ok)
            *ok = contains(key);
        return value(key, defaultValue).toDouble();
    }
    inline QT_COMPAT bool readBoolEntry(const QString &key, bool defaultValue = false,
                                        bool *ok = 0)
    {
        if (ok)
            *ok = contains(key);
        return value(key, defaultValue).toBool();
    }
    inline QT_COMPAT bool removeEntry(const QString &key)
    { remove(key); return true; }

    enum System { Unix, Windows, Mac };
    inline QT_COMPAT void insertSearchPath(System, const QString &) {}
    inline QT_COMPAT void removeSearchPath(System, const QString &) {}

    inline QT_COMPAT void setPath(const QString &organization, const QString &application,
                                  Scope scope = Global)
    {
        QObject *parent = this->parent();
        this->~QSettings();
        new (this) QSettings(scope == Global ? QSettings::SystemScope : QSettings::UserScope,
                             organization, application, parent);
    }
    inline QT_COMPAT void resetGroup()
    {
        while (!group().isEmpty())
            endGroup();
    }
    inline QT_COMPAT QStringList entryList(const QString &key) const
    {
        QSettings *that = const_cast<QSettings *>(this);
        QStringList result;

        that->beginGroup(key);
        result = that->childKeys();
        that->endGroup();
        return result;
    }
    inline QT_COMPAT QStringList subkeyList(const QString &key) const
    {
        QSettings *that = const_cast<QSettings *>(this);
        QStringList result;

        that->beginGroup(key);
        result = that->childGroups();
        that->endGroup();
        return result;
    }
#endif
   
    
protected:
#ifndef QT_NO_QOBJECT
    bool event(QEvent *event);
#endif

private:
    Q_DISABLE_COPY(QSettings)
};

#endif // QSETTINGS_H
