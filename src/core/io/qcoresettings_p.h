#ifndef QCORESETTINGS_P_H
#define QCORESETTINGS_P_H

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
#ifndef QT_NO_QOBJECT
    : public QObjectPrivate
#endif
{
#ifdef QT_NO_QOBJECT
    QCoreSettings *q_ptr;
#endif
    Q_DECLARE_PUBLIC(QCoreSettings)

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
    virtual QString fileName() const;

    QString actualKey(const QString &key) const;
    void beginGroupOrArray(const QSettingsGroup &group);
    void setStatus(QCoreSettings::Status status);
    void requestUpdate();
    void update();

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

    // parser functions
    static QString &escapedLeadingAt(QString &s);
    static QString &unescapedLeadingAt(QString &s);
    static QString variantToStringCoreImpl(const QCoreVariant &v);
    static QCoreVariant stringToVariantCoreImpl(const QString &s);
    static void iniEscapedKey(const QString &key, QByteArray &result);
    static bool iniUnescapedKey(const QByteArray &key, int from, int to, QString &result);
    static void iniEscapedString(const QString &str, QByteArray &result);
    static void iniChopTrailingSpaces(QString *str);
    static void iniEscapedStringList(const QStringList &strs, QByteArray &result);
    static QStringList *iniUnescapedStringList(const QByteArray &str, int from, int to,
                                                QString &result);
    static QStringList splitArgs(const QString &s, int idx);
    static QString variantToStringGuiImpl(const QCoreVariant &v);
    static QCoreVariant stringToVariantGuiImpl(const QString &s);
    
protected:
    QStack<QSettingsGroup> groupStack;
    QString groupPrefix;
    int spec;
    bool fallbacks;
    bool pendingChanges;
    QCoreSettings::Status status;

    VariantToStringFunc variantToString;
    StringToVariantFunc stringToVariant;
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

/*
    This was the only way we could think of to give access to some internal parsing
    functions for testing by the autotests.
*/

#define QSETTINGS_DEFINE_GUI_PARSER_FUNCTIONS \
\
static QString variantToStringGuiImpl(const QCoreVariant &v)\
{\
    QVariant v2 = v;\
    QString result;\
\
    switch (v.type()) {\
        case QCoreVariant::Rect: {\
            QRect r = v2.toRect();\
            result += "@Rect("\
                        + QByteArray::number(r.x()) + ", "\
                        + QByteArray::number(r.y()) + ", "\
                        + QByteArray::number(r.width()) + ", "\
                        + QByteArray::number(r.height()) + ")";\
            break;\
        }\
\
        case QCoreVariant::Size: {\
            QSize s = v2.toSize();\
            result += "@Size("\
                        + QByteArray::number(s.width()) + ", "\
                        + QByteArray::number(s.height()) + ")";\
            break;\
        }\
\
        case QCoreVariant::Color: {\
            QColor c = v2.toColor();\
            result += "@Color("\
                        + QByteArray::number(c.red()) + ", "\
                        + QByteArray::number(c.green()) + ", "\
                        + QByteArray::number(c.blue()) + ")";\
            break;\
        }\
\
        case QCoreVariant::Point: {\
            QPoint p = v2.toPoint();\
            result += "@Point("\
                        + QByteArray::number(p.x()) + ", "\
                        + QByteArray::number(p.y()) + ")";\
            break;\
        }\
\
        default:\
            result = QCoreSettingsPrivate::variantToStringCoreImpl(v);\
            break;\
    }\
\
    return result;\
}\
\
static QCoreVariant stringToVariantGuiImpl(const QString &s)\
{\
    if (s.length() > 3\
            && s.at(0) == QLatin1Char('@')\
            && s.at(s.length() - 1) == QLatin1Char(')')) {\
\
        if (s.startsWith(QLatin1String("@Rect("))) {\
            QStringList args = QCoreSettingsPrivate::splitArgs(s, 5);\
            if (args.size() == 4) {\
                return QVariant(QRect(args[0].toInt(), args[1].toInt(),\
                                        args[2].toInt(), args[3].toInt()));\
            }\
        } else if (s.startsWith(QLatin1String("@Size("))) {\
            QStringList args = QCoreSettingsPrivate::splitArgs(s, 5);\
            if (args.size() == 2) {\
                return QVariant(QSize(args[0].toInt(), args[1].toInt()));\
            }\
        } else if (s.startsWith(QLatin1String("@Color("))) {\
            QStringList args = QCoreSettingsPrivate::splitArgs(s, 6);\
            if (args.size() == 3) {\
                return QVariant(QColor(args[0].toInt(), args[1].toInt(),\
                                        args[2].toInt()));\
            }\
        } else if (s.startsWith(QLatin1String("@Point("))) {\
            QStringList args = QCoreSettingsPrivate::splitArgs(s, 6);\
            if (args.size() == 2) {\
                return QVariant(QPoint(args[0].toInt(), args[1].toInt()));\
            }\
        }\
    }\
\
    return QCoreSettingsPrivate::stringToVariantCoreImpl(s);\
}

#endif // QCORESETTINGS_P_H
