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

#include "qsettings.h"
#include "qsettings_p.h"
#include <private/qcore_mac_p.h>
#include <qdatetime.h>
#include <qvarlengtharray.h>

static const CFStringRef hostNames[2] = { kCFPreferencesCurrentHost, kCFPreferencesAnyHost };
static const int numHostNames = 2;

/*
    On the Mac, it is more natural to use '.' as the key separator
    than '/'. Therefore, it makes sense to replace '/' with '.' and
    vice versa.
*/
static QString swapSlashesAndDots(const QString &key)
{
    QString result = key;
    for (int i = 0; i < result.size(); ++i) {
        if (result.at(i) == QLatin1Char('/'))
            result[i] = QLatin1Char('.');
        else if (result.at(i) == QLatin1Char('.'))
            result[i] = QLatin1Char('/');
    }
    return result;
}

static QCFType<CFStringRef> macKey(const QString &key)
{
    return QCFString::toCFStringRef(swapSlashesAndDots(key));
}

static QString qtKey(CFStringRef cfkey)
{
    return swapSlashesAndDots(QCFString::toQString(cfkey));
}

static QCFType<CFPropertyListRef> macValue(const QCoreVariant &value);

static CFArrayRef macList(const QList<QCoreVariant> &list)
{
    int n = list.size();
    QVarLengthArray<QCFType<CFPropertyListRef> > cfvalues(n);
    for (int i = 0; i < n; ++i)
        cfvalues[i] = macValue(list.at(i));
    return CFArrayCreate(kCFAllocatorDefault, reinterpret_cast<const void **>(cfvalues.data()),
                         CFIndex(n), &kCFTypeArrayCallBacks);
}

static QCFType<CFPropertyListRef> macValue(const QCoreVariant &value)
{
    CFPropertyListRef result;

    switch (value.type()) {
    case QCoreVariant::ByteArray:
        {
            QByteArray ba = value.toByteArray();
            result = CFDataCreate(kCFAllocatorDefault, reinterpret_cast<const UInt8 *>(ba.data()),
                                  CFIndex(ba.size()));
        }
        break;
    case QCoreVariant::List:
    case QCoreVariant::StringList:
    case QCoreVariant::Polygon:
        result = macList(value.toList());
        break;
    case QCoreVariant::Map:
        {
            /*
                QMap<QString, QCoreVariant> is potentially a multimap,
                whereas CFDictionary is a single-valued map. To allow
                for multiple values with the same key, we store
                each CFDictonary value as a CFArray.
            */
            QMap<QString, QCoreVariant> map = value.toMap();
            QMap<QString, QCoreVariant>::const_iterator i = map.constBegin();

            int maxUniqueKeys = map.size();
            int numUniqueKeys = 0;
            QVarLengthArray<QCFType<CFPropertyListRef> > cfkeys(maxUniqueKeys);
            QVarLengthArray<QCFType<CFPropertyListRef> > cfvalues(maxUniqueKeys);

            while (i != map.constEnd()) {
                const QString &key = i.key();
                QList<QCoreVariant> values;

                do {
                    values << i.value();
                    ++i;
                } while (i != map.constEnd() && i.key() == key);

                cfkeys[numUniqueKeys] = QCFString::toCFStringRef(key);
                cfvalues[numUniqueKeys] = macList(values);
                ++numUniqueKeys;
            }

            result = CFDictionaryCreate(kCFAllocatorDefault,
                                        reinterpret_cast<const void **>(cfkeys.data()),
                                        reinterpret_cast<const void **>(cfvalues.data()),
                                        CFIndex(numUniqueKeys),
                                        &kCFTypeDictionaryKeyCallBacks,
                                        &kCFTypeDictionaryValueCallBacks);
        }
        break;
    case QCoreVariant::DateTime:
        {
            /*
                CFDate, unlike QDateTime, doesn't store timezone information.
            */
            QDateTime dt = value.toDateTime();
            if (dt.timeSpec() == Qt::LocalTime) {
                QDateTime reference;
                reference.setTime_t((uint)kCFAbsoluteTimeIntervalSince1970);
                result = CFDateCreate(kCFAllocatorDefault, CFAbsoluteTime(reference.secsTo(dt)));
            } else {
                goto string_case;
            }
        }
        break;
    case QCoreVariant::Bool:
        result = value.toBool() ? kCFBooleanTrue : kCFBooleanFalse;
        break;
    case QCoreVariant::Int:
    case QCoreVariant::UInt:
        {
            int n = value.toInt();
            result = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &n);
        }
        break;
    case QCoreVariant::Double:
        {
            double n = value.toDouble();
            result = CFNumberCreate(kCFAllocatorDefault, kCFNumberDoubleType, &n);
        }
        break;
    case QCoreVariant::LongLong:
    case QCoreVariant::ULongLong:
        {
            Q_LONGLONG n = value.toLongLong();
            result = CFNumberCreate(0, kCFNumberLongLongType, &n);
        }
        break;
    case QCoreVariant::String:
    string_case:
    default:
        result = QCFString::toCFStringRef(QSettingsPrivate::variantToString(value));
    }
    return result;
}

