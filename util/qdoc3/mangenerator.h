/*
  mangenerator.h
*/

#ifndef MANGENERATOR_H
#define MANGENERATOR_H

#include "pagegenerator.h"

class ManGenerator : public PageGenerator
{
public:
    ManGenerator();
    ~ManGenerator();

    virtual QString format();

protected:
    virtual int generateAtom( const Atom *atom, const Node *relative,
			      CodeMarker *marker );
    virtual void generateClassLikeNode(const InnerNode *node, CodeMarker *marker);
    virtual void generateFakeNode( const FakeNode *fake, CodeMarker *marker );
    virtual QString fileExtension(const Node *node);

private:
    void generateHeader( const QString& name );
    void generateFooter();
    QString protectArg( const QString& str );
    QString protectTextLine( const QString& str );

    QString date;
};

#endif
