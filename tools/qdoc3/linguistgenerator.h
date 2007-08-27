/****************************************************************************
**
** Copyright (C) 2007-$THISYEAR$ $TROLLTECH$. All rights reserved.
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
  LinguistGenerator.h
*/

#ifndef LINGUISTGENERATOR_H
#define LINGUISTGENERATOR_H

#include <qmap.h>
#include <qregexp.h>
#include <qdom.h>

#include "codemarker.h"
#include "config.h"
#include "pagegenerator.h"

class LinguistGenerator : public PageGenerator
{
public:
    LinguistGenerator();
    ~LinguistGenerator();

    virtual void initializeGenerator( const Config& config );
    virtual void terminateGenerator();
    virtual QString format();

protected:
    virtual void generateClassLikeNode(const InnerNode *inner,
                                       CodeMarker *marker);
    virtual void generateFakeNode( const FakeNode *fake, CodeMarker *marker );
    virtual QString fileExtension(const Node *node);

    QList<QDomElement> generateIndexSections(QDomDocument &document,
                       const Node *node, CodeMarker *marker);
    virtual QString fullName(const Node *node) const;

private:
    QString simplified(const QString &text) const;
};

#endif