static QCoreVariant qtValue(CFPropertyListRef cfvalue)
{
    if (!cfvalue)
        return QCoreVariant();

    CFTypeID typeId = CFGetTypeID(cfvalue);

    /*
        Sorted grossly from most to least frequent type.
    */
    if (typeId == CFStringGetTypeID()) {
        return QSettingsPrivate::stringToVariant(QCFString::toQString(static_cast<CFStringRef>(cfvalue)));
    } else if (typeId == CFNumberGetTypeID()) {
        CFNumberRef cfnumber = static_cast<CFNumberRef>(cfvalue);
        if (CFNumberIsFloatType(cfnumber)) {
            double d;
            CFNumberGetValue(cfnumber, kCFNumberDoubleType, &d);
            return d;
        } else {
            int i;
            Q_LONGLONG ll;

            if (CFNumberGetValue(cfnumber, kCFNumberIntType, &i))
                return i;
            CFNumberGetValue(cfnumber, kCFNumberLongLongType, &ll);
            return ll;
        }
    } else if (typeId == CFArrayGetTypeID()) {
        CFArrayRef cfarray = static_cast<CFArrayRef>(cfvalue);
        QList<QCoreVariant> list;
        CFIndex size = CFArrayGetCount(cfarray);
        for (CFIndex i = 0; i < size; ++i)
            list << qtValue(CFArrayGetValueAtIndex(cfarray, i));
        return list;
    } else if (typeId == CFBooleanGetTypeID()) {
        return (bool)CFBooleanGetValue(static_cast<CFBooleanRef>(cfvalue));
    } else if (typeId == CFDataGetTypeID()) {
        CFDataRef cfdata = static_cast<CFDataRef>(cfvalue);
        return QByteArray(reinterpret_cast<const char *>(CFDataGetBytePtr(cfdata)),
                          CFDataGetLength(cfdata));
    } else if (typeId == CFDictionaryGetTypeID()) {
        CFDictionaryRef cfdict = static_cast<CFDictionaryRef>(cfvalue);
        CFTypeID arrayTypeId = CFArrayGetTypeID();
        int size = (int)CFDictionaryGetCount(cfdict);
        QVarLengthArray<CFPropertyListRef> keys(size);
        QVarLengthArray<CFPropertyListRef> values(size);
        CFDictionaryGetKeysAndValues(cfdict, keys.data(), values.data());

        QMultiMap<QString, QCoreVariant> map;
        for (int i = 0; i < size; ++i) {
            QString key = QCFString::toQString(static_cast<CFStringRef>(keys[i]));

            if (CFGetTypeID(values[i]) == arrayTypeId) {
                CFArrayRef cfarray = static_cast<CFArrayRef>(values[i]);
                CFIndex arraySize = CFArrayGetCount(cfarray);
                for (CFIndex j = arraySize - 1; j >= 0; --j)
                    map.insert(key, qtValue(CFArrayGetValueAtIndex(cfarray, j)));
            } else {
                map.insert(key, qtValue(values[i]));
            }
        }
        return map;
    } else if (typeId == CFDateGetTypeID()) {
        QDateTime dt;
        dt.setTime_t((uint)kCFAbsoluteTimeIntervalSince1970);
        return dt.addSecs((int)CFDateGetAbsoluteTime(static_cast<CFDateRef>(cfvalue)));
    }
    return QCoreVariant();
}

class QMacSettingsPrivate : public QSettingsPrivate
{
public:
    QMacSettingsPrivate(QSettings::Scope scope, const QString &organization,
                        const QString &application);
    ~QMacSettingsPrivate();

