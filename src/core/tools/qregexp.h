/****************************************************************************
**
** Definition of QRegExp class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QREGEXP_H
#define QREGEXP_H

#ifndef QT_H
#include "qstring.h"
#endif // QT_H

struct QRegExpPrivate;
class QStringList;

class Q_CORE_EXPORT QRegExp
{
public:
    enum PatternSyntax { RegExp, Wildcard };
    enum CaretMode { CaretAtZero, CaretAtOffset, CaretWontMatch };

    QRegExp();
    explicit QRegExp(const QString &pattern, QString::CaseSensitivity cs = QString::CaseSensitive,
		     PatternSyntax syntax = RegExp);
    QRegExp(const QRegExp &rx);
    ~QRegExp();
    QRegExp &operator=(const QRegExp &rx);

    bool operator==(const QRegExp &rx) const;
    inline bool operator!=(const QRegExp &rx) const { return !operator==(rx); }

    bool isEmpty() const;
    bool isValid() const;
    QString pattern() const;
    void setPattern(const QString &pattern);
    QString::CaseSensitivity caseSensitivity() const;
    void setCaseSensitivity(QString::CaseSensitivity cs);
#ifdef QT_COMPAT
    inline QT_COMPAT bool caseSensitive() const { return caseSensitivity() == QString::CaseSensitive; }
    inline QT_COMPAT void setCaseSensitive(bool sensitive)
    { setCaseSensitivity(sensitive ? QString::CaseSensitive : QString::CaseInsensitive); }
#endif
#ifndef QT_NO_REGEXP_WILDCARD
    PatternSyntax patternSyntax() const;
    void setPatternSyntax(PatternSyntax syntax);
#ifdef QT_COMPAT
    inline QT_COMPAT bool wildcard() const { return patternSyntax() == Wildcard; }
    inline QT_COMPAT void setWildcard(bool wildcard)
    { setPatternSyntax(wildcard ? Wildcard : RegExp); }
#endif
#endif

    bool isMinimalMatching() const;
    void setMinimalMatching(bool minimal);
#ifdef QT_COMPAT
    inline QT_COMPAT bool minimal() const { return isMinimalMatching(); }
    inline QT_COMPAT void setMinimal(bool minimal) { setMinimalMatching(minimal); }
#endif

    bool exactMatch(const QString &str) const;

    int indexIn(const QString &str, int offset = 0, CaretMode caretMode = CaretAtZero) const;
    int lastIndexIn(const QString &str, int offset = -1, CaretMode caretMode = CaretAtZero) const;
#ifdef QT_COMPAT
    inline QT_COMPAT int search(const QString &str, int from = 0,
                                CaretMode caretMode = CaretAtZero) const
    { return indexIn(str, from, caretMode); }
    inline QT_COMPAT int searchRev(const QString &str, int from = -1,
                                   CaretMode caretMode = CaretAtZero) const
    { return lastIndexIn(str, from, caretMode); }
#endif
    int matchedLength() const;
#ifndef QT_NO_REGEXP_CAPTURE
    int numCaptures() const;
    QStringList capturedTexts();
    QString cap(int nth = 0);
    int pos(int nth = 0);
    QString errorString();
#endif

    static QString escape(const QString &str);

#ifdef QT_COMPAT
    inline QRegExp(const QString &pattern, bool cs, bool wildcard = false)
    {
        new (this)
            QRegExp(pattern, cs ? QString::CaseSensitive : QString::CaseInsensitive,
                    wildcard ? Wildcard : RegExp);
    }
#endif

private:
    QRegExpPrivate *priv;
};

#endif // QREGEXP_H
