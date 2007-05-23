/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qscriptecmadate_p.h"

#ifndef QT_NO_SCRIPT

#include "qscriptengine_p.h"
#include "qscriptvalueimpl_p.h"
#include "qscriptcontext_p.h"
#include "qscriptmember_p.h"
#include "qscriptobject_p.h"

#include <QtCore/QDateTime>
#include <QtCore/QRegExp>
#include <QtCore/QtDebug>
#include <QtCore/QLocale>
#include <QtCore/qnumeric.h>

#include <math.h>

#ifndef Q_WS_WIN
#include <time.h>
#include <sys/time.h>
#else
#include <windows.h>
#endif

static const qsreal HoursPerDay = 24.0;
static const qsreal MinutesPerHour = 60.0;
static const qsreal SecondsPerMinute = 60.0;
static const qsreal msPerSecond = 1000.0;
static const qsreal msPerMinute = 60000.0;
static const qsreal msPerHour = 3600000.0;
static const qsreal msPerDay = 86400000.0;

static qsreal LocalTZA = 0.0; // initialized at startup

static inline qsreal TimeWithinDay(qsreal t)
{
    qsreal r = ::fmod(t, msPerDay);
    return (r >= 0) ? r : r + msPerDay;
}

static inline int HourFromTime(qsreal t)
{
    int r = int(::fmod(::floor(t / msPerHour), HoursPerDay));
    return (r >= 0) ? r : r + int(HoursPerDay);
}

static inline int MinFromTime(qsreal t)
{
    int r = int(::fmod(::floor(t / msPerMinute), MinutesPerHour));
    return (r >= 0) ? r : r + int(MinutesPerHour);
}

static inline int SecFromTime(qsreal t)
{
    int r = int(::fmod(::floor(t / msPerSecond), SecondsPerMinute));
    return (r >= 0) ? r : r + int(SecondsPerMinute);
}

static inline int msFromTime(qsreal t)
{
    int r = int(::fmod(t, msPerSecond));
    return (r >= 0) ? r : r + int(msPerSecond);
}

static inline qsreal Day(qsreal t)
{
    return ::floor(t / msPerDay);
}

static inline qsreal DaysInYear(qsreal y)
{
    if (::fmod(y, 4))
        return 365;

    else if (::fmod(y, 100))
        return 366;

    else if (::fmod(y, 400))
        return 365;

    return 366;
}

static inline qsreal DayFromYear(qsreal y)
{
    return 365 * (y - 1970)
        + ::floor((y - 1969) / 4)
        - ::floor((y - 1901) / 100)
        + ::floor((y - 1601) / 400);
}

static inline qsreal TimeFromYear(qsreal y)
{
    return msPerDay * DayFromYear(y);
}

static inline qsreal YearFromTime(qsreal t)
{
    int y = 1970;
    y += (int) ::floor(t / (msPerDay * 365.2425));

    qsreal t2 = TimeFromYear(y);
    return (t2 > t) ? y - 1 : ((t2 + msPerDay * DaysInYear(y)) <= t) ? y + 1 : y;
}

static inline bool InLeapYear(qsreal t)
{
    qsreal x = DaysInYear(YearFromTime(t));
    if (x == 365)
        return 0;

    Q_ASSERT (x == 366);
    return 1;
}

static inline qsreal DayWithinYear(qsreal t)
{
    return Day(t) - DayFromYear(YearFromTime(t));
}

static inline qsreal MonthFromTime(qsreal t)
{
    qsreal d = DayWithinYear(t);
    qsreal l = InLeapYear(t);

    if (d < 31.0)
        return 0;

    else if (d < 59.0 + l)
        return 1;

    else if (d < 90.0 + l)
        return 2;

    else if (d < 120.0 + l)
        return 3;

    else if (d < 151.0 + l)
        return 4;

    else if (d < 181.0 + l)
        return 5;

    else if (d < 212.0 + l)
        return 6;

    else if (d < 243.0 + l)
        return 7;

    else if (d < 273.0 + l)
        return 8;

    else if (d < 304.0 + l)
        return 9;

    else if (d < 334.0 + l)
        return 10;

    else if (d < 365.0 + l)
        return 11;

    return qSNaN(); // ### assert?
}

static inline qsreal DateFromTime(qsreal t)
{
    int m = (int) QScriptEnginePrivate::toInteger(MonthFromTime(t));
    qsreal d = DayWithinYear(t);
    qsreal l = InLeapYear(t);

    switch (m) {
    case 0: return d + 1.0;
    case 1: return d - 30.0;
    case 2: return d - 58.0 - l;
    case 3: return d - 89.0 - l;
    case 4: return d - 119.0 - l;
    case 5: return d - 150.0 - l;
    case 6: return d - 180.0 - l;
    case 7: return d - 211.0 - l;
    case 8: return d - 242.0 - l;
    case 9: return d - 272.0 - l;
    case 10: return d - 303.0 - l;
    case 11: return d - 333.0 - l;
    }

    return qSNaN(); // ### assert
}

