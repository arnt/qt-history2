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
