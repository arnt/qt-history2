/****************************************************************************
**
** Definition of the QSyntaxHighlighter class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSYNTAXHIGHLIGHTER_H
#define QSYNTAXHIGHLIGHTER_H

#ifndef QT_H
#include "qfont.h"
#include "qcolor.h"
#include "qstring.h"
#endif // QT_H

class Q3TextEdit;
class QSyntaxHighlighterInternal;
class QSyntaxHighlighterPrivate;
class Q3TextParagraph;

class Q_COMPAT_EXPORT QSyntaxHighlighter : public Qt
{
    friend class QSyntaxHighlighterInternal;

public:
    QSyntaxHighlighter(Q3TextEdit *textEdit);
    virtual ~QSyntaxHighlighter();

    virtual int highlightParagraph(const QString &text, int endStateOfLastPara) = 0;

    void setFormat(int start, int count, const QFont &font, const QColor &color);
    void setFormat(int start, int count, const QColor &color);
    void setFormat(int start, int count, const QFont &font);
    Q3TextEdit *textEdit() const { return edit; }

    void rehighlight();

    int currentParagraph() const;

private:
    Q3TextParagraph *para;
    Q3TextEdit *edit;
    QSyntaxHighlighterPrivate *d;

};

#endif
