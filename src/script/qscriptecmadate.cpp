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

#include "qscriptengine.h"

#include "qscriptengine_p.h"
#include "qscriptecmadate_p.h"
#include "qscriptecmastring_p.h"
#include "qscriptecmaobject_p.h"
#include "qscriptecmafunction_p.h"
#include "qscriptcontext_p.h"

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

static const qnumber HoursPerDay = 24.0;
static const qnumber MinutesPerHour = 60.0;
static const qnumber SecondsPerMinute = 60.0;
static const qnumber msPerSecond = 1000.0;
static const qnumber msPerMinute = 60000.0;
static const qnumber msPerHour = 3600000.0;
static const qnumber msPerDay = 86400000.0;

static qnumber LocalTZA = 0.0; // initialized at startup

static inline qnumber TimeWithinDay(qnumber t)
{
    qnumber r = ::fmod(t, msPerDay);
    return (r >= 0) ? r : r + msPerDay;
}

static inline int HourFromTime(qnumber t)
{
    int r = int(::fmod(::floor(t / msPerHour), HoursPerDay));
    return (r >= 0) ? r : r + int(HoursPerDay);
}

static inline int MinFromTime(qnumber t)
{
    int r = int(::fmod(::floor(t / msPerMinute), MinutesPerHour));
    return (r >= 0) ? r : r + int(MinutesPerHour);
}

static inline int SecFromTime(qnumber t)
{
    int r = int(::fmod(::floor(t / msPerSecond), SecondsPerMinute));
    return (r >= 0) ? r : r + int(SecondsPerMinute);
}

static inline int msFromTime(qnumber t)
{
    int r = int(::fmod(t, msPerSecond));
    return (r >= 0) ? r : r + int(msPerSecond);
}

static inline qnumber Day(qnumber t)
{
    return ::floor(t / msPerDay);
}

static inline qnumber DaysInYear(qnumber y)
{
    if (::fmod(y, 4))
        return 365;

    else if (::fmod(y, 100))
        return 366;

    else if (::fmod(y, 400))
        return 365;

    return 366;
}

static inline qnumber DayFromYear(qnumber y)
{
    return 365 * (y - 1970)
        + ::floor((y - 1969) / 4)
        - ::floor((y - 1901) / 100)
        + ::floor((y - 1601) / 400);
}

static inline qnumber TimeFromYear(qnumber y)
{
    return msPerDay * DayFromYear(y);
}

static inline qnumber YearFromTime(qnumber t)
{
    int y = 1970;
    y += (int) ::floor(t / (msPerDay * 365.2425));

    qnumber t2 = TimeFromYear(y);
    return (t2 > t) ? y - 1 : ((t2 + msPerDay * DaysInYear(y)) <= t) ? y + 1 : y;
}

static inline bool InLeapYear(qnumber t)
{
    qnumber x = DaysInYear(YearFromTime(t));
    if (x == 365)
        return 0;

    Q_ASSERT (x == 366);
    return 1;
}

static inline qnumber DayWithinYear(qnumber t)
{
    return Day(t) - DayFromYear(YearFromTime(t));
}

static inline qnumber MonthFromTime(qnumber t)
{
    qnumber d = DayWithinYear(t);
    qnumber l = InLeapYear(t);

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

    return qSNan(); // ### assert?
}

static inline qnumber DateFromTime(qnumber t)
{
    int m = (int) QScriptEnginePrivate::toInteger(MonthFromTime(t));
    qnumber d = DayWithinYear(t);
    qnumber l = InLeapYear(t);

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

    return qSNan(); // ### assert
}

static inline qnumber WeekDay(qnumber t)
{
    qnumber r = ::fmod (Day(t) + 4.0, 7.0);
    return (r >= 0) ? r : r + 7.0;
}


static inline qnumber MakeTime(qnumber hour, qnumber min, qnumber sec, qnumber ms)
{
    return ((hour * MinutesPerHour + min) * SecondsPerMinute + sec) * msPerSecond + ms;
}

static inline qnumber DayFromMonth(qnumber month, qnumber leap)
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

    return qSNan(); // ### assert?
}

static qnumber MakeDay(qnumber year, qnumber month, qnumber day)
{
    year += ::floor(month / 12.0);

    month = ::fmod(month, 12.0);
    if (month < 0)
        month += 12.0;

    qnumber t = TimeFromYear(year);
    qnumber leap = InLeapYear(t);

    day += ::floor(t / msPerDay);
    day += DayFromMonth(month, leap);

    return day - 1;
}

