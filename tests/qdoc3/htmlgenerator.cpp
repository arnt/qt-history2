/*
  htmlgenerator.cpp
*/

#include "codemarker.h"
#include "htmlgenerator.h"
#include "node.h"
#include "separator.h"
#include "tree.h"

#define COMMAND_VERSION                 Doc::alias("version")

HtmlGenerator::HtmlGenerator()
    : inLink(false), funcLeftParen("\\S(\\()")
{
}

HtmlGenerator::~HtmlGenerator()
{
}

void HtmlGenerator::initializeGenerator(const Config &config)
{
    static const struct {
	const char *key;
	const char *left;
	const char *right;
    } defaults[] = {
	{ ATOM_FORMATTING_BOLD, "<b>", "</b>" },
	{ ATOM_FORMATTING_INDEX, "<!--", "-->" },
	{ ATOM_FORMATTING_ITALIC, "<i>", "</i>" },
	{ ATOM_FORMATTING_PARAMETER, "<i>", "</i>" },
	{ ATOM_FORMATTING_SUBSCRIPT, "<sub>", "</sub>" },
	{ ATOM_FORMATTING_SUPERSCRIPT, "<sup>", "</sup>" },
	{ ATOM_FORMATTING_TELETYPE, "<tt>", "</tt>" },
	{ ATOM_FORMATTING_UNDERLINE, "<u>", "</u>" },
	{ 0, 0, 0 }
    };

    Generator::initializeGenerator( config );
    setImageFileExtensions(QStringList() << "png" << "jpg" << "jpeg" << "gif");
    int i = 0;
    while (defaults[i].key) {
	formattingLeftMap().insert(defaults[i].key, defaults[i].left);
	formattingRightMap().insert(defaults[i].key, defaults[i].right);
	i++;
    }

    style = config.getString(HtmlGenerator::format() + Config::dot + HTMLGENERATOR_STYLE);
    postHeader = config.getString(HtmlGenerator::format() + Config::dot + HTMLGENERATOR_POSTHEADER);
    footer = config.getString(HtmlGenerator::format() + Config::dot + HTMLGENERATOR_FOOTER);
    address = config.getString(HtmlGenerator::format() + Config::dot + HTMLGENERATOR_ADDRESS);
}

void HtmlGenerator::terminateGenerator()
{
    Generator::terminateGenerator();
}

QString HtmlGenerator::format()
{
    return "HTML";
}

void HtmlGenerator::generateTree(const Tree *tree, CodeMarker *marker)
{
    version = tree->version();
    PageGenerator::generateTree(tree, marker);
}

void HtmlGenerator::startText(const Node * /* relative */, CodeMarker * /* marker */)
{
    inLink = FALSE;
    link = "";
}

