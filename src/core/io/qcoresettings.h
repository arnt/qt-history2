#ifndef QCORESETTINGS_H
#define QCORESETTINGS_H

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

class QCoreSettingsPrivate;

class Q_CORE_EXPORT QCoreSettings 
#ifndef QT_NO_QOBJECT
    : public QObject
#endif
{
#ifndef QT_NO_QOBJECT
    Q_OBJECT
#else
    QCoreSettingsPrivate *d_ptr;
#endif
    Q_DECLARE_PRIVATE(QCoreSettings)

public:
    enum Status { NoError = 0, AccessError, FormatError };

#ifndef QT_NO_QOBJECT
    QCoreSettings(const QString &organization, const QString &application = QString(),
                  QObject *parent = 0);
    QCoreSettings(Qt::SettingsScope scope, const QString &organization,
                  const QString &application = QString(), QObject *parent = 0);
    QCoreSettings(Qt::SettingsFormat format, Qt::SettingsScope scope,
                  const QString &organization, const QString &application = QString(),
                  QObject *parent = 0);
    QCoreSettings(const QString &fileName, Qt::SettingsFormat format, QObject *parent = 0);
    QCoreSettings(QObject *parent = 0);
#else
    QCoreSettings(const QString &organization, const QString &application = QString());
    QCoreSettings(Qt::SettingsScope scope, const QString &organization,
                  const QString &application = QString());
    QCoreSettings(Qt::SettingsFormat format, Qt::SettingsScope scope,
                  const QString &organization, const QString &application = QString());
    QCoreSettings(const QString &fileName, Qt::SettingsFormat format);
#endif
    ~QCoreSettings();

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
    QCoreSettings(QCoreSettingsPrivate *p, QObject *parent = 0);
    bool event(QEvent *event);
#endif

private:
    Q_DISABLE_COPY(QCoreSettings)
};

#endif // QCORESETTINGS_H
