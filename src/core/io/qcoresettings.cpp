#include "qplatformdefs.h"

#include "qcoresettings.h"

#ifndef QT_NO_SETTINGS

#include "qcache.h"
#include "qcoreapplication.h"
#include "private/qcoresettings_p.h"
#include "qfile.h"
#include "qdir.h"
#include "qfileinfo.h"
#include "qmutex.h"

// ************************************************************************
// QConfFile

/*
    QConfFile objects are explicitly shared within the application.
    This ensures that modification to the settings done through one
    QCoreSettings or QSettings object are immediately reflected in
    other setting objects of the same application.
*/

typedef QHash<QString, QConfFile *> ConfFileHash;
typedef QCache<QString, QConfFile> ConfFileCache;

Q_GLOBAL_STATIC(ConfFileHash, usedHashFunc)
Q_GLOBAL_STATIC(ConfFileCache, unusedCacheFunc)

Q_GLOBAL_STATIC(QMutex, mutex)

static ConfFileHash *usedHash;
static ConfFileCache *unusedCache;

QConfFile::QConfFile(const QString &fileName)
    : name(fileName), size(0)
{
    ref = 1;
    usedHash->insert(name, this);
#ifdef Q_OS_WIN
    semHandle = 0;
#endif
}

bool QConfFile::mergeKeyMaps()
{
    if (addedKeys.isEmpty() && removedKeys.isEmpty())
        return false;

    SettingsKeyMap::const_iterator i;

    for (i = removedKeys.begin(); i != removedKeys.end(); ++i)
        originalKeys.remove(i.key());
    removedKeys.clear();

    for (i = addedKeys.begin(); i != addedKeys.end(); ++i)
        originalKeys.insert(i.key(), i.value());
    addedKeys.clear();

    return true;
}

QConfFile *QConfFile::fromName(const QString &fileName)
{
    QConfFile *confFile;
    QString absPath = QFileInfo(fileName).absoluteFilePath();

    usedHash = usedHashFunc();
    unusedCache = unusedCacheFunc();

    QMutexLocker locker(mutex());

    if (!(confFile = usedHash->value(absPath))) {
        if ((confFile = unusedCache->take(absPath)))
            usedHash->insert(absPath, confFile);
    }
    if (confFile) {
        ++confFile->ref;
        return confFile;
    }
    return new QConfFile(absPath);
}

void QConfFile::clearCache()
{
    unusedCache = unusedCacheFunc();

    QMutexLocker locker(mutex());
    unusedCache->clear();
}

// ************************************************************************
// QCoreSettingsPrivate

QCoreSettingsPrivate::QCoreSettingsPrivate()
    : spec(0), fallbacks(true), pendingChanges(false), status(QCoreSettings::NoError)
{
}

QCoreSettingsPrivate::~QCoreSettingsPrivate()
{
}

QString QCoreSettingsPrivate::fileName() const
{
    return QString();
}

QString QCoreSettingsPrivate::actualKey(const QString &key) const
{
    QString n = normalizedKey(key);
    Q_ASSERT_X(!n.isEmpty(), "QCoreSettings", "empty key");
    n.prepend(groupPrefix);
    return n;
}

/*
    Returns a string that never starts nor ends with a slash (or an
    empty string). Examples:

            "foo"            becomes   "foo"
            "/foo//bar///"   becomes   "foo/bar"
            "///"            becomes   ""

    This function is optimized to avoid a QString deep copy in the
    common case where the key is already normalized.
*/
QString QCoreSettingsPrivate::normalizedKey(const QString &key)
{
    QString result = key;

    int i = 0;
    while (i < result.size()) {
        while (result.at(i) == QLatin1Char('/')) {
            result.remove(i, 1);
            if (i == result.size())
                goto after_loop;
        }
        while (result.at(i) != QLatin1Char('/')) {
            ++i;
            if (i == result.size())
                return result;
        }
        ++i; // leave the slash alone
    }

after_loop:
    if (!result.isEmpty())
        result.truncate(i - 1); // remove the trailing slash
    return result;
}

void QCoreSettingsPrivate::setStreamingFunctions(VariantToStringFunc vts,
                                                    StringToVariantFunc stv)
{
    variantToString = vts;
    stringToVariant = stv;
}

// see also qcoresettings_win.cpp and qcoresettings_mac.cpp

#if !defined(Q_OS_WIN) && !defined(Q_OS_MAC)
QCoreSettingsPrivate *QCoreSettingsPrivate::create(Qt::SettingsFormat format,
                                                   Qt::SettingsScope scope,
                                                   const QString &organization,
                                                   const QString &application,
                                                   VariantToStringFunc vts,
                                                   StringToVariantFunc stv)
{
    QConfFileSettingsPrivate *p = new QConfFileSettingsPrivate(format, scope,
                                                               organization, application);
    p->setStreamingFunctions(vts, stv);
    p->init();
    return p;
}
#endif

#if !defined(Q_OS_WIN)
QCoreSettingsPrivate *QCoreSettingsPrivate::create(const QString &fileName,
                                                   Qt::SettingsFormat format,
                                                   VariantToStringFunc vts,
                                                   StringToVariantFunc stv)
{
    QConfFileSettingsPrivate *p = new QConfFileSettingsPrivate(fileName, format);
    p->setStreamingFunctions(vts, stv);
    p->init();
    return p;
}
#endif

void QCoreSettingsPrivate::processChild(QString key, ChildSpec spec, QMap<QString, QString> &result)
{
    if (spec != AllKeys) {
        int slashPos = key.indexOf(QLatin1Char('/'));
        if (slashPos == -1) {
            if (spec != ChildKeys)
                return;
        } else {
            if (spec != ChildGroups)
                return;
            key.truncate(slashPos);
        }
    }
    result.insert(key, QString());
}

void QCoreSettingsPrivate::beginGroupOrArray(const QSettingsGroup &group)
{
    groupStack.push(group);
    if (!group.name().isEmpty()) {
        groupPrefix += group.name();
        groupPrefix += QLatin1Char('/');
    }
}

/*
    We only set an error if there isn't one set already. This way the user always gets the
    first error that occurred. We always allow clearing errors.
*/

void QCoreSettingsPrivate::setStatus(QCoreSettings::Status status)
{
    if (status == QCoreSettings::NoError || this->status == QCoreSettings::NoError)
        this->status = status;
}

void QCoreSettingsPrivate::update()
{
    Q_Q(QCoreSettings);

    q->sync();
    pendingChanges = false;
}

void QCoreSettingsPrivate::requestUpdate()
{
    Q_Q(QCoreSettings);

    if (!pendingChanges) {
        pendingChanges = true;
#ifndef QT_NO_QOBJECT
        QCoreApplication::postEvent(q, new QEvent(QEvent::UpdateRequest));
#else
        update();
#endif
    }
}

QStringList QCoreSettingsPrivate::variantListToStringList(const QCoreVariantList &l) const
{
    QStringList result;
    QCoreVariantList::const_iterator it = l.constBegin();
    for (; it != l.constEnd(); ++it)
        result.append(variantToString(*it));
    return result;
}

QCoreVariantList QCoreSettingsPrivate::stringListToVariantList(const QStringList &l) const
{
    QCoreVariantList result;
    QStringList::const_iterator it = l.constBegin();
    for (; it != l.constEnd(); ++it)
        result.append(stringToVariant(*it));
    return result;
}

QString &QCoreSettingsPrivate::escapedLeadingAt(QString &s)
{
    if (s.length() > 0 && s.at(0) == QLatin1Char('@'))
        s.prepend(QLatin1Char('@'));
    return s;
}

QString &QCoreSettingsPrivate::unescapedLeadingAt(QString &s)
{
    if (s.length() >= 2
        && s.at(0) == QLatin1Char('@')
        && s.at(1) == QLatin1Char('@'))
        s.remove(0, 1);
    return s;
}

QString QCoreSettingsPrivate::variantToStringCoreImpl(const QCoreVariant &v)
{
    QString result;

    switch (v.type()) {
        case QCoreVariant::Invalid:
            result = QLatin1String("@Invalid()");
            break;

        case QCoreVariant::ByteArray: {
            QByteArray a = v.toByteArray();
            result = QLatin1String("@ByteArray(");
            result += QString::fromLatin1(a.constData(), a.size());
            result += QLatin1Char(')');
            break;
        }

        case QCoreVariant::String:
        case QCoreVariant::LongLong:
        case QCoreVariant::ULongLong:
        case QCoreVariant::Int:
        case QCoreVariant::UInt:
        case QCoreVariant::Bool:
        case QCoreVariant::Double: {
            result = v.toString();
            result = escapedLeadingAt(result);
            break;
        }

        default: {
            QByteArray a;
            {
                QDataStream s(&a, QIODevice::WriteOnly);
                s << v;
            }

            result = QLatin1String("@Variant(");
            result += QString::fromLatin1(a.constData(), a.size());
            result += QLatin1Char(')');
            break;
        }
    }

    return result;
}