static inline qsreal WeekDay(qsreal t)
{
    qsreal r = ::fmod (Day(t) + 4.0, 7.0);
    return (r >= 0) ? r : r + 7.0;
}


static inline qsreal MakeTime(qsreal hour, qsreal min, qsreal sec, qsreal ms)
{
    return ((hour * MinutesPerHour + min) * SecondsPerMinute + sec) * msPerSecond + ms;
}

static inline qsreal DayFromMonth(qsreal month, qsreal leap)
{
    switch ((int) month) {
    case 0: return 0;
    case 1: return 31.0;
    case 2: return 59.0 + leap;
    case 3: return 90.0 + leap;
    case 4: return 120.0 + leap;
    case 5: return 151.0 + leap;
    case 6: return 181.0 + leap;
    case 7: return 212.0 + leap;
    case 8: return 243.0 + leap;
    case 9: return 273.0 + leap;
    case 10: return 304.0 + leap;
    case 11: return 334.0 + leap;
    }

    return qSNaN(); // ### assert?
}

static qsreal MakeDay(qsreal year, qsreal month, qsreal day)
{
    year += ::floor(month / 12.0);

    month = ::fmod(month, 12.0);
    if (month < 0)
        month += 12.0;

    qsreal t = TimeFromYear(year);
    qsreal leap = InLeapYear(t);

    day += ::floor(t / msPerDay);
    day += DayFromMonth(month, leap);

    return day - 1;
}

static inline qsreal MakeDate(qsreal day, qsreal time)
{
    return day * msPerDay + time;
}

static inline qsreal DaylightSavingTA(double t)
{
#ifndef Q_WS_WIN
    long int tt = (long int)(t / msPerSecond);
    struct tm *tmtm = localtime((const time_t*)&tt);
    if (! tmtm)
        return 0;
    return (tmtm->tm_isdst > 0) ? msPerHour : 0;
#else
    Q_UNUSED(t);
    /// ### implement me
    return 0;
#endif
}

static inline qsreal LocalTime(qsreal t)
{
    return t + LocalTZA + DaylightSavingTA(t);
}

static inline qsreal UTC(qsreal t)
{
    return t - LocalTZA - DaylightSavingTA(t - LocalTZA);
}

static inline qsreal currentTime()
{
#ifndef Q_WS_WIN
    struct timeval tv;

    gettimeofday(&tv, 0);
    return ::floor(tv.tv_sec * msPerSecond + (tv.tv_usec / 1000.0));
#else
    SYSTEMTIME st;
    GetSystemTime(&st);
    FILETIME ft;
    SystemTimeToFileTime(&st, &ft);
    LARGE_INTEGER li;
    li.LowPart = ft.dwLowDateTime;
    li.HighPart = ft.dwHighDateTime;
    return double(li.QuadPart - Q_INT64_C(116444736000000000)) / 10000.0;
#endif
}

static inline qsreal TimeClip(qsreal t)
{
    if (! qIsFinite(t) || fabs(t) > 8.64e15)
        return qSNaN();
    return QScriptEnginePrivate::toInteger(t);
}

static inline qsreal ParseString(const QString &s)
{
    QDateTime dt = QDateTime::fromString(s);
    int year = dt.date().year();
    int month = dt.date().month();
    int day = dt.date().day();
    int hours = dt.time().hour();
    int mins = dt.time().minute();
    int secs = dt.time().second();
    int ms = dt.time().msec();
    double t = MakeDate(MakeDay(year, month, day), MakeTime(hours, mins, secs, ms));
    return t = TimeClip(UTC(t));
}

static inline QDateTime ToDateTime(qsreal t)
{
    if (qIsNaN(t))
        return QDateTime();
    int year = int(YearFromTime(t));
    int month = int(MonthFromTime(t) + 1);
    int day = int(DateFromTime(t));
    int hours = HourFromTime(t);
    int mins = MinFromTime(t);
    int secs = SecFromTime(t);
    int ms = msFromTime(t);
    return QDateTime(QDate(year, month, day), QTime(hours, mins, secs, ms));
}

static inline qsreal FromDateTime(const QDateTime &dt)
{
    QDate date = dt.date();
    QTime taim = dt.time();
    int year = date.year();
    int month = date.month() - 1;
    int day = date.day();
    int hours = taim.hour();
    int mins = taim.minute();
    int secs = taim.second();
    int ms = taim.msec();
    return MakeDate(MakeDay(year, month, day),
                    MakeTime(hours, mins, secs, ms));
}

static inline QString ToString(qsreal t)
{
    t = LocalTime(t);
    return ToDateTime(t).toString();
}

static inline QString ToUTCString(qsreal t)
{
    return ToDateTime(t).toString();
}