    void remove(const QString &key);
    void set(const QString &key, const QCoreVariant &value);
    bool get(const QString &key, QCoreVariant *value) const;
    QStringList children(const QString &prefix, ChildSpec spec) const;
    void clear();
    void sync();
    void flush();
    bool isWritable() const;

private:
    struct SearchDomain
    {
        CFStringRef userName;
        CFStringRef applicationOrSuiteId;
    };

    QCFString applicationId;
    QCFString suiteId;
    QCFString hostName;
    SearchDomain domains[6];
    int numDomains;
};

QMacSettingsPrivate::QMacSettingsPrivate(QSettings::Scope scope, const QString &organization,
                                         const QString &application)
{
    QString javaPackageName;
    int curPos = 0;
    int nextDot;

    QString org = organization;
    if (org.isEmpty()) {
        setStatus(QSettings::AccessError);
        org = QLatin1String("unknown-organization.trolltech.com");
    }

    while ((nextDot = org.indexOf(QLatin1Char('.'), curPos)) != -1) {
        javaPackageName.prepend(org.mid(curPos, nextDot - curPos));
        javaPackageName.prepend(QLatin1Char('.'));
        curPos = nextDot + 1;
    }
    javaPackageName.prepend(org.mid(curPos));
    javaPackageName = javaPackageName.toLower();
    if (curPos == 0)
        javaPackageName.prepend(QLatin1String("com."));
    suiteId = javaPackageName;

    if (scope == QSettings::SystemScope)
        spec |= F_Global;

    if (application.isEmpty()) {
        spec |= F_Organization;
    } else {
        javaPackageName += QLatin1Char('.');
        javaPackageName += application;
        applicationId = javaPackageName;
    }

    numDomains = 0;
    for (int i = (spec & F_Global) ? 1 : 0; i < 2; ++i) {
        for (int j = (spec & F_Organization) ? 1 : 0; j < 3; ++j) {
            SearchDomain &domain = domains[numDomains++];
            domain.userName = (i == 0) ? kCFPreferencesCurrentUser : kCFPreferencesAnyUser;
            if (j == 0)
                domain.applicationOrSuiteId = applicationId;
            else if (j == 1)
                domain.applicationOrSuiteId = suiteId;
            else
                domain.applicationOrSuiteId = kCFPreferencesAnyApplication;
        }
    }

    hostName = (scope == QSettings::SystemScope) ? kCFPreferencesCurrentHost : kCFPreferencesAnyHost;
}

QMacSettingsPrivate::~QMacSettingsPrivate()
{
}

void QMacSettingsPrivate::remove(const QString &key)
{
    QStringList keys = children(key + QLatin1Char('/'), AllKeys);

    // If i == -1, then delete "key" itself.
    for (int i = -1; i < keys.size(); ++i) {
        QString subKey = key;
        if (i >= 0) {
            subKey += QLatin1Char('/');
            subKey += keys.at(i);
        }
        CFPreferencesSetValue(macKey(subKey), 0, domains[0].applicationOrSuiteId,
                              domains[0].userName, hostName);
    }
}

void QMacSettingsPrivate::set(const QString &key, const QCoreVariant &value)
{
    CFPreferencesSetValue(macKey(key), macValue(value), domains[0].applicationOrSuiteId,
                          domains[0].userName, hostName);
}

bool QMacSettingsPrivate::get(const QString &key, QCoreVariant *value) const
{
    QCFString k = macKey(key);
    for (int i = 0; i < numDomains; ++i) {
        for (int j = 0; j < numHostNames; ++j) {
            QCFType<CFPropertyListRef> ret =
                    CFPreferencesCopyValue(k, domains[i].applicationOrSuiteId, domains[i].userName,
                                           hostNames[j]);
            if (ret) {
                if (value)
                    *value = qtValue(ret);
                return true;
            }
        }

        if (!fallbacks)
            break;
    }
    return false;
}

QStringList QMacSettingsPrivate::children(const QString &prefix, ChildSpec spec) const
{
    QMap<QString, QString> result;
    int startPos = prefix.size();

    for (int i = 0; i < numDomains; ++i) {
        for (int j = 0; j < numHostNames; ++j) {
            QCFType<CFArrayRef> cfarray = CFPreferencesCopyKeyList(domains[i].applicationOrSuiteId,
                                                                   domains[i].userName,
                                                                   hostNames[j]);
            if (cfarray) {
                CFIndex size = CFArrayGetCount(cfarray);
                for (CFIndex k = 0; k < size; ++k) {
                    QString currentKey =
                            qtKey(static_cast<CFStringRef>(CFArrayGetValueAtIndex(cfarray, k)));
                    if (currentKey.startsWith(prefix))
                        processChild(currentKey.mid(startPos), spec, result);
                }
            }
        }

        if (!fallbacks)
            break;
    }
    return result.keys();
}

