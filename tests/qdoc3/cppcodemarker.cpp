/*
  cppcodemarker.cpp
*/

#include "cppcodemarker.h"
#include "node.h"

CppCodeMarker::CppCodeMarker()
{
}

CppCodeMarker::~CppCodeMarker()
{
}

bool CppCodeMarker::recognizeCode( const QString& /* code */ )
{
    return FALSE;
}

bool CppCodeMarker::recognizeExtension( const QString& ext )
{
    return ext == "c" || ext == "c++" || ext == "cc" || ext == "cpp" ||
	   ext == "cxx" || ext == "ch" || ext == "h" || ext == "h++" ||
	   ext == "hh" || ext == "hpp" || ext == "hxx";
}

bool CppCodeMarker::recognizeLanguage( const QString& lang )
{
    return lang == "C" || lang == "C++";
}

QString CppCodeMarker::markedUpCode( const QString& code,
				     const Node *relative,
				     const QString& dirPath )
{
    return addMarkUp( protect(code), relative, dirPath );
}

QString CppCodeMarker::markedUpSynopsis( const Node *node, const Node *relative,
					 SynopsisStyle style )
{
    const FunctionNode *func;
    const PropertyNode *property;
    QString synopsis;
    QString extra;
    QString name;

    name = taggedNode( node );
    if ( style == Summary )
	name = linkTag( node, name );
    name = "<@name>" + name + "</@name>";
    if ( style == Detailed && !node->parent()->name().isEmpty() &&
	 node->type() != Node::Property )
	name.prepend( taggedNode(node->parent()) + "::" );

    switch ( node->type() ) {
    case Node::Namespace:
	synopsis = "namespace " + name;
	break;
    case Node::Class:
	synopsis = "class " + name;
	break;
    case Node::Function:
	func = (const FunctionNode *) node;
	if ( !func->returnType().isEmpty() )
	    synopsis = protect( func->returnType() ) + " ";
	synopsis += name + " (";
	if ( !func->parameters().isEmpty() ) {
	    synopsis += " ";
	    QList<Parameter>::ConstIterator p = func->parameters().begin();
	    while ( p != func->parameters().end() ) {
		if ( p != func->parameters().begin() )
		    synopsis += ", ";
		synopsis += protect( (*p).leftType() ) + " <@param>" +
			    protect( (*p).name() ) + "</@param>" +
			    protect( (*p).rightType() );
		if ( !(*p).defaultValue().isEmpty() )
		    synopsis += " = " + protect( (*p).defaultValue() );
		++p;
	    }
	    synopsis += " ";
	}
	synopsis += ")";
	if ( func->isConst() )
	    synopsis += " const";

	if ( style == Summary ) {
	    if ( func->virtualness() != FunctionNode::NonVirtual )
		synopsis.prepend( "virtual " );
	    if ( func->virtualness() == FunctionNode::PureVirtual )
		synopsis.append( " = 0" );
	} else {
	    QStringList bracketed;
	    if ( func->isStatic() ) {
		bracketed += "static";
	    } else if ( func->virtualness() != FunctionNode::NonVirtual ) {
		if ( func->virtualness() == FunctionNode::PureVirtual )
		    bracketed += "pure";
		bracketed += "virtual";
	    }

	    if ( func->access() == Node::Protected ) {
		bracketed += "protected";
	    } else if ( func->access() == Node::Private ) {
		bracketed += "private";
	    }

	    if ( func->metaness() == FunctionNode::Signal ) {
		bracketed += "signal";
	    } else if ( func->metaness() == FunctionNode::Slot ) {
		bracketed += "slot";
	    }
	    if ( !bracketed.isEmpty() )
		extra += " [" + bracketed.join(" ") + "]";
	}
	break;
    case Node::Enum:
	synopsis = "enum " + name;
	break;
    case Node::Typedef:
	synopsis = "typedef " + name;
	break;
    case Node::Property:
	property = (const PropertyNode *) node;
	synopsis = property->dataType() + " " + name;
	if (style == Summary) {
	    if (!property->setter())
		extra += " (read only)";
	}
	break;
    default:
	synopsis = name;
    }

    if ( style == Summary ) {
	if ( node->status() == Node::Preliminary ) {
	    extra += " (preliminary)";
	} else if ( node->status() == Node::Deprecated ) {
	    extra += " (deprecated)";
	} else if ( node->status() == Node::Obsolete ) {
	    extra += " (obsolete)";
	}
    }

    if ( !extra.isEmpty() ) {
	extra.prepend( "<@extra>" );
	extra.append( "</@extra>" );
    }
    return addMarkUp( synopsis, relative, "" ) + extra;
}

