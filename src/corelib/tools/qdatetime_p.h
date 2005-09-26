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

#include "qplatformdefs.h"
#include "QtCore/qatomic.h"
#include "QtCore/qdatetime.h"
#include "QtCore/qstringlist.h"
#include "QtCore/qlist.h"
#include "QtCore/qvariant.h"

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


class QDateTimeParser
{
public:
    enum QDateTimeParserSkipMode {
        SkipNone,
        SkipForward,
        SkipBackward
    };
    enum Section {
        NoSection = 0x0000,

        DaySection = 0x0001,

        MonthSection = 0x0002,

        YearSection = 0x0004,
        DateMask = (DaySection|MonthSection|YearSection),

        HourSection = 0x0010,
        MinuteSection = 0x0020,
        SecondSection = 0x0040,
        MSecSection = 0x0080,
        AmPmSection = 0x0100,
        TimeMask = (HourSection|MinuteSection|SecondSection|MSecSection)
    };

    struct SectionNode
    {
        QDateTimeParser::Section type;
        int index;
        int count;
    };


    QDateTimeParser(const QString &f = QString(), QVariant::Type t = QVariant::DateTime);
    bool isSpecial(const QChar &c) const;
    void parseFormat(const QString &format, QVariant::Type t);
    bool fromString(const QString &string, QDate *dateIn, QTime *timeIn);
    QString sectionFormat(int index) const;
    QString sectionFormat(Section s, int count) const;

    static bool withinBounds(const SectionNode &sec, int num);
    static int getNumber(int index, const QString &str, int mindigits, int maxdigits, bool *ok, int *digits);

    QVariant::Type formatType;
    QList<SectionNode> sectionNodes;
    QStringList separators;
    QString format, reversedFormat;
    uint display;
    Qt::LayoutDirection layoutDirection;
};


#endif // QDATETIME_P_H
