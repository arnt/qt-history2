#ifndef QSETTINGS_H
#define QSETTINGS_H

#include "qobject.h"
#include "qcorevariant.h"
#include "qstring.h"
#ifdef QT_COMPAT
#include "qstringlist.h"
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
    enum Status { NoError = 0, AccessError, FormatError };

#ifndef QT_NO_QOBJECT
    QSettings(const QString &organization, const QString &application = QString(),
              QObject *parent = 0);
    QSettings(Qt::SettingsScope scope, const QString &organization,
              const QString &application = QString(), QObject *parent = 0);
    QSettings(Qt::SettingsFormat format, Qt::SettingsScope scope,
              const QString &organization, const QString &application = QString(),
              QObject *parent = 0);
    QSettings(const QString &fileName, Qt::SettingsFormat format, QObject *parent = 0);
    QSettings(QObject *parent = 0);
#else
    QSettings(const QString &organization, const QString &application = QString());
    QSettings(Qt::SettingsScope scope, const QString &organization,
              const QString &application = QString());
    QSettings(Qt::SettingsFormat format, Qt::SettingsScope scope,
              const QString &organization, const QString &application = QString());
    QSettings(const QString &fileName, Qt::SettingsFormat format);
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

protected:
#ifndef QT_NO_QOBJECT
    bool event(QEvent *event);
#endif

private:
    Q_DISABLE_COPY(QSettings)
};

#endif // QSETTINGS_H