static inline QString ToDateString(qsreal t)
{
    t = LocalTime(t);
    return ToDateTime(t).date().toString();
}

static inline QString ToTimeString(qsreal t)
{
    t = LocalTime(t);
    return ToDateTime(t).time().toString();
}

static inline QString ToLocaleString(qsreal t)
{
    t = LocalTime(t);
    return ToDateTime(t).toString(Qt::LocaleDate);
}

static inline QString ToLocaleDateString(qsreal t)
{
    t = LocalTime(t);
    return ToDateTime(t).date().toString(Qt::LocaleDate);
}

static inline QString ToLocaleTimeString(qsreal t)
{
    t = LocalTime(t);
    return ToDateTime(t).time().toString(Qt::LocaleDate);
}

static qsreal getLocalTZA()
{
#ifndef Q_WS_WIN
    struct tm* t;
    time_t curr;
    time(&curr);
    t = localtime(&curr);
    time_t locl = mktime(t);
    t = gmtime(&curr);
    time_t globl = mktime(t);
    return double(locl - globl) * 1000.0;
#else
    TIME_ZONE_INFORMATION tzInfo;
    GetTimeZoneInformation(&tzInfo);
    return tzInfo.Bias * 60.0 * 1000.0;
#endif
}

namespace QScript { namespace Ecma {


Date::Date(QScriptEnginePrivate *eng):
    Core(eng)
{
    LocalTZA = getLocalTZA();

    m_classInfo = eng->registerClass(QLatin1String("Date"));

    publicPrototype.invalidate();
    newDate(&publicPrototype, qSNaN());

    const QScriptValue::PropertyFlags flags = QScriptValue::SkipInEnumeration;

    eng->newConstructor(&ctor, this, publicPrototype);
    ctor.setProperty(QLatin1String("parse"),
                     eng->createFunction(method_parse, 1, m_classInfo), flags);
    ctor.setProperty(QLatin1String("UTC"),
                     eng->createFunction(method_UTC, 1, m_classInfo), flags);

    publicPrototype.setProperty(QLatin1String("toString"),
                                eng->createFunction(method_toString, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("toDateString"),
                                eng->createFunction(method_toDateString, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("toTimeString"),
                                eng->createFunction(method_toTimeString, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("toLocaleString"),
                                eng->createFunction(method_toLocaleString, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("toLocaleDateString"),
                                eng->createFunction(method_toLocaleDateString, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("toLocaleTimeString"),
                                eng->createFunction(method_toLocaleTimeString, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("valueOf"),
                                eng->createFunction(method_valueOf, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("getTime"),
                                eng->createFunction(method_getTime, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("getYear"),
                                eng->createFunction(method_getYear, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("getFullYear"),
                                eng->createFunction(method_getFullYear, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("getUTCFullYear"),
                                eng->createFunction(method_getUTCFullYear, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("getMonth"),
                                eng->createFunction(method_getMonth, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("getUTCMonth"),
                                eng->createFunction(method_getUTCMonth, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("getDate"),
                                eng->createFunction(method_getDate, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("getUTCDate"),
                                eng->createFunction(method_getUTCDate, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("getDay"),
                                eng->createFunction(method_getDay, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("getUTCDay"),
                                eng->createFunction(method_getUTCDay, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("getHours"),
                                eng->createFunction(method_getHours, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("getUTCHours"),
                                eng->createFunction(method_getUTCHours, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("getMinutes"),
                                eng->createFunction(method_getMinutes, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("getUTCMinutes"),
                                eng->createFunction(method_getUTCMinutes, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("getSeconds"),
                                eng->createFunction(method_getSeconds, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("getUTCSeconds"),
                                eng->createFunction(method_getUTCSeconds, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("getMilliseconds"),
                                eng->createFunction(method_getMilliseconds, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("getUTCMilliseconds"),
                                eng->createFunction(method_getUTCMilliseconds, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("getTimezoneOffset"),
                                eng->createFunction(method_getTimezoneOffset, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("setTime"),
                                eng->createFunction(method_setTime, 1, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("setMilliseconds"),
                                eng->createFunction(method_setMilliseconds, 1, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("setUTCMilliseconds"),
                                eng->createFunction(method_setUTCMilliseconds, 1, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("setSeconds"),
                                eng->createFunction(method_setSeconds, 2, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("setUTCSeconds"),
                                eng->createFunction(method_setUTCSeconds, 2, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("setMinutes"),
                                eng->createFunction(method_setMinutes, 3, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("setUTCMinutes"),
                                eng->createFunction(method_setUTCMinutes, 3, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("setHours"),
                                eng->createFunction(method_setHours, 4, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("setUTCHours"),
                                eng->createFunction(method_setUTCHours, 4, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("setDate"),
                                eng->createFunction(method_setDate, 1, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("setUTCDate"),
                                eng->createFunction(method_setUTCDate, 1, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("setMonth"),
                                eng->createFunction(method_setMonth, 2, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("setUTCMonth"),
                                eng->createFunction(method_setUTCMonth, 2, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("setYear"),
                                eng->createFunction(method_setYear, 1, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("setFullYear"),
                                eng->createFunction(method_setFullYear, 3, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("setUTCFullYear"),
                                eng->createFunction(method_setUTCFullYear, 3, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("toUTCString"),
                                eng->createFunction(method_toUTCString, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("toGMTString"),
                                eng->createFunction(method_toUTCString, 0, m_classInfo), flags);
}

Date::~Date()
{
}

void Date::execute(QScriptContextPrivate *context)
{
    if (!context->calledAsConstructor()) {
        double t = currentTime();
        context->setReturnValue(QScriptValueImpl(engine(), ToString(t)));
        return;
    }

    // called as constructor
    qsreal t;

    if (context->argumentCount() == 0)
        t = currentTime();

    else if (context->argumentCount() == 1) {
        QScriptValue arg = context->argument(0).toPrimitive();
        if (arg.isString())
            t = ParseString(arg.toString());
        else
            t = TimeClip(arg.toNumber());
    }

    else { // context->argumentCount() > 1
        qsreal year  = context->argument(0).toNumber();
        qsreal month = context->argument(1).toNumber();
        qsreal day  = context->argumentCount() >= 3 ? context->argument(2).toNumber() : 1;
        qsreal hours = context->argumentCount() >= 4 ? context->argument(3).toNumber() : 0;
        qsreal mins = context->argumentCount() >= 5 ? context->argument(4).toNumber() : 0;
        qsreal secs = context->argumentCount() >= 6 ? context->argument(5).toNumber() : 0;
        qsreal ms    = context->argumentCount() >= 7 ? context->argument(6).toNumber() : 0;
        if (year >= 0 && year <= 99)
            year += 1900;
        t = MakeDate(MakeDay(year, month, day), MakeTime(hours, mins, secs, ms));
        t = TimeClip(UTC(t));
    }

    QScriptValueImpl &obj = context->m_thisObject;
    obj.setClassInfo(classInfo());
    obj.setInternalValue(QScriptValueImpl(engine(), t));
    obj.setPrototype(publicPrototype);
    context->setReturnValue(obj);
}

void Date::newDate(QScriptValueImpl *result, qsreal t)
{
    engine()->newObject(result, publicPrototype, classInfo());
    result->setInternalValue(QScriptValueImpl(engine(), t));
}

void Date::newDate(QScriptValueImpl *result, const QDateTime &dt)
{
    newDate(result, FromDateTime(dt));
}

void Date::newDate(QScriptValueImpl *result, const QDate &d)
{
    newDate(result, QDateTime(d));
}

QDateTime Date::toDateTime(const QScriptValueImpl &date) const
{
    Q_ASSERT(date.classInfo() == m_classInfo);
    qsreal t = date.internalValue().toNumber();
    return ToDateTime(t);
}

QScriptValueImpl Date::method_parse(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *)
{
    return QScriptValueImpl(eng, ParseString(context->argument(0).toString()));
}

QScriptValueImpl Date::method_UTC(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *)
{
    const int numArgs = context->argumentCount();
    if (numArgs >= 2) {
        qsreal year  = context->argument(0).toNumber();
        qsreal month = context->argument(1).toNumber();
        qsreal day   = numArgs >= 3 ? context->argument(2).toNumber() : 1;
        qsreal hours = numArgs >= 4 ? context->argument(3).toNumber() : 0;
        qsreal mins  = numArgs >= 5 ? context->argument(4).toNumber() : 0;
        qsreal secs  = numArgs >= 6 ? context->argument(5).toNumber() : 0;
        qsreal ms    = numArgs >= 7 ? context->argument(6).toNumber() : 0;
        if (year >= 0 && year <= 99)
            year += 1900;
        qsreal t = MakeDate(MakeDay(year, month, day),
                            MakeTime(hours, mins, secs, ms));
        return QScriptValueImpl(eng, TimeClip(t));
    }
    return (eng->undefinedValue());
}

QScriptValueImpl Date::method_toString(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo)
{
    QScriptValueImpl self = context->thisObject();
    if (self.classInfo() == classInfo) {
        qsreal t = self.internalValue().toNumber();
        return QScriptValueImpl(eng, ToString(t));
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.toString"));
}

QScriptValueImpl Date::method_toDateString(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo)
{
    QScriptValueImpl self = context->thisObject();
    if (self.classInfo() == classInfo) {
        qsreal t = self.internalValue().toNumber();
        return QScriptValueImpl(eng, ToDateString(t));
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.toDateString"));
}

QScriptValueImpl Date::method_toTimeString(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo)
{
    QScriptValueImpl self = context->thisObject();
    if (self.classInfo() == classInfo) {
        qsreal t = self.internalValue().toNumber();
        return QScriptValueImpl(eng, ToTimeString(t));
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.toTimeString"));
}

QScriptValueImpl Date::method_toLocaleString(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo)
{
    QScriptValueImpl self = context->thisObject();
    if (self.classInfo() == classInfo) {
        qsreal t = self.internalValue().toNumber();
        return QScriptValueImpl(eng, ToLocaleString(t));
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.toLocaleString"));
}

QScriptValueImpl Date::method_toLocaleDateString(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo)
{
    QScriptValueImpl self = context->thisObject();
    if (self.classInfo() == classInfo) {
        qsreal t = self.internalValue().toNumber();
        return QScriptValueImpl(eng, ToLocaleDateString(t));
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.toLocaleDateString"));
}

QScriptValueImpl Date::method_toLocaleTimeString(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo)
{
    QScriptValueImpl self = context->thisObject();
    if (self.classInfo() == classInfo) {
        qsreal t = self.internalValue().toNumber();
        return QScriptValueImpl(eng, ToLocaleTimeString(t));
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.toLocaleTimeString"));
}

QScriptValueImpl Date::method_valueOf(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo)
{
    QScriptValueImpl self = context->thisObject();
    if (self.classInfo() == classInfo)
        return QScriptValueImpl(eng, self.internalValue().toNumber());
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.valueOf"));
}

QScriptValueImpl Date::method_getTime(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo)
{
    QScriptValueImpl self = context->thisObject();
    if (self.classInfo() == classInfo)
        return QScriptValueImpl(eng, self.internalValue().toNumber());
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.getTime"));
}

QScriptValueImpl Date::method_getYear(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo)
{
    QScriptValueImpl self = context->thisObject();
    if (self.classInfo() == classInfo) {
        qsreal t = self.internalValue().toNumber();
        if (! qIsNaN(t))
            t = YearFromTime(LocalTime(t)) - 1900;
        return QScriptValueImpl(eng, t);
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.getYear"));
}

QScriptValueImpl Date::method_getFullYear(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo)
{
    QScriptValueImpl self = context->thisObject();
    if (self.classInfo() == classInfo) {
        qsreal t = self.internalValue().toNumber();
        if (! qIsNaN(t))
            t = YearFromTime(LocalTime(t));
        return QScriptValueImpl(eng, t);
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.getFullYear"));
}

QScriptValueImpl Date::method_getUTCFullYear(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo)
{
    QScriptValueImpl self = context->thisObject();
    if (self.classInfo() == classInfo) {
        qsreal t = self.internalValue().toNumber();
        if (! qIsNaN(t))
            t = YearFromTime(t);
        return QScriptValueImpl(eng, t);
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.getUTCFullYear"));
}

QScriptValueImpl Date::method_getMonth(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo)
{
    QScriptValueImpl self = context->thisObject();
    if (self.classInfo() == classInfo) {
        qsreal t = self.internalValue().toNumber();
        if (! qIsNaN(t))
            t = MonthFromTime(LocalTime(t));
        return QScriptValueImpl(eng, t);
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.getMonth"));
}

QScriptValueImpl Date::method_getUTCMonth(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo)
{
    QScriptValueImpl self = context->thisObject();
    if (self.classInfo() == classInfo) {
        qsreal t = self.internalValue().toNumber();
        if (! qIsNaN(t))
            t = MonthFromTime(t);
        return QScriptValueImpl(eng, t);
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.getUTCMonth"));
}

QScriptValueImpl Date::method_getDate(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo)
{
    QScriptValueImpl self = context->thisObject();
    if (self.classInfo() == classInfo) {
        qsreal t = self.internalValue().toNumber();
        if (! qIsNaN(t))
            t = DateFromTime(LocalTime(t));
        return QScriptValueImpl(eng, t);
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.getDate"));
}

QScriptValueImpl Date::method_getUTCDate(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo)
{
    QScriptValueImpl self = context->thisObject();
    if (self.classInfo() == classInfo) {
        qsreal t = self.internalValue().toNumber();
        if (! qIsNaN(t))
            t = DateFromTime(t);
        return QScriptValueImpl(eng, t);
    }
    return context->throwError(QScriptContext::TypeError, QLatin1String("Date.prototype.getUTCDate"));
}

QScriptValueImpl Date::method_getDay(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo)
{
    QScriptValueImpl self = context->thisObject();
    if (self.classInfo() == classInfo) {
        qsreal t = self.internalValue().toNumber();
        if (! qIsNaN(t))
            t = WeekDay(LocalTime(t));
        return QScriptValueImpl(eng, t);
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.getDay"));
}

QScriptValueImpl Date::method_getUTCDay(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo)
{
    QScriptValueImpl self = context->thisObject();
    if (self.classInfo() == classInfo) {
        qsreal t = self.internalValue().toNumber();
        if (! qIsNaN(t))
            t = WeekDay(t);
        return QScriptValueImpl(eng, t);
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.getUTCDay"));
}

QScriptValueImpl Date::method_getHours(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo)
{
    QScriptValueImpl self = context->thisObject();
    if (self.classInfo() == classInfo) {
        qsreal t = self.internalValue().toNumber();
        if (! qIsNaN(t))
            t = HourFromTime(LocalTime(t));
        return QScriptValueImpl(eng, t);
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.getHours"));
}

QScriptValueImpl Date::method_getUTCHours(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo)
{
    QScriptValueImpl self = context->thisObject();
    if (self.classInfo() == classInfo) {
        qsreal t = self.internalValue().toNumber();
        if (! qIsNaN(t))
            t = HourFromTime(t);
        return QScriptValueImpl(eng, t);
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.getUTCHours"));
}

QScriptValueImpl Date::method_getMinutes(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo)
{
    QScriptValueImpl self = context->thisObject();
    if (self.classInfo() == classInfo) {
        qsreal t = self.internalValue().toNumber();
        if (! qIsNaN(t))
            t = MinFromTime(LocalTime(t));
        return QScriptValueImpl(eng, t);
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.getMinutes"));
}

QScriptValueImpl Date::method_getUTCMinutes(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo)
{
    QScriptValueImpl self = context->thisObject();
    if (self.classInfo() == classInfo) {
        qsreal t = self.internalValue().toNumber();
        if (! qIsNaN(t))
            t = MinFromTime(t);
        return QScriptValueImpl(eng, t);
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.getUTCMinutes"));
}

QScriptValueImpl Date::method_getSeconds(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo)
{
    QScriptValueImpl self = context->thisObject();
    if (self.classInfo() == classInfo) {
        qsreal t = self.internalValue().toNumber();
        if (! qIsNaN(t))
            t = SecFromTime(LocalTime(t));
        return QScriptValueImpl(eng, t);
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.getSeconds"));
}

QScriptValueImpl Date::method_getUTCSeconds(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo)
{
    QScriptValueImpl self = context->thisObject();
    if (self.classInfo() == classInfo) {
        qsreal t = self.internalValue().toNumber();
        if (! qIsNaN(t))
            t = SecFromTime(t);
        return QScriptValueImpl(eng, t);
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.getUTCSeconds"));
}

QScriptValueImpl Date::method_getMilliseconds(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo)
{
    QScriptValueImpl self = context->thisObject();
    if (self.classInfo() == classInfo) {
        qsreal t = self.internalValue().toNumber();
        if (! qIsNaN(t))
            t = msFromTime(LocalTime(t));
        return QScriptValueImpl(eng, t);
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.getMilliseconds"));
}

QScriptValueImpl Date::method_getUTCMilliseconds(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo)
{
    QScriptValueImpl self = context->thisObject();
    if (self.classInfo() == classInfo) {
        qsreal t = self.internalValue().toNumber();
        if (! qIsNaN(t))
            t = msFromTime(t);
        return QScriptValueImpl(eng, t);
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.getUTCMilliseconds"));
}

QScriptValueImpl Date::method_getTimezoneOffset(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo)
{
    QScriptValueImpl self = context->thisObject();
    if (self.classInfo() == classInfo) {
        qsreal t = self.internalValue().toNumber();
        if (! qIsNaN(t))
            t = (t - LocalTime(t)) / msPerMinute;
        return QScriptValueImpl(eng, t);
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.getTimezoneOffset"));
}

QScriptValueImpl Date::method_setTime(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo)
{
    QScriptValueImpl self = context->thisObject();
    if (self.classInfo() == classInfo) {
        qsreal t = TimeClip(context->argument(0).toNumber());
        QScriptValueImpl r(eng, t);
        self.setInternalValue(r);
        return r;
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.setTime"));
}

QScriptValueImpl Date::method_setMilliseconds(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo)
{
    QScriptValueImpl self = context->thisObject();
    if (self.classInfo() == classInfo) {
        qsreal t = LocalTime(self.internalValue().toNumber());
        qsreal ms = context->argument(0).toNumber();
        t = TimeClip(UTC(MakeDate(Day(t), MakeTime(HourFromTime(t), MinFromTime(t), SecFromTime(t), ms))));
        QScriptValueImpl r(eng, t);
        self.setInternalValue(r);
        return r;
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.setMilliseconds"));
}

QScriptValueImpl Date::method_setUTCMilliseconds(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo)
{
    QScriptValueImpl self = context->thisObject();
    if (self.classInfo() == classInfo) {
        qsreal t = self.internalValue().toNumber();
        qsreal ms = context->argument(0).toNumber();
        t = TimeClip(MakeDate(Day(t), MakeTime(HourFromTime(t), MinFromTime(t), SecFromTime(t), ms)));
        QScriptValueImpl r(eng, t);
        self.setInternalValue(r);
        return r;
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.setUTCMilliseconds"));
}

QScriptValueImpl Date::method_setSeconds(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo)
{
    QScriptValueImpl self = context->thisObject();
    if (self.classInfo() == classInfo) {
        qsreal t = LocalTime(self.internalValue().toNumber());
        qsreal sec = context->argument(0).toNumber();
        qsreal ms = (context->argumentCount() < 2) ? msFromTime(t) : context->argument(1).toNumber();
        t = TimeClip(UTC(MakeDate(Day(t), MakeTime(HourFromTime(t), MinFromTime(t), sec, ms))));
        QScriptValueImpl r(eng, t);
        self.setInternalValue(r);
        return r;
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.setSeconds"));
}

QScriptValueImpl Date::method_setUTCSeconds(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo)
{
    QScriptValueImpl self = context->thisObject();
    if (self.classInfo() == classInfo) {
        qsreal t = self.internalValue().toNumber();
        qsreal sec = context->argument(0).toNumber();
        qsreal ms = (context->argumentCount() < 2) ? msFromTime(t) : context->argument(1).toNumber();
        t = TimeClip(MakeDate(Day(t), MakeTime(HourFromTime(t), MinFromTime(t), sec, ms)));
        QScriptValueImpl r(eng, t);
        self.setInternalValue(r);
        return r;
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.setUTCSeconds"));
}

QScriptValueImpl Date::method_setMinutes(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo)
{
    QScriptValueImpl self = context->thisObject();
    if (self.classInfo() == classInfo) {
        qsreal t = LocalTime(self.internalValue().toNumber());
        qsreal min = context->argument(0).toNumber();
        qsreal sec = (context->argumentCount() < 2) ? SecFromTime(t) : context->argument(1).toNumber();
        qsreal ms = (context->argumentCount() < 3) ? msFromTime(t) : context->argument(2).toNumber();
        t = TimeClip(UTC(MakeDate(Day(t), MakeTime(HourFromTime(t), min, sec, ms))));
        QScriptValueImpl r(eng, t);
        self.setInternalValue(r);
        return r;
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.setMinutes"));
}

QScriptValueImpl Date::method_setUTCMinutes(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo)
{
    QScriptValueImpl self = context->thisObject();
    if (self.classInfo() == classInfo) {
        qsreal t = self.internalValue().toNumber();
        qsreal min = context->argument(0).toNumber();
        qsreal sec = (context->argumentCount() < 2) ? SecFromTime(t) : context->argument(1).toNumber();
        qsreal ms = (context->argumentCount() < 3) ? msFromTime(t) : context->argument(2).toNumber();
        t = TimeClip(MakeDate(Day(t), MakeTime(HourFromTime(t), min, sec, ms)));
        QScriptValueImpl r(eng, t);
        self.setInternalValue(r);
        return r;
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.setUTCMinutes"));
}

QScriptValueImpl Date::method_setHours(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo)
{
    QScriptValueImpl self = context->thisObject();
    if (self.classInfo() == classInfo) {
        qsreal t = LocalTime(self.internalValue().toNumber());
        qsreal hour = context->argument(0).toNumber();
        qsreal min = (context->argumentCount() < 2) ? MinFromTime(t) : context->argument(1).toNumber();
        qsreal sec = (context->argumentCount() < 3) ? SecFromTime(t) : context->argument(2).toNumber();
        qsreal ms = (context->argumentCount() < 4) ? msFromTime(t) : context->argument(3).toNumber();
        t = TimeClip(UTC(MakeDate(Day(t), MakeTime(hour, min, sec, ms))));
        QScriptValueImpl r(eng, t);
        self.setInternalValue(r);
        return r;
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.setHours"));
}

QScriptValueImpl Date::method_setUTCHours(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo)
{
    QScriptValueImpl self = context->thisObject();
    if (self.classInfo() == classInfo) {
        qsreal t = self.internalValue().toNumber();
        qsreal hour = context->argument(0).toNumber();
        qsreal min = (context->argumentCount() < 2) ? MinFromTime(t) : context->argument(1).toNumber();
        qsreal sec = (context->argumentCount() < 3) ? SecFromTime(t) : context->argument(2).toNumber();
        qsreal ms = (context->argumentCount() < 4) ? msFromTime(t) : context->argument(3).toNumber();
        t = TimeClip(MakeDate(Day(t), MakeTime(hour, min, sec, ms)));
        QScriptValueImpl r(eng, t);
        self.setInternalValue(r);
        return r;
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.setUTCHours"));
}

QScriptValueImpl Date::method_setDate(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo)
{
    QScriptValueImpl self = context->thisObject();
    if (self.classInfo() == classInfo) {
        qsreal t = LocalTime(self.internalValue().toNumber());
        qsreal date = context->argument(0).toNumber();
        t = TimeClip(UTC(MakeDate(MakeDay(YearFromTime(t), MonthFromTime(t), date), TimeWithinDay(t))));
        QScriptValueImpl r(eng, t);
        self.setInternalValue(r);
        return r;
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.setDate"));
}

QScriptValueImpl Date::method_setUTCDate(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo)
{
    QScriptValueImpl self = context->thisObject();
    if (self.classInfo() == classInfo) {
        qsreal t = self.internalValue().toNumber();
        qsreal date = context->argument(0).toNumber();
        t = TimeClip(MakeDate(MakeDay(YearFromTime(t), MonthFromTime(t), date), TimeWithinDay(t)));
        QScriptValueImpl r(eng, t);
        self.setInternalValue(r);
        return r;
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.setUTCDate"));
}

QScriptValueImpl Date::method_setMonth(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo)
{
    QScriptValueImpl self = context->thisObject();
    if (self.classInfo() == classInfo) {
        qsreal t = LocalTime(self.internalValue().toNumber());
        qsreal month = context->argument(0).toNumber();
        qsreal date = (context->argumentCount() < 2) ? DateFromTime(t) : context->argument(1).toNumber();
        t = TimeClip(UTC(MakeDate(MakeDay(YearFromTime(t), month, date), TimeWithinDay(t))));
        QScriptValueImpl r(eng, t);
        self.setInternalValue(r);
        return r;
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.setMonth"));
}

QScriptValueImpl Date::method_setUTCMonth(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo)
{
    QScriptValueImpl self = context->thisObject();
    if (self.classInfo() == classInfo) {
        qsreal t = self.internalValue().toNumber();
        qsreal month = context->argument(0).toNumber();
        qsreal date = (context->argumentCount() < 2) ? DateFromTime(t) : context->argument(1).toNumber();
        t = TimeClip(MakeDate(MakeDay(YearFromTime(t), month, date), TimeWithinDay(t)));
        QScriptValueImpl r(eng, t);
        self.setInternalValue(r);
        return r;
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.setUTCMonth"));
}

QScriptValueImpl Date::method_setFullYear(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo)
{
    QScriptValueImpl self = context->thisObject();
    if (self.classInfo() == classInfo) {
        qsreal t = LocalTime(self.internalValue().toNumber());
        qsreal year = context->argument(0).toNumber();
        qsreal month = (context->argumentCount() < 2) ? MonthFromTime(t) : context->argument(1).toNumber();
        qsreal date = (context->argumentCount() < 3) ? DateFromTime(t) : context->argument(2).toNumber();
        t = TimeClip(UTC(MakeDate(MakeDay(year, month, date), TimeWithinDay(t))));
        QScriptValueImpl r(eng, t);
        self.setInternalValue(r);
        return r;
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.setFullYear"));
}

QScriptValueImpl Date::method_setUTCFullYear(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo)
{
    QScriptValueImpl self = context->thisObject();
    if (self.classInfo() == classInfo) {
        qsreal t = self.internalValue().toNumber();
        qsreal year = context->argument(0).toNumber();
        qsreal month = (context->argumentCount() < 2) ? MonthFromTime(t) : context->argument(1).toNumber();
        qsreal date = (context->argumentCount() < 3) ? DateFromTime(t) : context->argument(2).toNumber();
        t = TimeClip(MakeDate(MakeDay(year, month, date), TimeWithinDay(t)));
        QScriptValueImpl r(eng, t);
        self.setInternalValue(r);
        return r;
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.setUTCFullYear"));
}

QScriptValueImpl Date::method_setYear(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo)
{
    QScriptValueImpl self = context->thisObject();
    if (self.classInfo() == classInfo) {
        qsreal t = self.internalValue().toNumber();
        if (qIsNaN(t))
            t = 0;
        else
            t = LocalTime(t);
        qsreal year = context->argument(0).toNumber();
        qsreal r;
        if (qIsNaN(year)) {
            r = qSNaN();
        } else {
            if ((eng->toInteger(year) >= 0) && (eng->toInteger(year) <= 99))
                year += 1900;
            r = MakeDay(year, MonthFromTime(t), DateFromTime(t));
            r = UTC(MakeDate(r, TimeWithinDay(t)));
            r = TimeClip(r);
        }
        QScriptValueImpl v = QScriptValueImpl(eng, r);
        self.setInternalValue(v);
        return v;
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.setYear"));
}

QScriptValueImpl Date::method_toUTCString(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo)
{
    QScriptValueImpl self = context->thisObject();
    if (self.classInfo() == classInfo) {
        qsreal t = self.internalValue().toNumber();
        return QScriptValueImpl(eng, ToUTCString(t));
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.toUTCString"));
}

} } // namespace QScript::Ecma

#endif // QT_NO_SCRIPT
