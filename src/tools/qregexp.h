/****************************************************************************
**
** Definition of QRegExp class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
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
#include "qstringlist.h"
#endif // QT_H

struct QRegExpPrivate;

class Q_CORE_EXPORT QRegExp
{
public:
    enum CaretMode { CaretAtZero, CaretAtOffset, CaretWontMatch };

    QRegExp();
    QRegExp(const QString &pattern, QString::CaseSensitivity cs = QString::CaseSensitive,
	    bool wildcard = false);
    QRegExp(const QRegExp &rx);
    ~QRegExp();
    QRegExp &operator=(const QRegExp &rx);

    bool operator==(const QRegExp &rx) const;
    inline bool operator!=(const QRegExp &rx) const { return !operator==(rx); }

    bool isEmpty() const;
    bool isValid() const;
    QString pattern() const;
    void setPattern(const QString &pattern);
    bool caseSensitive() const;
    void setCaseSensitive(bool sensitive);
#ifndef QT_NO_REGEXP_WILDCARD
    bool wildcard() const;
    void setWildcard(bool wildcard);
#endif
    bool minimal() const;
    void setMinimal(bool minimal);

    bool exactMatch(const QString &str) const;

    int search(const QString &str, int offset = 0, CaretMode caretMode = CaretAtZero) const;
    int searchRev(const QString &str, int offset = -1, CaretMode caretMode = CaretAtZero) const;
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
    QRegExp(const QString &pattern, bool caseSensitive, bool wildcard = false);
#endif

private:
    QRegExpPrivate *priv;
};

#endif // QREGEXP_H
