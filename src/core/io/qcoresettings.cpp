#include "qplatformdefs.h"

// POSIX Large File Support redefines open -> open64
static inline int qt_open(const char *pathname, int flags, mode_t mode)
{ return ::open(pathname, flags, mode); }
#if defined(open)
#undef open
#endif

// POSIX Large File Support redefines truncate -> truncate64
#if defined(truncate)
#undef truncate
#endif

#include "qcoresettings.h"

#ifndef QT_NO_SETTINGS

#include "qcache.h"
#include "qcoreapplication.h"
#include "private/qcoresettings_p.h"
#include "qdir.h"
#include "qfile.h"
#include "qfileinfo.h"
#include "qstring.h"

#define d d_func()
#define q q_func()

QSETTINGS_DEFINE_CORE_PARSER_FUNCTIONS

// ************************************************************************
// QConfFile

/*
    QConfFile objects are explicitly shared within the application.
    This ensures that modification to the settings done through one
    QSettings object are immediately reflected in other setting
    objects of the same application.
*/

typedef QHash<QString, QConfFile *> ConfFileHash;
typedef QCache<QString, QConfFile> ConfFileCache;

Q_GLOBAL_STATIC(QSpinLock, globalMutexFunc)
Q_GLOBAL_STATIC(ConfFileHash, usedHashFunc)
Q_GLOBAL_STATIC(ConfFileCache, unusedCacheFunc)

static QSpinLock *globalSpinLock;
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

    globalSpinLock = globalMutexFunc();
    usedHash = usedHashFunc();
    unusedCache = unusedCacheFunc();

    QSpinLockLocker locker(globalSpinLock);

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
    globalSpinLock = globalMutexFunc();
    unusedCache = unusedCacheFunc();

    QSpinLockLocker locker(globalSpinLock);
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

