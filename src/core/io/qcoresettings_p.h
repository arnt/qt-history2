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

class Q_CORE_EXPORT QCoreSettingsPrivate : public QObjectPrivate
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
    void requestUpdate();

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

/*
    This was the only way we could think of to give access to some internal parsing functions
    for testing by the autotests.
*/
#define QSETTINGS_DEFINE_CORE_PARSER_FUNCTIONS \
\
static QString &escapedLeadingAt(QString &s)\
{\
    if (s.length() > 0 && s.at(0) == QLatin1Char('@'))\
        s.prepend(QLatin1Char('@'));\
    return s;\
}\
\
static QString &unescapedLeadingAt(QString &s)\
{\
    if (s.length() >= 2\
        && s.at(0) == QLatin1Char('@')\
        && s.at(1) == QLatin1Char('@'))\
        s.remove(0, 1);\
    return s;\
}\
\
static QString variantToStringCoreImpl(const QCoreVariant &v)\
{\
    QString result;\
\
    switch (v.type()) {\
        case QCoreVariant::Invalid:\
            break;\
\
        case QCoreVariant::ByteArray: {\
            QByteArray a = v.toByteArray();\
            result = QLatin1String("@ByteArray(");\
            result += QString::fromLatin1(a.constData(), a.size());\
            result += QLatin1Char(')');\
            break;\
        }\
\
        case QCoreVariant::String:\
        case QCoreVariant::LongLong:\
        case QCoreVariant::ULongLong:\
        case QCoreVariant::Int:\
        case QCoreVariant::UInt:\
        case QCoreVariant::Bool:\
        case QCoreVariant::Double: {\
            result = v.toString();\
            result = escapedLeadingAt(result);\
            break;\
        }\
\
        default: {\
            QByteArray a;\
            {\
                QDataStream s(&a, QIODevice::WriteOnly);\
                s << v;\
            }\
\
            result = QLatin1String("@Variant(");\
            result += QString::fromLatin1(a.constData(), a.size());\
            result += QLatin1Char(')');\
            break;\
        }\
    }\
\
    return result;\
}\
\
static QCoreVariant stringToVariantCoreImpl(const QString &s)\
{\
    if (s.length() > 3\
            && s.at(0) == QLatin1Char('@')\
            && s.at(s.length() - 1) == QLatin1Char(')')) {\
\
        if (s.startsWith(QLatin1String("@ByteArray("))) {\
            return QCoreVariant(QByteArray(s.latin1() + 11, s.length() - 12));\
        } else if (s.startsWith(QLatin1String("@Variant("))) {\
            QByteArray a(s.latin1() + 9, s.length() - 10);\
            QDataStream stream(&a, QIODevice::ReadOnly);\
            QCoreVariant result;\
            stream >> result;\
            return result;\
        }\
    }\
\
    QString tmp = s;\
    return QCoreVariant(unescapedLeadingAt(tmp));\
}\
\
static const char hexDigits[] = "0123456789ABCDEF";\
\
static void iniEscapedKey(const QString &key, QByteArray &result)\
{\
    for (int i = 0; i < key.size(); ++i) {\
        uint ch = key.at(i).unicode();\
\
        if (ch == '/') {\
            result += '\\';\
        } else if (ch >= 'a' && ch <= 'z' || ch >= 'A' && ch <= 'Z' || ch >= '0' && ch <= '9'\
                || ch == '_' || ch == '-' || ch == '.') {\
            result += (char)ch;\
        } else if (ch <= 0xFF) {\
            result += '%';\
            result += hexDigits[ch / 16];\
            result += hexDigits[ch % 16];\
        } else {\
            result += "%U";\
            QByteArray hexCode;\
            for (int i = 0; i < 4; ++i) {\
                hexCode.prepend(hexDigits[ch % 16]);\
                ch >>= 4;\
            }\
            result += hexCode;\
        }\
    }\
}\
\
static bool iniUnescapedKey(const QByteArray &key, int from, int to, QString &result)\
{\
    bool lowerCaseOnly = true;\
    int i = from;\
    while (i < to) {\
        int ch = (uchar)key.at(i);\
\
        if (ch == '\\') {\
            result += QLatin1Char('/');\
            ++i;\
            continue;\
        }\
\
        if (ch != '%' || i == to - 1) {\
            if (isupper((uchar)ch))\
                lowerCaseOnly = false;\
            result += QLatin1Char(ch);\
            ++i;\
            continue;\
        }\
\
        int numDigits = 2;\
        int firstDigitPos = i + 1;\
\
        ch = key.at(i + 1);\
        if (ch == 'U') {\
            ++firstDigitPos;\
            numDigits = 4;\
        }\
\
        if (firstDigitPos + numDigits > to) {\
            result += QLatin1Char('%');\
            ++i;\
            continue;\
        }\
\
        bool ok;\
        ch = key.mid(firstDigitPos, numDigits).toInt(&ok, 16);\
        if (!ok) {\
            result += QLatin1Char('%');\
            ++i;\
            continue;\
        }\
\
        QChar qch(ch);\
        if (qch.toLower() != qch)\
            lowerCaseOnly = false;\
        result += qch;\
        i = firstDigitPos + numDigits;\
    }\
    return lowerCaseOnly;\
}\
\
static void iniEscapedString(const QString &str, QByteArray &result)\
{\
    bool needsQuotes = false;\
    bool escapeNextIfDigit = false;\
    int i;\
    int startPos = result.size();\
\
    for (i = 0; i < str.size(); ++i) {\
        uint ch = str.at(i).unicode();\
        if (ch == ';' || ch == ',' || ch == '=')\
            needsQuotes = true;\
\
        if (escapeNextIfDigit\
                && ((ch >= '0' && ch <= '9')\
                    || (ch >= 'a' && ch <= 'f')\
                    || (ch >= 'A' && ch <= 'F'))) {\
            result += "\\x";\
            result += QByteArray::number(ch, 16);\
            continue;\
        }\
\
        escapeNextIfDigit = false;\
\
        switch (ch) {\
        case '\0':\
            result += "\\0";\
            break;\
        case '\a':\
            result += "\\a";\
            break;\
        case '\b':\
            result += "\\b";\
            break;\
        case '\f':\
            result += "\\f";\
            break;\
        case '\n':\
            result += "\\n";\
            break;\
        case '\r':\
            result += "\\r";\
            break;\
        case '\t':\
            result += "\\t";\
            break;\
        case '\v':\
            result += "\\v";\
            break;\
        case '"':\
        case '\\':\
            result += '\\';\
            result += (char)ch;\
            break;\
        default:\
            if (ch <= 0x1F || ch >= 0x7F) {\
                result += "\\x";\
                result += QByteArray::number(ch, 16);\
                escapeNextIfDigit = true;\
            } else {\
                result += (char)ch;\
            }\
        }\
    }\
\
    if (needsQuotes\
            || (startPos < result.size() && (result.at(startPos) == ' '\
                                                || result.at(result.size() - 1) == ' '))) {\
        result.insert(startPos, '"');\
        result += '"';\
    }\
}\
\
static void iniChopTrailingSpaces(QString *str)\
{\
    int n = str->size();\
    while (n > 0\
            && (str->at(n - 1) == QLatin1Char(' ') || str->at(n - 1) == QLatin1Char('\t')))\
        str->truncate(--n);\
}\
\
static void iniEscapedStringList(const QStringList &strs, QByteArray &result)\
{\
    for (int i = 0; i < strs.size(); ++i) {\
        if (i != 0)\
            result += ", ";\
        iniEscapedString(strs.at(i), result);\
    }\
}\
\
static QStringList *iniUnescapedStringList(const QByteArray &str, int from, int to,\
                                            QString &result)\
{\
    static const char escapeCodes[][2] =\
    {\
        { 'a', '\a' },\
        { 'b', '\b' },\
        { 'f', '\f' },\
        { 'n', '\n' },\
        { 'r', '\r' },\
        { 't', '\t' },\
        { 'v', '\v' },\
        { '"', '"' },\
        { '?', '?' },\
        { '\'', '\'' },\
        { '\\', '\\' }\
    };\
    static const int numEscapeCodes = sizeof(escapeCodes) / sizeof(escapeCodes[0]);\
\
    QStringList *strList = 0;\
    int i = from;\
\
    enum State { StNormal, StSkipSpaces, StEscape, StHexEscapeFirstChar, StHexEscape,\
                    StOctEscape };\
    State state = StSkipSpaces;\
    int escapeVal = 0;\
    bool inQuotedString = false;\
    bool currentValueIsQuoted = false;\
\
    while (i < to) {\
        char ch = str.at(i);\
\
        switch (state) {\
        case StNormal:\
            switch (ch) {\
            case '\\':\
                state = StEscape;\
                break;\
            case '"':\
                currentValueIsQuoted = true;\
                inQuotedString = !inQuotedString;\
                if (!inQuotedString)\
                    state = StSkipSpaces;\
                break;\
            case ',':\
                if (!inQuotedString) {\
                    if (!currentValueIsQuoted)\
                        iniChopTrailingSpaces(&result);\
                    if (!strList)\
                        strList = new QStringList;\
                    strList->append(result);\
                    result.clear();\
                    currentValueIsQuoted = false;\
                    state = StSkipSpaces;\
                    break;\
                }\
                /* fallthrough */\
            default:\
                result += QLatin1Char(ch);\
            }\
            ++i;\
            break;\
        case StSkipSpaces:\
            if (ch == ' ' || ch == '\t')\
                ++i;\
            else\
                state = StNormal;\
            break;\
        case StEscape:\
            for (int j = 0; j < numEscapeCodes; ++j) {\
                if (ch == escapeCodes[j][0]) {\
                    result += QLatin1Char(escapeCodes[j][1]);\
                    ++i;\
                    state = StNormal;\
                    goto end_of_switch;\
                }\
            }\
\
            if (ch == 'x') {\
                escapeVal = 0;\
                state = StHexEscapeFirstChar;\
            } else if (ch >= '0' && ch <= '7') {\
                escapeVal = ch - '0';\
                state = StOctEscape;\
            } else {\
                state = StNormal;\
            }\
            ++i;\
            break;\
        case StHexEscapeFirstChar:\
            if ((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'F')\
                    || (ch >= 'a' && ch <= 'f'))\
                state = StHexEscape;\
            else\
                state = StNormal;\
            break;\
        case StHexEscape:\
            if (ch >= 'a')\
                ch -= 'a' - 'A';\
            if ((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'F')) {\
                escapeVal <<= 4;\
                escapeVal += strchr(hexDigits, ch) - hexDigits;\
                ++i;\
            } else {\
                result += QChar(escapeVal);\
                state = StNormal;\
            }\
            break;\
        case StOctEscape:\
            if (ch >= '0' && ch <= '7') {\
                escapeVal <<= 3;\
                escapeVal += ch - '0';\
                ++i;\
            } else {\
                result += QChar(escapeVal);\
                state = StNormal;\
            }\
        }\
end_of_switch:\
        ;\
    }\
\
    if (state == StHexEscape || state == StOctEscape)\
        result += QChar(escapeVal);\
    if (!currentValueIsQuoted)\
        iniChopTrailingSpaces(&result);\
    if (strList)\
        strList->append(result);\
    return strList;\
}\

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
            result = variantToStringCoreImpl(v);\
            break;\
    }\
