/*
  main.cpp
*/

#include <qstring.h>

#include "config.h"
#include "cppcodemarker.h"
#include "cppcodeparser.h"
#include "htmlgenerator.h"
#include "mangenerator.h"
#include "plaincodemarker.h"
#include "tree.h"

int main( int argc, char **argv )
{
    config = new Config( argc, argv );
    (void) new PlainCodeMarker;
    CodeMarker *cppCodeMarker = new CppCodeMarker;

    Tree cppTree;
    CppCodeParser cppCodeParser;
    cppCodeParser.parseHeaderFile( "include/qregexp.h", &cppTree );
    cppCodeParser.parseSourceFile( "src/qregexp.cpp", &cppTree );
    cppTree.freeze();
    cppCodeParser.convertTree( &cppTree );

    ManGenerator manGenerator;
    manGenerator.generateTree( &cppTree, cppCodeMarker );

    HtmlGenerator htmlGenerator;
    htmlGenerator.generateTree( &cppTree, cppCodeMarker );

    return 0;
}