void QCoreSettingsPrivate::requestUpdate()
{
    if (!d->pendingChanges) {
        QCoreApplication::postEvent(q, new QEvent(QEvent::UpdateRequest));
        d->pendingChanges = true;
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
    return QDir::homePath() + QDir::separator() + QLatin1String(".qt4") + QDir::separator();
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

    if (organization.isEmpty()) {
        setStatus(QCoreSettings::AccessError);
        return;
    }

    const char *extension = ".ini";

    QString appFile = organization + QDir::separator() + application + QLatin1String(extension);
    QString orgFile = organization + QLatin1String(extension);

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
    QSpinLockLocker locker(globalSpinLock);

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
    QSpinLockLocker locker(globalSpinLock);

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
    QSpinLockLocker locker(globalSpinLock);
    confFile->removedKeys.remove(theKey);
    confFile->addedKeys.insert(theKey, value);
}

bool QConfFileSettingsPrivate::get(const QString &key, QCoreVariant *value) const
{
    QSettingsKey theKey(key, cs);
    SettingsKeyMap::const_iterator j;
    bool found = false;

    QSpinLockLocker locker(globalSpinLock);

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

    QSpinLockLocker locker(globalSpinLock);

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
    QSpinLockLocker locker(globalSpinLock);
    confFile->addedKeys.clear();
    confFile->removedKeys = confFiles[spec]->originalKeys;
}

void QConfFileSettingsPrivate::sync()
{
    QSpinLockLocker locker(globalSpinLock);

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

QString QConfFileSettingsPrivate::path() const
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
    int fd = qt_open(QFile::encodeName(file.fileName()), flags, S_IRUSR | S_IWUSR);
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
                                         : QIODevice::ReadOnly,
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
    int ch, ch2;
    int pos = 0;

    equalsCharPos = -1;

    while ((ch = device.getch()) != -1) {
    process_ch:
        MAYBE_GROW();

        switch (ch) {
        case '"':
            data[pos++] = '"';
            while ((ch = device.getch()) != '"') {
                MAYBE_GROW();

                if (ch == -1)
                    goto end;

                if (ch == '\\') {
                    data[pos++] = '\\';
                    ch = device.getch();
                    if (ch == -1)
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
            ch2 = device.getch();
            if (ch2 == -1)
                goto end;
            if ((ch2 != '\n' && ch2 != '\r') || ch == ch2)
                device.ungetch(ch2);
#endif
            if (pos > 0)
                goto end;
            break;
        case '\\':
            ch = device.getch();
            if (ch == -1)
                goto end;

            if (ch == '\n' || ch == '\r') {
                ch2 = device.getch();
                if (ch2 != -1) {
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
            while ((ch = device.getch()) != -1) {
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

    for (i = iniMap.constBegin(); i != iniMap.constEnd(); ++i) {
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
            device.write(block);
        }
    }
    return device.status() == QIODevice::Ok;
}

// ************************************************************************
// QCoreSettings

/*! \class QCoreSettings
    \brief The QCoreSettings class provides persistent platform-independent application settings.

    \ingroup io
    \ingroup misc
    \mainclass

    Users normally expect an application to remember its settings
    (window sizes and positions, options, etc.) across sessions. This
    information is often stored in the system registry on Windows,
    and in XML preferences files on Mac OS X. On X11, in
    the absense of a standard, many applications (including the KDE
    applications) use .ini text files.

    QCoreSettings is an abstraction around these technologies,
    enabling you to save and restore application settings in a
    portable manner.

    \tableofcontents section1

    \section1 Basic Usage

    When creating a QCoreSettings object, you must pass the domain
    name of your company or organization as well as the name of your
    application. For example:

    \code
        QCoreSettings settings("software.org", "DataMill");
    \endcode

    QCoreSettings objects can be created either on the stack or on
    the heap (using \c new). Constructing and destroying a
    QCoreSettings object is very fast.

    QCoreSettings stores settings. Each setting consists of a QString
    that identifies the key and a QCoreVariant that stores the data
    associated with key. To write a setting, use setValue(). For
    example:

    \code
        settings.setValue("wrap margin", 68);
    \endcode

    If there already exists a setting with the same key, the existing
    value is overwritten by the new value. For efficiency, the
    changes may not be saved to permanent storage immediately. (You
    can always call sync() to commit your changes.)

    You can get a setting's value back using value():

    \code
        int margin = settings.value("wrap margin").toInt();
    \endcode

    If there is no setting with the specified name, QCoreSettings
    returns a null QCoreVariant (which converts to the integer 0).
    You can specify another default value by passing a second
    argument to value():

    \code
        int margin = settings.value("wrap margin", 80).toInt();
    \endcode

    To test whether a given key exists, call contains(). To remove
    the setting associated with a key, call remove(). To obtain the
    list of all keys, call allKeys(). To remove all keys, call
    clear().

    If your application links against the QtGui library, you can use
    QSettings rather than QCoreSettings. QSettings's API is based on
    QVariant instead of QCoreVariant, which allows you to save
    GUI-related types such as QRect, QSize, and QColor.

    \section1 Key Syntax

    The keys can contain any Unicode characters. The Windows registry
    and .ini files use case-insensitive keys, whereas the Carbon
    Preferences API on Mac OS X uses case-sensitive keys. To avoid
    portability problems, we recommend that you follow these two
    rules:

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

    If a group is set using beginGroup(), allKeys() returns only the
    keys relative to that group. Groups can be set recursively.

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

    On Unix with X11, these locations are the following files:

    \list 1
    \i \c{$HOME/.qt/org.software.DataMill}
    \i \c{$HOME/.qt/org.software}
    \i \c{/opt/.qt/org.software.DataMill}
    \i \c{/opt/.qt/org.software}
    \endlist

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
    application at hand) is accessible for writing. If you want to
    write to any of the other files, you need to omit the application
    name and/or specify Qt::SystemScope (as opposed to Qt::UserScope,
    the default).

    Let's see with an example:

    \code
        QCoreSettings obj1("software.org", "DataMill");
        QCoreSettings obj2("software.org");
        QCoreSettings obj3(Qt::SystemScope, "software.org", "DataMill");
        QCoreSettings obj4(Qt::SystemScope, "software.org");
    \endcode

    The table below summarizes which QCoreSettings objects access
    which location. "X" means that the location is the main location
    associated to the QCoreSettings object and is used both for
    reading and for writing; "o" means that the location is used as a
    fallback when reading.

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

    If you want to use .ini file on all platforms instead of the
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

    On X11, Qt::IniFormat and Qt::NativeFormat are synonyms.

    \section1 Restoring the State of a GUI Application

    In the following example, we will use QSettings to save and
    restore the geometry of an application's main window.

    \code
        void MainWindow::readSettings()
        {
            QSettings settings("www.moosetech.co.uk", "Clipper");
            settings.beginGroup("MainWindow");
            resize(settings.value("size", QSize(400, 400)));
            move(settings.value("pos", QPoint(200, 200)));
            settings.endGroup("MainWindow");
        }

        void MainWindow::writeSettings()
        {
            QSettings settings("www.moosetech.co.uk", "Clipper");
            settings.beginGroup("MainWindow");
            settings.setValue("size", size());
            settings.setValue("pos", pos());
            settings.endGroup("MainWindow");
        }
    \endcode

    (See \l{Window Geometry} for a discussion on why it is necessary
    to call resize() and move() rather than setGeometry() to restore
    a window's geometry.)

    These functions need to be called from the main window's
    constructor and close event handler as follows:

    \code
        MainWindow::MainWindow(QWidget *parent)
            : QMainWindow(parent)
        {
            ...
            readSettings();
        }

        void MainWindow::closeEvent(QCloseEvent *event)
        {
            writeSettings();
        }
    \endcode

    \section1 Accessing Settings from Multiple Threads or Processes Simultaneously

    QCoreSettings is \l{reentrant}. This means that you can use
    distinct QCoreSettings object in different threads
    simultaneously. This guarantee stands even when the QCoreSettings
    objects refer to the same files on disk (or to the same entries
    in the system registry). If a setting is modified through one
    QCoreSettings object, the change will immediately be visible in
    any other QCoreSettings object that operates on the same location
    and that lives in the same process.

    QCoreSettings can safely be used from different processes (i.e.
    different instances of your application running at the same time
    or different applications altogether) to read and write to the
    same system locations. It uses a smart merging algorithm to
    ensure data integrity. Changes performed by another process
    aren't visible in the current process until sync() is called.

    \section1 Platform-Specific Notes

    The Windows system registry has the following limitations: a
    subkey may not exceed 255 characters, an entry's value may not
    exceed 16,383 characters, and all the values of a key may not
    exceed 65,535 characters.

    On Mac OS X, allKeys() will return some extra keys for global
    settings that apply to \e all applications. These keys can be
    read using value() but cannot be change, only shadowed. These
    settings usually have carefully chosen names such as
    "AppleDoubleClick" and "###" that are unlikely to clash with your
    application's settings. If you want to hide these global
    settings, call setFallbackEnabled(false).

    \sa QSettings, QSessionManager
*/

/*!
    Constructs a QCoreSettings object for accessing settings of the
    application called \a application from the organization with the
    Internet domain name \a organization, and with parent \a parent.

    Example:
    \code
        QCoreSettings settings("www.technopro.co.uk", "Facturo-Pro");
    \endcode

    The scope is Qt::UserScope and the format is Qt::NativeFormat.

    \sa \l{Locations for Storing Settings}
*/
QCoreSettings::QCoreSettings(const QString &organization, const QString &application,
                                QObject *parent)
    : QObject(*QCoreSettingsPrivate::create(Qt::NativeFormat, Qt::UserScope,
                                            organization, application,
                                            variantToStringCoreImpl, stringToVariantCoreImpl),
                parent)
{
}

/*!
    Constructs a QCoreSettings object for accessing settings of the
    application called \a application from the organization with the
    Internet domain name \a organization, and with parent \a parent.

    If the scope is Qt::UserScope, the QCoreSettings object searches
    user-specific settings first, before it seaches system-wide
    settings as a fallback. If the scope is Qt::SystemScope, the
    QCoreSettings object ignores user-specific settings and provides
    access to system-wide settings.

    The format is Qt::NativeFormat.

    If no application name is given, the QCoreSettings object will
    access the organization-wide file(s) only.

    \sa {Locations for Storing Settings}

*/
QCoreSettings::QCoreSettings(Qt::SettingsScope scope, const QString &organization,
                                const QString &application,
                                QObject *parent)
    : QObject(*QCoreSettingsPrivate::create(Qt::NativeFormat, scope, organization, application,
                                            variantToStringCoreImpl, stringToVariantCoreImpl),
                parent)
{
}

/*!
    Constructs a QCoreSettings object for accessing settings of the
    application called \a application from the organization with the
    Internet domain name \a organization, and with parent \a parent.

    If the scope is Qt::UserScope, the QCoreSettings object searches
    user-specific settings first, before it seaches system-wide
    settings as a fallback. If the scope is Qt::SystemScope, the
    QCoreSettings object ignores user-specific settings and provides
    access to system-wide settings.

    If the format is Qt::NativeFormat, the native API is used for
    writing settings. If the format is Qt::IniFormat, the .ini format
    is used.

    If no application name is given, the QCoreSettings object will
    access the organization-wide file(s) only.

    \sa {Locations for Storing Settings}
*/
QCoreSettings::QCoreSettings(Qt::SettingsFormat format, Qt::SettingsScope scope,
                                const QString &organization, const QString &application,
                                QObject *parent)
    : QObject(*QCoreSettingsPrivate::create(format, scope, organization, application,
                                            variantToStringCoreImpl, stringToVariantCoreImpl),
                parent)
{
}

/*!
    Constructs a QCoreSettings object for accessing the settings
    stored in the file called \a fileName, with parent \a parent. If
    the file doesn't already exist, it is created.

    If the format is Qt::NativeFormat, the meaning of \a fileName
    depends on the platform. On Unix/X11, \a fileName is the name of
    an .ini file. On Mac OS X, \a fileName is the name of a .plist
    file. On Windows, \a fileName is a path in the system registry.

    If the format is Qt::IniFormat, \a fileName is the name of an
    .ini file.

    \sa {Locations for Storing Settings}
*/
QCoreSettings::QCoreSettings(const QString &fileName, Qt::SettingsFormat format,
                                QObject *parent)
    : QObject(*QCoreSettingsPrivate::create(fileName, format,
                                            variantToStringCoreImpl, stringToVariantCoreImpl),
                parent)
{
}

#ifndef QT_BUILD_QMAKE
// qmake doesn't link against qcoreapplication, which this ctor needs
/*!
    Constructs a QCoreSettings object for accessing settings of the
    application and organization set previously with a call to
    QApplication::setProductInfo().

    The scope is Qt::UserScope and the format is Qt::NativeFormat.

    The following code:
    \code
        QCoreSettings settings("www.technopro.co.uk", "Facturo-Pro");
    \endcode
    is equivalent to:
    \code
        qApp->setProductInfo("www.technopro.co.uk", "Facturo-Pro");
        QCoreSettings settings;
    \endcode

    If QApplication::setProductInfo() has not been previously called,
    the QSettings object will not be able to read or write any settings,
    and status() will return AccessError.

    \sa \l{Locations for Storing Settings}
*/
QCoreSettings::QCoreSettings(QObject *parent)
    : QObject(*QCoreSettingsPrivate::create(Qt::NativeFormat, Qt::UserScope,
                                            QCoreApplication::instance()->organization(),
                                            QCoreApplication::instance()->application(),
                                            variantToStringCoreImpl, stringToVariantCoreImpl),
                parent)
{
}
#endif

/*!
    Destroys the QCoreSettings object.
*/
QCoreSettings::~QCoreSettings()
{
    if (d->pendingChanges)
        d->sync();
}

/*!
    \internal
*/
QCoreSettings::QCoreSettings(QCoreSettingsPrivate *p, QObject *parent)
    : QObject(*p, parent)
{
}

/*!
    Removes all entries in the settings file associated to this
    QCoreSettings object.

    Entries in fallback files are not removed.

    \sa remove(), setFallbacksEnabled()
*/
void QCoreSettings::clear()
{
    d->clear();
    d->requestUpdate();
}

/*!

*/
void QCoreSettings::sync()
{
    d->sync();
}

QString QCoreSettings::path() const
{
    return d->path();
}

/*!

*/
QCoreSettings::Status QCoreSettings::status() const
{
    return d->status;
}

/*!

    \sa endGroup(), group()
*/
void QCoreSettings::beginGroup(const QString &prefix)
{
    d->beginGroupOrArray(QSettingsGroup(d->normalizedKey(prefix)));
}

/*!

    \sa beginGroup(), group()
*/
void QCoreSettings::endGroup()
{
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

    \sa beginGroup(), endGroup()
*/
QString QCoreSettings::group() const
{
    return d->groupPrefix.left(d->groupPrefix.size() - 1);
}

/*!

    \sa beginWriteArray(), endArray(), setArrayIndex()
*/
int QCoreSettings::beginReadArray(const QString &prefix)
{
    d->beginGroupOrArray(QSettingsGroup(d->normalizedKey(prefix), false));
    return value("size").toInt();
}

/*!

    \sa beginReadArray(), endArray(), setArrayIndex()
*/
void QCoreSettings::beginWriteArray(const QString &prefix, int size)
{
    d->beginGroupOrArray(QSettingsGroup(d->normalizedKey(prefix), size < 0));

    if (size < 0)
        remove(QLatin1String("size"));
    else
        setValue(QLatin1String("size"), size);
}

/*!

    \sa beginReadArray(), beginWriteArray()
*/
void QCoreSettings::endArray()
{
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

    \sa beginReadArray(), beginWriteArray()
*/
void QCoreSettings::setArrayIndex(int i)
{
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

*/
QStringList QCoreSettings::allKeys() const
{
    return d->children(d->groupPrefix, QCoreSettingsPrivate::AllKeys);
}

QStringList QCoreSettings::childKeys() const
{
    return d->children(d->groupPrefix, QCoreSettingsPrivate::ChildKeys);
}

QStringList QCoreSettings::childGroups() const
{
    return d->children(d->groupPrefix, QCoreSettingsPrivate::ChildGroups);
}

/*!

*/
bool QCoreSettings::isWritable() const
{
    return d->isWritable();
}

/*!

*/
void QCoreSettings::setValue(const QString &key, const QCoreVariant &value)
{
    QString k = d->actualKey(key);
    d->set(k, value);
    d->requestUpdate();
}

/*!

*/
void QCoreSettings::remove(const QString &key)
{
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

*/
bool QCoreSettings::contains(const QString &key) const
{
    QString k = d->actualKey(key);
    return d->get(k, 0);
}

/*!

*/
void QCoreSettings::setFallbacksEnabled(bool b)
{
    d->fallbacks = !!b;
}

/*!

*/
bool QCoreSettings::fallbacksEnabled() const
{
    return d->fallbacks;
}

/*!
    \reimp
*/
bool QCoreSettings::event(QEvent *event)
{
    if (event->type() == QEvent::UpdateRequest) {
        sync();
        d->pendingChanges = false;
        return true;
    }
    return QObject::event(event);
}

/*!

*/
QCoreVariant QCoreSettings::value(const QString &key, const QCoreVariant &defaultValue) const
{
    QCoreVariant result = defaultValue;
    QString k = d->actualKey(key);
    d->get(k, &result);
    return result;
}

#endif // QT_NO_SETTINGS
