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
    virtual void generateAtom( const Atom *atom, const Node *relative,
			       CodeMarker *marker );
    virtual void generateNamespaceNode( const NamespaceNode *namespasse,
					CodeMarker *marker );
    virtual void generateClassNode( const ClassNode *classe,
				    CodeMarker *marker );
    virtual void generateFakeNode( const FakeNode *fake, CodeMarker *marker );
    virtual QString fileBase( const Node *node );
    virtual QString fileExtension( const Node *node );

private:
    void generateHeader( const QString& name );
    void generateFooter();
    QString protectArg( const QString& str );
    QString protectTextLine( const QString& str );

    QString date;
};

#endif
