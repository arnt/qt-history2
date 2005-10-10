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

#include <QtCore/qobject.h>
#include <QtCore/qvariant.h>
#include <QtCore/qstring.h>

QT_MODULE(Core)

#ifndef QT_NO_SETTINGS

#ifdef QT3_SUPPORT
#include <QtCore/qstringlist.h>
#endif

#include <ctype.h>

#ifdef Status // ### we seem to pick up a macro Status --> int somewhere
#undef Status
#endif

class QIODevice;
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
        IniFormat,

        InvalidFormat = 16,
        CustomFormat1,
        CustomFormat2,
        CustomFormat3,
        CustomFormat4,
        CustomFormat5,
        CustomFormat6,
        CustomFormat7,
        CustomFormat8,
        CustomFormat9,
        CustomFormat10,
        CustomFormat11,
        CustomFormat12,
        CustomFormat13,
        CustomFormat14,
        CustomFormat15,
        CustomFormat16
    };

    enum Scope {
        UserScope,
        SystemScope
#ifdef QT3_SUPPORT
        ,
        User = UserScope,
        Global = SystemScope
#endif
    };

#ifndef QT_NO_QOBJECT
    explicit QSettings(const QString &organization,
                       const QString &application = QString(), QObject *parent = 0);
    QSettings(Scope scope, const QString &organization,
              const QString &application = QString(), QObject *parent = 0);
    QSettings(Format format, Scope scope, const QString &organization,
	      const QString &application = QString(), QObject *parent = 0);
    QSettings(const QString &fileName, Format format, QObject *parent = 0);
    explicit QSettings(QObject *parent = 0);
#else
    explicit QSettings(const QString &organization,
                       const QString &application = QString());
    QSettings(Scope scope, const QString &organization,
              const QString &application = QString());
    QSettings(Format format, Scope scope, const QString &organization,
              const QString &application = QString());
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

    void setValue(const QString &key, const QVariant &value);
    QVariant value(const QString &key, const QVariant &defaultValue = QVariant()) const;

    void remove(const QString &key);
    bool contains(const QString &key) const;

    void setFallbacksEnabled(bool b);
    bool fallbacksEnabled() const;

    QString fileName() const;

    static void setSystemIniPath(const QString &dir); // ### remove in 5.0 (use setPath() instead)
    static void setUserIniPath(const QString &dir);   // ### remove in 5.0 (use setPath() instead)
    static void setPath(Format format, Scope scope, const QString &path);

    typedef QMap<QString, QVariant> SettingsMap;
    typedef bool (*ReadFunc)(QIODevice &device, SettingsMap &map);
    typedef bool (*WriteFunc)(QIODevice &device, const SettingsMap &map);

    static Format registerFormat(const QString &extension, ReadFunc readFunc, WriteFunc writeFunc,
                                 Qt::CaseSensitivity caseSensitivity = Qt::CaseSensitive);

#ifdef QT3_SUPPORT
    inline QT3_SUPPORT bool writeEntry(const QString &key, bool value)
    { setValue(key, value); return isWritable(); }
    inline QT3_SUPPORT bool writeEntry(const QString &key, double value)
    { setValue(key, value); return isWritable(); }
    inline QT3_SUPPORT bool writeEntry(const QString &key, int value)
    { setValue(key, value); return isWritable(); }
    inline QT3_SUPPORT bool writeEntry(const QString &key, const char *value)
    { setValue(key, value); return isWritable(); }
    inline QT3_SUPPORT bool writeEntry(const QString &key, const QString &value)
    { setValue(key, value); return isWritable(); }
    inline QT3_SUPPORT bool writeEntry(const QString &key, const QStringList &value)
    { setValue(key, value); return isWritable(); }
    inline QT3_SUPPORT bool writeEntry(const QString &key, const QStringList &value, QChar separator)
    { setValue(key, value.join(QString(separator))); return isWritable(); }
    inline QT3_SUPPORT QStringList readListEntry(const QString &key, bool *ok = 0)
    {
        if (ok)
            *ok = contains(key);
        return value(key).toStringList();
    }
    inline QT3_SUPPORT QStringList readListEntry(const QString &key, QChar separator, bool *ok = 0)
    {
        if (ok)
            *ok = contains(key);
        QString str = value(key).toString();
        if (str.isEmpty())
            return QStringList();
        return str.split(separator);
    }
    inline QT3_SUPPORT QString readEntry(const QString &key, const QString &defaultValue = QString(),
                                         bool *ok = 0)
    {
        if (ok)
            *ok = contains(key);
        return value(key, defaultValue).toString();
    }
    inline QT3_SUPPORT int readNumEntry(const QString &key, int defaultValue = 0, bool *ok = 0)
    {
        if (ok)
            *ok = contains(key);
        return value(key, defaultValue).toInt();
    }
    inline QT3_SUPPORT double readDoubleEntry(const QString &key, double defaultValue = 0,
                                              bool *ok = 0)
    {
        if (ok)
            *ok = contains(key);
        return value(key, defaultValue).toDouble();
    }
    inline QT3_SUPPORT bool readBoolEntry(const QString &key, bool defaultValue = false,
                                          bool *ok = 0)
    {
        if (ok)
            *ok = contains(key);
        return value(key, defaultValue).toBool();
    }
    inline QT3_SUPPORT bool removeEntry(const QString &key)
    { remove(key); return true; }

    enum System { Unix, Windows, Mac };
    inline QT3_SUPPORT void insertSearchPath(System, const QString &) {}
    inline QT3_SUPPORT void removeSearchPath(System, const QString &) {}

    inline QT3_SUPPORT void setPath(const QString &organization, const QString &application,
                                    Scope scope = Global)
    {
#ifndef QT_NO_QOBJECT
        QObject *parent = this->parent();
        this->~QSettings();
        new (this) QSettings(scope == Global ? QSettings::SystemScope : QSettings::UserScope,
                             organization, application, parent);
#else
        this->~QSettings();
        new (this) QSettings(scope == Global ? QSettings::SystemScope : QSettings::UserScope,
                             organization, application);
#endif
    }
    inline QT3_SUPPORT void resetGroup()
    {
        while (!group().isEmpty())
            endGroup();
    }
    inline QT3_SUPPORT QStringList entryList(const QString &key) const
    {
        QSettings *that = const_cast<QSettings *>(this);
        QStringList result;

        that->beginGroup(key);
        result = that->childKeys();
        that->endGroup();
        return result;
    }
    inline QT3_SUPPORT QStringList subkeyList(const QString &key) const
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

#endif // QT_NO_SETTINGS

#endif // QSETTINGS_H
