/*
  mangenerator.h
*/

#ifndef MANGENERATOR_H
#define MANGENERATOR_H

#include "webgenerator.h"

class ManGenerator : public WebGenerator
{
public:
    ManGenerator();
    ~ManGenerator();

protected:
    virtual QString fileBase( const Node *node );
    virtual QString fileExtension( const Node *node );
    virtual void generateNamespaceNode( const NamespaceNode *namespasse,
					const CodeMarker *marker );
    virtual void generateClassNode( const ClassNode *classe,
				    const CodeMarker *marker );

private:
    virtual void generateAtom( const Atom *atom, const Node *relative,
			       const CodeMarker *marker );
    void generateHeader( const InnerNode *node );
    void generateFooter( const InnerNode *node );
    QString protectArg( const QString& str );
    QString protectTextLine( const QString& str );

    QString date;
};

#endif