QCoreVariant QCoreSettingsPrivate::stringToVariantCoreImpl(const QString &s)
{
    if (s.length() > 3
            && s.at(0) == QLatin1Char('@')
            && s.at(s.length() - 1) == QLatin1Char(')')) {

        if (s.startsWith(QLatin1String("@ByteArray("))) {
            return QCoreVariant(QByteArray(s.latin1() + 11, s.length() - 12));
        } else if (s.startsWith(QLatin1String("@Variant("))) {
            QByteArray a(s.latin1() + 9, s.length() - 10);
            QDataStream stream(&a, QIODevice::ReadOnly);
            QCoreVariant result;
            stream >> result;
            return result;
        } else if (s == QLatin1String("@Invalid()")) {
            return QCoreVariant();
        }
    }

    QString tmp = s;
    return QCoreVariant(unescapedLeadingAt(tmp));
}

static const char hexDigits[] = "0123456789ABCDEF";

void QCoreSettingsPrivate::iniEscapedKey(const QString &key, QByteArray &result)
{
    for (int i = 0; i < key.size(); ++i) {
        uint ch = key.at(i).unicode();

        if (ch == '/') {
            result += '\\';
        } else if (ch >= 'a' && ch <= 'z' || ch >= 'A' && ch <= 'Z' || ch >= '0' && ch <= '9'
                || ch == '_' || ch == '-' || ch == '.') {
            result += (char)ch;
        } else if (ch <= 0xFF) {
            result += '%';
            result += hexDigits[ch / 16];
            result += hexDigits[ch % 16];
        } else {
            result += "%U";
            QByteArray hexCode;
            for (int i = 0; i < 4; ++i) {
                hexCode.prepend(hexDigits[ch % 16]);
                ch >>= 4;
            }
            result += hexCode;
        }
    }
}

bool QCoreSettingsPrivate::iniUnescapedKey(const QByteArray &key, int from, int to,
                                            QString &result)
{
    bool lowerCaseOnly = true;
    int i = from;
    while (i < to) {
        int ch = (uchar)key.at(i);

        if (ch == '\\') {
            result += QLatin1Char('/');
            ++i;
            continue;
        }

        if (ch != '%' || i == to - 1) {
            if (isupper((uchar)ch))
                lowerCaseOnly = false;
            result += QLatin1Char(ch);
            ++i;
            continue;
        }

        int numDigits = 2;
        int firstDigitPos = i + 1;

        ch = key.at(i + 1);
        if (ch == 'U') {
            ++firstDigitPos;
            numDigits = 4;
        }

        if (firstDigitPos + numDigits > to) {
            result += QLatin1Char('%');
            ++i;
            continue;
        }

        bool ok;
        ch = key.mid(firstDigitPos, numDigits).toInt(&ok, 16);
        if (!ok) {
            result += QLatin1Char('%');
            ++i;
            continue;
        }

        QChar qch(ch);
        if (qch.toLower() != qch)
            lowerCaseOnly = false;
        result += qch;
        i = firstDigitPos + numDigits;
    }
    return lowerCaseOnly;
}

void QCoreSettingsPrivate::iniEscapedString(const QString &str, QByteArray &result)
{
    bool needsQuotes = false;
    bool escapeNextIfDigit = false;
    int i;
    int startPos = result.size();

    for (i = 0; i < str.size(); ++i) {
        uint ch = str.at(i).unicode();
        if (ch == ';' || ch == ',' || ch == '=')
            needsQuotes = true;

        if (escapeNextIfDigit
                && ((ch >= '0' && ch <= '9')
                    || (ch >= 'a' && ch <= 'f')
                    || (ch >= 'A' && ch <= 'F'))) {
            result += "\\x";
            result += QByteArray::number(ch, 16);
            continue;
        }

        escapeNextIfDigit = false;

        switch (ch) {
        case '\0':
            result += "\\0";
            break;
        case '\a':
            result += "\\a";
            break;
        case '\b':
            result += "\\b";
            break;
        case '\f':
            result += "\\f";
            break;
        case '\n':
            result += "\\n";
            break;
        case '\r':
            result += "\\r";
            break;
        case '\t':
            result += "\\t";
            break;
        case '\v':
            result += "\\v";
            break;
        case '"':
        case '\\':
            result += '\\';
            result += (char)ch;
            break;
        default:
            if (ch <= 0x1F || ch >= 0x7F) {
                result += "\\x";
                result += QByteArray::number(ch, 16);
                escapeNextIfDigit = true;
            } else {
                result += (char)ch;
            }
        }
    }

    if (needsQuotes
            || (startPos < result.size() && (result.at(startPos) == ' '
                                                || result.at(result.size() - 1) == ' '))) {
        result.insert(startPos, '"');
        result += '"';
    }
}

void QCoreSettingsPrivate::iniChopTrailingSpaces(QString *str)
{
    int n = str->size();
    while (n > 0
            && (str->at(n - 1) == QLatin1Char(' ') || str->at(n - 1) == QLatin1Char('\t')))
        str->truncate(--n);
}

void QCoreSettingsPrivate::iniEscapedStringList(const QStringList &strs, QByteArray &result)
{
    for (int i = 0; i < strs.size(); ++i) {
        if (i != 0)
            result += ", ";
        iniEscapedString(strs.at(i), result);
    }
}

QStringList *QCoreSettingsPrivate::iniUnescapedStringList(const QByteArray &str, int from,
                                                            int to, QString &result)
{
    static const char escapeCodes[][2] =
    {
        { 'a', '\a' },
        { 'b', '\b' },
        { 'f', '\f' },
        { 'n', '\n' },
        { 'r', '\r' },
        { 't', '\t' },
        { 'v', '\v' },
        { '"', '"' },
        { '?', '?' },
        { '\'', '\'' },
        { '\\', '\\' }
    };
    static const int numEscapeCodes = sizeof(escapeCodes) / sizeof(escapeCodes[0]);

    QStringList *strList = 0;
    int i = from;

    enum State { StNormal, StSkipSpaces, StEscape, StHexEscapeFirstChar, StHexEscape,
                    StOctEscape };
    State state = StSkipSpaces;
    int escapeVal = 0;
    bool inQuotedString = false;
    bool currentValueIsQuoted = false;

    while (i < to) {
        char ch = str.at(i);

        switch (state) {
        case StNormal:
            switch (ch) {
            case '\\':
                state = StEscape;
                break;
            case '"':
                currentValueIsQuoted = true;
                inQuotedString = !inQuotedString;
                if (!inQuotedString)
                    state = StSkipSpaces;
                break;
            case ',':
                if (!inQuotedString) {
                    if (!currentValueIsQuoted)
                        iniChopTrailingSpaces(&result);
                    if (!strList)
                        strList = new QStringList;
                    strList->append(result);
                    result.clear();
                    currentValueIsQuoted = false;
                    state = StSkipSpaces;
                    break;
                }
                /* fallthrough */
            default:
                result += QLatin1Char(ch);
            }
            ++i;
            break;
        case StSkipSpaces:
            if (ch == ' ' || ch == '\t')
                ++i;
            else
                state = StNormal;
            break;
        case StEscape:
            for (int j = 0; j < numEscapeCodes; ++j) {
                if (ch == escapeCodes[j][0]) {
                    result += QLatin1Char(escapeCodes[j][1]);
                    ++i;
                    state = StNormal;
                    goto end_of_switch;
                }
            }

            if (ch == 'x') {
                escapeVal = 0;
                state = StHexEscapeFirstChar;
            } else if (ch >= '0' && ch <= '7') {
                escapeVal = ch - '0';
                state = StOctEscape;
            } else {
                state = StNormal;
            }
            ++i;
            break;
        case StHexEscapeFirstChar:
            if ((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'F')
                    || (ch >= 'a' && ch <= 'f'))
                state = StHexEscape;
            else
                state = StNormal;
            break;
        case StHexEscape:
            if (ch >= 'a')
                ch -= 'a' - 'A';
            if ((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'F')) {
                escapeVal <<= 4;
                escapeVal += strchr(hexDigits, ch) - hexDigits;
                ++i;
            } else {
                result += QChar(escapeVal);
                state = StNormal;
            }
            break;
        case StOctEscape:
            if (ch >= '0' && ch <= '7') {
                escapeVal <<= 3;
                escapeVal += ch - '0';
                ++i;
            } else {
                result += QChar(escapeVal);
                state = StNormal;
            }
        }
end_of_switch:
        ;
    }

    if (state == StHexEscape || state == StOctEscape)
        result += QChar(escapeVal);
    if (!currentValueIsQuoted)
        iniChopTrailingSpaces(&result);
    if (strList)
        strList->append(result);
    return strList;
}

