/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtGui>

#include "highlighter.h"

Highlighter::Highlighter(QObject *parent)
    : QObject(parent)
{
}

void Highlighter::addToDocument(QTextDocument *doc)
{
    connect(doc, SIGNAL(contentsChange(int, int, int)),
            this, SLOT(highlight(int, int, int)));
}

void Highlighter::addMapping(const QString &pattern,
                             const QTextCharFormat &format)
{
    mappings[pattern] = format;
}

void Highlighter::highlight(int position, int removed, int added)
{
    QTextDocument *doc = qobject_cast<QTextDocument *>(sender());

    QTextBlock block = doc->findBlock(position);
    if (!block.isValid())
        return;

    QTextBlock endBlock;
    if (added > removed)
        endBlock = doc->findBlock(position + added);
    else
        endBlock = block;

    while (block.isValid() && !(endBlock < block)) {
        highlightBlock(block);
        block = block.next();
    }
}

void Highlighter::highlightBlock(QTextBlock block)
{
    QTextLayout *layout = block.layout();
    const QString text = block.text();

    QList<QTextLayout::FormatRange> overrides;

    foreach (QString pattern, mappings.keys()) {
        QRegExp expression(pattern);
        int i = text.indexOf(expression);
        while (i >= 0) {
            QTextLayout::FormatRange range;
            range.start = i;
            range.length = expression.matchedLength();
            range.format = mappings[pattern];
            overrides << range;

            i = text.indexOf(expression, i + expression.matchedLength());
        }
    }

    layout->setAdditionalFormats(overrides);
    const_cast<QTextDocument *>(block.document())->markContentsDirty(
        block.position(), block.length());
}
