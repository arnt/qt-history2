/*
  htmlgenerator.cpp
*/

#include "codemarker.h"
#include "htmlgenerator.h"
#include "messages.h"
#include "node.h"
#include "separator.h"

HtmlGenerator::HtmlGenerator()
    : amp( "&" ), lt( "<" ), gt( ">" ), quot( "\"" )
{
}

QString HtmlGenerator::fileBase( const Node *node )
{
    if ( !node->isInnerNode() )
	node = node->parent();

    QString base = node->name();
    base.replace( QRegExp("[^A-Za-z0-9]+"), " " );
    base = base.simplifyWhiteSpace();
    base.replace( QRegExp(" "), "_" );
    base = base.lower();
    return "html/" + base;
}

QString HtmlGenerator::fileExtension( const Node * /* node */ )
{
    return "html";
}

void HtmlGenerator::generateNamespaceNode( const NamespaceNode *namespasse,
					   const CodeMarker *marker )
{
    generateHeader( out() );
    generateDoc( namespasse->doc(), namespasse, marker );
    generateFooter( out() );
}

void HtmlGenerator::generateClassNode( const ClassNode *classe,
				       const CodeMarker *marker )
{
    QValueList<RelatedClass> relatedClasses;
    QValueList<RelatedClass>::ConstIterator r;
    QValueList<ClassSection> sections;
    QValueList<ClassSection>::ConstIterator s;
    NodeList::ConstIterator m;
    int index;

    generateHeader( out() );
    out() << "<h1 align=\"center\">" << protect( classe->name() )
	  << " Class Reference" << "</h1>\n";

    const Atom *begin;
    const Atom *end;
    if ( findAtomSubList(classe->doc().atomList(), Atom::BriefBegin,
			 Atom::ParagraphEnd, &begin, &end) ) {
	generateAtomSubList( begin, end, classe, marker );
	out() << "<a href=\"#" << "details" << "\"> More...</a></p>\n";
    }

    if ( !classe->includes().isEmpty() ) {
	out() << "<pre>"
	      << highlightedCode( marker->markedUpIncludes(classe->includes()),
				  classe )
	      << "</pre>";
    }

    relatedClasses = classe->baseClasses();
    if ( !relatedClasses.isEmpty() ) {
	out() << "<p>Inherits ";
	index = 0;
	r = relatedClasses.begin();
	while ( r != relatedClasses.end() ) {
	    out() << protect( (*r).node->name() ); // ###
	    if ( (*r).access == Node::Protected ) {
		out() << " (protected)";
	    } else if ( (*r).access == Node::Private ) {
		out() << " (private)";
	    }
	    out() << protect( separator(index++, relatedClasses.count()) );
	    ++r;
	}
	out() << "</p>\n";
    }

    relatedClasses = classe->derivedClasses();
    if ( !relatedClasses.isEmpty() ) {
	out() << "<p>Inherited by ";
	index = 0;
	r = relatedClasses.begin();
	while ( r != relatedClasses.end() ) {
	    out() << protect( (*r).node->name() ); // ###
	    if ( (*r).access == Node::Protected ) {
		out() << " (protected)";
	    } else if ( (*r).access == Node::Private ) {
		out() << " (private)";
	    }
	    out() << protect( separator(index++, relatedClasses.count()) );
	    ++r;
	}
	out() << "</p>\n";
    }

    generateListOfAllMemberFunctions( classe, marker );
    out() << "<p><a href=\"foo\">" << "List of all member functions."
	  << "</a></p>\n";

    sections = classe->overviewSections();
    s = sections.begin();
    while ( s != sections.end() ) {
	out() << "<h2>" << protect( (*s).name ) << "</h2>\n";
	out() << "<ul>\n";

	m = (*s).members.begin();
	while ( m != (*s).members.end() ) {
	    out() << "<li><div class=\"fn\">";
	    out() << synopsys( *m, classe, marker, CodeMarker::Overview );
	    out() << "</li>\n";
	    ++m;
	}
	out() << "</ul>\n";
	++s;
    }

    out() << "<hr>\n";
    out() << "<a name=\"" << "details" << "\"/>\n";
    out() << "<h2>" << "Detailed Description" << "</h2>\n";

    generateDoc( classe->doc(), classe, marker );

    sections = classe->detailedSections();
    s = sections.begin();
    while ( s != sections.end() ) {
	out() << "<hr>\n";
	out() << "<h2>" << protect( (*s).name ) << "</h2>\n";

	m = (*s).members.begin();
	while ( m != (*s).members.end() ) {
	    if ( (*m)->access() != Node::Private ) {
		out() << "<h3 class=\"fn\">";
		out() << "<a name=\"" + refForNode( *m ) + "\"/>";
		out() << synopsys( *m, classe, marker, CodeMarker::Detailed );
		out() << "</h3>\n";
		generateDoc( (*m)->doc(), *m, marker );
	    }
	    ++m;
	}
	++s;
    }
    generateFooter( out() );
}