QString CppCodeMarker::markedUpName( const Node *node )
{
    QString name = linkTag( node, taggedNode(node) );
    if ( node->type() == Node::Function )
	name += "()";
    return name;
}

QString CppCodeMarker::markedUpFullName( const Node *node,
					 const Node * /* relative */ )
{
    QString fullName;
    for ( ;; ) {
	fullName.prepend( markedUpName(node) );
	if ( node->parent()->name().isEmpty() )
	    break;
	node = node->parent();
	fullName.prepend( "<@op>::</@op>" );
    }
    return fullName;
}

QString CppCodeMarker::markedUpIncludes( const QStringList& includes )
{
    QString code;

    QStringList::ConstIterator inc = includes.begin();
    while ( inc != includes.end() ) {
	code += "#include &lt;<@include>" + *inc + "</@include>" + "&gt;\n";
	++inc;
    }
    return addMarkUp( code, 0, "" );
}

QString CppCodeMarker::functionBeginRegExp( const QString& funcName )
{
    return "^[ \t]*" +
#if QT_VERSION >= 0x030100
	   QRegExp::escape
#endif
	   ( funcName ) + "[^\n]*(\n[ \t][^\n]*)*\\{";

}

QString CppCodeMarker::functionEndRegExp( const QString& /* funcName */ )
{
    return "^}";
}

