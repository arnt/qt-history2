#include <qfile.h>

#include "qsakernelparser.h"
#include "tokenizer.h"
#include "tree.h"

QsaKernelParser::QsaKernelParser( Tree *cppTree )
    : cppTre( cppTree )
{
}

QsaKernelParser::~QsaKernelParser()
{
}

QString QsaKernelParser::language()
{
    return "QSA Kernel C++";
}

QString QsaKernelParser::sourceFileNameFilter()
{
    return "*.cpp";
}

void QsaKernelParser::parseSourceFile( const Location& location,
				       const QString& filePath,
				       Tree * /* tree */ )
{
    FILE *in = fopen( QFile::encodeName(filePath), "r" );
    if ( in == 0 ) {
        location.error( tr("Cannot open QSA kernel file '%1'").arg(filePath) );
        return;
    }

    Location fileLocation( filePath );
    FileTokenizer fileTokenizer( fileLocation, in );
    tokenizer = &fileTokenizer;
    readToken();

    QString ident;
    QString className;
    int delimDepth = 0;

    while ( tok != Tok_Eoi ) {
	if ( tok == Tok_Ident ) {
	    ident = tokenizer->lexeme();
	    readToken();
	    if ( tok == Tok_Gulbrandsen && tokenizer->braceDepth() == 0 &&
		 tokenizer->parenDepth() == 0 ) {
		className = ident;
	    } else if ( ident.startsWith("add") && ident.endsWith("Member") &&
			tok == Tok_LeftParen ) {
		bool isProperty = ident.endsWith( "VariableMember" );
		bool isStatic = ident.startsWith( "addStatic" );
		bool isWritable = !isStatic;

		readToken();
		if ( tok == Tok_String ) {
		    QString member = tokenizer->lexeme();
		    member = member.mid( 1, member.length() - 2 );

		    readToken();
		    if ( tok == Tok_Comma )
			readToken();
		    if ( tok == Tok_Ident && tokenizer->lexeme() == "QSMember" )
			readToken();
		    if ( tok == Tok_LeftParen ) {
			delimDepth++;
			readToken();
		    }

		    while ( tok != Tok_Eoi && tok != Tok_RightParen &&
			    tok != Tok_Semicolon ) {
			if ( tok == Tok_Ident ) {
			    ident = tokenizer->lexeme();
			    if ( ident == "Custom" ) {
				isProperty = true;
			    } else if ( ident == "AttributeNonWritable" ) {
				isWritable = false;
			    } else if ( ident == "AttributeStatic" ) {
				isStatic = true;
			    }
			}
			readToken();
		    }

		    ClassNode *classe =
			    (ClassNode *) cppTre->findNode( QStringList(className),
							    Node::Class );
		    if ( classe == 0 ) {
			classe = new ClassNode( cppTre->root(), className );
			classe->setLocation( tokenizer->location() );
		    }

		    if ( isProperty ) {
			PropertyNode *property = new PropertyNode(classe, member);
			property->setLocation( tokenizer->location() );
			property->setDataType( "Object" );
#if 0
			property->setGetter( member );
			if ( isWritable ) {
			    QString setter = member;
			    setter[0] = setter[0].toUpper();
			    setter.prepend( "set" );
			    property->setSetter( setter );
			}
#endif
		    } else {
			FunctionNode *func = new FunctionNode( classe, member );
			func->setLocation( tokenizer->location() );
			func->setAccess( FunctionNode::Public );
			func->setMetaness( FunctionNode::Slot );
			if ( member == "toLocaleString" ||
			     member == "toString" ) {
			    func->setReturnType( "QString" );
			} else if ( member == "valueOf" ) {
			    func->setReturnType( "Object" );
			} else {
			    func->setReturnType( "Object" );
			    func->addParameter( Parameter("...") );
			}
			func->setStatic( false ); // ###
		    }
		}
	    }
	} else {
	    readToken();
	}
    }
    fclose( in );
}

void QsaKernelParser::doneParsingSourceFiles( Tree * /* tree */ )
{
}

void QsaKernelParser::readToken()
{
    tok = tokenizer->getToken();
}
