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

QString CppCodeMarker::markedUpCode( const QString& code,
				     const Node *relative,
				     const QString& dirPath ) const
{
    return addMarkUp( protect(code), relative, dirPath );
}

QString CppCodeMarker::markedUpSynopsys( const Node *node,
					 const Node *relative,
					 SynopsysStyle style ) const
{
    const FunctionNode *func;
    QString synopsys;
    QString extra;
    QString name;

    name = taggedNode( node );
    if ( style == Overview )
	name = linkTag( node, name );
    name = "<@name>" + name + "</@name>";
    if ( style != Overview && !node->parent()->name().isEmpty() &&
	 node->type() != Node::Property )
	name.prepend( taggedNode(node->parent()) + "::" );

    switch ( node->type() ) {
    case Node::Namespace:
	synopsys = "namespace " + name;
	break;
    case Node::Class:
	synopsys = "class " + name;
	break;
    case Node::Function:
	func = (const FunctionNode *) node;
	if ( !func->returnType().isEmpty() )
	    synopsys = protect( func->returnType() ) + " ";
	synopsys += name + " (";
	if ( !func->parameters().isEmpty() ) {
	    synopsys += " ";
	    QValueList<Parameter>::ConstIterator p = func->parameters().begin();
	    while ( p != func->parameters().end() ) {
		if ( p != func->parameters().begin() )
		    synopsys += ", ";
		synopsys += protect( (*p).leftType() ) + " <@param>" +
			    protect( (*p).name() ) + "</@param>" +
			    protect( (*p).rightType() );
		++p;
	    }
	    synopsys += " ";
	}
	synopsys += ")";
	if ( func->isConst() )
	    synopsys += " const";

	if ( style == Overview ) {
	    if ( func->virtualness() != FunctionNode::NonVirtual )
		synopsys.prepend( "virtual " );
	    if ( func->virtualness() == FunctionNode::PureVirtual )
		synopsys.append( " = 0" );
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
	synopsys = "enum " + name;
	break;
    case Node::Typedef:
	synopsys = "typedef " + name;
	break;
    case Node::Property:
	synopsys = name;
	break;
    default:
	synopsys = name;
    }

    if ( style == Overview ) {
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
    return addMarkUp( synopsys, relative, "" ) + extra;
}

QString CppCodeMarker::markedUpName( const Node *node ) const
{
    QString name = linkTag( node, taggedNode(node) );
    if ( node->type() == Node::Function )
	name += "()";
    return name;
}

QString CppCodeMarker::markedUpFullName( const Node *node ) const
{
    QString fullName;
    for ( ;; ) {
	fullName += markedUpName( node );
	if ( node->parent()->name().isEmpty() )
	    break;
	node = node->parent();
	fullName.prepend( "<@op>::</@op>" );
    }
    return fullName;
}

QString CppCodeMarker::markedUpIncludes( const QStringList& includes ) const
{
    QString code;

    QStringList::ConstIterator inc = includes.begin();
    while ( inc != includes.end() ) {
	code += "#include &lt;<@include>" + *inc + "</@include>" + "&gt\n";
	++inc;
    }
    return addMarkUp( code, 0, "" );
}

const Node *CppCodeMarker::resolveTarget( const QString& /* target */,
					  const Node * /* relative */ ) const
{
    return 0;
}

bool CppCodeMarker::recognizeCode( const QString& /* code */ ) const
{
    return FALSE;
}

bool CppCodeMarker::recognizeExtension( const QString& ext ) const
{
    return ext == "c" || ext == "c++" || ext == "cc" || ext == "cpp" ||
	   ext == "cxx" || ext == "h" || ext == "hpp" || ext == "hxx";
}

bool CppCodeMarker::recognizeLanguage( const QString& lang ) const
{
    return lang == "c" || lang == "c++";
}

QString CppCodeMarker::addMarkUp( const QString& protectedCode,
				  const Node * /* relative */,
				  const QString& /* dirPath */ ) const
{
    QString t = protectedCode;
    return t;
}
