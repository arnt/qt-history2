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
    : inLink(false), inTableHeader(false), numTableRows(0), funcLeftParen("\\S(\\()"), root(0)
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
    root = tree->root(); // ### necessary?
    allClasses.clear();
    mainClasses.clear();
    funcIndex.clear();
    legaleseTexts.clear();
    findAllClasses(tree->root());
    findAllFunctions(tree->root());
    findAllLegaleseTexts(tree->root());
    PageGenerator::generateTree(tree, marker);
}

void HtmlGenerator::startText(const Node * /* relative */, CodeMarker * /* marker */)
{
    inLink = false;
    inTableHeader = false;
    numTableRows = 0;
    link = "";
    sectionNumber.clear();
}

int HtmlGenerator::generateAtom(const Atom *atom, const Node *relative, CodeMarker *marker)
{
    int skipAhead = 0;

    switch (atom->type()) {
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
	    str[0] = str[0].toLower();
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
	out() << "<pre>" << trimmedTrailing(protect(plainCode(indent(4, atom->string()))))
	      << "</pre>\n";
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
	if (atom->string() == "annotatedclasses") {
	    generateAnnotatedList(relative, marker, allClasses);
	} else if (atom->string() == "classes") {
	    generateCompactList(relative, marker, allClasses);
	} else if (atom->string() == "classhierarchy") {
	    generateClassHierarchy(relative, marker, allClasses);
	} else if (atom->string() == "functionindex") {
	    generateFunctionIndex(relative, marker);
	} else if (atom->string() == "legalese") {
	    generateLegaleseList(relative, marker);
	} else if (atom->string() == "mainclasses") {
	    generateCompactList(relative, marker, mainClasses);
	}
	break;
    case Atom::Image:
    case Atom::InlineImage:
	{
	    QString fileName = imageFileName(relative->doc().location(), atom->string());
	    QString text;
	    if ( atom->next() != 0 )
		text = atom->next()->string();
	    if (atom->type() == Atom::Image)
		out() << "<center>";
	    if ( fileName.isEmpty() ) {
		out() << "<font color=\"red\">[Missing image "
		      << protect( atom->string() ) << "]</font>";
	    } else {
		out() << "<img src=\"" << protect( fileName ) << "\"";
		if ( !text.isEmpty() )
		    out() << " alt=\"" << protect( text ) << "\"";
		out() << ">";
	    }
	    if (atom->type() == Atom::Image)
		out() << "</center>";
	}
	break;
    case Atom::ImageText:
	break;
    case Atom::LegaleseLeft:
	break;
    case Atom::LegaleseRight:
	break;
    case Atom::Link:
	link = linkForNode( marker->resolveTarget(atom->string(), relative),
			    relative );
	if ( link.isEmpty() ) {
	    out() << "<font color=\"red\">";
	} else {
	    out() << "<a href=\"" << link << "\">";
	}
	inLink = true;
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
	inLink = true;
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
	{
	    int nextLevel = atom->string().toInt();
            if (sectionNumber.size() < nextLevel) {
		do {
		    sectionNumber.append("1");
                } while (sectionNumber.size() < nextLevel);
            } else {
		while (sectionNumber.size() > nextLevel) {
		    sectionNumber.removeLast();
                }
                sectionNumber.last() = QString::number(sectionNumber.last().toInt() + 1);
            }
            out() << "<a name=\"sec-" << sectionNumber.join(".") << "\">\n";
        }
	break;
    case Atom::SectionRight:
	break;
    case Atom::SectionHeadingLeft:
	out() << "<h" + QString::number(atom->string().toInt() + hOffset(relative)) + ">";
	break;
    case Atom::SectionHeadingRight:
	out() << "</h" + QString::number(atom->string().toInt() + hOffset(relative)) + ">\n";
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
	    inLink = false;
	    out() << protect( atom->string().mid(k) );
	} else {
	    out() << protect( atom->string() );
	}
	break;
    case Atom::TableLeft:
	out() << "<center><table cellpadding=\"4\" cellspacing=\"2\" border=\"0\">\n";
        numTableRows = 0;
	break;
    case Atom::TableRight:
	out() << "</table></center>\n";
	break;
    case Atom::TableHeaderLeft:
	out() << "<tr bgcolor=\"#a2c511\">";
        inTableHeader = true;
        break;
    case Atom::TableHeaderRight:
	out() << "</tr>\n";
        inTableHeader = false;
        break;
    case Atom::TableRowLeft:
	if (++numTableRows % 2 == 1)
	    out() << "<tr bgcolor=\"#f0f0f0\">";
	else
	    out() << "<tr bgcolor=\"#d0d0d0\">";
        break;
    case Atom::TableRowRight:
	out() << "</tr>\n";
        break;
    case Atom::TableItemLeft:
	if (inTableHeader)
	    out() << "<th>";
	else
	    out() << "<td>";
        break;
    case Atom::TableItemRight:
	if (inTableHeader)
	    out() << "</th>";
	else
	    out() << "</td>";
        break;
    case Atom::TableOfContents:
	generateTableOfContents(relative, marker);
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
qDebug("Generate class '%s'", classe->name().ascii());
    QList<Section> sections;
    QList<Section>::ConstIterator s;

    QString title = marker->plainFullName(classe) + " Class Reference";
    generateHeader(title, classe);
    generateTitle(title);

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

	generateSectionList(*s, classe, marker, CodeMarker::Summary);

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
		      << protect(marker->plainFullName((*p).first, classe))
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
	    if ( (*m)->access() != Node::Private ) // ### check necessary?
		generateDetailedMember(*m, classe, marker);
	    ++m;
	}
	++s;
    }
    generateFooter( classe );
