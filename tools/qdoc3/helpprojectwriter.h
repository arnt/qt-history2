/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef HELPPROJECTWRITER_H
#define HELPPROJECTWRITER_H

#include <QString>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

class Node;
class Tree;

class HelpProjectWriter
{
public:
    HelpProjectWriter(const QHash<QString, QString> defs);
    void generate(const Tree *tre, const QString &outputDir);
    void addExtraFile(const QString &file);
    void addExtraFiles(const QSet<QString> &files);

private:
    void generateSections(QXmlStreamWriter &writer, const Node *node);
    bool generateSection(QXmlStreamWriter &writer, const Node *node);

    const Tree *tree;

    QList<QStringList> keywords;
    QSet<QString> files;
    QString helpNamespace;
    QString virtualFolder;
    QString fileName;
    QString indexPage;
    QString indexTitle;
    QSet<QString> extraFiles;
};

#endif
