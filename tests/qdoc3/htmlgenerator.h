/*
  htmlgenerator.h
*/

#ifndef HTMLGENERATOR_H
#define HTMLGENERATOR_H

#include <qregexp.h>

#include "webgenerator.h"

class QTextStream;

class HtmlGenerator : public WebGenerator
{
public:
    HtmlGenerator();
    ~HtmlGenerator() { }

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
    void generateHeader( QTextStream& outStream );
    void generateFooter( QTextStream& outStream );
    void generateListOfAllMemberFunctions( const ClassNode *classe,
					   const CodeMarker *marker );
    QString protect( const QString& string );
    QString synopsys( const Node *node, const InnerNode *relative,
		      const CodeMarker *marker,
		      CodeMarker::SynopsysStyle style );
    QString highlightedCode( const QString& markedCode, const Node *relative );
    QString refForNode( const Node *node );
    QString linkForNode( const Node *node, const Node *relative );

    QString link;
    QRegExp amp;
    QRegExp lt;
    QRegExp gt;
    QRegExp quot;
};

#endif