QList<ClassSection> CppCodeMarker::classSections(const ClassNode *classe, SynopsisStyle style)
{
    QList<ClassSection> sections;

    if ( style == Summary ) {
	FastClassSection privateFunctions( classe, "Private Functions",
					   "function", "functions" );
	FastClassSection privateSlots( classe, "Private Slots", "slot",
				       "slots" );
	FastClassSection privateTypes( classe, "Private Types", "type",
				       "types" );
	FastClassSection properties( classe, "Properties", "property",
				     "properties" );
	FastClassSection protectedFunctions( classe, "Protected Functions",
					     "function", "functions" );
	FastClassSection protectedSlots( classe, "Protected Slots", "slot",
					 "slots" );
	FastClassSection protectedTypes( classe, "Protected Types", "type",
					 "types" );
	FastClassSection publicFunctions( classe, "Public Functions",
					  "function", "functions" );
	FastClassSection publicSignals( classe, "Signals", "signal",
					"signals" );
	FastClassSection publicSlots( classe, "Public Slots", "slot", "slots" );
	FastClassSection publicTypes( classe, "Public Types", "type", "types" );
	FastClassSection relatedNonMemberFunctions( classe,
		"Related Non-member Functions", "function", "functions" );
	FastClassSection staticPrivateMembers( classe,
		"Static Private Members" );
	FastClassSection staticProtectedMembers( classe,
		"Static Protected Members" );
	FastClassSection staticPublicMembers( classe, "Static Public Members" );


	QStack<const ClassNode *> stack;
	stack.push( classe );

	while ( !stack.isEmpty() ) {
	    const ClassNode *ancestorClass = stack.pop();

	    NodeList::ConstIterator c = ancestorClass->childNodes().begin();
	    while ( c != ancestorClass->childNodes().end() ) {
	        bool isSlot = FALSE;
	        bool isSignal = FALSE;
	        bool isStatic = FALSE;
	        if ( (*c)->type() == Node::Function ) {
		    const FunctionNode *func = (const FunctionNode *) *c;
		    isSlot = ( func->metaness() == FunctionNode::Slot );
		    isSignal = ( func->metaness() == FunctionNode::Signal );
		    isStatic = func->isStatic();
	        }

	        switch ( (*c)->access() ) {
	        case Node::Public:
		    if ( isSlot ) {
		        insert( publicSlots, *c, style );
		    } else if ( isSignal ) {
		        insert( publicSignals, *c, style );
		    } else if ( isStatic ) {
		        insert( staticPublicMembers, *c, style );
		    } else if ( (*c)->type() == Node::Property ) {
		        insert( properties, *c, style );
		    } else if ( (*c)->type() == Node::Function ) {
		        insert( publicFunctions, *c, style );
		    } else {
		        insert( publicTypes, *c, style );
		    }
		    break;
	        case Node::Protected:
		    if ( isSlot ) {
		        insert( protectedSlots, *c, style );
		    } else if ( isStatic ) {
		        insert( staticProtectedMembers, *c, style );
		    } else if ( (*c)->type() == Node::Function ) {
		        insert( protectedFunctions, *c, style );
		    } else {
		        insert( protectedTypes, *c, style );
		    }
		    break;
	        case Node::Private:
		    if ( isSlot ) {
		        insert( privateSlots, *c, style );
		    } else if ( isStatic ) {
		        insert( staticPrivateMembers, *c, style );
		    } else if ( (*c)->type() == Node::Function ) {
		        insert( privateFunctions, *c, style );
		    } else {
		        insert( privateTypes, *c, style );
		    }
	        }
	        ++c;
	    }

	    QList<RelatedClass>::ConstIterator r = ancestorClass->baseClasses().begin();
	    while ( r != ancestorClass->baseClasses().end() ) {
		stack.prepend( (*r).node );
		++r;
	    }
	}

	append( sections, properties );
	append( sections, publicTypes );
	append( sections, publicFunctions );
	append( sections, publicSlots );
	append( sections, publicSignals );
	append( sections, staticPublicMembers );
	append( sections, protectedTypes );
	append( sections, protectedFunctions );
	append( sections, protectedSlots );
	append( sections, staticProtectedMembers );
	append( sections, privateTypes );
	append( sections, privateFunctions );
	append( sections, privateSlots );
	append( sections, staticPrivateMembers );
	append( sections, relatedNonMemberFunctions );
    } else if (style == Detailed) {
	FastClassSection memberFunctions(classe, "Member Function Documentation");
	FastClassSection memberTypes(classe, "Member Type Documentation");
	FastClassSection properties(classe, "Property Documentation");
	FastClassSection relatedNonMemberFunctions(classe,
						   "Related Non-member Function Documentation");

	NodeList::ConstIterator c = classe->childNodes().begin();
	while ( c != classe->childNodes().end() ) {
	    if ( (*c)->type() == Node::Enum || (*c)->type() == Node::Typedef ) {
		insert( memberTypes, *c, style );
	    } else if ( (*c)->type() == Node::Property ) {
		insert( properties, *c, style );
	    } else if ( (*c)->type() == Node::Function ) {
		FunctionNode *function = static_cast<FunctionNode *>(*c);
                if (!function->associatedProperty())
		    insert( memberFunctions, function, style );
	    }
	    ++c;
	}

	append( sections, memberTypes );
	append( sections, properties );
	append( sections, memberFunctions );
	append( sections, relatedNonMemberFunctions );
    } else {
	FastClassSection all( classe );

	QStack<const ClassNode *> stack;
	stack.push( classe );

	while ( !stack.isEmpty() ) {
	    const ClassNode *ancestorClass = stack.pop();

	    NodeList::ConstIterator c = ancestorClass->childNodes().begin();
	    while ( c != ancestorClass->childNodes().end() ) {
		if ( (*c)->access() == Node::Public )
		    insert( all, *c, style );
		++c;
	    }

	    QList<RelatedClass>::ConstIterator r = ancestorClass->baseClasses().begin();
	    while ( r != ancestorClass->baseClasses().end() ) {
		stack.prepend( (*r).node );
		++r;
	    }
	}
	append( sections, all );
    }
    return sections;
}

const Node *CppCodeMarker::resolveTarget( const QString& /* target */,
					  const Node * /* relative */ )
{
    return 0;
}

QString CppCodeMarker::addMarkUp( const QString& protectedCode, const Node * /* relative */,
				  const QString& /* dirPath */ )
{
    QStringList lines = QStringList::split( "\n", protectedCode, TRUE );
    QStringList::Iterator li = lines.begin();
    while ( li != lines.end() ) {
	QString s = *li;
	if ( s.startsWith("#") ) {
	    s = "<@preprocessor>" + s + "</@preprocessor>";
	} else {
	    // ###
	}
	*li = s;
	++li;
    }
    return lines.join( "\n" );
}
