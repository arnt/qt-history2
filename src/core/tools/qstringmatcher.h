/****************************************************************************
**
** Definition of QStringMatcher class.
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

#ifndef QSTRINGMATCHER_H
#define QSTRINGMATCHER_H

#ifndef QT_H
#include "qstring.h"
#endif // QT_H

class QStringMatcherPrivate;

class Q_CORE_EXPORT QStringMatcher
{
public:
    QStringMatcher();
    explicit QStringMatcher(const QString &pattern,
                            Qt::CaseSensitivity cs = Qt::CaseSensitive);
    QStringMatcher(const QStringMatcher &other);
    ~QStringMatcher();

    QStringMatcher &operator=(const QStringMatcher &other);

    void setPattern(const QString &pattern);
    void setCaseSensitivity(Qt::CaseSensitivity cs);

    int indexIn(const QString &str, int from = 0) const;
    inline QString pattern() const { return q_pattern; }
    inline Qt::CaseSensitivity caseSensitivity() const { return q_cs; }

private:
    QStringMatcherPrivate *d_ptr;
    QString q_pattern;
    Qt::CaseSensitivity q_cs;
    uint q_skiptable[256];
};

#endif // QSTRINGMATCHER_H
