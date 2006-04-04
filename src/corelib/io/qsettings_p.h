/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSETTINGS_P_H
#define QSETTINGS_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "QtCore/qdatetime.h"
#include "QtCore/qmap.h"
#include "QtCore/qmutex.h"
#include "QtCore/qiodevice.h"
#include "QtCore/qstack.h"
#include "QtCore/qstringlist.h"
#ifndef QT_NO_QOBJECT
#include "private/qobject_p.h"
#endif

#ifdef Q_OS_WIN
#include "QtCore/qt_windows.h"
#endif

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

typedef QMap<QSettingsKey, QVariant> InternalSettingsMap;

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
    InternalSettingsMap mergedKeyMap() const;

    static QConfFile *fromName(const QString &name, bool _userPerms);
    static void clearCache();

    QString name;
    QDateTime timeStamp;
    qint64 size;
    InternalSettingsMap originalKeys;
    InternalSettingsMap addedKeys;
    InternalSettingsMap removedKeys;
    QAtomic ref;
    QMutex mutex;
    bool userPerms;

private:
#ifdef Q_DISABLE_COPY
    QConfFile(const QConfFile &);
    QConfFile &operator=(const QConfFile &);
#endif
    QConfFile(const QString &name, bool _userPerms);
    friend class QConfFile_createsItself; // supress compiler warning
};

class Q_INTERNAL_EXPORT QSettingsPrivate
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
    virtual void set(const QString &key, const QVariant &value) = 0;
    virtual bool get(const QString &key, QVariant *value) const = 0;

    enum ChildSpec { AllKeys, ChildKeys, ChildGroups };
    virtual QStringList children(const QString &prefix, ChildSpec spec) const = 0;

    virtual void clear() = 0;
    virtual void sync() = 0;
    virtual void flush() = 0;
    virtual bool isWritable() const = 0;
    virtual QString fileName() const = 0;

    QString actualKey(const QString &key) const;
    void beginGroupOrArray(const QSettingsGroup &group);
    void setStatus(QSettings::Status status);
    void requestUpdate();
    void update();

    static QString normalizedKey(const QString &key);
    static QSettingsPrivate *create(QSettings::Format format, QSettings::Scope scope,
                                        const QString &organization, const QString &application);
    static QSettingsPrivate *create(const QString &fileName, QSettings::Format format);

    static void processChild(QString key, ChildSpec spec, QMap<QString, QString> &result);

    // Variant streaming functions
    static QStringList variantListToStringList(const QVariantList &l);
    static QVariant stringListToVariantList(const QStringList &l);

    // parser functions
    static QString &escapedLeadingAt(QString &s);
    static QString &unescapedLeadingAt(QString &s);
    static QString variantToString(const QVariant &v);
    static QVariant stringToVariant(const QString &s);
    static void iniEscapedKey(const QString &key, QByteArray &result);
    static bool iniUnescapedKey(const QByteArray &key, int from, int to, QString &result);
    static void iniEscapedString(const QString &str, QByteArray &result);
    static void iniChopTrailingSpaces(QString *str);
    static void iniEscapedStringList(const QStringList &strs, QByteArray &result);
    static QStringList *iniUnescapedStringList(const QByteArray &str, int from, int to,
                                                QString &result);
    static QStringList splitArgs(const QString &s, int idx);

    /*
    The numeric values of these enums define their search order. For example,
    F_User | F_Organization is searched before F_System | F_Application,
    because their values are respectively 1 and 2.
    */
    enum {
       F_Application = 0x0,
       F_Organization = 0x1,
       F_User = 0x0,
       F_System = 0x2,
       NumConfFiles = 4
    };

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
    QConfFileSettingsPrivate(QSettings::Format format, QSettings::Scope scope,
                             const QString &organization, const QString &application);
    QConfFileSettingsPrivate(const QString &fileName, QSettings::Format format);
    ~QConfFileSettingsPrivate();

    void remove(const QString &key);
    void set(const QString &key, const QVariant &value);
    bool get(const QString &key, QVariant *value) const;

    QStringList children(const QString &prefix, ChildSpec spec) const;

    void clear();
    void sync();
    void flush();
    bool isWritable() const;
    QString fileName() const;

    static bool readIniLine(QIODevice &device, QByteArray &line, int &len, int &equalsCharPos);
    static bool readIniFile(QIODevice &device, InternalSettingsMap *map);

private:
    void initFormat();
    void initAccess();
    void syncConfFile(int confFileNo);
    bool writeIniFile(QIODevice &device, const InternalSettingsMap &map);
#ifdef Q_OS_MAC
    bool readPlistFile(const QString &fileName, InternalSettingsMap *map) const;
    bool writePlistFile(const QString &fileName, const InternalSettingsMap &map) const;
#endif

    QConfFile *confFiles[NumConfFiles];
    QSettings::Format format;
    QSettings::ReadFunc readFunc;
    QSettings::WriteFunc writeFunc;
    QString extension;
    Qt::CaseSensitivity caseSensitivity;
};

#endif // QSETTINGS_P_H