QStringList QCoreSettingsPrivate::splitArgs(const QString &s, int idx)
{
    int l = s.length();
    Q_ASSERT(l > 0);
    Q_ASSERT(s.at(idx) == '(');
    Q_ASSERT(s.at(l - 1) == ')');

    QStringList result;
    QString item;

    for (++idx; idx < l; ++idx) {
        QChar c = s.at(idx);
        if (c == QLatin1Char(')')) {
            Q_ASSERT(idx == l - 1);
            result.append(item);
        } else if (c == QLatin1Char(',')) {
            result.append(item);
            item.clear();
        } else {
            item.append(c);
        }
    }

    return result;
}

// ************************************************************************
// QConfFileSettingsPrivate

static void checkAccess(const QString &name, bool *read, bool *write)
{
    /*
        The best way to check that a file is writable is to open it for
        writing. This will also work if the file doesn't already exist
        but can be created.
    */
    QDir dir;
    if (QDir::isRelativePath(name))
        dir = QDir::current();
    else
        dir = QDir::root();

#if 1
    // Create the directories
    // ### bloat + slow down; we need something simpler
    QStringList pathElements = name.split(QLatin1Char('/'), QString::SkipEmptyParts);
    for (int i = 0; i < pathElements.size() - 1; ++i) {
        QString elt = pathElements[i];
        if (dir.cd(elt))
            continue;

        if (!dir.mkdir(elt) || !dir.cd(elt))
            break;
    }
#endif

    // check if we can open or create the file
    QFile file(name);
    *write = file.open(QIODevice::Append);

    // check if we can read from the file
    file.close();
    *read = file.open(QIODevice::ReadOnly);
}

void QConfFileSettingsPrivate::init()
{
    if (confFiles[spec] == 0) {
        readAccess = false;
        writeAccess = false;
    } else {
        checkAccess(confFiles[spec]->name, &readAccess, &writeAccess);
    }
    if (!readAccess)
        setStatus(QCoreSettings::AccessError);
    cs = Qt::CaseInsensitive;
    sync();       // loads the files the first time
}

static QString systemIniPath()
{
    return QLatin1String(qInstallPathSysconf()) + QDir::separator();
}

static QString userIniPath()
{
    return QDir::homePath() + QDir::separator() + QLatin1String(".settings") + QDir::separator();
}

QConfFileSettingsPrivate::QConfFileSettingsPrivate(Qt::SettingsFormat format,
                                                   Qt::SettingsScope scope,
                                                   const QString &organization,
                                                   const QString &application)
{
    int i;
    this->format = format;
    for (i = 0; i < NumConfFiles; ++i)
        confFiles[i] = 0;
    cs = Qt::CaseSensitive;

    QString org = organization;
    if (org.isEmpty()) {
        setStatus(QCoreSettings::AccessError);
        org = QLatin1String("unknown-organization.trolltech.com");
    }

    const char *extension = format == Qt::IniFormat ? ".ini" : ".conf";

    QString appFile = org + QDir::separator() + application + QLatin1String(extension);
    QString orgFile = org + QLatin1String(extension);

    if (scope == Qt::UserScope) {
        if (!application.isEmpty())
            confFiles[F_User | F_Application] = QConfFile::fromName(userIniPath() + appFile);
        confFiles[F_User | F_Organization] = QConfFile::fromName(userIniPath() + orgFile);
    }
    if (!application.isEmpty())
        confFiles[F_Global | F_Application] = QConfFile::fromName(systemIniPath() + appFile);
    confFiles[F_Global | F_Organization] = QConfFile::fromName(systemIniPath() + orgFile);

    for (i = 0; i < NumConfFiles; ++i) {
        if (confFiles[i]) {
            spec = i;
            break;
        }
    }
}

QConfFileSettingsPrivate::QConfFileSettingsPrivate(const QString &fileName,
                                                   Qt::SettingsFormat format)
{
    confFiles[0] = QConfFile::fromName(fileName);
    for (int i = 1; i < NumConfFiles; ++i)
        confFiles[i] = 0;
    this->format = format;
    readAccess = writeAccess = false;
    cs = Qt::CaseSensitive;
}

QConfFileSettingsPrivate::~QConfFileSettingsPrivate()
{
    QMutexLocker locker(mutex());

    for (int i = 0; i < NumConfFiles; ++i) {
        if (confFiles[i] && !--confFiles[i]->ref) {
            usedHash->remove(confFiles[i]->name);

            if (confFiles[i]->size == 0) {
                delete confFiles[i];
            } else {
                unusedCache->insert(confFiles[i]->name, confFiles[i],
                                      10 + (confFiles[i]->originalKeys.size() / 4));
            }
        }
    }
}

void QConfFileSettingsPrivate::remove(const QString &key)
{
    if (!writeAccess) {
        setStatus(QCoreSettings::AccessError);
        return;
    }

    QSettingsKey theKey(key, cs);
    QSettingsKey prefix(key + QLatin1Char('/'), cs);

    QConfFile *confFile = confFiles[spec];
    QMutexLocker locker(mutex());

    SettingsKeyMap::iterator i = confFile->addedKeys.lowerBound(prefix);
    while (i != confFile->addedKeys.end() && i.key().startsWith(prefix))
        i = confFile->addedKeys.erase(i);
    confFile->addedKeys.remove(theKey);

    SettingsKeyMap::const_iterator j = confFile->originalKeys.lowerBound(prefix);
    while (j != confFile->originalKeys.constEnd() && j.key().startsWith(prefix)) {
        confFile->removedKeys.insert(j.key(), QCoreVariant());
        ++j;
    }
    if (confFile->originalKeys.contains(theKey))
        confFile->removedKeys.insert(theKey, QCoreVariant());
}

void QConfFileSettingsPrivate::set(const QString &key, const QCoreVariant &value)
{
    if (!writeAccess) {
        setStatus(QCoreSettings::AccessError);
        return;
    }

    QSettingsKey theKey(key, cs);

    QConfFile *confFile = confFiles[spec];
    QMutexLocker locker(mutex());
    confFile->removedKeys.remove(theKey);
    confFile->addedKeys.insert(theKey, value);
}

bool QConfFileSettingsPrivate::get(const QString &key, QCoreVariant *value) const
{
    QSettingsKey theKey(key, cs);
    SettingsKeyMap::const_iterator j;
    bool found = false;

    QMutexLocker locker(mutex());

    for (int i = 0; i < NumConfFiles; ++i) {
        if (QConfFile *confFile = confFiles[i]) {
            if (!confFile->addedKeys.isEmpty()) {
                j = confFile->addedKeys.find(theKey);
                found = (j != confFile->addedKeys.constEnd());
            }
            if (!found) {
                j = confFile->originalKeys.find(theKey);
                found = (j != confFile->originalKeys.constEnd()
                         && !confFile->removedKeys.contains(theKey));
            }

            if (found && value)
                *value = *j;

            if (found)
                return true;
            if (!fallbacks)
                break;
        }
    }
    return false;
}

QStringList QConfFileSettingsPrivate::children(const QString &prefix, ChildSpec spec) const
{
    QMap<QString, QString> result;
    SettingsKeyMap::const_iterator j;

    QSettingsKey thePrefix(prefix, cs);
    int startPos = prefix.size();

    QMutexLocker locker(mutex());

    for (int i = 0; i < NumConfFiles; ++i) {
        if (QConfFile *confFile = confFiles[i]) {
            j = confFile->originalKeys.lowerBound(thePrefix);
            while (j != confFile->originalKeys.constEnd() && j.key().startsWith(thePrefix)) {
                if (!confFile->removedKeys.contains(j.key()))
                    processChild(j.key().realKey().mid(startPos), spec, result);
                ++j;
            }

            j = confFile->addedKeys.lowerBound(thePrefix);
            while (j != confFile->addedKeys.constEnd() && j.key().startsWith(thePrefix)) {
                processChild(j.key().realKey().mid(startPos), spec, result);
                ++j;
            }

            if (!fallbacks)
                break;
        }
    }
    return result.keys();
}

void QConfFileSettingsPrivate::clear()
{
    if (!writeAccess) {
        setStatus(QCoreSettings::AccessError);
        return;
    }

    QConfFile *confFile = confFiles[spec];
    QMutexLocker locker(mutex());
    confFile->addedKeys.clear();
    confFile->removedKeys = confFiles[spec]->originalKeys;
}

void QConfFileSettingsPrivate::sync()
{
    QMutexLocker locker(mutex());

    // people probably won't be checking the status a whole lot, so in case of
    // error we just try to go on and make the best of it

    for (int i = 0; i < NumConfFiles; ++i) {
        QConfFile *confFile = confFiles[i];
        if (confFile) {
            if (!readFile(confFile)) {
                // Only problems with the file we actually write to change the status. The
                // other files are just optional fallbacks.
                if (i == spec)
                    setStatus(QCoreSettings::FormatError);
            }
            if (i == spec && confFile->mergeKeyMaps()) {
                if (!writeFile(confFile))
                    setStatus(QCoreSettings::AccessError);
            }
        }
    }
}

