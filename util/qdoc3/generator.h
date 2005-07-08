/*
  generator.h
*/

#ifndef GENERATOR_H
#define GENERATOR_H

#include <qlist.h>
#include <qmap.h>
#include <qregexp.h>
#include <qstring.h>
#include <qstringlist.h>

#include "node.h"
#include "text.h"

class ClassNode;
class Config;
class CodeMarker;
class FakeNode;
class FunctionNode;
class InnerNode;
class Location;
class NamespaceNode;
class Node;
class Tree;

class Generator
{
public:
    Generator();
    virtual ~Generator();

    virtual void initializeGenerator(const Config &config);
    virtual void terminateGenerator();
    virtual QString format() = 0;
    virtual void generateTree(const Tree *tree, CodeMarker *marker) = 0;

    static void initialize( const Config& config );
    static void terminate();
    static Generator *generatorForFormat( const QString& format );

protected:
    virtual void startText( const Node *relative, CodeMarker *marker );
    virtual void endText( const Node *relative, CodeMarker *marker );
    virtual int generateAtom( const Atom *atom, const Node *relative,
			      CodeMarker *marker );
    virtual void generateClassLikeNode(const InnerNode *inner, CodeMarker *marker);
    virtual void generateFakeNode( const FakeNode *fake, CodeMarker *marker );

    virtual void generateText( const Text& text, const Node *relative,
			       CodeMarker *marker );
    virtual void generateBody( const Node *node, CodeMarker *marker );
    virtual void generateAlsoList( const Node *node, CodeMarker *marker );
    virtual void generateInherits( const ClassNode *classe,
				   CodeMarker *marker );
    virtual void generateInheritedBy( const ClassNode *classe,
				      CodeMarker *marker );

    void generateThreadSafeness( const Node *node, CodeMarker *marker );
    void generateSince(const Node *node, CodeMarker *marker);
    void generateStatus( const Node *node, CodeMarker *marker );
    const Atom *generateAtomList( const Atom *atom, const Node *relative,
				  CodeMarker *marker, bool generate,
				  int& numGeneratedAtoms );
    void generateExampleFiles(const FakeNode *fake, CodeMarker *marker);
    void generateModuleName( const ClassNode *classe, CodeMarker *marker);

    const QString& outputDir() { return outDir; }
    QString indent( int level, const QString& markedCode );
    QString plainCode( const QString& markedCode );
    QString typeString( const Node *node );
    QString imageFileName( const Location& location, const QString& fileBase );
    void setImageFileExtensions( const QStringList& extensions );
    void unknownAtom( const Atom *atom );
    QMap<QString, QString> &formattingLeftMap();
    QMap<QString, QString> &formattingRightMap();

    static QString trimmedTrailing(const QString &string);
    static bool matchAhead( const Atom *atom, Atom::Type expectedAtomType );

private:
    void generateOverload( const Node *node, CodeMarker *marker );
    void generateReimplementedFrom( const FunctionNode *func,
				    CodeMarker *marker );
    void appendFullName( Text& text, const Node *apparentNode,
			 const Node *relative, CodeMarker *marker,
			 const Node *actualNode = 0 );
    void appendSortedNames(Text& text, const ClassNode *classe,
                           const QList<RelatedClass> &classes,
                           CodeMarker *marker);

    QRegExp amp;
    QRegExp lt;
    QRegExp gt;
    QRegExp quot;
    QRegExp tag;

    static QList<Generator *> generators;
    static QMap<QString, QMap<QString, QString> > fmtLeftMaps;
    static QMap<QString, QMap<QString, QString> > fmtRightMaps;
    static QMap<QString, QStringList> imgFileExts;
    static QStringList imageFiles;
    static QStringList imageDirs;
    static QString outDir;
    static QString project;
};

#endif