int HtmlGenerator::generateAtom( const Atom *atom, const Node *relative,
				 CodeMarker *marker )
{
    int skipAhead = 0;

    switch ( atom->type() ) {
    case Atom::AbstractLeft:
	break;
    case Atom::AbstractRight:
	break;
    case Atom::BaseName:
	break;
    case Atom::BriefLeft:
	out() << "<p>";
	if ( relative->type() == Node::Property ) {
	    QString str;
	    atom = atom->next();
	    while ( atom != 0 && atom->type() != Atom::BriefRight ) {
		if ( atom->type() == Atom::String )
		    str += atom->string();
		skipAhead++;
		atom = atom->next();
	    }
	    str[0] = str[0].lower();
	    if ( str.right(1) == "." )
		str.truncate( str.length() - 1 );
	    out() << "This property holds " << str << ".";
	}
	break;
    case Atom::BriefRight:
	out() << "</p>\n";
	break;
    case Atom::C:
	out() << formattingLeftMap()[ATOM_FORMATTING_TELETYPE];
	if ( inLink ) {
	    out() << protect( plainCode(atom->string()) );
	} else {
	    out() << highlightedCode( atom->string(), relative );
	}
	out() << formattingRightMap()[ATOM_FORMATTING_TELETYPE];
	break;
    case Atom::Code:
	out() << "<pre>" << protect(plainCode(indent(4, atom->string()))) << "</pre>\n";
	break;
    case Atom::FootnoteLeft:
	break;
    case Atom::FootnoteRight:
	break;
    case Atom::FormatElse:
    case Atom::FormatEndif:
    case Atom::FormatIf:
	break;
    case Atom::FormattingLeft:
	out() << formattingLeftMap()[atom->string()];
	if ( atom->string() == ATOM_FORMATTING_PARAMETER ) {
	    if ( atom->next() != 0 && atom->next()->type() == Atom::String ) {
		QRegExp subscriptRegExp( "([a-z]+)_([0-9n])" );
		if ( subscriptRegExp.exactMatch(atom->next()->string()) ) {
		    out() << subscriptRegExp.cap( 1 ) << "<sub>"
			  << subscriptRegExp.cap( 2 ) << "</sub>";
		    skipAhead = 1;
		}
	    }
	}
	break;
    case Atom::FormattingRight:
	if ( atom->string() == ATOM_FORMATTING_LINK ) {
	    if ( link.isEmpty() ) {
		out() << "</font>";
	    } else {
		out() << "</a>";
	    }
	} else {
	    out() << formattingRightMap()[atom->string()];
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
	{
	    QString fileName = imageFileName(relative->doc().location(), atom->string());
	    QString text;
	    if ( atom->next() != 0 )
		text = atom->next()->string();
	    if ( fileName.isEmpty() ) {
		out() << "<font color=\"red\">[Missing image "
		      << protect( atom->string() ) << "]</font>";
	    } else {
		out() << "<img src=\"" << protect( fileName ) << "\"";
		if ( !text.isEmpty() )
		    out() << " alt=\"" << protect( text ) << "\"";
		out() << ">";
	    }
	}
	break;
    case Atom::ImageText:
	break;
    case Atom::Link:
	link = linkForNode( marker->resolveTarget(atom->string(), relative),
			    relative );
	if ( link.isEmpty() ) {
	    out() << "<font color=\"red\">";
	} else {
	    out() << "<a href=\"" << link << "\">";
	}
	inLink = TRUE;
	skipAhead = 1;
	break;
    case Atom::LinkNode:
	link = linkForNode( CodeMarker::nodeForString(atom->string()),
			    relative );
	if ( link.isEmpty() ) {
	    out() << "<font color=\"red\">";
	} else {
	    out() << "<a href=\"" << link << "\">";
	}
	inLink = TRUE;
	skipAhead = 1;
	break;
    case Atom::ListLeft:
	if ( atom->string() == ATOM_LIST_BULLET ) {
	    out() << "<ul>\n";
	} else if ( atom->string() == ATOM_LIST_TAG ) {
	    out() << "<dl>\n";
	} else if ( atom->string() == ATOM_LIST_VALUE ) {
	    out() << "<table border=\"1\" cellpadding=\"2\" cellspacing=\"1\""
		     " width=\"100%\">\n";
	} else {
            out() << "<ol type=";
            if ( atom->string() == ATOM_LIST_UPPERALPHA ) {
		out() << "A";
            } else if ( atom->string() == ATOM_LIST_LOWERALPHA ) {
		out() << "a";
            } else if ( atom->string() == ATOM_LIST_UPPERROMAN ) {
		out() << "I";
            } else if ( atom->string() == ATOM_LIST_LOWERROMAN ) {
		out() << "i";
            } else { // ( atom->string() == ATOM_LIST_NUMERIC )
		out() << "1";
            }
            if ( atom->next() != 0 && atom->next()->string().toInt() != 1 )
		out() << " start=" << atom->next()->string();
            out() << ">\n";
	}
	break;
    case Atom::ListItemNumber:
	break;
    case Atom::ListTagLeft:
	if ( atom->string() == ATOM_LIST_TAG ) {
	    out() << "<dt>";
	} else { // ( atom->string() == ATOM_LIST_VALUE )
	    out() << "<tr><td valign=\"top\" width=\"20%\">";
	}
	break;
    case Atom::ListTagRight:
	if ( atom->string() == ATOM_LIST_TAG )
	    out() << "</dt>\n";
	break;
    case Atom::ListItemLeft:
	if ( atom->string() == ATOM_LIST_TAG ) {
	    out() << "<dd>";
	} else if ( atom->string() == ATOM_LIST_VALUE ) {
	    out() << "</td><td>";
	    if ( matchAhead(atom, Atom::ListItemRight) )
		out() << "&nbsp;";
	} else {
	    out() << "<li>";
	}
	if ( matchAhead(atom, Atom::ParaLeft) )
	    skipAhead = 1;
	break;
    case Atom::ListItemRight:
	if ( atom->string() == ATOM_LIST_TAG ) {
	    out() << "</dd>\n";
	} else if ( atom->string() == ATOM_LIST_VALUE ) {
	    out() << "</td></tr>\n";
	} else {
	    out() << "</li>\n";
	}
	break;
    case Atom::ListRight:
	if ( atom->string() == ATOM_LIST_BULLET ) {
	    out() << "</ul>\n";
	} else if ( atom->string() == ATOM_LIST_TAG ) {
	    out() << "</dl>\n";
	} else if ( atom->string() == ATOM_LIST_VALUE ) {
	    out() << "</table>\n";
	} else {
	    out() << "</ol>\n";
	}
	break;
    case Atom::Nop:
	break;
    case Atom::ParaLeft:
	out() << "<p>";
	break;
    case Atom::ParaRight:
	if ( !matchAhead(atom, Atom::ListItemRight) )
	    out() << "</p>\n";
	break;
    case Atom::QuotationLeft:
	out() << "<blockquote>";
	break;
    case Atom::QuotationRight:
	out() << "</blockquote>\n";
	break;
    case Atom::RawString:
	out() << atom->string();
	break;
    case Atom::SectionLeft:
	break;
    case Atom::SectionRight:
	break;
    case Atom::SectionHeadingLeft:
	out() << "<h" + QString::number( atom->string().toInt() ) + ">";
	break;
    case Atom::SectionHeadingRight:
	out() << "</h" + QString::number( atom->string().toInt() ) + ">\n";
	break;
    case Atom::SidebarLeft:
	break;
    case Atom::SidebarRight:
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
    case Atom::TableLeft:
	break;
    case Atom::TableRight:
	break;
    case Atom::TableOfContents:
	break;
    case Atom::Target:
	break;
    case Atom::UnhandledFormat:
	out() << "<font color=\"red\"><b>&lt;Missing HTML&gt;</b></font>";
	break;
    case Atom::UnknownCommand:
	out() << "<font color=\"red\"><b><code>\\" << protect( atom->string() )
	      << "</code></b></font>";
	break;
    default:
	unknownAtom( atom );
    }
    return skipAhead;
}

void HtmlGenerator::generateNamespaceNode( const NamespaceNode *namespasse,
					   CodeMarker *marker )
{
    QString title = namespasse->name() + " Namespace Reference";
    generateHeader( title, namespasse );
    generateTitle( title );
    generateStatus(namespasse, marker);
    generateThreadSafeness(namespasse, marker);
    generateBody(namespasse, marker);
    generateAlsoList( namespasse, marker );
    generateFooter( namespasse );
}

void HtmlGenerator::generateClassNode(const ClassNode *classe, CodeMarker *marker)
{
    QList<ClassSection> sections;
    QList<ClassSection>::ConstIterator s;

    QString title = classe->name() + " Class Reference";
    generateHeader( title, classe );
    generateTitle( title );

    Text brief = classe->doc().briefText();
    if ( !brief.isEmpty() ) {
	out() << "<p>";
	generateText( brief, classe, marker );
	out() << " <a href=\"#" << registerRef( "details" )
	      << "\">More...</a></p>\n";
    }

    if ( !classe->includes().isEmpty() ) {
	QString code = highlightedCode(marker->markedUpIncludes(classe->includes()), classe);
	out() << "<pre>" << trimmedTrailing(code) << "</pre>";
    }

    generateStatus(classe, marker);
    generateInherits(classe, marker);
    generateInheritedBy(classe, marker);
    generateThreadSafeness(classe, marker);

    QString link = generateListOfAllMemberFile(classe, marker);
    out() << "<p><a href=\"" << link << "\">" << "List of all members.</a></p>\n";

    sections = marker->classSections( classe, CodeMarker::Summary );
    s = sections.begin();
    while ( s != sections.end() ) {
	out() << "<a name=\"" << registerRef((*s).name) << "\"/>\n";
	out() << "<h3>" << protect((*s).name) << "</h3>\n";

	generateClassSectionList(*s, classe, marker, CodeMarker::Summary);

	if ( !(*s).inherited.isEmpty() ) {
	    out() << "<ul>\n";
	    QList<QPair<ClassNode *, int> >::ConstIterator p = (*s).inherited.begin();
	    while ( p != (*s).inherited.end() ) {
		out() << "<li><div class=\"fn\"/>";
		out() << (*p).second << " ";
		if ( (*p).second == 1 ) {
		    out() << (*s).singularMember;
		} else {
		    out() << (*s).pluralMember;
		}
		out() << " inherited from <a href=\"" << fileName((*p).first)
		      << "#" << cleanRef((*s).name) << "\">"
		      << protect(plainCode(marker->markedUpFullName((*p).first, classe)))
		      << "</a></li>\n";
		++p;
	    }
	    out() << "</ul>\n";
	}
	++s;
    }

    out() << "<a name=\"" << registerRef( "details" ) << "\"/>\n";

    if ( !classe->doc().isEmpty() ) {
	out() << "<hr>\n" << "<h2>" << "Detailed Description" << "</h2>\n";
	generateBody( classe, marker );
	generateAlsoList( classe, marker );
    }

    sections = marker->classSections( classe, CodeMarker::Detailed );
    s = sections.begin();
    while ( s != sections.end() ) {
	out() << "<hr>\n";
	out() << "<h2>" << protect( (*s).name ) << "</h2>\n";

	NodeList::ConstIterator m = (*s).members.begin();
	while ( m != (*s).members.end() ) {
	    if ( (*m)->access() != Node::Private ) {
		out() << "<h3 class=\"fn\">";
		out() << "<a name=\"" + refForNode( *m ) + "\"/>";
		generateSynopsis( *m, classe, marker, CodeMarker::Detailed );
		out() << "</h3>\n";
		generateStatus(*m, marker);
		generateBody(*m, marker);
		generateThreadSafeness(*m, marker);
                if ((*m)->type() == Node::Property) {
		    PropertyNode *property = static_cast<PropertyNode *>(*m);
                    ClassSection section;

                    if (property->getter())
			section.members.append(const_cast<FunctionNode *>(property->getter()));
                    if (property->setter())
			section.members.append(const_cast<FunctionNode *>(property->setter()));
                    if (property->resetter())
			section.members.append(const_cast<FunctionNode *>(property->resetter()));

		    if (!section.members.isEmpty()) {
			out() << "<p>Access functions:</p>\n";
			generateClassSectionList(section, classe, marker, CodeMarker::Summary);
		    }
                }
		generateAlsoList( *m, marker );
	    }
	    ++m;
	}
	++s;
    }
    generateFooter( classe );
}

void HtmlGenerator::generateFakeNode( const FakeNode *fake, CodeMarker *marker )
{
/*
    NavigationBar bar;
    QString title = fake->name();

    bar.next = SectionIterator( fake->doc() );
    currentNavigationBar = bar;

    generateHeader( title, fake );
    generateNavigationBar( bar, fake, marker );
    generateTitle( title );
    generateBody( fake, marker );
    generateAlsoList( fake, marker );
    generateNavigationBar( bar, fake, marker );
    generateFooter( fake );
*/

    generateHeader( fake->name(), fake );
    generateTitle( fake->name() );
    generateBody( fake, marker );
    generateAlsoList( fake, marker );
    generateFooter( fake );
}

QString HtmlGenerator::fileBase( const Node *node )
{
    if ( !node->isInnerNode() )
	node = node->parent();

    QString base = node->doc().baseName();
    if ( base.isEmpty() ) {
	base = node->name();
	base.replace( QRegExp("[^A-Za-z0-9]+"), " " );
	base = base.simplifyWhiteSpace();
	base.replace( " ", "-" );
	base = base.lower();
    }
    return base;
}

QString HtmlGenerator::fileExtension( const Node * /* node */ )
{
    return "html";
}

void HtmlGenerator::generateHeader(const QString& title, const Node * /* node */)
{
#if O
    out() << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
#endif
    out() << "<!DOCTYPE html\n"
	     "    PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"DTD/xhtml1-strict.dtd\">\n"
	     "<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\" lang=\"en\">\n"
	     "<head>\n"
	     "    <title>" << protect( title ) << "</title>\n";
    if (!style.isEmpty())
	out() << "    <style>" << style << "</style>\n";
    out() << "</head>\n"
             "<body>\n"
	  << QString(postHeader).replace("\\" + COMMAND_VERSION, version);
}

void HtmlGenerator::generateTitle( const QString& title )
{
    out() << "<h1 align=\"center\">" << protect( title ) << "</h1>\n";
}

void HtmlGenerator::generateFooter( const Node * /* node */ )
{
    out() << QString(footer).replace("\\" + COMMAND_VERSION, version)
	  << QString(address).replace("\\" + COMMAND_VERSION, version)
	  << "</body>\n"
	     "</html>\n";
}

#if 0
void HtmlGenerator::generateNavigationBar( const NavigationBar& bar,
					   const Node *node,
					   CodeMarker *marker )
{
    if ( bar.prev.begin() != 0 || bar.current.begin() != 0 ||
	 bar.next.begin() != 0 ) {
	out() << "<p align=\"right\">";
	if ( bar.prev.begin() != 0 ) {
#if 0
	    out() << "[<a href=\"" << section.previousBaseName()
		  << ".html\">Prev: ";
	    generateText( section.previousHeading(), node, marker );
	    out() << "</a>]\n";
#endif
	}
	if ( bar.current.begin() != 0 ) {
	    out() << "[<a href=\"" << "home"
		  << ".html\">Home</a>]\n";
	}
	if ( bar.next.begin() != 0 ) {
	    out() << "[<a href=\"" << fileBase( node, bar.next )
		  << ".html\">Next: ";
	    generateText( sectionHeading(bar.next.begin()), node, marker );
	    out() << "</a>]\n";
	}
	out() << "</p>\n";
    }
}
#endif

QString HtmlGenerator::generateListOfAllMemberFile(const ClassNode *classe, CodeMarker *marker)
{
    QList<ClassSection> sections = marker->classSections(classe, CodeMarker::SeparateList);
    if (sections.size() == 1) {
        QString fileName = fileBase( classe ) + "-members." + fileExtension( classe );
        beginSubPage( classe->location(), fileName );
        generateHeader( "List of All Members for " + classe->name() );
        out() << "<p>This is the complete list of members for ";
        generateFullName( classe, 0, marker );
        out() << ", including inherited members.</p>\n";

        ClassSection section = sections.first();
        generateClassSectionList(section, 0, marker, CodeMarker::SeparateList);

        generateFooter();
        endSubPage();
        return fileName;
    } else {
	return "";
    }
}

void HtmlGenerator::generateSynopsis(const Node *node, const InnerNode *relative,
				     CodeMarker *marker, CodeMarker::SynopsisStyle style)
{
    QString marked = marker->markedUpSynopsis( node, relative, style );
    marked.replace(QRegExp("<@param>([a-z]+)_([1-9n])</@param>"), "<i>\\1<sub>\\2</sub></i>");
    marked.replace("<@param>", "<i>");
    marked.replace("</@param>", "</i>");

    if (style == CodeMarker::Summary) {
	if (node->status() == Node::Commendable || node->status() == Node::Preliminary) {
	    marked.replace("@name>", "b>");
	} else {
	    marked.replace(QRegExp("</?@link>"), "");
	}
    }
    if (style == CodeMarker::SeparateList) {
	QRegExp extraRegExp("<@extra>.*</@extra>");
        extraRegExp.setMinimal(true);
	marked.replace(extraRegExp, "");
    } else {
        marked.replace( "<@extra>", "&nbsp;&nbsp;<tt>" );
        marked.replace( "</@extra>", "</tt>" );
    }

    out() << highlightedCode( marked, relative );
}

void HtmlGenerator::generateClassSectionList(const ClassSection& section, const ClassNode *relative,
					     CodeMarker *marker, CodeMarker::SynopsisStyle style)
{
    if ( !section.members.isEmpty() ) {
	bool twoColumn = FALSE;
	if ( style == CodeMarker::SeparateList ) {
	    twoColumn = ( section.members.count() >= 16 );
	} else if ( section.members.first()->type() == Node::Property ) {
	    twoColumn = ( section.members.count() >= 5 );
	}
	if ( twoColumn )
	    out() << "<table width=\"100%\" border=\"0\" cellpadding=\"0\""
		     " cellspacing=\"0\">\n"
		  << "<tr><td width=\"45%\" valign=\"top\">";
	out() << "<ul>\n";

	int i = 0;
	NodeList::ConstIterator m = section.members.begin();
	while ( m != section.members.end() ) {
	    if ( twoColumn && i == (int) (section.members.count() + 1) / 2 )
		out() << "</ul></td><td valign=\"top\"><ul>\n";

	    out() << "<li><div class=\"fn\"/>";
	    generateSynopsis( *m, relative, marker, style );
	    out() << "</li>\n";
	    i++;
	    ++m;
	}
	out() << "</ul>\n";
	if ( twoColumn )
	    out() << "</td></tr>\n</table>\n";
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
	} else if ( ref[i].isSpace() ) {
	    clean += "-";
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
    html.replace( "&", "&amp;" );
    html.replace( "<", "&lt;" );
    html.replace( ">", "&gt;" );
    html.replace( "\"", "&quot;" );
    return html;
}

QString HtmlGenerator::highlightedCode(const QString& markedCode, const Node *relative)
{
    QRegExp linkTag("(<@link node=\"([^\"]+)\">).*(</@link>)");
    linkTag.setMinimal(true);

    QString html = markedCode;

    int k = 0;
    while ( (k = html.find(linkTag, k)) != -1 ) {
	QString begin;
	QString end;
	QString link = linkForNode(CodeMarker::nodeForString(linkTag.cap(2)), relative);

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

#if 0
QString HtmlGenerator::fileBase( const Node *node,
				 const SectionIterator& section )
{
    QStringList::ConstIterator s = section.sectionNumber().end();
    QStringList::ConstIterator b = section.baseNameStack().end();

    QString suffix;
    QString base = fileBase( node );

    while ( s != section.sectionNumber().begin() ) {
	--s;
	--b;
	if ( !(*b).isEmpty() ) {
	    base = *b;
	    break;
	}
	suffix.prepend( "-" + *s );
    }
    return base + suffix;
}
#endif

QString HtmlGenerator::refForNode(const Node *node)
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
	func = static_cast<const FunctionNode *>(node);
        if (func->associatedProperty()) {
	    return refForNode(func->associatedProperty());
        } else {
	    ref = func->name();
	    if (func->overloadNumber() != 1)
	        ref += "-" + QString::number(func->overloadNumber());
	}
	break;
    case Node::Property:
	ref = node->name() + "-prop";
    }
    return registerRef( ref );
}

QString HtmlGenerator::linkForNode(const Node *node, const Node *relative)
{
    QString aname;
    QString link;
    QString fn;

    if ( node == 0 )
	return "";

    fn = fileName( node );
    if (relative == 0 || fn != fileName(relative))
	link += fn;
    if (!node->isInnerNode())
	link += "#" + refForNode(node);
    return link;
}

void HtmlGenerator::generateFullName( const Node *apparentNode, const Node *relative,
				      CodeMarker *marker, const Node *actualNode )
{
    if ( actualNode == 0 )
	actualNode = apparentNode;
    out() << "<a href=\"" << linkForNode( actualNode, relative ) << "\">"
	  << protect( plainCode(marker->markedUpFullName(apparentNode,
							 relative)) )
	  << "</a>";
}