static inline qnumber MakeDate(qnumber day, qnumber time)
{
    return day * msPerDay + time;
}

static inline qnumber DaylightSavingTA(double t)
{
#ifndef Q_WS_WIN
    long int tt = (long int)(t / msPerSecond);
    struct tm *tmtm = localtime((const time_t*)&tt);
    if (! tmtm)
        return 0;
    return (tmtm->tm_isdst > 0) ? msPerHour : 0;
#else
    /// ### implement me
    return 0;
#endif
}

static inline qnumber LocalTime(qnumber t)
{
    return t + LocalTZA + DaylightSavingTA(t);
}

static inline qnumber UTC(qnumber t)
{
    return t - LocalTZA - DaylightSavingTA(t - LocalTZA);
}

static inline qnumber currentTime()
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

static inline qnumber TimeClip(qnumber t)
{
    if (! qIsFinite(t) || fabs(t) > 8.64e15)
        return qSNan();
    return QScriptEnginePrivate::toInteger(t);
}

static inline qnumber ParseString(const QString &s)
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

static inline QString ToString(qnumber t)
{
    t = LocalTime(t);

    int year = (int) YearFromTime(t);
    int month = (int) MonthFromTime(t);
    int day = (int) DateFromTime(t);

    QDate date(year, 1 + month, day);
    QTime time(HourFromTime(t), MinFromTime(t), SecFromTime(t));

    return QDateTime (date, time).toString();
}

static qnumber getLocalTZA()
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


