#ifndef QSETTINGS_P_H
#define QSETTINGS_P_H

#include <qdatetime.h>
#include <qmap.h>
#include <qmutex.h>
#include <qiodevice.h>
#include <qstack.h>
#include <qstringlist.h>
#include <private/qobject_p.h>

#ifdef Q_OS_WIN
#include <qt_windows.h>
#endif

/*
    The numeric values of these enums define their search order. For example,
    F_User | F_Organization is searched before F_Global |
    F_Application, because their values are respectively 1 and 2.
*/
enum {
    F_Application = 0x0,
    F_Organization = 0x1,
    F_User = 0x0,
    F_Global = 0x2,
    NumConfFiles = 4
};

class QSettingsKey : public QString
{
public:
    inline QSettingsKey(const QString &key, Qt::CaseSensitivity cs)
         : QString(key), theRealKey(key)
    {
        if (cs == Qt::CaseInsensitive)
            QString::operator=(toLower());
    }

    inline QString realKey() const { return theRealKey; }

private:
    QString theRealKey;
};

typedef QMap<QSettingsKey, QCoreVariant> SettingsKeyMap;

class QSettingsGroup
{
public:
    inline QSettingsGroup()
        : num(-1), maxNum(-1) {}
    inline QSettingsGroup(const QString &s)
        : str(s), num(-1), maxNum(-1) {}
    inline QSettingsGroup(const QString &s, bool guessArraySize)
        : str(s), num(0), maxNum(guessArraySize ? 0 : -1) {}

    inline QString name() const { return str; }
    inline QString toString() const;
    inline bool isArray() const { return num != -1; }
    inline int arraySizeGuess() const { return maxNum; }
    inline void setArrayIndex(int i)
    { num = i + 1; if (maxNum != -1 && num > maxNum) maxNum = num; }

    QString str;
    int num;
    int maxNum;
};

inline QString QSettingsGroup::toString() const
{
    QString result;
    result = str;
    if (num > 0) {
        result += QLatin1Char('/');
        result += QString::number(num);
    }
    return result;
}

class Q_CORE_EXPORT QConfFile
{
public:
    bool mergeKeyMaps();

    static QConfFile *fromName(const QString &name);
    static void clearCache();

    QString name;
    QDateTime timeStamp;
    Q_LONGLONG size;
    SettingsKeyMap originalKeys;
    SettingsKeyMap addedKeys;
    SettingsKeyMap removedKeys;
    QAtomic ref;

#ifdef Q_OS_WIN
    HANDLE semHandle; // semaphore used for synchronizing access to this file
#endif

private:
#ifdef Q_DISABLE_COPY
    QConfFile(const QConfFile &);
    QConfFile &operator=(const QConfFile &);
#endif
    QConfFile(const QString &name);
    friend class QConfFile_createsItself; // supress compiler warning
};

class Q_CORE_EXPORT QSettingsPrivate
#ifndef QT_NO_QOBJECT
    : public QObjectPrivate
#endif
{
#ifdef QT_NO_QOBJECT
    QSettings *q_ptr;
#endif
    Q_DECLARE_PUBLIC(QSettings)

public:
    QSettingsPrivate();
    virtual ~QSettingsPrivate();

    virtual void remove(const QString &key) = 0;
    virtual void set(const QString &key, const QCoreVariant &value) = 0;
    virtual bool get(const QString &key, QCoreVariant *value) const = 0;

    enum ChildSpec { AllKeys, ChildKeys, ChildGroups };
    virtual QStringList children(const QString &prefix, ChildSpec spec) const = 0;

    virtual void clear() = 0;
    virtual void sync() = 0;
    virtual bool isWritable() const = 0;
    virtual QString fileName() const;

    QString actualKey(const QString &key) const;
    void beginGroupOrArray(const QSettingsGroup &group);
    void setStatus(QSettings::Status status);
    void requestUpdate();
    void update();

    static QString normalizedKey(const QString &key);
    static QSettingsPrivate *create(Qt::SettingsFormat format, Qt::SettingsScope scope,
                                        const QString &organization, const QString &application);
    static QSettingsPrivate *create(const QString &fileName, Qt::SettingsFormat format);

    static void processChild(QString key, ChildSpec spec, QMap<QString, QString> &result);

    // Variant streaming functions
    QStringList variantListToStringList(const QCoreVariantList &l) const;
    QCoreVariantList stringListToVariantList(const QStringList &l) const;

    // parser functions
    static QString &escapedLeadingAt(QString &s);
    static QString &unescapedLeadingAt(QString &s);
    static QString variantToString(const QCoreVariant &v);
    static QCoreVariant stringToVariant(const QString &s);
    static void iniEscapedKey(const QString &key, QByteArray &result);
    static bool iniUnescapedKey(const QByteArray &key, int from, int to, QString &result);
    static void iniEscapedString(const QString &str, QByteArray &result);
    static void iniChopTrailingSpaces(QString *str);
    static void iniEscapedStringList(const QStringList &strs, QByteArray &result);
    static QStringList *iniUnescapedStringList(const QByteArray &str, int from, int to,
                                                QString &result);
    static QStringList splitArgs(const QString &s, int idx);

protected:
    QStack<QSettingsGroup> groupStack;
    QString groupPrefix;
    int spec;
    bool fallbacks;
    bool pendingChanges;
    QSettings::Status status;
};

class QConfFileSettingsPrivate : public QSettingsPrivate
{
public:
    QConfFileSettingsPrivate(Qt::SettingsFormat format, Qt::SettingsScope scope,
                             const QString &organization, const QString &application);
    QConfFileSettingsPrivate(const QString &fileName, Qt::SettingsFormat format);
    ~QConfFileSettingsPrivate();

    void remove(const QString &key);
    void set(const QString &key, const QCoreVariant &value);
    bool get(const QString &key, QCoreVariant *value) const;

    QStringList children(const QString &prefix, ChildSpec spec) const;

    void clear();
    void sync();
    bool isWritable() const;
    QString fileName() const;

    void init();

private:

    bool readFile(QConfFile *confFile);
    bool writeFile(QConfFile *confFile);

    bool readIniLine(QIODevice &device, QByteArray &line, int &len, int &equalsCharPos);
    bool readIniFile(QIODevice &device, SettingsKeyMap *map);
    bool writeIniFile(QIODevice &device, const SettingsKeyMap &map);
#ifdef Q_OS_MAC
    bool readPlistFile(const QString &fileName, SettingsKeyMap *map) const;
    bool writePlistFile(const QString &fileName, const SettingsKeyMap &map) const;
#endif

    QConfFile *confFiles[NumConfFiles];
    Qt::SettingsFormat format;
    Qt::CaseSensitivity cs;
    bool readAccess;
    bool writeAccess;
};

#endif // QSETTINGS_P_H