QString QConfFileSettingsPrivate::fileName() const
{
    QConfFile *confFile = confFiles[spec];
    if (confFile == 0)
        return QString();
    return confFile->name;
}

bool QConfFileSettingsPrivate::isWritable() const
{
    return writeAccess;
}

/*
    The following openFile() and closeFile() functions lock the file
    (using fcntl() on Unix and a global mutex on Windows), ensuring
    that if two instances of the same applications access the file at
    the same time, the file isn't corrupted.
*/

#ifdef Q_OS_UNIX
const int ReadFlags = O_RDONLY | O_CREAT;
const int WriteFlags = O_WRONLY | O_CREAT | O_APPEND;
#else
static const int FileLockSemMax = 50;
static const int ReadFlags = 1;
static const int WriteFlags = 2;
const char SemNamePrefix[] = "QSettings semaphore ";
#endif


static bool openFile(QFile &file, QConfFile &confFile, int flags)
{
#ifdef Q_OS_UNIX
    Q_UNUSED(confFile);
    int fd = QT_OPEN(QFile::encodeName(file.fileName()), flags, S_IRUSR | S_IWUSR);
    if (fd < 0)
        return false;

    struct flock fl;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;
    fl.l_type = (flags == WriteFlags) ? F_WRLCK : F_RDLCK;
    fcntl(fd, F_SETLKW, &fl);

    if (flags == WriteFlags)
        ftruncate(fd, 0);

    return file.open(flags == WriteFlags ? QIODevice::WriteOnly | QIODevice::Translate
                     : QIODevice::DeviceMode(QIODevice::ReadOnly),
                     fd);

#else
    // on windows we use a named semaphore
    if (confFile.semHandle == 0) {
        QString semName = QString::fromAscii(SemNamePrefix);
        semName.append(file.fileName());
        QT_WA( {
            confFile.semHandle = CreateSemaphoreW(0, FileLockSemMax, FileLockSemMax, semName.utf16());
        } , {
            confFile.semHandle = CreateSemaphoreA(0, FileLockSemMax, FileLockSemMax, semName.local8Bit());
        } );
    }

    if (confFile.semHandle != 0) {
        int decrement;
        if (flags == ReadFlags)
            decrement = 1;
        else
            decrement = FileLockSemMax;

        for (int i = 0; i < decrement; ++i)
            WaitForSingleObject(confFile.semHandle, INFINITE);
    }

    return file.open(flags == WriteFlags ? QIODevice::WriteOnly | QIODevice::Translate
                                         : QIODevice::ReadOnly);
#endif
}

static void closeFile(QFile &file, QConfFile &confFile)
{
#ifdef Q_OS_UNIX
    Q_UNUSED(confFile);
    int fd = file.handle();
    file.close();
    close(fd);
#else
    int increment;
    if (file.mode() & QIODevice::ReadOnly)
        increment = 1;
    else
        increment = FileLockSemMax;

    ReleaseSemaphore(confFile.semHandle, increment, 0);
    CloseHandle(confFile.semHandle);
    confFile.semHandle = 0;

    file.close();
#endif
}

/*
    This only returns false on format errors. Files which don't exist, or which
    we don't have read permission for, are treated as empty sets of keys.
*/
bool QConfFileSettingsPrivate::readFile(QConfFile *confFile)
{
    QFileInfo fileInfo(confFile->name);
    int actualSize = fileInfo.size();
    QDateTime actualTimeStamp = fileInfo.lastModified();

    if (confFile->size == actualSize) {
        // no need to reload the file if the timestamps and file sizes match
        if (confFile->timeStamp == actualTimeStamp)
            return true;
    }

    SettingsKeyMap newKeys;

    bool ok = true; // we treat unexisting/unreadable files the same as empty files

    if (actualSize != 0) {
#ifdef Q_OS_MAC
        if (format == Qt::NativeFormat) {
            ok = readPlistFile(confFile->name, &newKeys);
        } else
#endif
        {
            QFile file(confFile->name);
            if (openFile(file, *confFile, ReadFlags)) {
                ok = readIniFile(file, &newKeys);
                closeFile(file, *confFile);
            }
        }
    }

    confFile->originalKeys = newKeys;
    confFile->size = actualSize;
    confFile->timeStamp = actualTimeStamp;

    return ok;
}

bool QConfFileSettingsPrivate::writeFile(QConfFile *confFile)
{
    bool ok = false;

#ifdef Q_OS_MAC
    if (format == Qt::NativeFormat) {
        ok = writePlistFile(confFile->name, confFile->originalKeys);
    } else
#endif
    {
        QFile file(confFile->name);
        if (openFile(file, *confFile, WriteFlags)) {
            ok = writeIniFile(file, confFile->originalKeys);
            closeFile(file, *confFile);
        } else {
            return false;
        }
    }

    QFileInfo fileInfo(confFile->name);
    confFile->size = fileInfo.size();
    confFile->timeStamp = fileInfo.lastModified();

    return ok;
}

bool QConfFileSettingsPrivate::readIniLine(QIODevice &device, QByteArray &line, int &len,
                                           int &equalsCharPos)
{
#define MAYBE_GROW() \
    if (pos + 4 > line.size()) { \
        line.resize(pos << 1); \
        data = line.data(); \
    }

    char *data = line.data();
    char ch, ch2;
    int pos = 0;

    equalsCharPos = -1;

    while (device.getChar(&ch)) {
    process_ch:
        MAYBE_GROW();

        switch (ch) {
        case '"':
            data[pos++] = '"';
            while (!device.getChar(&ch) || ch != '"') {
                MAYBE_GROW();

                if (ch == -1)
                    goto end;

                if (ch == '\\') {
                    data[pos++] = '\\';
                    if (!device.getChar(&ch))
                        goto end;
                }
                data[pos++] = ch;
            }
            data[pos++] = '"';
            break;
        case ' ':
        case '\t':
            if (pos > 0)
                data[pos++] = ch;
            break;
        case '\n':
        case '\r':
        process_newline:
#if 0
            /*
                According to the specs, a line ends with CF, LF,
                CR+LF, or LF+CR. In practice, this is irrelevant and
                the ungetch() call is expensive, so let's not do it.
            */
            if (!device.getChar(&ch2))
                goto end;
            if ((ch2 != '\n' && ch2 != '\r') || ch == ch2)
                device.ungetChar(ch2);
#endif
            if (pos > 0)
                goto end;
            break;
        case '\\':
            if (!device.getChar(&ch))
                goto end;

            if (ch == '\n' || ch == '\r') {
                if (!device.getChar(&ch2)) {
                    if ((ch2 != '\n' && ch2 != '\r') || ch == ch2) {
                        ch = ch2;
                        goto process_ch;
                    }
                }
            } else {
                data[pos++] = '\\';
                data[pos++] = ch;
            }
            break;
        case ';':
            while (device.getChar(&ch)) {
                if (ch == '\n' || ch == '\r')
                    goto process_newline;
            }
            break;
        case '=':
            if (equalsCharPos == -1) {
                while (pos > 0 && (ch = data[pos - 1]) == ' ' || ch == '\t')
                    --pos;
                equalsCharPos = pos;
            }
            data[pos++] = '=';
            break;
        default:
            data[pos++] = ch;
        }
    }

end:
    data[pos] = '\0';
    len = pos;
    return pos > 0;
}

/*
    Returns false parse error. However, as many keys are read as possible, so if
    the user doesn't check the status he will get the most out of the file anyway.
*/
bool QConfFileSettingsPrivate::readIniFile(QIODevice &device, SettingsKeyMap *map)
{
    QString currentSection;
    bool currentSectionIsLowerCase = true;
    QByteArray line;
    line.resize(512);
    int equalsCharPos;
    int len;

    bool ok = true;

    while (readIniLine(device, line, len, equalsCharPos)) {
        if (line.at(0) == '[') {
            // this is a section
            QByteArray iniSection;
            int idx = line.indexOf(']');
            if (idx == -1) {
                ok = false;
                iniSection = line.mid(1);
            } else {
                iniSection = line.mid(1, idx - 1);
            }

            iniSection = iniSection.trimmed();

            if (qstricmp(iniSection, "general") == 0) {
                currentSection.clear();
            } else if (qstricmp(iniSection, "%general") == 0) {
                currentSection = QLatin1String("general");
                currentSection += QLatin1Char('/');
            } else {
                currentSection.clear();
                currentSectionIsLowerCase
                    = iniUnescapedKey(iniSection, 0, iniSection.size(),
                                                            currentSection);
                currentSection += QLatin1Char('/');
            }
        } else {
            if (equalsCharPos < 1) {
                ok = false;
                continue;
            }

            QString key = currentSection;
            bool keyIsLowerCase
                    = (iniUnescapedKey(line, 0, equalsCharPos, key)
                                   && currentSectionIsLowerCase);

            QString strValue;
            strValue.reserve(len - equalsCharPos);
            QStringList *strListValue
                    = iniUnescapedStringList(line, equalsCharPos + 1, len,
                                                               strValue);
            QCoreVariant variant;
            if (strListValue) {
                variant = stringListToVariantList(*strListValue);
                delete strListValue;
            } else {
                variant = stringToVariant(strValue);
            }

            /*
                We try to avoid the expensive toLower() call in
                QSettingsKey by passing Qt::CaseSensitive when the
                key is already in lower-case.
            */
            map->insert(QSettingsKey(key, keyIsLowerCase ? Qt::CaseSensitive : Qt::CaseInsensitive),
                        variant);
        }
    }

    return ok;
}