void HtmlGenerator::generateAtom( const Atom *atom, const Node *relative,
				  const CodeMarker *marker )
{
    switch ( atom->type() ) {
    case Atom::AbstractBegin:
	break;
    case Atom::AbstractEnd:
	break;
    case Atom::Alias:
	break;
    case Atom::C:
	out() << "<code>"
	      << protect( plainCode(atom->string()).simplifyWhiteSpace() )
	      << "</code>";
	break;
    case Atom::CaptionBegin:
	break;
    case Atom::CaptionEnd:
	break;
    case Atom::CitationBegin:
	out() << "<blockquote>";
	break;
    case Atom::CitationEnd:
	out() << "</blockquote>\n";
	break;
    case Atom::Code:
	out() << "<pre>" << protect( plainCode(atom->string()) ) << "</pre>\n";
	break;
    case Atom::DocBegin:
	break;
    case Atom::DocEnd:
	break;
    case Atom::FootnoteBegin:
	break;
    case Atom::FootnoteEnd:
	break;
    case Atom::FormatBegin:
	if ( atom->string() == "bold" ) {
	    out() << "<b>";
	} else if ( atom->string() == "italic" ) {
	    out() << "<i>";
	} else if ( atom->string() == "link" ) {
	    if ( link.isEmpty() ) {
		out() << "<font color=\"red\">";
	    } else {
		out() << "<a href=\"" << link << "\">";
	    }
	} else if ( atom->string() == "parameter" ) {
	    out() << "<i>";
	} else if ( atom->string() == "underlined" ) {
	    out() << "<u>";
	}
	break;
    case Atom::FormatEnd:
	if ( atom->string() == "bold" ) {
	    out() << "</b>";
	} else if ( atom->string() == "italic" ) {
	    out() << "</i>";
	} else if ( atom->string() == "link" ) {
	    if ( link.isEmpty() ) {
		out() << "</font>";
	    } else {
		out() << "</a>";
	    }
	    link = "";
	} else if ( atom->string() == "parameter" ) {
	    out() << "</i>";
	} else if ( atom->string() == "underlined" ) {
	    out() << "</u>";
	}
	break;
    case Atom::GeneratedList:
	if ( atom->string() == "annotatedclasses" ) {
	
	} else if ( atom->string() == "classes" ) {
	
	} else if ( atom->string() == "classhierarchy" ) {

	} else if ( atom->string() == "headerfiles" ) {
	
	} else if ( atom->string() == "index" ) {
	
	} else if ( atom->string() == "mainclasses" ) {
	
	}
	break;
    case Atom::Img:
	break;
    case Atom::Index:
	break;
    case Atom::Link:
	link = linkForNode( marker->resolveTarget(atom->string(), relative),
			    relative );
	break;
    case Atom::ListBegin:
	if ( atom->string() == "bullet" ) {
	    out() << "<ul>\n";
	} else {
	    out() << "<ol type=";
	    if ( atom->string() == "upperalpha" ) {
		out() << "A";
	    } else if ( atom->string() == "loweralpha" ) {
		out() << "a";
	    } else if ( atom->string() == "upperroman" ) {
		out() << "I";
	    } else if ( atom->string() == "lowerroman" ) {
		out() << "i";
	    } else {
		out() << "1";
	    }
	    if ( atom->next()->string() != "1" )
		out() << " start=" << atom->next()->string();
	    out() << ">\n";
	}
	break;
    case Atom::ListItemBegin:
	out() << "<li>";
	break;
    case Atom::ListItemEnd:
	out() << "</li>\n";
	break;
    case Atom::ListEnd:
	if ( atom->string() == "bullet" ) {
	    out() << "</ul>\n";
	} else {
	    out() << "</ol>\n";
	}
	break;
    case Atom::ParagraphBegin:
	out() << "<p>";
	break;
    case Atom::ParagraphEnd:
	out() << "</p>\n";
	break;
    case Atom::RawFormat:
	if ( atom->string() == "html" )
	    out() << atom->next()->string();
	break;
    case Atom::RawString:
	break;
    case Atom::SectionBegin:
	break;
    case Atom::SectionEnd:
	break;
    case Atom::SidebarBegin:
	break;
    case Atom::SidebarEnd:
	break;
    case Atom::String:
	out() << protect( atom->string() );
	break;
    case Atom::TableOfContents:
	break;
    case Atom::Target:
	;
    }
}

