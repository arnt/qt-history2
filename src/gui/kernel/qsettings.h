#ifndef QSETTINGS_H
#define QSETTINGS_H

#include "qcoresettings.h"
#include "qvariant.h"

class Q_GUI_EXPORT QSettings : public QCoreSettings
{
public:
    QSettings(const QString &organization, const QString &application = QString());
    QSettings(Qt::SettingsScope scope, const QString &organization,
              const QString &application = QString());
    QSettings(Qt::SettingsFormat format, Qt::SettingsScope scope, const QString &organization,
              const QString &application = QString());
    QSettings(const QString &fileName, Qt::SettingsFormat format);
    ~QSettings();

    inline void setValue(const QString &key, const QVariant &value)
    { QCoreSettings::setValue(key, value); }
    inline QVariant value(const QString &key, const QVariant &defaultValue = QVariant()) const
    { return QCoreSettings::value(key, defaultValue); }

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
    inline QT_COMPAT bool writeEntry(const QString &key, const QStringList &value, QChar sep)
    { setValue(key, value.join(QString(sep))); return isWritable(); }
    inline QT_COMPAT QStringList readListEntry(const QString &key, bool *ok = 0)
    {
        if (ok)
            *ok = contains(key);
        return value(key).toStringList();
    }
    inline QT_COMPAT QStringList readListEntry(const QString &key, QChar sep, bool *ok = 0)
    {
        if (ok)
            *ok = contains(key);
        QString str = value(key).toString();
        if (str.isEmpty())
            return QStringList();
        return str.split(sep);
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
    inline QT_COMPAT bool readBoolEntry(const QString &key, bool defaultValue = false, bool *ok = 0)
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

    enum Scope { User, Global };
    inline QT_COMPAT void setPath(const QString &organization, const QString &application,
                                  Scope scope = Global)
    {
	this->~QSettings();
	new (this) QSettings(scope == Global ? Qt::GlobalScope : Qt::UserScope,
                                organization, application);
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

private:
#ifdef Q_DISABLE_COPY
    QSettings(const QSettings &);
    QSettings &operator=(const QSettings &);
#endif

#ifdef QSETTINGS_EXPORT_PARSER_FUNCTIONS_FOR_TESTING
public:
#endif
    static QString variantToStringGuiImpl(const QCoreVariant &v);
    static QCoreVariant stringToVariantGuiImpl(const QString &s);
};

#endif // QSETTINGS_H
