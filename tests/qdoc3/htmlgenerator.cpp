/*
  htmlgenerator.cpp
*/

#include "codemarker.h"
#include "htmlgenerator.h"
#include "messages.h"
#include "molecule.h"
#include "node.h"
#include "separator.h"

HtmlGenerator::HtmlGenerator()
    : inLink( FALSE ), funcLeftParen( "\\S(\\()" ), amp( "&" ), lt( "<" ),
      gt( ">" ), quot( "\"" )
{
}

HtmlGenerator::~HtmlGenerator()
{
}

QString HtmlGenerator::formatString() const
{
    return "html";
}

void HtmlGenerator::startMolecule( const Node * /* relative */,
				   const CodeMarker * /* marker */ )
{
    inLink = FALSE;
    link = "";
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
    case Atom::AliasArg:
	break;
    case Atom::BriefBegin:
	out() << "<p>";
	break;
    case Atom::BriefEnd:
	out() << "</p>\n";
	break;
    case Atom::C:
	out() << "<code>" << protect( plainCode(atom->string()) ) << "</code>";
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
	    inLink = TRUE;
	} else if ( atom->string() == "parameter" ) {
	    out() << "<i>";
	} else if ( atom->string() == "subscript" ) {
	    out() << "<sub>";
	} else if ( atom->string() == "superscript" ) {
	    out() << "<sup>";
	} else if ( atom->string() == "teletype" ) {
	    out() << "<tt>";
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
	    if ( inLink ) {
		if ( link.isEmpty() ) {
		    out() << "</font>";
		} else {
		    out() << "</a>";
		}
	    }
	} else if ( atom->string() == "parameter" ) {
	    out() << "</i>";
	} else if ( atom->string() == "subscript" ) {
	    out() << "</sub>";
	} else if ( atom->string() == "superscript" ) {
	    out() << "</sup>";
	} else if ( atom->string() == "teletype" ) {
	    out() << "</tt>";
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
    case Atom::Image:
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
    case Atom::ListItemNumber:
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
    case Atom::Nop:
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
    case Atom::SectionHeadingBegin:
	out() << "<h1>";
	break;
    case Atom::SectionHeadingEnd:
	out() << "</h1>\n";
	break;
    case Atom::SidebarBegin:
	break;
    case Atom::SidebarEnd:
	break;
    case Atom::String:
	if ( inLink && funcLeftParen.search(atom->string()) != -1 ) {
	    int k = funcLeftParen.pos( 1 );
	    out() << protect( atom->string().left(k) );
	    if ( link.isEmpty() ) {
		out() << "</font>";
	    } else {
		out() << "</a>";
	    }
	    inLink = FALSE;
	    out() << protect( atom->string().mid(k) );
	} else {
	    out() << protect( atom->string() );
	}
	break;
    case Atom::TableBegin:
	break;
    case Atom::TableEnd:
	break;
    case Atom::TableOfContents:
	break;
    case Atom::Target:
	;
    case Atom::TitleBegin:
	break;
    case Atom::TitleEnd:
	break;
    default:
	message( 1, "HTML generator: Unknown atom type '%s'",
		 atom->typeString().latin1() );
    }
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
    generateAlso( namespasse->doc(), namespasse, marker );
    generateFooter( out() );
}