bool QConfFileSettingsPrivate::writeIniFile(QIODevice &device, const SettingsKeyMap &map)
{
    typedef QMap<QString, QCoreVariantMap> IniMap;
    IniMap iniMap;
    IniMap::const_iterator i;

    for (SettingsKeyMap::const_iterator j = map.constBegin(); j != map.constEnd(); ++j) {
        QString section;
        QString key = j.key().realKey();
        int slashPos;

        if ((slashPos = key.indexOf(QLatin1Char('/'))) != -1) {
            section = key.left(slashPos);
            key.remove(0, slashPos + 1);
        }
        iniMap[section][key] = j.value();
    }

    bool writeError = false;
    for (i = iniMap.constBegin(); !writeError && i != iniMap.constEnd(); ++i) {
        QByteArray realSection;

        iniEscapedKey(i.key(), realSection);

        if (realSection.isEmpty()) {
            realSection = "[General]";
        } else if (qstricmp(realSection, "general") == 0) {
            realSection = "[%General]";
        } else {
            realSection.prepend('[');
            realSection.append(']');
        }

        if (i != iniMap.constBegin())
            realSection.prepend('\n');
        realSection += '\n';

        device.write(realSection);

        const QCoreVariantMap &ents = i.value();
        for (QCoreVariantMap::const_iterator j = ents.constBegin(); j != ents.constEnd(); ++j) {
            QByteArray block;
            iniEscapedKey(j.key(), block);
            block += '=';

            const QCoreVariant &value = j.value();
            if (value.type() == QCoreVariant::StringList || value.type() == QCoreVariant::List) {
                iniEscapedStringList(variantListToStringList(value.toList()), block);
            } else {
                iniEscapedString(variantToString(value), block);
            }
            block += '\n';
            if(device.write(block) == -1) {
                writeError = true;
                break;
            }
        }
    }
    return !writeError;
}

/*! \class QCoreSettings
    \brief The QCoreSettings class provides persistent platform-independent application settings.

    \ingroup io
    \ingroup misc
    \mainclass

    Users normally expect an application to remember its settings
    (window sizes and positions, options, etc.) across sessions. This
    information is often stored in the system registry on Windows,
    and in XML preferences files on Mac OS X. On X11 and embedded Linux,
    in the absense of a standard, many applications (including the KDE
    applications) use INI text files.

    QCoreSettings is an abstraction around these technologies,
    enabling you to save and restore application settings in a
    portable manner.

    If your application links against the QtGui library, you can use
    QSettings rather than QCoreSettings. QSettings's API is based on
    QVariant instead of QCoreVariant, which allows you to save
    GUI-related types such as QRect, QSize, and QColor.

    \tableofcontents section1

    \section1 Basic Usage

    When creating a QCoreSettings object, you must pass the domain
    name of your company or organization as well as the name of your
    application. For example, if your product is called DataMill and
    you own the software.org Internet domain name, you would
    construct the QCoreSettings object as follows:

    \code
        QCoreSettings settings("software.org", "DataMill");
    \endcode

    QCoreSettings objects can be created either on the stack or on
    the heap (i.e. using \c new). Constructing and destroying a
    QCoreSettings object is very fast.

    If you use QCoreSettings from many places in your application, you
    might want to specify the organization domain name and the
    application name using QCoreApplication::setOrganizationDomain()
    and QCoreApplication::setApplicationName(), and then use the
    default QCoreSettings constructor:

    \code
        QCoreSettings::setOrganizationDomain("software.org");
        QCoreSettings::setApplicationName("DataMill");
        ...
        QCoreSettings settings;
    \endcode

    QCoreSettings stores settings. Each setting consists of a QString
    that specifies the setting's name (the \e key) and a QCoreVariant
    that stores the data associated with the key. To write a setting,
    use setValue(). For example:

    \code
        settings.setValue("wrapMargin", 68);
    \endcode

    If there already exists a setting with the same key, the existing
    value is overwritten by the new value. For efficiency, the
    changes may not be saved to permanent storage immediately. (You
    can always call sync() to commit your changes.)

    You can get a setting's value back using value():

    \code
        int margin = settings.value("wrapMargin").toInt();
    \endcode

    If there is no setting with the specified name, QCoreSettings
    returns a null QCoreVariant (which converts to the integer 0).
    You can specify another default value by passing a second
    argument to value():

    \code
        int margin = settings.value("wrapMargin", 80).toInt();
    \endcode

    To test whether a given key exists, call contains(). To remove
    the setting associated with a key, call remove(). To obtain the
    list of all keys, call allKeys(). To remove all keys, call
    clear().

    \section1 Key Syntax

    Setting keys can contain any Unicode characters. The Windows
    registry and INI files use case-insensitive keys, whereas the
    Carbon Preferences API on Mac OS X uses case-sensitive keys. To
    avoid portability problems, follow these two simple rules:

    \list 1
    \i Always refer to the same key using the same case. For example,
       if you refer to a key as "text fonts" in one place in your
       code, don't refer to it as "Text Fonts" elsewhere.

    \i Avoid key names that are identical except for the case. For
       example, if you have a key called "MainWindow", don't try to
       save another key as "mainwindow".
    \endlist

    You can form hierarchical keys using the '/' character as a
    separator, similar to Unix file paths. For example:

    \code
        settings.setValue("mainwindow/size", win->size());
        settings.setValue("mainwindow/fullScreen", win->isFullScreen());
        settings.setValue("outputpanel/visible", panel->isVisible());
    \endcode

    If you want to save many settings with the same prefix, you can
    specify the prefix using beginGroup() and call endGroup() at the
    end. Here's the same example again, but this time using the group
    mechanism:

    \code
        settings.beginGroup("mainwindow");
        settings.setValue("size", win->size());
        settings.setValue("fullScreen", win->isFullScreen());
        settings.endGroup();

        settings.beginGroup("outputpanel");
        settings.setValue("visible", panel->isVisible());
        settings.endGroup();
    \endcode

    If a group is set using beginGroup(), the behavior of most
    functions changes consequently. Groups can be set recursively.

    In addition to groups, QCoreSettings also supports an "array"
    concept. See beginReadArray() and beginWriteArray() for details.

    \section1 Fallback Mechanism

    Let's assume that you have created a QCoreSettings object with
    the organization domain name "software.org" and the application
    name "DataMill". When you look up a value, up to four locations
    are searched in that order:

    \list 1
    \i a user-specific location for the DataMill application
    \i a user-spefific location for all applications by software.org
    \i a system-wide location for the DataMill application
    \i a system-wide location for all applications by software.org
    \endlist

    On Unix with X11 and on embedded Linux, these locations are the
    following files:

    \list 1
    \i \c{$HOME/.qt4/software.org/DataMill.conf}
    \i \c{$HOME/.qt4/software.org.conf}
    \i \c{$QTDIR/.qt4/software.org/DataMill.conf}
    \i \c{$QTDIR/.qt4/software.org.conf}
    \endlist

    ($QTDIR is the location where Qt is installed.)

    On Mac OS X versions 10.2 and 10.3, these files are used:

    \list 1
    \i \c{$HOME/Library/Preferences/org.software.DataMill.plist}
    \i \c{$HOME/Library/Preferences/org.software.plist}
    \i \c{/Library/Preferences/org.software.DataMill.plist}
    \i \c{/Library/Preferences/org.software.plist}
    \endlist

    On Windows, the settings are stored in the following registry
    paths:

    \list 1
    \i \c{HKEY_CURRENT_USER\Software\software.org\DataMill}
    \i \c{HKEY_CURRENT_USER\Software\software.org}
    \i \c{HKEY_LOCAL_MACHINE\Software\software.org\DataMill}
    \i \c{HKEY_LOCAL_MACHINE\Software\software.org}
    \endlist

    If a key cannot be found in the first location, the search goes
    on in the second location, and so on. This enables you to store
    system-wide or organization-wide settings and to override them on
    a per-user or per-application basis. To turn off this mechanism,
    call setFallbacksEnabled(false).

    Although keys from all four locations are available for reading,
    only the first file (the user-specific location for the
    application at hand) is accessible for writing. To write to any
    of the other files, omit the application name and/or specify
    Qt::SystemScope (as opposed to Qt::UserScope, the default).

    Let's see with an example:

    \code
        QCoreSettings obj1("software.org", "DataMill");
        QCoreSettings obj2("software.org");
        QCoreSettings obj3(Qt::SystemScope, "software.org", "DataMill");
        QCoreSettings obj4(Qt::SystemScope, "software.org");
    \endcode

    The table below summarizes which QCoreSettings objects access
    which location. "\bold{X}" means that the location is the main
    location associated to the QCoreSettings object and is used both
    for reading and for writing; "o" means that the location is used
    as a fallback when reading.

    \table
    \header \i Locations               \i \c{obj1} \i \c{obj2} \i \c{obj3} \i \c{obj4}
    \row    \i 1. User, Application    \i \bold{X} \i          \i          \i
    \row    \i 2. User, Organization   \i o        \i \bold{X} \i          \i
    \row    \i 3. System, Application  \i o        \i          \i \bold{X} \i
    \row    \i 4. System, Organization \i o        \i o        \i o        \i \bold{X}
    \endtable

    The beauty of this mechanism is that it works on all platforms
    supported by Qt and that it still gives you a lot of flexibility,
    without requiring you to specify any file names or registry
    paths.

    If you want to use INI files on all platforms instead of the
    native API, you can pass Qt::IniFormat as the first argument to
    the QCoreSettings constructor, followed by the scope, the
    organization domain name, and the application name:

    \code
        QCoreSettings settings(Qt::IniFormat, Qt::UserScope,
                               "software.org", "DataMill");
    \endcode

    Sometimes you do want to access settings stored in a specific
    file or registry path. In that case, you can use a constructor
    that takes a file name (or registry path) and a file format. For
    example:

    \code
        QCoreSettings settings("datamill.ini", Qt::IniFormat);
    \endcode

    The file format can either be Qt::IniFormat or Qt::NativeFormat.
    On Mac OS X, the native format is an XML-based format called \e
    plist. On Windows, the native format is the Windows registry, and
    the first argument is a path in the registry rather than a file
    name, for example:

    \code
        QCoreSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft",
                               Qt::NativeFormat);
    \endcode

    On X11 and embedded Linux, Qt::IniFormat and Qt::NativeFormat have
    the same meaning.

    \section1 Restoring the State of a GUI Application

    QCoreSettings is often used to store the state of a GUI
    application. The following example will illustrate how to use we
    will use QCoreSettings (actually, QSettings) to save and restore
    the geometry of an application's main window.

    \code
        void MainWindow::writeSettings()
        {
            QSettings settings("www.moose-soft.co.uk", "Clipper");

            settings.beginGroup("MainWindow");
            settings.setValue("size", size());
            settings.setValue("pos", pos());
            settings.endGroup();
        }

        void MainWindow::readSettings()
        {
            QSettings settings("www.moose-soft.co.uk", "Clipper");

            settings.beginGroup("MainWindow");
            resize(settings.value("size", QSize(400, 400)));
            move(settings.value("pos", QPoint(200, 200)));
            settings.endGroup();
        }
    \endcode

    See \l{Window Geometry} for a discussion on why it is better to
    call resize() and move() rather than setGeometry() to restore a
    window's geometry.

    The readSettings() and writeSettings() functions need to be
    called from the main window's constructor and close event handler
    as follows:

    \code
        MainWindow::MainWindow(QWidget *parent)
            : QMainWindow(parent)
        {
            ...
            readSettings();
        }

        void MainWindow::closeEvent(QCloseEvent *event)
        {
            if (userReallyWantsToQuit()) {
                writeSettings();
                event->accept();
            } else {
                event->ignore();
            }
        }
    \endcode

    See the \c gui/application example provided in Qt's \c example
    directory for a self-contained example that uses QSettings.

    \section1 Accessing Settings from Multiple Threads or Processes Simultaneously

    QCoreSettings is \l{reentrant}. This means that you can use
    distinct QCoreSettings object in different threads
    simultaneously. This guarantee stands even when the QCoreSettings
    objects refer to the same files on disk (or to the same entries
    in the system registry). If a setting is modified through one
    QCoreSettings object, the change will immediately be visible in
    any other QCoreSettings object that operates on the same location
    and that lives in the same process.

    QCoreSettings can safely be used from different processes (which
    can be different instances of your application running at the
    same time or different applications altogether) to read and write
    to the same system locations. It uses a smart merging algorithm
    to ensure data integrity. Changes performed by another process
    aren't visible in the current process until sync() is called.

    \section1 Platform-Specific Notes

    While QCoreSettings attempts to smooth over the differences
    between the different supported platforms, there are still a few
    differences that you should be aware of when porting your
    application:

    \list
    \i  The Windows system registry has the following limitations: a
        subkey may not exceed 255 characters, an entry's value may not
        exceed 16,383 characters, and all the values of a key may not
        exceed 65,535 characters.

    \i  On Mac OS X, allKeys() will return some extra keys for global
        settings that apply to \e all applications. These keys can be
        read using value() but cannot be change, only shadowed. You
        can hide these global settings by calling
        setFallbackEnabled(false).
    \endlist

    \sa QSettings, QSessionManager
*/