qDebug("  DONE");
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

    QList<Section> sections;
    QList<Section>::ConstIterator s;

    generateHeader(fake->fullTitle(), fake);
    generateTitle(fake->fullTitle());

    Text brief = fake->doc().briefText();
    if (!brief.isEmpty()) {
	out() << "<p>";
	generateText( brief, fake, marker );
	out() << " <a href=\"#" << registerRef("details") << "\">More...</a></p>\n";
    }

    generateStatus(fake, marker);

    sections = marker->nonclassSections(fake, CodeMarker::Summary);
    s = sections.begin();
    while (s != sections.end()) {
	out() << "<a name=\"" << registerRef((*s).name) << "\"/>\n";
	out() << "<h3>" << protect((*s).name) << "</h3>\n";
	generateSectionList(*s, fake, marker, CodeMarker::Summary);
	++s;
    }

    if (!brief.isEmpty())
	out() << "<a name=\"" << registerRef("details") << "\"/>\n";

    if (!fake->doc().isEmpty()) {
	if (!brief.isEmpty())
	    out() << "<hr>\n" << "<h2>" << "Detailed Description" << "</h2>\n";
	generateBody(fake, marker);
	generateAlsoList(fake, marker);
    }

    sections = marker->nonclassSections(fake, CodeMarker::Detailed);
    s = sections.begin();
    while (s != sections.end()) {
	out() << "<hr>\n";
	out() << "<h2>" << protect((*s).name) << "</h2>\n";

	NodeList::ConstIterator m = (*s).members.begin();
	while ( m != (*s).members.end() ) {
	    generateDetailedMember(*m, fake, marker);
	    ++m;
	}
	++s;
    }
    generateFooter(fake);
}

QString HtmlGenerator::fileExtension()
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

void HtmlGenerator::generateTableOfContents(const Node *node, CodeMarker *marker)
{
    if (!node->doc().hasTableOfContents())
	return;
    QList<Atom *> toc = node->doc().tableOfContents();
    if (toc.isEmpty())
	return;

    QStringList sectionNumber;

    foreach (Atom *atom, toc) {
	int nextLevel = atom->string().toInt();
	if (sectionNumber.size() < nextLevel) {
            do {
	        out() << "<ul>";
                sectionNumber.append("1");
            } while (sectionNumber.size() < nextLevel);
	} else {
            while (sectionNumber.size() > nextLevel) {
	        out() << "</ul>\n";
                sectionNumber.removeLast();
            }
            sectionNumber.last() = QString::number(sectionNumber.last().toInt() + 1);
	}
	int numAtoms;
	Text headingText = sectionHeading(atom);
	out() << "<li>";
        out() << "<a href=\"#sec-" << sectionNumber.join(".") << "\">";
	generateAtomList(headingText.firstAtom(), node, marker, true, numAtoms);
        out() << "</a></li>\n";
    }
    while (!sectionNumber.isEmpty()) {
	out() << "</ul>\n";
	sectionNumber.removeLast();
    }
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
    QList<Section> sections = marker->classSections(classe, CodeMarker::SeparateList);
    if (sections.size() == 1) {
        QString fileName = fileBase( classe ) + "-members." + fileExtension();
        beginSubPage( classe->location(), fileName );
        generateHeader( "List of All Members for " + classe->name() );
        out() << "<p>This is the complete list of members for ";
        generateFullName( classe, 0, marker );
        out() << ", including inherited members.</p>\n";

        Section section = sections.first();
        generateSectionList(section, 0, marker, CodeMarker::SeparateList);

        generateFooter();
        endSubPage();
        return fileName;
    } else {
	return "";
    }
}