Date::Date(QScriptEngine *eng):
    Core(eng)
{
    LocalTZA = getLocalTZA();

    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(eng);
    m_classInfo = eng_p->registerClass(QLatin1String("Date"));

    publicPrototype.invalidate();
    newDate(&publicPrototype, qSNan());

    const QScriptValue::PropertyFlags flags = QScriptValue::SkipInEnumeration;

    eng_p->newConstructor(&ctor, this, publicPrototype);
    ctor.setProperty(QLatin1String("parse"),
                     eng_p->createFunction(method_parse, 1, m_classInfo), flags);
    ctor.setProperty(QLatin1String("UTC"),
                     eng_p->createFunction(method_UTC, 1, m_classInfo), flags);

    publicPrototype.setProperty(QLatin1String("toString"),
                                eng_p->createFunction(method_toString, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("toDateString"),
                                eng_p->createFunction(method_toDateString, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("toTimeString"),
                                eng_p->createFunction(method_toTimeString, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("toLocaleString"),
                                eng_p->createFunction(method_toLocaleString, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("toLocaleDateString"),
                                eng_p->createFunction(method_toLocaleDateString, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("toLocaleTimeString"),
                                eng_p->createFunction(method_toLocaleTimeString, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("valueOf"),
                                eng_p->createFunction(method_valueOf, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("getTime"),
                                eng_p->createFunction(method_getTime, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("getYear"),
                                eng_p->createFunction(method_getYear, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("getFullYear"),
                                eng_p->createFunction(method_getFullYear, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("getUTCFullYear"),
                                eng_p->createFunction(method_getUTCFullYear, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("getMonth"),
                                eng_p->createFunction(method_getMonth, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("getUTCMonth"),
                                eng_p->createFunction(method_getUTCMonth, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("getDate"),
                                eng_p->createFunction(method_getDate, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("getUTCDate"),
                                eng_p->createFunction(method_getUTCDate, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("getDay"),
                                eng_p->createFunction(method_getDay, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("getUTCDay"),
                                eng_p->createFunction(method_getUTCDay, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("getHours"),
                                eng_p->createFunction(method_getHours, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("getUTCHours"),
                                eng_p->createFunction(method_getUTCHours, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("getMinutes"),
                                eng_p->createFunction(method_getMinutes, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("getUTCMinutes"),
                                eng_p->createFunction(method_getUTCMinutes, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("getSeconds"),
                                eng_p->createFunction(method_getSeconds, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("getUTCSeconds"),
                                eng_p->createFunction(method_getUTCSeconds, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("getMilliseconds"),
                                eng_p->createFunction(method_getMilliseconds, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("getUTCMilliseconds"),
                                eng_p->createFunction(method_getUTCMilliseconds, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("getTimezoneOffset"),
                                eng_p->createFunction(method_getTimezoneOffset, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("setTime"),
                                eng_p->createFunction(method_setTime, 1, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("setMilliseconds"),
                                eng_p->createFunction(method_setMilliseconds, 1, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("setUTCMilliseconds"),
                                eng_p->createFunction(method_setUTCMilliseconds, 1, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("setSeconds"),
                                eng_p->createFunction(method_setSeconds, 2, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("setUTCSeconds"),
                                eng_p->createFunction(method_setUTCSeconds, 2, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("setMinutes"),
                                eng_p->createFunction(method_setMinutes, 3, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("setUTCMinutes"),
                                eng_p->createFunction(method_setUTCMinutes, 3, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("setHours"),
                                eng_p->createFunction(method_setHours, 4, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("setUTCHours"),
                                eng_p->createFunction(method_setUTCHours, 4, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("setDate"),
                                eng_p->createFunction(method_setDate, 1, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("setUTCDate"),
                                eng_p->createFunction(method_setUTCDate, 1, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("setMonth"),
                                eng_p->createFunction(method_setMonth, 2, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("setUTCMonth"),
                                eng_p->createFunction(method_setUTCMonth, 2, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("setYear"),
                                eng_p->createFunction(method_setYear, 1, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("setFullYear"),
                                eng_p->createFunction(method_setFullYear, 3, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("setUTCFullYear"),
                                eng_p->createFunction(method_setUTCFullYear, 3, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("toUTCString"),
                                eng_p->createFunction(method_toUTCString, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("toGMTString"),
                                eng_p->createFunction(method_toUTCString, 0, m_classInfo), flags);
}

Date::~Date()
{
}

void Date::execute(QScriptContext *context)
{
    QScriptEngine *eng = context->engine();

    if (!context->calledAsConstructor()) {
        double t = currentTime();
        context->setReturnValue(eng->scriptValue(ToString(t)));
        return;
    }

    // called as constructor
    qnumber t;

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
        qnumber year  = context->argument(0).toNumber();
        qnumber month = context->argument(1).toNumber();
        qnumber day  = context->argumentCount() >= 3 ? context->argument(2).toNumber() : 1;
        qnumber hours = context->argumentCount() >= 4 ? context->argument(3).toNumber() : 0;
        qnumber mins = context->argumentCount() >= 5 ? context->argument(4).toNumber() : 0;
        qnumber secs = context->argumentCount() >= 6 ? context->argument(5).toNumber() : 0;
        qnumber ms    = context->argumentCount() >= 7 ? context->argument(6).toNumber() : 0;
        if (year >= 0 && year <= 99)
            year += 1900;
        t = MakeDate(MakeDay(year, month, day), MakeTime(hours, mins, secs, ms));
        t = TimeClip(UTC(t));
    }

    QScriptValue &obj = QScriptContextPrivate::get(context)->thisObject;
    QScriptValueImpl::get(obj)->setClassInfo(classInfo());
    QScriptValueImpl::get(obj)->setInternalValue(engine()->scriptValue(t));
    obj.setPrototype(publicPrototype);
}

void Date::newDate(QScriptValue *result, qnumber t)
{
    QScriptEnginePrivate::get(engine())->newObject(result, publicPrototype, classInfo());
    QScriptValueImpl::get(*result)->setInternalValue(engine()->scriptValue(t));
}

void Date::newDate(QScriptValue *result, const QDateTime &dt)
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
    double t = MakeDate(MakeDay(year, month, day),
                        MakeTime(hours, mins, secs, ms));
    newDate(result, t);
}

void Date::newDate(QScriptValue *result, const QDate &d)
{
    newDate(result, QDateTime(d));
}

QDateTime Date::toDateTime(const QScriptValue &date)
{
    Q_ASSERT(QScriptValueImpl::get(date)->classInfo() == m_classInfo);
    qnumber t = QScriptValueImpl::get(date)->internalValue().toNumber();
    if (qIsNan(t))
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

QScriptValue Date::method_parse(QScriptEngine *eng, QScriptClassInfo *)
{
    QScriptContext *context = eng->currentContext();
    return eng->scriptValue(ParseString(context->argument(0).toString()));
}

QScriptValue Date::method_UTC(QScriptEngine *eng, QScriptClassInfo *)
{
    QScriptContext *context = eng->currentContext();
    const int numArgs = context->argumentCount();
    if (numArgs >= 2) {
        qnumber year  = context->argument(0).toNumber();
        qnumber month = context->argument(1).toNumber();
        qnumber day   = numArgs >= 3 ? context->argument(2).toNumber() : 1;
        qnumber hours = numArgs >= 4 ? context->argument(3).toNumber() : 0;
        qnumber mins  = numArgs >= 5 ? context->argument(4).toNumber() : 0;
        qnumber secs  = numArgs >= 6 ? context->argument(5).toNumber() : 0;
        qnumber ms    = numArgs >= 7 ? context->argument(6).toNumber() : 0;
        if (year >= 0 && year <= 99)
            year += 1900;
        qnumber t = MakeDate(MakeDay(year, month, day),
                            MakeTime(hours, mins, secs, ms));
        return eng->scriptValue(TimeClip(t));
    }
    return (eng->undefinedScriptValue());
}

QScriptValue Date::method_toString(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    QScriptContext *context = eng->currentContext();
    if (QScriptValueImpl::get(context->thisObject())->classInfo() == classInfo) {
        qnumber t = QScriptValueImpl::get(context->thisObject())->internalValue().toNumber();
        return eng->scriptValue(ToString(t));
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.toString"));
}

QScriptValue Date::method_toDateString(QScriptEngine *eng, QScriptClassInfo *)
{
    QScriptContext *context = eng->currentContext();
    return QScriptContextPrivate::get(context)->throwNotImplemented(
        QLatin1String("Date.prototype.toDateString"));
}

QScriptValue Date::method_toTimeString(QScriptEngine *eng, QScriptClassInfo *)
{
    QScriptContext *context = eng->currentContext();
    return QScriptContextPrivate::get(context)->throwNotImplemented(
        QLatin1String("Date.prototype.toTimeString"));
}

QScriptValue Date::method_toLocaleString(QScriptEngine *eng, QScriptClassInfo *)
{
    QScriptContext *context = eng->currentContext();
    return QScriptContextPrivate::get(context)->throwNotImplemented(
        QLatin1String("Date.prototype.toLocaleString"));
}

QScriptValue Date::method_toLocaleDateString(QScriptEngine *eng, QScriptClassInfo *)
{
    QScriptContext *context = eng->currentContext();
    return QScriptContextPrivate::get(context)->throwNotImplemented(
        QLatin1String("Date.prototype.toLocaleDateString"));
}

QScriptValue Date::method_toLocaleTimeString(QScriptEngine *eng, QScriptClassInfo *)
{
    QScriptContext *context = eng->currentContext();
    return QScriptContextPrivate::get(context)->throwNotImplemented(
        QLatin1String("Date.prototype.toLocaleTimeString"));
}

QScriptValue Date::method_valueOf(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    QScriptContext *context = eng->currentContext();
    if (QScriptValueImpl::get(context->thisObject())->classInfo() == classInfo)
        return eng->scriptValue(QScriptValueImpl::get(context->thisObject())->internalValue().toNumber());
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.valueOf"));
}

QScriptValue Date::method_getTime(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    QScriptContext *context = eng->currentContext();
    if (QScriptValueImpl::get(context->thisObject())->classInfo() == classInfo)
        return eng->scriptValue(QScriptValueImpl::get(context->thisObject())->internalValue().toNumber());
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.getTime"));
}

QScriptValue Date::method_getYear(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    QScriptContext *context = eng->currentContext();
    if (QScriptValueImpl::get(context->thisObject())->classInfo() == classInfo) {
        qnumber t = QScriptValueImpl::get(context->thisObject())->internalValue().toNumber();
        if (! qIsNan(t))
            t = YearFromTime(LocalTime(t)) - 1900;
        return eng->scriptValue(t);
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.getYear"));
}

QScriptValue Date::method_getFullYear(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    QScriptContext *context = eng->currentContext();
    if (QScriptValueImpl::get(context->thisObject())->classInfo() == classInfo) {
        qnumber t = QScriptValueImpl::get(context->thisObject())->internalValue().toNumber();
        if (! qIsNan(t))
            t = YearFromTime(LocalTime(t));
        return eng->scriptValue(t);
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.getFullYear"));
}

QScriptValue Date::method_getUTCFullYear(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    QScriptContext *context = eng->currentContext();
    if (QScriptValueImpl::get(context->thisObject())->classInfo() == classInfo) {
        qnumber t = QScriptValueImpl::get(context->thisObject())->internalValue().toNumber();
        if (! qIsNan(t))
            t = YearFromTime(t);
        return eng->scriptValue(t);
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.getUTCFullYear"));
}

QScriptValue Date::method_getMonth(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    QScriptContext *context = eng->currentContext();
    if (QScriptValueImpl::get(context->thisObject())->classInfo() == classInfo) {
        qnumber t = QScriptValueImpl::get(context->thisObject())->internalValue().toNumber();
        if (! qIsNan(t))
            t = MonthFromTime(LocalTime(t));
        return eng->scriptValue(t);
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.getMonth"));
}

QScriptValue Date::method_getUTCMonth(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    QScriptContext *context = eng->currentContext();
    if (QScriptValueImpl::get(context->thisObject())->classInfo() == classInfo) {
        qnumber t = QScriptValueImpl::get(context->thisObject())->internalValue().toNumber();
        if (! qIsNan(t))
            t = MonthFromTime(t);
        return eng->scriptValue(t);
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.getUTCMonth"));
}

QScriptValue Date::method_getDate(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    QScriptContext *context = eng->currentContext();
    if (QScriptValueImpl::get(context->thisObject())->classInfo() == classInfo) {
        qnumber t = QScriptValueImpl::get(context->thisObject())->internalValue().toNumber();
        if (! qIsNan(t))
            t = DateFromTime(LocalTime(t));
        return eng->scriptValue(t);
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.getDate"));
}

QScriptValue Date::method_getUTCDate(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    QScriptContext *context = eng->currentContext();
    if (QScriptValueImpl::get(context->thisObject())->classInfo() == classInfo) {
        qnumber t = QScriptValueImpl::get(context->thisObject())->internalValue().toNumber();
        if (! qIsNan(t))
            t = DateFromTime(t);
        return eng->scriptValue(t);
    }
    return context->throwError(QScriptContext::TypeError, QLatin1String("Date.prototype.getUTCDate"));
}

QScriptValue Date::method_getDay(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    QScriptContext *context = eng->currentContext();
    if (QScriptValueImpl::get(context->thisObject())->classInfo() == classInfo) {
        qnumber t = QScriptValueImpl::get(context->thisObject())->internalValue().toNumber();
        if (! qIsNan(t))
            t = WeekDay(LocalTime(t));
        return eng->scriptValue(t);
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.getDay"));
}

QScriptValue Date::method_getUTCDay(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    QScriptContext *context = eng->currentContext();
    if (QScriptValueImpl::get(context->thisObject())->classInfo() == classInfo) {
        qnumber t = QScriptValueImpl::get(context->thisObject())->internalValue().toNumber();
        if (! qIsNan(t))
            t = WeekDay(t);
        return eng->scriptValue(t);
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.getUTCDay"));
}

QScriptValue Date::method_getHours(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    QScriptContext *context = eng->currentContext();
    if (QScriptValueImpl::get(context->thisObject())->classInfo() == classInfo) {
        qnumber t = QScriptValueImpl::get(context->thisObject())->internalValue().toNumber();
        if (! qIsNan(t))
            t = HourFromTime(LocalTime(t));
        return eng->scriptValue(t);
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.getHours"));
}

QScriptValue Date::method_getUTCHours(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    QScriptContext *context = eng->currentContext();
    if (QScriptValueImpl::get(context->thisObject())->classInfo() == classInfo) {
        qnumber t = QScriptValueImpl::get(context->thisObject())->internalValue().toNumber();
        if (! qIsNan(t))
            t = HourFromTime(t);
        return eng->scriptValue(t);
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.getUTCHours"));
}

QScriptValue Date::method_getMinutes(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    QScriptContext *context = eng->currentContext();
    if (QScriptValueImpl::get(context->thisObject())->classInfo() == classInfo) {
        qnumber t = QScriptValueImpl::get(context->thisObject())->internalValue().toNumber();
        if (! qIsNan(t))
            t = MinFromTime(LocalTime(t));
        return eng->scriptValue(t);
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.getMinutes"));
}

QScriptValue Date::method_getUTCMinutes(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    QScriptContext *context = eng->currentContext();
    if (QScriptValueImpl::get(context->thisObject())->classInfo() == classInfo) {
        qnumber t = QScriptValueImpl::get(context->thisObject())->internalValue().toNumber();
        if (! qIsNan(t))
            t = MinFromTime(t);
        return eng->scriptValue(t);
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.getUTCMinutes"));
}

QScriptValue Date::method_getSeconds(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    QScriptContext *context = eng->currentContext();
    if (QScriptValueImpl::get(context->thisObject())->classInfo() == classInfo) {
        qnumber t = QScriptValueImpl::get(context->thisObject())->internalValue().toNumber();
        if (! qIsNan(t))
            t = SecFromTime(LocalTime(t));
        return eng->scriptValue(t);
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.getSeconds"));
}

QScriptValue Date::method_getUTCSeconds(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    QScriptContext *context = eng->currentContext();
    if (QScriptValueImpl::get(context->thisObject())->classInfo() == classInfo) {
        qnumber t = QScriptValueImpl::get(context->thisObject())->internalValue().toNumber();
        if (! qIsNan(t))
            t = SecFromTime(t);
        return eng->scriptValue(t);
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.getUTCSeconds"));
}

QScriptValue Date::method_getMilliseconds(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    QScriptContext *context = eng->currentContext();
    if (QScriptValueImpl::get(context->thisObject())->classInfo() == classInfo) {
        qnumber t = QScriptValueImpl::get(context->thisObject())->internalValue().toNumber();
        if (! qIsNan(t))
            t = msFromTime(LocalTime(t));
        return eng->scriptValue(t);
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.getMilliseconds"));
}

QScriptValue Date::method_getUTCMilliseconds(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    QScriptContext *context = eng->currentContext();
    if (QScriptValueImpl::get(context->thisObject())->classInfo() == classInfo) {
        qnumber t = QScriptValueImpl::get(context->thisObject())->internalValue().toNumber();
        if (! qIsNan(t))
            t = msFromTime(t);
        return eng->scriptValue(t);
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.getUTCMilliseconds"));
}

QScriptValue Date::method_getTimezoneOffset(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    QScriptContext *context = eng->currentContext();
    if (QScriptValueImpl::get(context->thisObject())->classInfo() == classInfo) {
        qnumber t = QScriptValueImpl::get(context->thisObject())->internalValue().toNumber();
        if (! qIsNan(t))
            t = (t - LocalTime(t)) / msPerMinute;
        return eng->scriptValue(t);
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.getTimezoneOffset"));
}

QScriptValue Date::method_setTime(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    QScriptContext *context = eng->currentContext();
    if (QScriptValueImpl::get(context->thisObject())->classInfo() == classInfo) {
        qnumber t = TimeClip(context->argument(0).toNumber());
        QScriptValue r = eng->scriptValue(t);
        QScriptValueImpl::get(context->thisObject())->setInternalValue(r);
        return r;
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.setTime"));
}

QScriptValue Date::method_setMilliseconds(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    QScriptContext *context = eng->currentContext();
    if (QScriptValueImpl::get(context->thisObject())->classInfo() == classInfo) {
        qnumber t = LocalTime(QScriptValueImpl::get(context->thisObject())->internalValue().toNumber());
        qnumber ms = context->argument(0).toNumber();
        t = TimeClip(UTC(MakeDate(Day(t), MakeTime(HourFromTime(t), MinFromTime(t), SecFromTime(t), ms))));
        QScriptValue r = eng->scriptValue(t);
        QScriptValueImpl::get(context->thisObject())->setInternalValue(r);
        return r;
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.setMilliseconds"));
}

QScriptValue Date::method_setUTCMilliseconds(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    QScriptContext *context = eng->currentContext();
    if (QScriptValueImpl::get(context->thisObject())->classInfo() == classInfo) {
        qnumber t = QScriptValueImpl::get(context->thisObject())->internalValue().toNumber();
        qnumber ms = context->argument(0).toNumber();
        t = TimeClip(MakeDate(Day(t), MakeTime(HourFromTime(t), MinFromTime(t), SecFromTime(t), ms)));
        QScriptValue r = eng->scriptValue(t);
        QScriptValueImpl::get(context->thisObject())->setInternalValue(r);
        return r;
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.setUTCMilliseconds"));
}

QScriptValue Date::method_setSeconds(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    QScriptContext *context = eng->currentContext();
    if (QScriptValueImpl::get(context->thisObject())->classInfo() == classInfo) {
        qnumber t = LocalTime(QScriptValueImpl::get(context->thisObject())->internalValue().toNumber());
        qnumber sec = context->argument(0).toNumber();
        qnumber ms = (context->argumentCount() < 2) ? msFromTime(t) : context->argument(1).toNumber();
        t = TimeClip(UTC(MakeDate(Day(t), MakeTime(HourFromTime(t), MinFromTime(t), sec, ms))));
        QScriptValue r = eng->scriptValue(t);
        QScriptValueImpl::get(context->thisObject())->setInternalValue(r);
        return r;
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.setSeconds"));
}

QScriptValue Date::method_setUTCSeconds(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    QScriptContext *context = eng->currentContext();
    if (QScriptValueImpl::get(context->thisObject())->classInfo() == classInfo) {
        qnumber t = QScriptValueImpl::get(context->thisObject())->internalValue().toNumber();
        qnumber sec = context->argument(0).toNumber();
        qnumber ms = (context->argumentCount() < 2) ? msFromTime(t) : context->argument(1).toNumber();
        t = TimeClip(MakeDate(Day(t), MakeTime(HourFromTime(t), MinFromTime(t), sec, ms)));
        QScriptValue r = eng->scriptValue(t);
        QScriptValueImpl::get(context->thisObject())->setInternalValue(r);
        return r;
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.setUTCSeconds"));
}

QScriptValue Date::method_setMinutes(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    QScriptContext *context = eng->currentContext();
    if (QScriptValueImpl::get(context->thisObject())->classInfo() == classInfo) {
        qnumber t = LocalTime(QScriptValueImpl::get(context->thisObject())->internalValue().toNumber());
        qnumber min = context->argument(0).toNumber();
        qnumber sec = (context->argumentCount() < 2) ? SecFromTime(t) : context->argument(1).toNumber();
        qnumber ms = (context->argumentCount() < 3) ? msFromTime(t) : context->argument(2).toNumber();
        t = TimeClip(UTC(MakeDate(Day(t), MakeTime(HourFromTime(t), min, sec, ms))));
        QScriptValue r = eng->scriptValue(t);
        QScriptValueImpl::get(context->thisObject())->setInternalValue(r);
        return r;
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.setMinutes"));
}

QScriptValue Date::method_setUTCMinutes(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    QScriptContext *context = eng->currentContext();
    if (QScriptValueImpl::get(context->thisObject())->classInfo() == classInfo) {
        qnumber t = QScriptValueImpl::get(context->thisObject())->internalValue().toNumber();
        qnumber min = context->argument(0).toNumber();
        qnumber sec = (context->argumentCount() < 2) ? SecFromTime(t) : context->argument(1).toNumber();
        qnumber ms = (context->argumentCount() < 3) ? msFromTime(t) : context->argument(2).toNumber();
        t = TimeClip(MakeDate(Day(t), MakeTime(HourFromTime(t), min, sec, ms)));
        QScriptValue r = eng->scriptValue(t);
        QScriptValueImpl::get(context->thisObject())->setInternalValue(r);
        return r;
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.setUTCMinutes"));
}

QScriptValue Date::method_setHours(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    QScriptContext *context = eng->currentContext();
    if (QScriptValueImpl::get(context->thisObject())->classInfo() == classInfo) {
        qnumber t = LocalTime(QScriptValueImpl::get(context->thisObject())->internalValue().toNumber());
        qnumber hour = context->argument(0).toNumber();
        qnumber min = (context->argumentCount() < 2) ? MinFromTime(t) : context->argument(1).toNumber();
        qnumber sec = (context->argumentCount() < 3) ? SecFromTime(t) : context->argument(2).toNumber();
        qnumber ms = (context->argumentCount() < 4) ? msFromTime(t) : context->argument(3).toNumber();
        t = TimeClip(UTC(MakeDate(Day(t), MakeTime(hour, min, sec, ms))));
        QScriptValue r = eng->scriptValue(t);
        QScriptValueImpl::get(context->thisObject())->setInternalValue(r);
        return r;
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.setHours"));
}

QScriptValue Date::method_setUTCHours(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    QScriptContext *context = eng->currentContext();
    if (QScriptValueImpl::get(context->thisObject())->classInfo() == classInfo) {
        qnumber t = QScriptValueImpl::get(context->thisObject())->internalValue().toNumber();
        qnumber hour = context->argument(0).toNumber();
        qnumber min = (context->argumentCount() < 2) ? MinFromTime(t) : context->argument(1).toNumber();
        qnumber sec = (context->argumentCount() < 3) ? SecFromTime(t) : context->argument(2).toNumber();
        qnumber ms = (context->argumentCount() < 4) ? msFromTime(t) : context->argument(3).toNumber();
        t = TimeClip(MakeDate(Day(t), MakeTime(hour, min, sec, ms)));
        QScriptValue r = eng->scriptValue(t);
        QScriptValueImpl::get(context->thisObject())->setInternalValue(r);
        return r;
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.setUTCHours"));
}

QScriptValue Date::method_setDate(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    QScriptContext *context = eng->currentContext();
    if (QScriptValueImpl::get(context->thisObject())->classInfo() == classInfo) {
        qnumber t = LocalTime(QScriptValueImpl::get(context->thisObject())->internalValue().toNumber());
        qnumber date = context->argument(0).toNumber();
        t = TimeClip(UTC(MakeDate(MakeDay(YearFromTime(t), MonthFromTime(t), date), TimeWithinDay(t))));
        QScriptValue r = eng->scriptValue(t);
        QScriptValueImpl::get(context->thisObject())->setInternalValue(r);
        return r;
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.setDate"));
}

QScriptValue Date::method_setUTCDate(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    QScriptContext *context = eng->currentContext();
    if (QScriptValueImpl::get(context->thisObject())->classInfo() == classInfo) {
        qnumber t = QScriptValueImpl::get(context->thisObject())->internalValue().toNumber();
        qnumber date = context->argument(0).toNumber();
        t = TimeClip(MakeDate(MakeDay(YearFromTime(t), MonthFromTime(t), date), TimeWithinDay(t)));
        QScriptValue r = eng->scriptValue(t);
        QScriptValueImpl::get(context->thisObject())->setInternalValue(r);
        return r;
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.setUTCDate"));
}

QScriptValue Date::method_setMonth(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    QScriptContext *context = eng->currentContext();
    if (QScriptValueImpl::get(context->thisObject())->classInfo() == classInfo) {
        qnumber t = LocalTime(QScriptValueImpl::get(context->thisObject())->internalValue().toNumber());
        qnumber month = context->argument(0).toNumber();
        qnumber date = (context->argumentCount() < 2) ? DateFromTime(t) : context->argument(1).toNumber();
        t = TimeClip(UTC(MakeDate(MakeDay(YearFromTime(t), month, date), TimeWithinDay(t))));
        QScriptValue r = eng->scriptValue(t);
        QScriptValueImpl::get(context->thisObject())->setInternalValue(r);
        return r;
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.setMonth"));
}

QScriptValue Date::method_setUTCMonth(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    QScriptContext *context = eng->currentContext();
    if (QScriptValueImpl::get(context->thisObject())->classInfo() == classInfo) {
        qnumber t = QScriptValueImpl::get(context->thisObject())->internalValue().toNumber();
        qnumber month = context->argument(0).toNumber();
        qnumber date = (context->argumentCount() < 2) ? DateFromTime(t) : context->argument(1).toNumber();
        t = TimeClip(MakeDate(MakeDay(YearFromTime(t), month, date), TimeWithinDay(t)));
        QScriptValue r = eng->scriptValue(t);
        QScriptValueImpl::get(context->thisObject())->setInternalValue(r);
        return r;
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.setUTCMonth"));
}

QScriptValue Date::method_setFullYear(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    QScriptContext *context = eng->currentContext();
    if (QScriptValueImpl::get(context->thisObject())->classInfo() == classInfo) {
        qnumber t = LocalTime(QScriptValueImpl::get(context->thisObject())->internalValue().toNumber());
        qnumber year = context->argument(0).toNumber();
        qnumber month = (context->argumentCount() < 2) ? MonthFromTime(t) : context->argument(1).toNumber();
        qnumber date = (context->argumentCount() < 3) ? DateFromTime(t) : context->argument(2).toNumber();
        t = TimeClip(UTC(MakeDate(MakeDay(year, month, date), TimeWithinDay(t))));
        QScriptValue r = eng->scriptValue(t);
        QScriptValueImpl::get(context->thisObject())->setInternalValue(r);
        return r;
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.setFullYear"));
}

QScriptValue Date::method_setUTCFullYear(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    QScriptContext *context = eng->currentContext();
    if (QScriptValueImpl::get(context->thisObject())->classInfo() == classInfo) {
        qnumber t = QScriptValueImpl::get(context->thisObject())->internalValue().toNumber();
        qnumber year = context->argument(0).toNumber();
        qnumber month = (context->argumentCount() < 2) ? MonthFromTime(t) : context->argument(1).toNumber();
        qnumber date = (context->argumentCount() < 3) ? DateFromTime(t) : context->argument(2).toNumber();
        t = TimeClip(MakeDate(MakeDay(year, month, date), TimeWithinDay(t)));
        QScriptValue r = eng->scriptValue(t);
        QScriptValueImpl::get(context->thisObject())->setInternalValue(r);
        return r;
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Date.prototype.setUTCFullYear"));
}

QScriptValue Date::method_setYear(QScriptEngine *eng, QScriptClassInfo *)
{
    QScriptContext *context = eng->currentContext();
    return QScriptContextPrivate::get(context)->throwNotImplemented(
        QLatin1String("Date.prototype.setYear"));
}

QScriptValue Date::method_toUTCString(QScriptEngine *eng, QScriptClassInfo *)
{
    QScriptContext *context = eng->currentContext();
    return QScriptContextPrivate::get(context)->throwNotImplemented(
        QLatin1String("Date.prototype.toUTCString"));
}

} } // namespace QScript::Ecma

