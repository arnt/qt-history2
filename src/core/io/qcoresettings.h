#ifndef QCORESETTINGS_H
#define QCORESETTINGS_H

#ifndef QT_H
#include "qcorevariant.h"
#include "qstring.h"
#ifdef QT_COMPAT
#include "qstringlist.h"
#endif
#endif // QT_H

#include <ctype.h>

#ifdef Status // ### we seem to pick up a macro Status --> int somewhere
#undef Status
#endif

class QCoreSettingsPrivate;

class Q_CORE_EXPORT QCoreSettings
{
    Q_DECLARE_PRIVATE(QCoreSettings);

public:
    enum Status { NoError = 0, AccessError, FormatError };

    QCoreSettings(const QString &organization, const QString &application = QString());
    QCoreSettings(Qt::SettingsScope scope, const QString &organization,
                  const QString &application = QString());
    QCoreSettings(Qt::SettingsFormat format, Qt::SettingsScope scope,
                    const QString &organization, const QString &application = QString());
    QCoreSettings(const QString &fileName, Qt::SettingsFormat format);
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

    QString path() const;

protected:
//    bool event(QEvent *event);
    QCoreSettings(QCoreSettingsPrivate *p);

    QCoreSettingsPrivate *d_ptr;

#ifdef QSETTINGS_EXPORT_PARSER_FUNCTIONS_FOR_TESTING
public:
#else
    friend class QConfFileSettingsPrivate;
#endif
    static QString variantToStringCoreImpl(const QCoreVariant &v);
    static QCoreVariant stringToVariantCoreImpl(const QString &s);
    static void iniEscapedKey(const QString &key, QByteArray &result);
    static bool iniUnescapedKey(const QByteArray &key, int from, int to, QString &result);
    static void iniEscapedString(const QString &str, QByteArray &result);
    static void iniChopTrailingSpaces(QString *str);
    static void iniEscapedStringList(const QStringList &strs, QByteArray &result);
    static QStringList *iniUnescapedStringList(const QByteArray &str, int from, int to,
                                                QString &result);

private:
#ifdef Q_DISABLE_COPY
    QCoreSettings(const QCoreSettings &);
    QCoreSettings &operator=(const QCoreSettings &);
#endif
};

#endif // QCORESETTINGS_H
