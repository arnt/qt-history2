#ifndef QCORESETTINGS_P_H
#define QCORESETTINGS_P_H

#include <qdatetime.h>
#include <qmap.h>
#include <qmutex.h>
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

class QConfFile
{
public:
    bool mergeKeyMaps();

    static QConfFile *fromName(const QString &name);
    static void clearCache();

    QString name;
    QDateTime timeStamp;
    QIODevice::Offset size;
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
};

class Q_CORE_EXPORT QCoreSettingsPrivate
{
    Q_DECLARE_PUBLIC(QCoreSettings);

public:
    typedef QString (*VariantToStringFunc)(const QCoreVariant &v);
    typedef QCoreVariant (*StringToVariantFunc)(const QString &s);

    QCoreSettingsPrivate();
    virtual ~QCoreSettingsPrivate();

    virtual void remove(const QString &key) = 0;
    virtual void set(const QString &key, const QCoreVariant &value) = 0;
    virtual bool get(const QString &key, QCoreVariant *value) const = 0;

    enum ChildSpec { AllKeys, ChildKeys, ChildGroups };
    virtual QStringList children(const QString &prefix, ChildSpec spec) const = 0;

    virtual void clear() = 0;
    virtual void sync() = 0;
    virtual bool isWritable() const = 0;
    virtual QString path() const = 0;

    QString actualKey(const QString &key) const;
    void beginGroupOrArray(const QSettingsGroup &group);
    void setStatus(QCoreSettings::Status status);
//    void requestUpdate();

    static QString normalizedKey(const QString &key);
    static QCoreSettingsPrivate *create(Qt::SettingsFormat format, Qt::SettingsScope scope,
                                        const QString &organization, const QString &application,
                                        VariantToStringFunc vts, StringToVariantFunc stv);
    static QCoreSettingsPrivate *create(const QString &fileName, Qt::SettingsFormat format,
                                        VariantToStringFunc vts, StringToVariantFunc stv);

    static void processChild(QString key, ChildSpec spec, QMap<QString, QString> &result);

    // Variant streaming functions
    QStringList variantListToStringList(const QCoreVariantList &l) const;
    QCoreVariantList stringListToVariantList(const QStringList &l) const;
    void setStreamingFunctions(VariantToStringFunc vts, StringToVariantFunc stv);

protected:
    QStack<QSettingsGroup> groupStack;
    QString groupPrefix;
    int spec;
    bool fallbacks;
    bool pendingChanges;
    QCoreSettings::Status status;

    VariantToStringFunc variantToString;
    StringToVariantFunc stringToVariant;

    QCoreSettings *q_ptr;
};

class QConfFileSettingsPrivate : public QCoreSettingsPrivate
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
    QString path() const;

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

#endif // QCORESETTINGS_P_H