/*! \enum QCoreSettings::Status

    The following status values are possible:

    \value NoError  No error occurred.
    \value AccessError  An access error occurred (e.g. trying to write to a read-only file).
    \value FormatError  A format error occurred (e.g. loading a malformed INI file).

    \sa status()
*/

#ifndef QT_NO_QOBJECT
/*!
    Constructs a QCoreSettings object for accessing settings of the
    application called \a application from the organization with the
    Internet domain name \a organization, and with parent \a parent.

    Example:
    \code
        QCoreSettings settings("www.technopro.co.jp", "Facturo-Pro");
    \endcode

    The scope is Qt::UserScope and the format is Qt::NativeFormat.

    \sa {Locations for Storing Settings}
*/
QCoreSettings::QCoreSettings(const QString &organization, const QString &application,
                             QObject *parent)
    : QObject(*QCoreSettingsPrivate::create(Qt::NativeFormat, Qt::UserScope,
                                            organization, application,
                                            QCoreSettingsPrivate::variantToStringCoreImpl, QCoreSettingsPrivate::stringToVariantCoreImpl),
              parent)
{
}

/*!
    Constructs a QCoreSettings object for accessing settings of the
    application called \a application from the organization with the
    Internet domain name \a organization, and with parent \a parent.

    If \a scope is Qt::UserScope, the QCoreSettings object searches
    user-specific settings first, before it seaches system-wide
    settings as a \l{Fallback Mechanism}{fallback}. If \a scope is
    Qt::SystemScope, the QCoreSettings object ignores user-specific
    settings and provides access to system-wide settings.

    The storage format is always Qt::NativeFormat.

    If no application name is given, the QCoreSettings object will
    only access the organization-wide
    \l{Locations for Storing Settings}{locations}.
*/
QCoreSettings::QCoreSettings(Qt::SettingsScope scope, const QString &organization,
                             const QString &application, QObject *parent)
    : QObject(*QCoreSettingsPrivate::create(Qt::NativeFormat, scope, organization, application,
                                            QCoreSettingsPrivate::variantToStringCoreImpl, QCoreSettingsPrivate::stringToVariantCoreImpl),
              parent)
{
}

/*!
    Constructs a QCoreSettings object for accessing settings of the
    application called \a application from the organization with the
    Internet domain name \a organization, and with parent \a parent.

    If \a scope is Qt::UserScope, the QCoreSettings object searches
    user-specific settings first, before it seaches system-wide
    settings as a \l{Fallback Mechanism}{fallback}. If \a scope is
    Qt::SystemScope, the QCoreSettings object ignores user-specific
    settings and provides access to system-wide settings.

    If \a format is Qt::NativeFormat, the native API is used for
    storing settings. If \a format is Qt::IniFormat, the INI format
    is used.

    If no application name is given, the QCoreSettings object will
    only access the organization-wide
    \l{Locations for Storing Settings}{locations}.
*/
QCoreSettings::QCoreSettings(Qt::SettingsFormat format, Qt::SettingsScope scope,
                             const QString &organization, const QString &application,
                             QObject *parent)
    : QObject(*QCoreSettingsPrivate::create(format, scope, organization, application,
                                            QCoreSettingsPrivate::variantToStringCoreImpl, QCoreSettingsPrivate::stringToVariantCoreImpl),
              parent)
{
}

/*!
    Constructs a QCoreSettings object for accessing the settings
    stored in the file called \a fileName, with parent \a parent. If
    the file doesn't already exist, it is created.

    If \a format is Qt::NativeFormat, the meaning of \a fileName
    depends on the platform. On Unix/X11, \a fileName is the name of
    an INI file. On Mac OS X, \a fileName is the name of a .plist
    file. On Windows, \a fileName is a path in the system registry.

    If \a format is Qt::IniFormat, \a fileName is the name of an INI
    file.

    \sa fileName()
*/
QCoreSettings::QCoreSettings(const QString &fileName, Qt::SettingsFormat format,
                             QObject *parent)
    : QObject(*QCoreSettingsPrivate::create(fileName, format,
                                            QCoreSettingsPrivate::variantToStringCoreImpl, QCoreSettingsPrivate::stringToVariantCoreImpl),
              parent)
{
}