void HtmlGenerator::generateClassNode( const ClassNode *classe,
				       const CodeMarker *marker )
{
    QValueList<ClassSection> sections;
    QValueList<ClassSection>::ConstIterator s;
    NodeList::ConstIterator m;

    generateHeader( out() );
    out() << "<h1 align=\"center\">" << protect( classe->name() )
	  << " Class Reference" << "</h1>\n";

    Molecule brief = classe->doc().molecule().subMolecule( Atom::BriefBegin,
							   Atom::BriefEnd );
    if ( !brief.isEmpty() ) {
	out() << "<p>";
	generateMolecule( brief, classe, marker );
	out() << " <a href=\"#" << registerRef( "details" )
	      << "\">More...</a></p>\n";
    }

    if ( !classe->includes().isEmpty() ) {
	out() << "<pre>"
	      << highlightedCode( marker->markedUpIncludes(classe->includes()),
				  classe )
	      << "</pre>";
    }

    generateInherits( classe, marker );
    generateInheritedBy( classe, marker );

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
	    out() << "<li><div class=\"fn\"/>";
	    generateSynopsis( *m, classe, marker, CodeMarker::Overview );
	    out() << "</li>\n";
	    ++m;
	}
	out() << "</ul>\n";
	++s;
    }

    out() << "<hr>\n";
    out() << "<a name=\"" << registerRef( "details" ) << "\"/>\n";
    out() << "<h2>" << "Detailed Description" << "</h2>\n";

    generateDoc( classe->doc(), classe, marker );
    generateAlso( classe->doc(), classe, marker );

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
		generateSynopsis( *m, classe, marker, CodeMarker::Detailed );
		out() << "</h3>\n";
		generateDoc( (*m)->doc(), *m, marker );
		generateAlso( (*m)->doc(), *m, marker );
	    }
	    ++m;
	}
	++s;
    }
    generateFooter( out() );
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
		  << highlightedCode( marker->markedUpFullName(classe, 0),
				      classe )
		  << ", including inherited functions.</p>\n";
	generateFooter( outStream );
	outFile.close();
    }
}

void HtmlGenerator::generateSynopsis( const Node *node,
				      const InnerNode *relative,
				      const CodeMarker *marker,
				      CodeMarker::SynopsisStyle style )
{
    QString marked = marker->markedUpSynopsis( node, relative, style );
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
    out() << highlightedCode( marked, relative );

    if ( node->type() == Node::Property ) {
	Molecule brief = node->doc().molecule().subMolecule( Atom::BriefBegin,
							     Atom::BriefEnd );
	if ( !brief.isEmpty() ) {
	    out() << " - ";
	    generateMolecule( brief, node, marker );
	}
    }
}

QString HtmlGenerator::cleanRef( const QString& ref )
{
    QString clean;

    if ( ref.isEmpty() )
	return clean;

    if ( ref[0].lower() >= 'a' && ref[0].lower() <= 'z' ) {
	clean += ref[0];
    } else if ( ref[0] == '~' ) {
	clean += "dtor-";
    } else if ( ref[0] == '_' ) {
	clean += "underscore-";
    } else {
	clean += "A";
    }

    for ( int i = 1; i < (int) ref.length(); i++ ) {
	if ( (ref[i].lower() >= 'a' && ref[i].lower() <= 'z') ||
	     (ref[i] >= '0' && ref[i] <= '9') || ref[i] == '-' ||
	     ref[i] == '_' || ref[i] == ':' || ref[i] == '.' ) {
	    clean += ref[i];
	} else if ( ref[i] == '!' ) {
	    clean += "-not";
	} else if ( ref[i] == '&' ) {
	    clean += "-and";
	} else if ( ref[i] == '<' ) {
	    clean += "-lt";
	} else if ( ref[i] == '=' ) {
	    clean += "-eq";
	} else if ( ref[i] == '>' ) {
	    clean += "-gt";
	} else {
	    clean += "-";
	    clean += QString::number( (int) ref[i].unicode(), 16 );
	}
    }
    return clean;
}

QString HtmlGenerator::registerRef( const QString& ref )
{
    QString clean = cleanRef( ref );

    for ( ;; ) {
	QString& prevRef = refMap[clean.lower()];
	if ( prevRef.isEmpty() ) {
	    prevRef = ref;
	    break;
	} else if ( prevRef == ref ) {
	    break;
	}
	clean += "x";
    }
    return clean;
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
	ref = node->name() + "-enum";
	break;
    case Node::Typedef:
	ref = node->name() + "-typedef";
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
    return registerRef( ref );
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