void HtmlGenerator::generateClassHierarchy(const Node *relative, CodeMarker *marker,
					   const QMap<QString, const Node *> &classMap)
{
    if (classMap.isEmpty())
	return;

    QMap<QString, const Node *> topLevel;
    QMap<QString, const Node *>::ConstIterator c = classMap.begin();
    while (c != classMap.end()) {
	const ClassNode *classe = static_cast<const ClassNode *>(*c);
        if (classe->baseClasses().isEmpty())
	    topLevel.insert(classe->name(), classe);
	++c;
    }

    QStack<QMap<QString, const Node *> > stack;
    stack.push(topLevel);

    out() << "<ul>\n";
    while (!stack.isEmpty()) {
	if (stack.top().isEmpty()) {
	    stack.pop();
	    out() << "</ul>\n";
	} else {
	    const ClassNode *child = static_cast<const ClassNode *>(*stack.top().begin());
	    out() << "<li>";
            generateFullName(child, relative, marker);
            out() << "\n";
	    stack.top().erase(stack.top().begin());

	    QMap<QString, const Node *> newTop;
	    foreach (RelatedClass d, child->derivedClasses()) {
		if (d.access != Node::Private)
		    newTop.insert(d.node->name(), d.node);
	    }
	    if (!newTop.isEmpty()) {
		stack.push(newTop);
		out() << "<ul>\n";
	    }
	}
    }
    out() << "</ul>\n";
}

void HtmlGenerator::generateAnnotatedList(const Node *relative, CodeMarker *marker,
				          const QMap<QString, const Node *> &classMap)
{
    out() << "<p><table width=\"100%\">\n";
    QMap<QString, const Node *>::ConstIterator c = classMap.begin();
    while (c != classMap.end()) {
	out() << "<tr bgcolor=#f0f0f0>";
	out() << "<td><b>";
	generateFullName(*c, relative, marker);
	out() << "</b>";
        Text brief = (*c)->doc().trimmedBriefText((*c)->name());
        if (!brief.isEmpty()) {
	    out() << "<td>";
            generateText(brief, *c, marker);
	}
	out() << '\n';
	++c;
    }
    out() << "</table>\n";
}