/*!
    Constructs a QCoreSettings object for accessing settings of the
    application and organization set previously with a call to
    QCoreApplication::setOrganizationDomain() and
    QCoreApplication::setApplicationName().

    The scope is Qt::UserScope and the format is Qt::NativeFormat.

    The code

    \code
        QCoreSettings settings("www.technopro.co.jp", "Facturo-Pro");
    \endcode

    is equivalent to

    \code
        QApplication::setOrganizationDomain("www.technopro.co.jp");
        QApplication::setApplicationName("Facturo-Pro");
        QCoreSettings settings;
    \endcode

    If QApplication::setOrganizationDomain() and
    QApplication::setApplicationName() has not been previously called,
    the QCoreSettings object will not be able to read or write any
    settings, and status() will return \c AccessError.
*/
QCoreSettings::QCoreSettings(QObject *parent)
    : QObject(*QCoreSettingsPrivate::create(Qt::NativeFormat, Qt::UserScope,
                                            QCoreApplication::organizationDomain(),
                                            QCoreApplication::applicationName(),
                                            QCoreSettingsPrivate::variantToStringCoreImpl, QCoreSettingsPrivate::stringToVariantCoreImpl),
                parent)
{
}

/*!
    \internal
*/
QCoreSettings::QCoreSettings(QCoreSettingsPrivate *p, QObject *parent)
    : QObject(*p, parent)
{
}
#else
QCoreSettings::QCoreSettings(const QString &organization, const QString &application)
    : d_ptr(QCoreSettingsPrivate::create(Qt::NativeFormat, Qt::UserScope,
                                         organization, application,
                                         QCoreSettingsPrivate::variantToStringCoreImpl,
                                         QCoreSettingsPrivate::stringToVariantCoreImpl))
{
    d_ptr->q_ptr = this;
}

QCoreSettings::QCoreSettings(Qt::SettingsScope scope, const QString &organization,
                             const QString &application)
    : d_ptr(QCoreSettingsPrivate::create(Qt::NativeFormat, scope, organization, application,
                                         QCoreSettingsPrivate::variantToStringCoreImpl,
                                         QCoreSettingsPrivate::stringToVariantCoreImpl))
{
    d_ptr->q_ptr = this;
}

QCoreSettings::QCoreSettings(Qt::SettingsFormat format, Qt::SettingsScope scope,
                             const QString &organization, const QString &application)
    : d_ptr(QCoreSettingsPrivate::create(format, scope, organization, application,
                                         QCoreSettingsPrivate::variantToStringCoreImpl,
                                         QCoreSettingsPrivate::stringToVariantCoreImpl))
{
    d_ptr->q_ptr = this;
}

QCoreSettings::QCoreSettings(const QString &fileName, Qt::SettingsFormat format)
    : d_ptr(QCoreSettingsPrivate::create(fileName, format,
                                         QCoreSettingsPrivate::variantToStringCoreImpl,
                                         QCoreSettingsPrivate::stringToVariantCoreImpl))
{
    d_ptr->q_ptr = this;
}
#endif

/*!
    Destroys the QCoreSettings object. Any unsaved changes will be
    written to permanent storage at that point.

    \sa sync()
*/
QCoreSettings::~QCoreSettings()
{
    Q_D(QCoreSettings);
    if (d->pendingChanges)
        d->sync();
}

/*!
    Removes all entries in the primary location associated to this
    QCoreSettings object.

    Entries in \l{Fallback Mechanism}{fallback} locations are not removed.

    \sa remove(), setFallbacksEnabled()
*/
void QCoreSettings::clear()
{
    Q_D(QCoreSettings);
    d->clear();
    d->requestUpdate();
}

/*!
    Writes any unsaved changes to permanent storage, and reloads any
    settings that have been changed in the meantime by another
    application.

    Unless you use QCoreSettings as a communication mechanism between
    different processes, you normally don't need to call this
    function.
*/
void QCoreSettings::sync()
{
    Q_D(QCoreSettings);
    d->sync();
}

/*!
    Returns the path where settings written using this QCoreSettings
    object are stored.

    On Windows, if format() is Qt::NativeFormat, the return value is
    a system registry path, not a file path.

    \sa isWritable()
*/
QString QCoreSettings::fileName() const
{
    Q_D(const QCoreSettings);
    return d->fileName();
}

/*!
    Returns a status code indicating the first error that was met by
    QCoreSettings, or \c QCoreSettings::NoError if no error occurred.
*/
QCoreSettings::Status QCoreSettings::status() const
{
    Q_D(const QCoreSettings);
    return d->status;
}

/*!
    Appends \a prefix to the current group.

    The current group is automatically prepended to all keys
    specified to QCoreSettings. In addition, query functions such as
    childGroups(), childKeys(), and allKeys() are based on the group.
    By default, no group is set.

    Groups are useful to avoid typing in the same setting paths over
    and over. For example:

    \code
        settings.beginGroup("mainwindow");
        settings.setValue("size", win->size());
        settings.setValue("fullScreen", win->isFullScreen());
        settings.endGroup();

        settings.beginGroup("outputpanel");
        settings.setValue("visible", panel->isVisible());
        settings.endGroup();
    \endcode

    This will set the value of three settings:

    \list
    \i \c mainwindow/size
    \i \c mainwindow/fullScreen
    \i \c outputpanel/visible
    \endlist

    Call endGroup() to reset the current group to what it was before
    the corresponding beginGroup() call. Groups can be nested.

    \sa endGroup(), group()
*/
void QCoreSettings::beginGroup(const QString &prefix)
{
    Q_D(QCoreSettings);
    d->beginGroupOrArray(QSettingsGroup(d->normalizedKey(prefix)));
}

/*!
    Resets the group to what it was before the corresponding
    beginGroup() call.

    Example:

    \code
        settings.beginGroup("alpha");
        // settings.group() == "alpha"

        settings.beginGroup("beta");
        // settings.group() == "alpha/beta"

        settings.endGroup();
        // settings.group() == "alpha"

        settings.endGroup();
        // settings.group() == ""
    \endcode

    \sa beginGroup(), group()
*/
void QCoreSettings::endGroup()
{
    Q_D(QCoreSettings);
    if (d->groupStack.isEmpty()) {
        qWarning("QCoreSettings::endGroup: No matching beginGroup()");
        return;
    }

    QSettingsGroup group = d->groupStack.pop();
    int len = group.toString().size();
    if (len > 0)
        d->groupPrefix.truncate(d->groupPrefix.size() - (len + 1));

    if (group.isArray())
        qWarning("QCoreSettings::endGroup: Expected endArray() instead");
}

/*!
    Returns the current group.

    \sa beginGroup(), endGroup()
*/
QString QCoreSettings::group() const
{
    Q_D(const QCoreSettings);
    return d->groupPrefix.left(d->groupPrefix.size() - 1);
}

/*!
    Adds \a prefix to the current group and starts reading from an
    array. Returns the size of the array.

    Example:

    \code
        struct Login {
            QString userName;
            QString password;
        };
        QList<Login> logins;
        ...

        QCoreSettings settings;
        int size = settings.beginReadArray("logins");
        for (int i = 0; i < size; ++i) {
            settings.setArrayIndex(i);
            Login login;
            login.userName = settings.value("userName");
            login.password = settings.value("password");
            logins.append(login);
        }
        settings.endArray();
    \endcode

    Use beginWriteArray() to write the array in the first place.

    \sa beginWriteArray(), endArray(), setArrayIndex()
*/
int QCoreSettings::beginReadArray(const QString &prefix)
{
    Q_D(QCoreSettings);
    d->beginGroupOrArray(QSettingsGroup(d->normalizedKey(prefix), false));
    return value("size").toInt();
}

/*!
    Adds \a prefix to the current group and starts writing an array
    of size \a size. If \a size is -1 (the default), it is automatically
    determined based on the indexes of the entries written.

    If you have many occurrences of a certain set of keys, you can
    use arrays to make your life easier. For example, let's suppose
    that you want to save a variable-length list of user names and
    passwords. You could then write:

    \code
        struct Login {
            QString userName;
            QString password;
        };
        QList<Login> logins;
        ...

        QCoreSettings settings;
        settings.beginWriteArray("logins");
        for (int i = 0; i < logins.size(); ++i) {
            settings.setArrayIndex(i);
            settings.setValue("userName", list.at(i).userName);
            settings.setValue("password", list.at(i).password);
        }
        settings.endArray();
    \endcode

    The generated keys will have the form

    \list
    \i \c logins/1/userName
    \i \c logins/1/password
    \i \c logins/2/userName
    \i \c logins/2/password
    \i \c logins/3/userName
    \i \c logins/3/password
    \i ...
    \endlist

    To read back an array, use beginReadArray().

    \sa beginReadArray(), endArray(), setArrayIndex()
*/
void QCoreSettings::beginWriteArray(const QString &prefix, int size)
{
    Q_D(QCoreSettings);
    d->beginGroupOrArray(QSettingsGroup(d->normalizedKey(prefix), size < 0));

    if (size < 0)
        remove(QLatin1String("size"));
    else
        setValue(QLatin1String("size"), size);
}

