/****************************************************************************
** $Id: $
**
** Definition of QRegExp class
**
** Created : 950126
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QREGEXP_H
#define QREGEXP_H

#ifndef QT_H
#include "qstringlist.h"
#endif // QT_H

class QRegExpEngine;
struct QRegExpPrivate;

class QRegExp
{
public:
    enum CaretMode { CaretAtZero, CaretAtOffset, CaretWontMatch };

    QRegExp();
    QRegExp(const QString& pattern,
	     QString::CaseSensitivity cs = QString::CaseSensitive,
	     bool wildcard = false);
    QRegExp(const QRegExp& rx);
    ~QRegExp();
    QRegExp& operator=(const QRegExp& rx);

    bool operator==(const QRegExp& rx) const;
    bool operator!=(const QRegExp& rx) const { return !operator==(rx); }

    bool isEmpty() const;
    bool isValid() const;
    QString pattern() const;
    void setPattern(const QString& pattern);
    bool caseSensitive() const;
    void setCaseSensitive(bool sensitive);
#ifndef QT_NO_REGEXP_WILDCARD
    bool wildcard() const;
    void setWildcard(bool wildcard);
#endif
    bool minimal() const;
    void setMinimal(bool minimal);

    bool exactMatch(const QString& str) const;

#if defined(Q_QDOC)
    int search(const QString& str, int offset = 0,
		CaretMode caretMode = CaretAtZero) const;
    int searchRev(const QString& str, int offset = -1,
		   CaretMode caretMode = CaretAtZero) const;
#else
    // ### Qt 4.0: reduce these four to two functions
    int search(const QString& str, int offset = 0) const;
    int search(const QString& str, int offset, CaretMode caretMode) const;
    int searchRev(const QString& str, int offset = -1) const;
    int searchRev(const QString& str, int offset, CaretMode caretMode) const;
#endif
    int matchedLength() const;
#ifndef QT_NO_REGEXP_CAPTURE
    int numCaptures() const;
    QStringList capturedTexts();
    QString cap(int nth = 0);
    int pos(int nth = 0);
    QString errorString();
#endif

    static QString escape(const QString& str);

#ifndef QT_NO_COMPAT
    QRegExp(const QString& pattern, bool caseSensitive, bool wildcard = false);
#endif
private:
    void prepareEngine() const;
    void prepareEngineForMatch(const QString& str) const;
    void invalidateEngine();

    static int caretIndex(int offset, CaretMode caretMode);

    QRegExpEngine *eng;
    QRegExpPrivate *priv;
};
#endif // QREGEXP_H
