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

#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H

#include <QHash>
#include <QString>
#include <QStringList>
#include <QTextBlock>
#include <QTextCharFormat>

class QTextDocument;

class Highlighter : public QObject
{
    Q_OBJECT
public:
    Highlighter(QObject *parent = 0);

    void addToDocument(QTextDocument *doc);
    void addMapping(const QString &pattern, const QTextCharFormat &format);

private slots:
    void highlight(int from, int removed, int added);

private:
    void highlightBlock(QTextBlock block);

    QHash<QString,QTextCharFormat> mappings;
};

#endif