/*!
    Closes the array that was started using beginReadArray() or
    beginWriteArray().

    \sa beginReadArray(), beginWriteArray()
*/
void QCoreSettings::endArray()
{
    Q_D(QCoreSettings);
    if (d->groupStack.isEmpty()) {
        qWarning("QCoreSettings::endArray: No matching beginArray()");
        return;
    }

    QSettingsGroup group = d->groupStack.top();
    int len = group.toString().size();
    d->groupStack.pop();
    if (len > 0)
        d->groupPrefix.truncate(d->groupPrefix.size() - (len + 1));

    if (group.arraySizeGuess() != -1)
        setValue(group.name() + QLatin1String("/size"), group.arraySizeGuess());

    if (!group.isArray())
        qWarning("QCoreSettings::endArray: Expected endGroup() instead");
}

/*!
    Sets the current array index to \a i. Calls to functions such as
    setValue(), value(), remove(), and contains() will operate on the
    array entry at that index.

    You must call beginReadArray() or beginWriteArray() before you
    can call this function.
*/
void QCoreSettings::setArrayIndex(int i)
{
    Q_D(QCoreSettings);
    if (d->groupStack.isEmpty() || !d->groupStack.top().isArray()) {
        qWarning("QCoreSettings::setArrayIndex: Missing beginArray()");
        return;
    }

    QSettingsGroup &top = d->groupStack.top();
    int len = top.toString().size();
    top.setArrayIndex(qMax(i, 0));
    d->groupPrefix.replace(d->groupPrefix.size() - len - 1, len, top.toString());
}

/*!
    Returns a list of all keys, including subkeys, that can be read
    using the QCoreSettings object.

    Example:

    \code
        QCoreSettings settings;
        settings.setValue("fridge/color", Qt::white);
        settings.setValue("fridge/size", QSize(32, 96));
        settings.setValue("sofa", true);
        settings.setValue("tv", false);

        QStringList keys = settings.allKeys();
        // keys: ["fridge/color", "fridge/size", "sofa", "tv"]
    \endcode

    If a group is set using beginGroup(), only the keys in the group
    are returned, without the group prefix:

    \code
        settings.beginGroup("fridge");
        keys = settings.allKeys();
        // keys: ["color", "size"]
    \endcode

    \sa childGroups(), childKeys()
*/
QStringList QCoreSettings::allKeys() const
{
    Q_D(const QCoreSettings);
    return d->children(d->groupPrefix, QCoreSettingsPrivate::AllKeys);
}

/*!
    Returns a list of all top-level keys that can be read using the
    QCoreSettings object.

    Example:

    \code
        QCoreSettings settings;
        settings.setValue("fridge/color", Qt::white);
        settings.setValue("fridge/size", QSize(32, 96));
        settings.setValue("sofa", true);
        settings.setValue("tv", false);

        QStringList keys = settings.childKeys();
        // keys: ["sofa", "tv"]
    \endcode

    If a group is set using beginGroup(), the top-level keys in that
    group are returned, without the group prefix:

    \code
        settings.beginGroup("fridge");
        keys = settings.childKeys();
        // keys: ["color", "size"]
    \endcode

    You can navigate through the entire setting hierarchy using
    childKeys() and childGroups() recursively.

    \sa childGroups(), allKeys()
*/
QStringList QCoreSettings::childKeys() const
{
    Q_D(const QCoreSettings);
    return d->children(d->groupPrefix, QCoreSettingsPrivate::ChildKeys);
}

/*!
    Returns a list of all key top-level groups that contain keys that
    can be read using the QCoreSettings object.

    Example:

    \code
        QCoreSettings settings;
        settings.setValue("fridge/color", Qt::white);
        settings.setValue("fridge/size", QSize(32, 96));
        settings.setValue("sofa", true);
        settings.setValue("tv", false);

        QStringList groups = settings.childGroups();
        // group: ["fridge"]
    \endcode

    If a group is set using beginGroup(), the first-level keys in
    that group are returned, without the group prefix.

    \code
        settings.beginGroup("fridge");
        groups = settings.childGroups();
        // groups: []
    \endcode

    You can navigate through the entire setting hierarchy using
    childKeys() and childGroups() recursively.

    \sa childKeys(), allKeys()
*/
QStringList QCoreSettings::childGroups() const
{
    Q_D(const QCoreSettings);
    return d->children(d->groupPrefix, QCoreSettingsPrivate::ChildGroups);
}

/*!
    Returns true if settings can be written using this QCoreSettings
    object; returns false otherwise.

    One reason why isWritable() might return false is if
    QCoreSettings operates on a read-only file.

    \sa fileName(), status()
*/
bool QCoreSettings::isWritable() const
{
    Q_D(const QCoreSettings);
    return d->isWritable();
}

/*!
    Sets the value of setting \a key to \a value.

    If the key already exists, the previous value is overwritten.

    Example:

    \code
        QCoreSettings settings;
        settings.setValue("interval", 30);
        settings.value("interval").toInt();     // returns 30

        settings.setValue("interval", 6.55);
        settings.value("interval").toDouble();  // returns 6.55
    \endcode

    \sa value(), remove(), contains()
*/
void QCoreSettings::setValue(const QString &key, const QCoreVariant &value)
{
    Q_D(QCoreSettings);
    QString k = d->actualKey(key);
    d->set(k, value);
    d->requestUpdate();
}

/*!
    Removes the setting \a key and any sub-settings of \a key.

    Example:

    \code
        QCoreSettings settings;
        settings.setValue("ape");
        settings.setValue("monkey", 1);
        settings.setValue("monkey/sea", 2);
        settings.setValue("monkey/doe", 4);

        settings.remove("monkey");
        QStringList keys = settings.allKeys();
        // keys: ["ape"]
    \endcode

    Be aware that if one of the \l{Fallback Mechanism}{fallback
    locations} contains a setting with the same key, that setting
    will be visible after calling remove().

    \sa setValue(), value(), contains()
*/
void QCoreSettings::remove(const QString &key)
{
    Q_D(QCoreSettings);
    /*
        We cannot use actualKey(), because remove() supports empty
        keys. The code is also tricky because of slash handling.
    */
    QString theKey = d->normalizedKey(key);
    if (theKey.isEmpty())
        theKey = group();
    else
        theKey.prepend(d->groupPrefix);

    if (theKey.isEmpty()) {
        d->clear();
    } else {
        d->remove(theKey);
    }
    d->requestUpdate();
}

/*!
    Returns true if there exists a setting called \a key; returns
    false otherwise.

    If a group is set using beginGroup(), \a key is taken to be
    relative to that group.

    \sa value(), setValue()
*/
bool QCoreSettings::contains(const QString &key) const
{
    Q_D(const QCoreSettings);
    QString k = d->actualKey(key);
    return d->get(k, 0);
}

/*!
    Sets whether fallbacks are enabled to \a b.

    By default, fallbacks are enabled.

    \sa fallbacksEnabled(), {Fallback Mechanism}
*/
void QCoreSettings::setFallbacksEnabled(bool b)
{
    Q_D(QCoreSettings);
    d->fallbacks = !!b;
}

/*!
    Returns true if fallbacks are enabled; returns false otherwise.

    By default, fallbacks are enabled.

    \sa setFallbacksEnabled(), {Fallback Mechanism}
*/
bool QCoreSettings::fallbacksEnabled() const
{
    Q_D(const QCoreSettings);
    return d->fallbacks;
}

#ifndef QT_NO_QOBJECT
/*!
    \reimp
*/
bool QCoreSettings::event(QEvent *event)
{
    Q_D(QCoreSettings);
    if (event->type() == QEvent::UpdateRequest) {
        d->update();
        return true;
    }
    return QObject::event(event);
}
#endif

/*!
    Returns the value for setting \a key. If the setting doesn't
    exist, returns \a defaultValue.

    If no default value is specified, a default QCoreVariant is
    returned.

    Example:

    \code
        QCoreSettings settings;
        settings.setValue("snake", 58);
        settings.value("snake", 1024).toInt();  // returns 58
        settings.value("zebra", 1024).toInt();  // returns 1024
        settings.value("zebra").toInt();        // returns 0
    \endcode

    \sa setValue(), contains(), remove()
*/
QCoreVariant QCoreSettings::value(const QString &key, const QCoreVariant &defaultValue) const
{
    Q_D(const QCoreSettings);
    QCoreVariant result = defaultValue;
    QString k = d->actualKey(key);
    d->get(k, &result);
    return result;
}

#endif // QT_NO_SETTINGS
