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

#ifndef QDATETIME_P_H
#define QDATETIME_P_H

#include "qplatformdefs.h"

#include "qatomic.h"
#include "qdatetime.h"
#include "qlist.h"
#include "qvariant.h"

class QDateTimePrivate
{
public:
    enum Spec { LocalUnknown = -1, LocalStandard = 0, LocalDST = 1, UTC = 2 };

    QDateTimePrivate() : ref(1), spec(LocalUnknown) {}
    QDateTimePrivate(const QDateTimePrivate &other)
        : ref(1), date(other.date), time(other.time), spec(other.spec)
    {}

    QAtomic ref;
    QDate date;
    QTime time;
    Spec spec;

    Spec getLocal(QDate &outDate, QTime &outTime) const;
    void getUTC(QDate &outDate, QTime &outTime) const;
};


class QFormatSection;
class QDateTimeParser
{
public:
    enum QDateTimeParserSkipMode {
        SkipNone,
        SkipForward,
        SkipBackward
    };
    enum Section {
        NoSection = 0x0000000,

        Day1 = 0x0000001,
        Day2 = 0x0000002,
        Day3 = 0x0000004,
        Day4 = 0x0000008,
        DayMask = (Day1|Day2|Day3|Day4),

        Month1 = 0x0000010,
        Month2 = 0x0000020,
        Month3 = 0x0000040,
        Month4 = 0x0000080,
        MonthMask = (Month1|Month2|Month3|Month4),

        Year2 = 0x0000100,
        Year4 = 0x0000200,
        YearMask = (Year2|Year4),
        DateMask = (DayMask|MonthMask|YearMask),

        Hour1 = 0x0000400,
        Hour2 = 0x0000800,
        HourMask = (Hour1|Hour2),

        Minute1 = 0x0001000,
        Minute2 = 0x0002000,
        MinuteMask = (Minute1|Minute2),

        Second1 = 0x0004000,
        Second2 = 0x0008000,
        SecondMask = (Second1|Second2),

        MSecond1 = 0x0010000,
        MSecond3 = 0x0020000,
        MSecondMask = (MSecond1|MSecond3),

        APLower = 0x0040000,
        APUpper = 0x0080000,
        APMask = (APLower|APUpper),

        TimeMask = (HourMask|MinuteMask|SecondMask|MSecondMask),

        Quote = 0x0100000,
        Separator = 0x0200000,
        FirstSection = 0x0400000,
        LastSection = 0x0800000
    };

    QDateTimeParser(const QString &f = QString(), QVariant::Type t = QVariant::DateTime);
    bool isSpecial(const QChar &c) const;
    QFormatSection findNextFormat(const QString &str, const int start);
    void parseFormat(const QString &format, QVariant::Type t);
    bool fromString(const QString &string, QDate *dateIn, QTime *timeIn);

    static bool bounds(QDateTimeParser::Section t, int num);
    static int getNumber(int index, const QString &str, int mindigits, int maxdigits, bool *ok, int *digits);

    static QFormatSection firstSection;
    static QFormatSection lastSection;

    QVariant::Type formatType;
    QList<QFormatSection> sect;
    QString format;
    uint display;
};

class QFormatSection
{
public:
    QFormatSection(int ind, const QString &sep);
    QFormatSection(int ind = -1, QDateTimeParser::Section typ = QDateTimeParser::NoSection);
    int length() const;
    static int length(QDateTimeParser::Section t);
    bool variableLength() const;

    int index;
    QString chars;
    QDateTimeParser::Section type;
};

#endif // QDATETIME_P_H
