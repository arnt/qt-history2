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

#ifndef QSYNTAXHIGHLIGHTER_P_H
#define QSYNTAXHIGHLIGHTER_P_H

#ifndef QT_NO_SYNTAXHIGHLIGHTER
#include "qsyntaxhighlighter.h"
#include "private/qrichtext_p.h"

class QSyntaxHighlighterPrivate
{
public:
    QSyntaxHighlighterPrivate() :
        currentParagraph(-1)
        {}

    int currentParagraph;
};

class QSyntaxHighlighterInternal : public Q3TextPreProcessor
{
public:
    QSyntaxHighlighterInternal(QSyntaxHighlighter *h) : highlighter(h) {}
    void process(Q3TextDocument *doc, Q3TextParagraph *p, int, bool invalidate) {
        if (p->prev() && p->prev()->endState() == -1)
            process(doc, p->prev(), 0, false);

        highlighter->para = p;
        QString text = p->string()->toString();
        int endState = p->prev() ? p->prev()->endState() : -2;
        int oldEndState = p->endState();
        highlighter->d->currentParagraph = p->paragId();
        p->setEndState(highlighter->highlightParagraph(text, endState));
        highlighter->d->currentParagraph = -1;
        highlighter->para = 0;

        p->setFirstPreProcess(false);
        Q3TextParagraph *op = p;
        p = p->next();
        if ((!!oldEndState || !!op->endState()) && oldEndState != op->endState() &&
             invalidate && p && !p->firstPreProcess() && p->endState() != -1) {
            while (p) {
                if (p->endState() == -1)
                    return;
                p->setEndState(-1);
                p = p->next();
            }
        }
    }
    Q3TextFormat *format(int) { return 0; }

private:
    QSyntaxHighlighter *highlighter;

    friend class Q3TextEdit;
};

#endif // QT_NO_SYNTAXHIGHLIGHTER
#endif // QSYNTAXHIGHLIGHTER_P_H