void HtmlGenerator::generateCompactList(const Node *relative, CodeMarker *marker,
					const QMap<QString, const Node *> &classMap)
{
    const int NumParagraphs = 27; // 26 letters plus symbols
    const int NumColumns = 5; // number of columns in the result

    if (classMap.isEmpty())
	return;

    /*
      First, find out the common prefix of all classes. For Qt, the
      prefix is Q. It can easily be derived from the first and last
      classes in alphabetical order (QAccel and QXtWidget in Qt 2.1).
    */
    int commonPrefixLen = 0;
    QString first = classMap.begin().key();
    QMap<QString, const Node *>::ConstIterator beforeEnd = classMap.end();
    QString last = (--beforeEnd).key();

    while (commonPrefixLen < first.length() + 1 && commonPrefixLen < last.length() + 1
	   && first[commonPrefixLen] == last[commonPrefixLen])
	++commonPrefixLen;

    /*
      Divide the data into 27 paragraphs: A, B, ..., Z, misc. QAccel
      will fall in paragraph 0 (A) and QXtWidget in paragraph 23 (X).
      This is the only place where we assume that NumParagraphs is
      27. Each paragraph is a QMap<QString, const Node *>.
    */
    QMap<QString, const Node *> paragraph[NumParagraphs];
    QString paragraphName[NumParagraphs];

    QMap<QString, const Node *>::ConstIterator c = classMap.begin();
    while (c != classMap.end()) {
	QString key = c.key().mid( commonPrefixLen ).toLower();
	int paragraphNo = NumParagraphs - 1;

	if ( key[0].unicode() >= 'a' && key[0].unicode() < 'z' ) {
	    paragraphNo = key[0].unicode() - 'a';
	    paragraphName[paragraphNo] = key[0].toUpper();
	} else {
	    paragraphNo = 26;
	    paragraphName[paragraphNo] = QChar( '?' );
	}
	paragraph[paragraphNo].insert( key, c.value() );
	++c;
    }

    /*
      Each paragraph j has a size: paragraph[j].count(). In the
      discussion, we will assume paragraphs 0 to 5 will have sizes
      3, 1, 4, 1, 5, 9.

      We now want to compute the paragraph offset. Paragraphs 0 to 6
      start at offsets 0, 3, 4, 8, 9, 14, 23.
    */
    int paragraphOffset[NumParagraphs + 1];
    int i, j, k;

    paragraphOffset[0] = 0;
    for ( j = 0; j < NumParagraphs; j++ )
	paragraphOffset[j + 1] = paragraphOffset[j] + paragraph[j].count();

    int firstOffset[NumColumns + 1];
    int currentOffset[NumColumns];
    int currentParagraphNo[NumColumns];
    int currentOffsetInParagraph[NumColumns];

    int numRows = ( classMap.count() + NumColumns - 1 ) / NumColumns;
    int curParagNo = 0;

    for ( i = 0; i < NumColumns; i++ ) {
	firstOffset[i] = i * numRows;
	currentOffset[i] = firstOffset[i];

	for ( j = curParagNo; j < NumParagraphs; j++ ) {
	    if ( paragraphOffset[j] > firstOffset[i] )
		break;
	    if ( paragraphOffset[j] <= firstOffset[i] )
		curParagNo = j;
	}
	currentParagraphNo[i] = curParagNo;
	currentOffsetInParagraph[i] = firstOffset[i] -
				      paragraphOffset[curParagNo];
    }
    firstOffset[NumColumns] = classMap.count();

    out() << "<p><table width=\"100%\">\n";
    for ( k = 0; k < numRows; k++ ) {
	out() << "<tr>\n";
	for ( i = 0; i < NumColumns; i++ ) {
	    if ( currentOffset[i] >= firstOffset[i + 1] ) {
		// this column is finished
		out() << "<td>\n<td>\n";
	    } else {
		while (currentOffsetInParagraph[i] == paragraph[currentParagraphNo[i]].count()) {
		    currentParagraphNo[i]++;
		    currentOffsetInParagraph[i] = 0;
		}

		out() << "<td align=\"right\">";
		if ( currentOffsetInParagraph[i] == 0 ) {
		    // start a new paragraph
		    out() << "<b>" << paragraphName[currentParagraphNo[i]] << "&nbsp;</b>";
		}
		out() << "\n";

		// bad loop
		QMap<QString, const Node *>::Iterator it;
		it = paragraph[currentParagraphNo[i]].begin();
		for ( j = 0; j < currentOffsetInParagraph[i]; j++ )
		    ++it;

		out() << "<td>";
                generateFullName(it.value(), relative, marker);
		out() << "\n";

		currentOffset[i]++;
		currentOffsetInParagraph[i]++;
	    }
	}
    }
    out() << "</table>\n";
}

void HtmlGenerator::generateFunctionIndex(const Node *relative, CodeMarker *marker)
{
    out() << "<p><center><font size=+1><b>";
    for ( int i = 0; i < 26; i++ ) {
	QChar ch( 'a' + i );
	out() << QString("<a href=\"#%1\">%2</a> ").arg(ch).arg(ch.toUpper());
    }
    out() << "</b></font></center>\n";

    char nextLetter = 'a';
    char currentLetter;

    out() << "<ul>\n";
    QMap<QString, QMap<QString, const Node *> >::ConstIterator f = funcIndex.begin();
    while (f != funcIndex.end()) {
	out() << "<li>";

	currentLetter = f.key()[0].unicode();
	while (islower(currentLetter) && currentLetter >= nextLetter) {
	    out() << QString("<a name=\"%1\"></a>\n").arg(nextLetter);
	    nextLetter++;
	}

	out() << protect(f.key()) << ":";
	QMap<QString, const Node *>::ConstIterator s = (*f).begin();
	while ( s != (*f).end() ) {
	    out() << " ";
	    generateFullName((*s)->parent(), relative, marker, *s);
	    ++s;
	}
	++f;
    }
    out() << "</ul>\n";
}

