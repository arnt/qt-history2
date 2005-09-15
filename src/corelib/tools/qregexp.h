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

#ifndef QREGEXP_H
#define QREGEXP_H

#ifndef QT_NO_REGEXP

#include "QtCore/qstring.h"

QT_MODULE(Core)

struct QRegExpPrivate;
class QStringList;

class Q_CORE_EXPORT QRegExp
{
public:
    enum PatternSyntax { RegExp, Wildcard, FixedString };
    enum CaretMode { CaretAtZero, CaretAtOffset, CaretWontMatch };

    QRegExp();
    explicit QRegExp(const QString &pattern, Qt::CaseSensitivity cs = Qt::CaseSensitive,
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
    Qt::CaseSensitivity caseSensitivity() const;
    void setCaseSensitivity(Qt::CaseSensitivity cs);
#ifdef QT3_SUPPORT
    inline QT3_SUPPORT bool caseSensitive() const { return caseSensitivity() == Qt::CaseSensitive; }
    inline QT3_SUPPORT void setCaseSensitive(bool sensitive)
    { setCaseSensitivity(sensitive ? Qt::CaseSensitive : Qt::CaseInsensitive); }
#endif
#ifndef QT_NO_REGEXP_WILDCARD
    PatternSyntax patternSyntax() const;
    void setPatternSyntax(PatternSyntax syntax);
#ifdef QT3_SUPPORT
    inline QT3_SUPPORT bool wildcard() const { return patternSyntax() == Wildcard; }
    inline QT3_SUPPORT void setWildcard(bool wildcard)
    { setPatternSyntax(wildcard ? Wildcard : RegExp); }
#endif
#endif

    bool isMinimal() const;
    void setMinimal(bool minimal);
#ifdef QT3_SUPPORT
    inline QT3_SUPPORT bool minimal() const { return isMinimal(); }
#endif

    bool exactMatch(const QString &str) const;

    int indexIn(const QString &str, int offset = 0, CaretMode caretMode = CaretAtZero) const;
    int lastIndexIn(const QString &str, int offset = -1, CaretMode caretMode = CaretAtZero) const;
#ifdef QT3_SUPPORT
    inline QT3_SUPPORT int search(const QString &str, int from = 0,
                                CaretMode caretMode = CaretAtZero) const
    { return indexIn(str, from, caretMode); }
    inline QT3_SUPPORT int searchRev(const QString &str, int from = -1,
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

#ifdef QT3_SUPPORT
    inline QT3_SUPPORT_CONSTRUCTOR QRegExp(const QString &pattern, bool cs, bool wildcard = false)
    {
        new (this)
            QRegExp(pattern, cs ? Qt::CaseSensitive : Qt::CaseInsensitive,
                    wildcard ? Wildcard : RegExp);
    }
#endif

private:
    QRegExpPrivate *priv;
};
Q_DECLARE_TYPEINFO(QRegExp, Q_MOVABLE_TYPE);

#endif

#endif // QREGEXP_H