void HtmlGenerator::generateHeader( QTextStream& outStream )
{
#if notyet
    out() << "<?xml version=\"1.0\" encoding=\"UTF-8\">\n"
#endif
    outStream << "<!DOCTYPE html\n"
		 "    PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\"\n"
		 "    \"DTD/xhtml1-strict.dtd\">\n"
		 "<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\""
		 " lang=\"en\">\n"
		 "<head>\n"
		 "    <title>"
	      << "</title>\n"
		 "</head>\n"
		 "<body>\n";
}

void HtmlGenerator::generateFooter( QTextStream& outStream )
{
    outStream << "</body>\n"
		 "</html>\n";
}

void HtmlGenerator::generateListOfAllMemberFunctions( const ClassNode *classe,
						      const CodeMarker *marker )
{
    QFile outFile( fileBase(classe) + "-members." + fileExtension(classe) );
    if ( !outFile.open(IO_WriteOnly) ) {
	syswarning( "Cannot open '%s' for writing", outFile.name().latin1() );
    } else {
	QTextStream outStream( &outFile );
	generateHeader( outStream );
	outStream << "<h1 align=\"center\">" << "Member Function List for "
		  << protect( classe->name() ) << "</h1>\n"
		  << "<p>This is the complete list of member functions for "
		  << highlightedCode( marker->markedUpFullName(classe), classe )
		  << ", including inherited functions.</p>\n";
	generateFooter( outStream );
	outFile.close();
    }
}

QString HtmlGenerator::protect( const QString& string )
{
    QString html = string;
    html.replace( amp, "&amp;" );
    html.replace( lt, "&lt;" );
    html.replace( gt, "&gt;" );
    html.replace( quot, "&quot;" );
    return html;
}

QString HtmlGenerator::synopsys( const Node *node, const InnerNode *relative,
				 const CodeMarker *marker,
				 CodeMarker::SynopsysStyle style )
{
    QString marked = marker->markedUpSynopsys( node, relative, style );

    marked.replace( QRegExp("@param>"), "u>" );

    if ( style == CodeMarker::Overview ) {
	if ( node->status() == Node::Approved ||
	     node->status() == Node::Preliminary ) {
	    marked.replace( QRegExp("@name>"), "b>" );
	} else {
	    marked.replace( QRegExp("</?@link>"), "" );
	}
	marked.replace( QRegExp("<@extra>"), "&nbsp;<i>" );
	marked.replace( QRegExp("</@extra>"), "</i>" );
    } else {
	marked.replace( QRegExp("<@extra>"), "&nbsp;<tt>" );
	marked.replace( QRegExp("</@extra>"), "</tt>" );
    }
    return highlightedCode( marked, relative );
}