void HtmlGenerator::generateLegaleseList(const Node *relative, CodeMarker *marker)
{
    QMap<Text, const Node *>::ConstIterator it = legaleseTexts.begin();
    while (it != legaleseTexts.end()) {
	Text text = it.key();
	out() << "<hr>\n";
        generateText(text, relative, marker);
        out() << "<ul>\n";
        do {
	    out() << "<li>";
            generateFullName(it.value(), relative, marker);
            out() << "</li>\n";
	    ++it;
        } while (it != legaleseTexts.end() && it.key() == text);
        out() << "</ul>\n";
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

void HtmlGenerator::generateSectionList(const Section& section, const InnerNode *relative,
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

    if ( ref[0].toLower() >= 'a' && ref[0].toLower() <= 'z' ) {
	clean += ref[0];
    } else if ( ref[0] == '~' ) {
	clean += "dtor-";
    } else if ( ref[0] == '_' ) {
	clean += "underscore-";
    } else {
	clean += "A";
    }

    for ( int i = 1; i < (int) ref.length(); i++ ) {
	if ( (ref[i].toLower() >= 'a' && ref[i].toLower() <= 'z') ||
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
	    clean += QString::number((int)ref[i].unicode(), 16);
	}
    }
    return clean;
}

QString HtmlGenerator::registerRef( const QString& ref )
{
    QString clean = cleanRef( ref );

    for ( ;; ) {
	QString& prevRef = refMap[clean.toLower()];
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
    while ((k = html.indexOf(linkTag, k)) != -1) {
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

void HtmlGenerator::generateFullName(const Node *apparentNode, const Node *relative,
				     CodeMarker *marker, const Node *actualNode)
{
    if ( actualNode == 0 )
	actualNode = apparentNode;
    out() << "<a href=\"" << linkForNode(actualNode, relative) << "\">"
	  << protect(marker->plainFullName(apparentNode, relative))
	  << "</a>";
}

void HtmlGenerator::generateDetailedMember(const Node *node, const InnerNode *relative,
					   CodeMarker *marker)
{
    out() << "<h3 class=\"fn\">";
    out() << "<a name=\"" + refForNode( node ) + "\"/>";
    generateSynopsis( node, relative, marker, CodeMarker::Detailed );
    out() << "</h3>\n";
    generateStatus(node, marker);
    generateBody(node, marker);
    generateThreadSafeness(node, marker);
    if (node->type() == Node::Property) {
	const PropertyNode *property = static_cast<const PropertyNode *>(node);
        Section section;

	section.members += property->getters();
	section.members += property->setters();
	section.members += property->resetters();

	if (!section.members.isEmpty()) {
	    out() << "<p>Accessor functions:</p>\n";
	    generateSectionList(section, relative, marker, CodeMarker::Summary);
	}
    }
    generateAlsoList( node, marker );
}

void HtmlGenerator::findAllClasses(const InnerNode *node)
{
    NodeList::ConstIterator c = node->childNodes().begin();
    while (c != node->childNodes().end()) {
	if ((*c)->access() != Node::Private) {
	    if ((*c)->type() == Node::Class && !(*c)->doc().isEmpty()) {
	        allClasses.insert((*c)->name(), *c);
                if ((*c)->status() == Node::Main)
		    mainClasses.insert((*c)->name(), *c);
	    } else if ((*c)->isInnerNode()) {
	        findAllClasses(static_cast<InnerNode *>(*c));
	    }
	}
	++c;
    }
}

void HtmlGenerator::findAllFunctions(const InnerNode *node)
{
    NodeList::ConstIterator c = node->childNodes().begin();
    while (c != node->childNodes().end()) {
	if ((*c)->access() != Node::Private) {
	    if ((*c)->isInnerNode()) {
		findAllFunctions(static_cast<const InnerNode *>(*c));
            } else if ((*c)->type() == Node::Function) {
		const FunctionNode *func = static_cast<const FunctionNode *>(*c);
                if (func->status() != Node::Obsolete && func->metaness() != FunctionNode::Ctor
			&& func->metaness() != FunctionNode::Dtor) {
		    funcIndex[(*c)->name()].insert((*c)->parent()->name(), *c);
		}
            }
        }
	++c;
    }
}

void HtmlGenerator::findAllLegaleseTexts(const InnerNode *node)
{
    NodeList::ConstIterator c = node->childNodes().begin();
    while (c != node->childNodes().end()) {
	if ((*c)->access() != Node::Private) {
	    if (!(*c)->doc().legaleseText().isEmpty())
		legaleseTexts.insertMulti((*c)->doc().legaleseText(), *c);
	    if ((*c)->isInnerNode())
		findAllLegaleseTexts(static_cast<const InnerNode *>(*c));
        }
	++c;
    }
}

int HtmlGenerator::hOffset(const Node *node)
{
    switch (node->type()) {
    case Node::Namespace:
    case Node::Class:
	return 2;
    case Node::Fake:
	if (node->doc().briefText().isEmpty())
	    return 1;
	else
	    return 2;
    case Node::Enum:
    case Node::Typedef:
    case Node::Function:
    case Node::Property:
    default:
	return 3;
    }
}
