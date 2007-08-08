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

/*
  pagegenerator.h
*/

#ifndef PAGEGENERATOR_H
#define PAGEGENERATOR_H

#include <QStack>
#include <qtextstream.h>

#include "generator.h"
#include "location.h"

class ClassNode;
class InnerNode;
class NamespaceNode;

class PageGenerator : public Generator
{
public:
    PageGenerator();
    ~PageGenerator();

    virtual void generateTree( const Tree *tree, CodeMarker *marker );

protected:
    virtual QString fileBase(const Node *node);
    virtual QString fileExtension(const Node *node) = 0;
    QString fileName( const Node *node );
    QString outFileName();
    void beginSubPage( const Location& location, const QString& fileName );
    void endSubPage();
    QTextStream& out();

private:
    void generateInnerNode( const InnerNode *node, CodeMarker *marker );

    QStack<QTextStream *> outStreamStack;
};

#endif