\
    return result;\
}\
\
static QStringList splitArgs(const QString &s, int idx)\
{\
    int l = s.length();\
    Q_ASSERT(l > 0);\
    Q_ASSERT(s.at(idx) == '(');\
    Q_ASSERT(s.at(l - 1) == ')');\
\
    QStringList result;\
    QString item;\
\
    for (++idx; idx < l; ++idx) {\
        QChar c = s.at(idx);\
        if (c == QLatin1Char(')')) {\
            Q_ASSERT(idx == l - 1);\
            result.append(item);\
        } else if (c == QLatin1Char(',')) {\
            result.append(item);\
            item.clear();\
        } else {\
            item.append(c);\
        }\
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
            QStringList args = splitArgs(s, 5);\
            if (args.size() == 4) {\
                return QVariant(QRect(args[0].toInt(), args[1].toInt(),\
                                        args[2].toInt(), args[3].toInt()));\
            }\
        } else if (s.startsWith(QLatin1String("@Size("))) {\
            QStringList args = splitArgs(s, 5);\
            if (args.size() == 2) {\
                return QVariant(QSize(args[0].toInt(), args[1].toInt()));\
            }\
        } else if (s.startsWith(QLatin1String("@Color("))) {\
            QStringList args = splitArgs(s, 6);\
            if (args.size() == 3) {\
                return QVariant(QColor(args[0].toInt(), args[1].toInt(),\
                                        args[2].toInt()));\
            }\
        } else if (s.startsWith(QLatin1String("@Point("))) {\
            QStringList args = splitArgs(s, 6);\
            if (args.size() == 2) {\
                return QVariant(QPoint(args[0].toInt(), args[1].toInt()));\
            }\
        }\
    }\
\
    return stringToVariantCoreImpl(s);\
}



#endif // QCORESETTINGS_P_H