void QMacSettingsPrivate::clear()
{
    QCFType<CFArrayRef> cfarray = CFPreferencesCopyKeyList(domains[0].applicationOrSuiteId,
                                                           domains[0].userName, hostName);
    CFPreferencesSetMultiple(0, cfarray, domains[0].applicationOrSuiteId, domains[0].userName,
                             hostName);
}

void QMacSettingsPrivate::sync()
{
    for (int i = 0; i < numDomains; ++i) {
        for (int j = 0; j < numHostNames; ++j) {
            CFPreferencesSynchronize(domains[i].applicationOrSuiteId, domains[i].userName,
                                     hostNames[j]);
        }
    }
}

void QMacSettingsPrivate::flush()
{
    sync();
}

bool QMacSettingsPrivate::isWritable() const
{
    return true; // ###
}

QSettingsPrivate *QSettingsPrivate::create(QSettings::Format format,
                                           QSettings::Scope scope,
                                           const QString &organization,
                                           const QString &application)
{
    if (format == QSettings::NativeFormat) {
        QMacSettingsPrivate *p = new QMacSettingsPrivate(scope, organization, application);
        return p;
    } else {
        QConfFileSettingsPrivate *p = new QConfFileSettingsPrivate(format, scope,
                                                                   organization, application);
        p->init();
        return p;
    }
}

static QCFType<CFURLRef> urlFromFileName(const QString &fileName)
{
    return CFURLCreateWithFileSystemPath(kCFAllocatorDefault, QCFString(fileName),
                                         kCFURLPOSIXPathStyle, false);
}

bool QConfFileSettingsPrivate::readPlistFile(const QString &fileName, SettingsKeyMap *map) const
{
    QCFType<CFDataRef> resource;
    SInt32 code;
    if (!CFURLCreateDataAndPropertiesFromResource(kCFAllocatorDefault, urlFromFileName(fileName),
                                                  &resource, 0, 0, &code))
        return false;

    QCFString errorStr;
    QCFType<CFPropertyListRef> propertyList =
            CFPropertyListCreateFromXMLData(kCFAllocatorDefault, resource, kCFPropertyListImmutable,
                                            &errorStr);

    if (!propertyList)
        return true;
    if (CFGetTypeID(propertyList) != CFDictionaryGetTypeID())
        return false;

    CFDictionaryRef cfdict =
            static_cast<CFDictionaryRef>(static_cast<CFPropertyListRef>(propertyList));
    int size = (int)CFDictionaryGetCount(cfdict);
    QVarLengthArray<CFPropertyListRef> keys(size);
    QVarLengthArray<CFPropertyListRef> values(size);
    CFDictionaryGetKeysAndValues(cfdict, keys.data(), values.data());

    for (int i = 0; i < size; ++i) {
        QString key = qtKey(static_cast<CFStringRef>(keys[i]));
        map->insert(QSettingsKey(key, Qt::CaseSensitive), qtValue(values[i]));
    }
    return true;
}

bool QConfFileSettingsPrivate::writePlistFile(const QString &fileName,
                                              const SettingsKeyMap &map) const
{
    QVarLengthArray<QCFType<CFStringRef> > cfkeys(map.size());
    QVarLengthArray<QCFType<CFPropertyListRef> > cfvalues(map.size());
    int i = 0;
    SettingsKeyMap::const_iterator j;
    for (j = map.constBegin(); j != map.constEnd(); ++j) {
        cfkeys[i] = macKey(j.key());
        cfvalues[i] = macValue(j.value());
        ++i;
    }

    QCFType<CFDictionaryRef> propertyList =
            CFDictionaryCreate(kCFAllocatorDefault,
                               reinterpret_cast<const void **>(cfkeys.data()),
                               reinterpret_cast<const void **>(cfvalues.data()),
                               CFIndex(map.size()),
                               &kCFTypeDictionaryKeyCallBacks,
                               &kCFTypeDictionaryValueCallBacks);

    QCFType<CFDataRef> xmlData = CFPropertyListCreateXMLData(kCFAllocatorDefault, propertyList);

    SInt32 code;
    return CFURLWriteDataAndPropertiesToResource(urlFromFileName(fileName), xmlData, 0, &code);
}