QString HtmlGenerator::highlightedCode( const QString& markedCode,
					const Node *relative )
{
    QRegExp linkTag( "(<@link node=\"([^\"]+)\">).*(</@link>)" );
    linkTag.setMinimal( TRUE );

    QString html = markedCode;

    int k = 0;
    while ( (k = html.find(linkTag, k)) != -1 ) {
	QString begin;
	QString end;
	QString link = linkForNode( CodeMarker::nodeForString(linkTag.cap(2)),
				    relative );

	if ( link.isEmpty() ) {
	    begin = "<font color=\"red\">";
	    end = "</font>";
	} else {
	    begin = "<a href=\"" + link + "\">";
	    end = "</a>";
	}

	html.replace( linkTag.pos(3), linkTag.cap(3).length(), end );
	html.replace( linkTag.pos(1), linkTag.cap(1).length(), begin );
	k++;
    }

#if 0
    html.replace( QRegExp("<@char>"), "<font color=blue>" );
    html.replace( QRegExp("</@char>"), "</font>" );
    html.replace( QRegExp("<@comment>"), "<font color=red>" );
    html.replace( QRegExp("</@comment>"), "</font>" );
    html.replace( QRegExp("<@func>"), "<font color=green>" );
    html.replace( QRegExp("</@func>"), "</font>" );
    html.replace( QRegExp("<@id>"), "<i>" );
    html.replace( QRegExp("</@id>"), "</i>" );
    html.replace( QRegExp("<@keyword>"), "<b>" );
    html.replace( QRegExp("</@keyword>"), "</b>" );
    html.replace( QRegExp("<@number>"), "<font color=yellow>" );
    html.replace( QRegExp("</@number>"), "</font>" );
    html.replace( QRegExp("<@op>"), "<b>" );
    html.replace( QRegExp("</@op>"), "</b>" );
    html.replace( QRegExp("<@param>"), "<i>" );
    html.replace( QRegExp("</@param>"), "</i>" );
    html.replace( QRegExp("<@preprocessor>"), "<font color=blue>" );
    html.replace( QRegExp("</@preprocessor>"), "</font>" );
    html.replace( QRegExp("<@string>"), "<font color=green>" );
    html.replace( QRegExp("</@string>"), "</font>" );
    html.replace( QRegExp("<@type>"), "<font color=brown>" );
    html.replace( QRegExp("</@type>"), "</font>" );
#endif
    html.replace( QRegExp("</?@[^>]*>"), "" );
    return html;
}

QString HtmlGenerator::refForNode( const Node *node )
{
    const FunctionNode *func;
    QString ref;

    switch ( node->type() ) {
    case Node::Namespace:
    case Node::Class:
    default:
	break;
    case Node::Enum:
	ref = node->name() + "-type";
	break;
    case Node::Typedef:
	ref = node->name() + "-type";
	break;
    case Node::Function:
	ref = node->name();
	func = (const FunctionNode *) node;
	if ( func->overloadNumber() != 1 )
	    ref += "-" + QString::number( func->overloadNumber() );
	break;
    case Node::Property:
	ref = node->name() + "-prop";
    }
    return ref;
}

QString HtmlGenerator::linkForNode( const Node *node, const Node *relative )
{
    QString aname;
    QString link;
    QString fn;

    if ( node == 0 )
	return "";

    fn = fileName( node );
    if ( fn != fileName(relative) )
	link += fn;
    if ( !node->isInnerNode() )
	link += "#" + refForNode( node );
    return link;
}
