/*
  htmlgenerator.cpp
*/

#include "codemarker.h"
#include "htmlgenerator.h"
#include "node.h"
#include "separator.h"
#include "tree.h"
#include <ctype.h>

#include <qlist.h>
#include <qiterator.h>

#define COMMAND_PROJECT                 Doc::alias("project")
#define COMMAND_VERSION                 Doc::alias("version")

static bool showBrokenLinks = false;

HtmlGenerator::HtmlGenerator()
    : inLink(false), inContents(false), inSectionHeading(false), inTableHeader(false), numTableRows(0), threeColumnEnumValueTable(true),
      funcLeftParen("\\S(\\()"), tre(0)
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

    project = config.getString(COMMAND_PROJECT);
    if (project.isEmpty())
        project = "Project";
}

void HtmlGenerator::terminateGenerator()
{
    dcfClassesRoot.ref = "classes.html";
    dcfClassesRoot.title = "Classes";
    qSort(dcfClassesRoot.subsections);

    dcfOverviewsRoot.ref = "overviews.html";
    dcfOverviewsRoot.title = "Overviews";
    qSort(dcfOverviewsRoot.subsections);

    dcfExamplesRoot.ref = "examples.html";
    dcfExamplesRoot.title = "Tutorial & Examples";
    qSort(dcfExamplesRoot.subsections);

    DcfSection qtRoot;
    appendDcfSubSection(&qtRoot, dcfClassesRoot);
    appendDcfSubSection(&qtRoot, dcfOverviewsRoot);
    appendDcfSubSection(&qtRoot, dcfExamplesRoot);

    generateDcf("qt", "index.html", "Qt Reference Documentation", qtRoot);
    generateDcf("designer", "designer-manual.html", "Qt Designer Manual", dcfDesignerRoot);
    generateDcf("linguist", "linguist-manual.html", "Qt Linguist Manual", dcfLinguistRoot);
    generateDcf("assistant", "assistant-manual.html", "Qt Assistant Manual", dcfAssistantRoot);
    generateDcf("qmake", "qmake-manual.html", "qmake Manual", dcfQmakeRoot);

    Generator::terminateGenerator();
}

QString HtmlGenerator::format()
{
    return "HTML";
}

void HtmlGenerator::generateTree(const Tree *tree, CodeMarker *marker)
{
    tre = tree;
    nonCompatClasses.clear();
    mainClasses.clear();
    compatClasses.clear();
    moduleClassMap.clear();
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
    inContents = false;
    inSectionHeading = false;
    inTableHeader = false;
    numTableRows = 0;
    threeColumnEnumValueTable = true;
    link = "";
    sectionNumber.clear();
}

int HtmlGenerator::generateAtom(const Atom *atom, const Node *relative, CodeMarker *marker)
{
    int skipAhead = 0;
    static bool in_para = false;

    switch (atom->type()) {
    case Atom::AbstractLeft:
	break;
    case Atom::AbstractRight:
	break;
    case Atom::AutoLink:
        if (!inLink && !inContents && !inSectionHeading) {
            link = getLink(atom, relative, marker);
	    if (!link.isEmpty()) {
	        out() << "<a href=\"" << link << "\">";
                inLink = true;
                generateLink(atom, relative, marker);
                if (inLink)
                    out() << "</a>";
                inLink = false;
            } else {
                out() << protect(atom->string());
            }
        } else {
            out() << protect(atom->string());
        }
        break;
    case Atom::BaseName:
	break;
    case Atom::BriefLeft:
	out() << "<p>";
	if ( relative->type() == Node::Property || relative->type() == Node::Variable ) {
	    QString str;
	    atom = atom->next();
	    while ( atom != 0 && atom->type() != Atom::BriefRight ) {
		if ( atom->type() == Atom::String || atom->type() == Atom::AutoLink )
		    str += atom->string();
		skipAhead++;
		atom = atom->next();
	    }
	    str[0] = str[0].toLower();
	    if ( str.right(1) == "." )
		str.truncate( str.length() - 1 );
	    out() << "This ";
	    if (relative->type() == Node::Property)
		out() << "property";
	    else
		out() << "variable";
	    out() << " holds " << str << ".";
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
	    out() << highlightedCode( atom->string(), marker, relative );
	}
	out() << formattingRightMap()[ATOM_FORMATTING_TELETYPE];
	break;
    case Atom::Code:
	out() << "<pre>" << trimmedTrailing(protectPreformatted(plainCode(indent(4, atom->string()))))
	      << "</pre>\n";
	break;
    case Atom::CodeBad:
        out() << "<pre><font color=\"#404040\">"
              << trimmedTrailing(protect(plainCode(indent(4, atom->string()))))
	      << "</font></pre>\n";
	break;
    case Atom::CodeNew:
        out() << "<p>you can rewrite it as</p>\n"
              << "<pre>" << trimmedTrailing(protect(plainCode(indent(4, atom->string()))))
              << "</pre>\n";
        break;
    case Atom::CodeOld:
        out() << "<p>For example, if you have code like</p>\n"
              << "<pre><font color=\"#404040\">"
	      << trimmedTrailing(protect(plainCode(indent(4, atom->string()))))
              << "</font></pre>\n";
        break;
    case Atom::FootnoteLeft:
        // ### For now
        if (in_para) {
            out() << "</p>\n";
            in_para = false;
        }
	out() << "<!-- ";
	break;
    case Atom::FootnoteRight:
        // ### For now
	out() << "-->";
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
            if (inLink) {
	        if ( link.isEmpty() ) {
                    if (showBrokenLinks)
                        out() << "</i>";
	        } else {
		    out() << "</a>";
	        }
            }
            inLink = false;
	} else {
	    out() << formattingRightMap()[atom->string()];
	}
	break;
    case Atom::GeneratedList:
	if (atom->string() == "annotatedclasses") {
	    generateAnnotatedList(relative, marker, nonCompatClasses);
	} else if (atom->string() == "classes") {
	    generateCompactList(relative, marker, nonCompatClasses);
	} else if (atom->string().contains("classesbymodule")) {
            QString arg = atom->string().trimmed();
            QString moduleName = atom->string().mid(atom->string().indexOf(
                "classesbymodule") + 15).trimmed();
            if (moduleClassMap.contains(moduleName))
	        generateAnnotatedList(relative, marker, moduleClassMap[moduleName]);
	} else if (atom->string() == "classhierarchy") {
	    generateClassHierarchy(relative, marker, nonCompatClasses);
	} else if (atom->string() == "compatclasses") {
	    generateCompactList(relative, marker, compatClasses);
	} else if (atom->string() == "functionindex") {
	    generateFunctionIndex(relative, marker);
	} else if (atom->string() == "legalese") {
	    generateLegaleseList(relative, marker);
	} else if (atom->string() == "mainclasses") {
	    generateCompactList(relative, marker, mainClasses);
	} else if (atom->string() == "overviews") {
            generateOverviewList(relative, marker);
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
		out() << " />";
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
        link = getLink(atom, relative, marker);
	if (link.isEmpty()) {
            if (showBrokenLinks)
                out() << "<i>";
            relative->doc().location().warning(tr("Cannot link to '%1'").arg(atom->string()));
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
            if (showBrokenLinks)
                out() << "<i>";
	} else {
	    out() << "<a href=\"" << link << "\">";
	}
	inLink = true;
	skipAhead = 1;
	break;
    case Atom::ListLeft:
        if (in_para) {
            out() << "</p>\n";
            in_para = false;
        }
	if ( atom->string() == ATOM_LIST_BULLET ) {
	    out() << "<ul>\n";
	} else if ( atom->string() == ATOM_LIST_TAG ) {
	    out() << "<dl>\n";
	} else if ( atom->string() == ATOM_LIST_VALUE ) {
            threeColumnEnumValueTable = isThreeColumnEnumValueTable(atom);
            if (threeColumnEnumValueTable) {
	        out() << "<table border=\"1\" cellpadding=\"2\" cellspacing=\"1\" width=\"100%\">\n"
                         "<tr><th width=\"25%\">Constant</th><th width=\"15%\">Value</th>"
                         "<th width=\"60%\">Description</th></tr>\n";
            } else {
                out() << "<table border=\"1\" cellpadding=\"2\" cellspacing=\"1\" width=\"40%\">\n"
                      << "<tr><th width=\"60%\">Constant</th><th width=\"40%\">Value</th></tr>\n";
            }
	} else {
            out() << "<ol type=";
            if ( atom->string() == ATOM_LIST_UPPERALPHA ) {
		out() << "\"A\"";
            } else if ( atom->string() == ATOM_LIST_LOWERALPHA ) {
		out() << "\"a\"";
            } else if ( atom->string() == ATOM_LIST_UPPERROMAN ) {
		out() << "\"I\"";
            } else if ( atom->string() == ATOM_LIST_LOWERROMAN ) {
		out() << "\"i\"";
            } else { // ( atom->string() == ATOM_LIST_NUMERIC )
		out() << "\"1\"";
            }
            if ( atom->next() != 0 && atom->next()->string().toInt() != 1 )
		out() << " start=\"" << atom->next()->string() << "\"";
            out() << ">\n";
	}
	break;
    case Atom::ListItemNumber:
	break;
    case Atom::ListTagLeft:
	if ( atom->string() == ATOM_LIST_TAG ) {
	    out() << "<dt>";
	} else { // ( atom->string() == ATOM_LIST_VALUE )
	    out() << "<tr><td valign=\"top\"><tt>"
                  << protect(plainCode(marker->markedUpEnumValue(atom->next()->string(),
                                                                 relative)))
                  << "</tt></td><td align=\"center\" valign=\"top\">";

            QString itemValue;
            if (relative->type() == Node::Enum) {
                const EnumNode *enume = static_cast<const EnumNode *>(relative);
                itemValue = enume->itemValue(atom->next()->string());
            }

            if (itemValue.isEmpty())
                out() << "&nbsp;";
            else
                out() << "<tt>" << protect(itemValue) << "</tt>";

            skipAhead = 1;
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
            if (threeColumnEnumValueTable) {
                out() << "</td><td valign=\"top\">";
	        if ( matchAhead(atom, Atom::ListItemRight) )
		    out() << "&nbsp;";
            }
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
        in_para = true;
	break;
    case Atom::ParaRight:
        if (in_para) {
            out() << "</p>\n";
            in_para = false;
        }
	//if (!matchAhead(atom, Atom::ListItemRight) && !matchAhead(atom, Atom::TableItemRight))
	//    out() << "</p>\n";
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
#if 0
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
            out() << "<a name=\"sec-" << sectionNumber.join("-") << "\"></a>\n";
        }
#else
        out() << "<a name=\"" << Doc::canonicalTitle(Text::sectionHeading(atom).toString())
              << "\"></a>\n";
#endif
	break;
    case Atom::SectionRight:
	break;
    case Atom::SectionHeadingLeft:
	out() << "<h" + QString::number(atom->string().toInt() + hOffset(relative)) + ">";
        inSectionHeading = true;
	break;
    case Atom::SectionHeadingRight:
	out() << "</h" + QString::number(atom->string().toInt() + hOffset(relative)) + ">\n";
        inSectionHeading = false;
	break;
    case Atom::SidebarLeft:
	break;
    case Atom::SidebarRight:
	break;
    case Atom::String:
	if (inLink && !inContents && !inSectionHeading) {
            generateLink(atom, relative, marker);
	} else {
	    out() << protect( atom->string() );
	}
	break;
    case Atom::TableLeft:
        if (in_para) {
            out() << "</p>\n";
            in_para = false;
        }
	if (!atom->string().isEmpty()) {
            if (atom->string().contains("%"))
                out() << "<table width=\"" << atom->string() << "\" "
                      << "align=\"center\" cellpadding=\"2\" "
                      << "cellspacing=\"1\" border=\"0\">\n";
            else
                out() << "<table align=\"center\" cellpadding=\"2\" cellspacing=\"1\" border=\"0\">\n";
        } else {
            out() << "<table align=\"center\" cellpadding=\"2\" cellspacing=\"1\" border=\"0\">\n";
        }
        numTableRows = 0;
	break;
    case Atom::TableRight:
	out() << "</table>\n";
	break;
    case Atom::TableHeaderLeft:
	out() << "<tr valign=\"top\" bgcolor=\"#a2c511\">";
        inTableHeader = true;
        break;
    case Atom::TableHeaderRight:
	out() << "</tr>\n";
        inTableHeader = false;
        break;
    case Atom::TableRowLeft:
	if (++numTableRows % 2 == 1)
	    out() << "<tr valign=\"top\" bgcolor=\"#f0f0f0\">";
	else
	    out() << "<tr valign=\"top\" bgcolor=\"#e0e0e0\">";
        break;
    case Atom::TableRowRight:
	out() << "</tr>\n";
        break;
    case Atom::TableItemLeft:
        {
	    if (inTableHeader)
	        out() << "<th";
	    else
	        out() << "<td";

            QStringList spans = atom->string().split(",");
	    if (spans.size() == 2) {
                if (spans.at(0) != "1")
                    out() << " colspan=\"" << spans.at(0) << "\"";
                if (spans.at(1) != "1")
                    out() << " rowspan=\"" << spans.at(1) << "\"";
                out() << ">";
	    }
	    if ( matchAhead(atom, Atom::ParaLeft) )
	        skipAhead = 1;
        }
        break;
    case Atom::TableItemRight:
	if (inTableHeader)
	    out() << "</th>";
	else
	    out() << "</td>";
	if ( matchAhead(atom, Atom::ParaLeft) )
	    skipAhead = 1;
        break;
    case Atom::TableOfContents:
        {
            int numColumns = 1;
            const Node *node = relative;

            Doc::SectioningUnit sectioningUnit = Doc::Section4;
            QStringList params = atom->string().split(",");
            QString columnText = params.at(0);
            QStringList pieces = columnText.split(" ", QString::SkipEmptyParts);
            if (pieces.size() >= 2) {
                columnText = pieces.at(0);
                pieces.pop_front();
                QString path = pieces.join(" ").trimmed();
                node = findNodeForTarget(path, relative, marker, atom);
            }

            if (params.size() == 2) {
                numColumns = qMax(columnText.toInt(), numColumns);
                sectioningUnit = (Doc::SectioningUnit)params.at(1).toInt();
            }

            if (node)
	        generateTableOfContents(node, marker, sectioningUnit, numColumns,
                                        relative);
        }
	break;
    case Atom::Target:
        out() << "<a name=\"" << Doc::canonicalTitle(atom->string()) << "\"></a>";
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

void HtmlGenerator::generateClassLikeNode(const InnerNode *inner, CodeMarker *marker)
{
    QList<Section> sections;
    QList<Section>::ConstIterator s;

    const ClassNode *classe = 0;
    const NamespaceNode *namespasse = 0;
    const FakeNode *fake = 0;

    QString title;
    if (inner->type() == Node::Namespace) {
        namespasse = static_cast<const NamespaceNode *>(inner);
        title = marker->plainFullName(inner) + " Namespace Reference";
    } else if (inner->type() == Node::Class) {
        classe = static_cast<const ClassNode *>(inner);
        title = marker->plainFullName(inner) + " Class Reference";
    } else if (inner->type() == Node::Fake) {
        fake = static_cast<const FakeNode *>(inner);
        title = static_cast<const FakeNode *>(inner)->fullTitle();
    }

    DcfSection classSection;
    classSection.title = title;
    classSection.ref = linkForNode(inner, 0);
    classSection.keywords += qMakePair(inner->name(), classSection.ref);

    generateHeader(title, inner);
    generateTitle(title);

    generateBrief(inner, marker);
    generateIncludes(inner, marker);
    generateStatus(inner, marker);
    if (classe) {
        generateModuleName(classe, marker);
	generateInherits(classe, marker);
	generateInheritedBy(classe, marker);
    }
    generateThreadSafeness(inner, marker);

    out() << "<ul>\n";

    QString membersLink = generateListOfAllMemberFile(inner, marker);
    if (!membersLink.isEmpty())
        out() << "<li><a href=\"" << membersLink << "\">"
              << "List of all members, including inherited members</a></li>\n";

    QString obsoleteLink = generateLowStatusMemberFile(inner, marker, CodeMarker::Obsolete);
    if (!obsoleteLink.isEmpty())
        out() << "<li><a href=\"" << obsoleteLink << "\">"
              << "Obsolete members</a></li>\n";

    QString compatLink = generateLowStatusMemberFile(inner, marker, CodeMarker::Compat);
    if (!compatLink.isEmpty())
        out() << "<li><a href=\"" << compatLink << "\">"
              << "Qt 3 support members</a></li>\n";

    out() << "</ul>\n";

    bool needOtherSection = false;

    sections = marker->sections(inner, CodeMarker::Summary, CodeMarker::Okay);
    s = sections.begin();
    while ( s != sections.end() ) {
        if (s->members.isEmpty()) {
            if (!s->inherited.isEmpty())
                needOtherSection = true;
        } else {
	    out() << "<a name=\"" << registerRef((*s).name.toLower()) << "\"></a>\n";
	    out() << "<h3>" << protect((*s).name) << "</h3>\n";

	    generateSectionList(*s, inner, marker, CodeMarker::Summary);
        }
	++s;
    }

    if (needOtherSection) {
	out() << "<h3>Additional Inherited Members</h3>\n"
                 "<ul>\n";

        s = sections.begin();
        while ( s != sections.end() ) {
            if (s->members.isEmpty() && !s->inherited.isEmpty())
                generateSectionInheritedList(*s, inner, marker);
            ++s;
        }
	out() << "</ul>\n";
    }

    out() << "<a name=\"" << registerRef( "details" ) << "\"></a>\n";

    if ( !inner->doc().isEmpty() ) {
	out() << "<hr />\n" << "<h2>" << "Detailed Description" << "</h2>\n";
	generateBody( inner, marker );
	generateAlsoList( inner, marker );
    }

    sections = marker->sections(inner, CodeMarker::Detailed, CodeMarker::Okay);
    s = sections.begin();
    while ( s != sections.end() ) {
	out() << "<hr />\n";
	out() << "<h2>" << protect( (*s).name ) << "</h2>\n";

	NodeList::ConstIterator m = (*s).members.begin();
	while ( m != (*s).members.end() ) {
	    if ( (*m)->access() != Node::Private ) { // ### check necessary?
		generateDetailedMember(*m, inner, marker);
                QStringList names;
                names << (*m)->name();
                if ((*m)->type() == Node::Function) {
                    const FunctionNode *func = reinterpret_cast<const FunctionNode *>(*m);
                    if (func->metaness() == FunctionNode::Ctor || func->metaness() == FunctionNode::Dtor
                            || func->overloadNumber() != 1)
                        names.clear();
                } else if ((*m)->type() == Node::Property) {
                    const PropertyNode *prop = reinterpret_cast<const PropertyNode *>(*m);
                    if (!prop->getters().isEmpty() && !names.contains(prop->getters().first()->name()))
                        names << prop->getters().first()->name();
                    if (!prop->setters().isEmpty())
                        names << prop->setters().first()->name();
                    if (!prop->resetters().isEmpty())
                        names << prop->resetters().first()->name();
                } else if ((*m)->type() == Node::Enum) {
                    const EnumNode *enume = reinterpret_cast<const EnumNode *>(*m);
                    if (enume->flagsType())
                        names << enume->flagsType()->name();
                    names << enume->doc().enumItemNames();
                }
                foreach (QString name, names)
                    classSection.keywords += qMakePair(name, linkForNode(*m, 0));
            }
	    ++m;
	}
	++s;
    }
    generateFooter( inner );

    if (!membersLink.isEmpty()) {
        DcfSection membersSection;
        membersSection.title = "List of all members";
        membersSection.ref = membersLink;
        appendDcfSubSection(&classSection, membersSection);
    }
    if (!obsoleteLink.isEmpty()) {
        DcfSection obsoleteSection;
        obsoleteSection.title = "Obsolete members";
        obsoleteSection.ref = obsoleteLink;
        appendDcfSubSection(&classSection, obsoleteSection);
    }
    if (!compatLink.isEmpty()) {
        DcfSection compatSection;
        compatSection.title = "Qt 3 support members";
        compatSection.ref = compatLink;
        appendDcfSubSection(&classSection, compatSection);
    }

    appendDcfSubSection(&dcfClassesRoot, classSection);
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

    DcfSection fakeSection;
    fakeSection.title = fake->fullTitle();
    fakeSection.ref = linkForNode(fake, 0);

    QList<Section> sections;
    QList<Section>::const_iterator s;

    QString htmlTitle = fake->fullTitle();
    if (!fake->subTitle().isEmpty())
        htmlTitle += " (" + fake->subTitle() + ")";

    generateHeader(htmlTitle, fake, marker);
    generateTitle(fake->fullTitle(), fake->subTitle());

    generateBrief(fake, marker);
    generateStatus(fake, marker);

    sections = marker->sections(fake, CodeMarker::Summary, CodeMarker::Okay);
    s = sections.begin();
    while (s != sections.end()) {
	out() << "<a name=\"" << registerRef((*s).name) << "\"></a>\n";
	out() << "<h3>" << protect((*s).name) << "</h3>\n";
	generateSectionList(*s, fake, marker, CodeMarker::Summary);
	++s;
    }

    Text brief = fake->doc().briefText();
    if (!brief.isEmpty())
	out() << "<a name=\"" << registerRef("details") << "\"></a>\n";

    if (!fake->doc().isEmpty()) {
	if (!brief.isEmpty())
	    out() << "<hr />\n" << "<h2>" << "Detailed Description" << "</h2>\n";
    }
    generateBody(fake, marker);
    generateAlsoList(fake, marker);

    if (!fake->groupMembers().isEmpty()) {
        QMap<QString, const Node *> groupMembersMap;
        foreach (Node *node, fake->groupMembers())
            groupMembersMap[node->name()] = node;
        generateAnnotatedList(fake, marker, groupMembersMap);
    }

    fakeSection.keywords += qMakePair(fakeSection.title, fakeSection.ref);

    sections = marker->sections(fake, CodeMarker::Detailed, CodeMarker::Okay);
    s = sections.begin();
    while (s != sections.end()) {
	out() << "<hr />\n";
	out() << "<h2>" << protect((*s).name) << "</h2>\n";

	NodeList::ConstIterator m = (*s).members.begin();
	while ( m != (*s).members.end() ) {
	    generateDetailedMember(*m, fake, marker);
            fakeSection.keywords += qMakePair((*m)->name(), linkForNode(*m, 0));
	    ++m;
	}
	++s;
    }
    generateFooter(fake);

    if (fake->subType() == FakeNode::Example) {
        appendDcfSubSection(&dcfExamplesRoot, fakeSection);
    } else if (fake->subType() != FakeNode::File) {
        QString contentsPage = fake->links().value(Node::ContentsLink).first;

        if (contentsPage == "Qt Designer Manual") {
            appendDcfSubSection(&dcfDesignerRoot, fakeSection);
        } else if (contentsPage == "Qt Linguist Manual") {
            appendDcfSubSection(&dcfLinguistRoot, fakeSection);
        } else if (contentsPage == "Qt Assistant Manual") {
            appendDcfSubSection(&dcfAssistantRoot, fakeSection);
        } else if (contentsPage == "qmake Manual") {
            appendDcfSubSection(&dcfQmakeRoot, fakeSection);
        } else {
            appendDcfSubSection(&dcfOverviewsRoot, fakeSection);
        }
    }
}

QString HtmlGenerator::fileExtension()
{
    return "html";
}

void HtmlGenerator::generateHeader(const QString& title, const Node *node,
                                   CodeMarker *marker)
{
    out() << "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\n";

    out() << "<!DOCTYPE html\n"
	     "    PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"DTD/xhtml1-strict.dtd\">\n"
	     "<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\" lang=\"en\">\n";

    if (node && !node->doc().location().isEmpty())
        out() << "<!-- " << node->doc().location().filePath() << " -->\n";

    QString shortVersion = tre->version();
    if (shortVersion.count(QChar('.')) == 2)
        shortVersion.truncate(shortVersion.lastIndexOf(QChar('.')));
    if (!shortVersion.isEmpty())
        shortVersion = "Qt " + shortVersion + ": ";

    out() << "<head>\n"
	     "    <title>" << shortVersion << protect( title ) << "</title>\n";
    if (!style.isEmpty())
	out() << "    <style>" << style << "</style>\n";

    const QMap<QString, QString> &metaMap = node->doc().metaTagMap();
    if (!metaMap.isEmpty()) {
        QMapIterator<QString, QString> i(metaMap);
        while (i.hasNext()) {
            i.next();
            out() << "    <meta name=\"" << protect(i.key()) << "\" contents=\""
                  << protect(i.value()) << "\" />\n";
        }
    }

    navigationLinks.clear();

    if (node && !node->links().empty()) {
        QPair<QString,QString> linkPair;
        QPair<QString,QString> anchorPair;
        const Node *linkNode;

        if (node->links().contains(Node::PreviousLink)) {
            linkPair = node->links()[Node::PreviousLink];
            linkNode = findNodeForTarget(linkPair.first, node, marker);
            if (!linkNode || linkNode == node)
                anchorPair = linkPair;
            else
                anchorPair = anchorForNode(linkNode);

            out() << "    <link rel=\"prev\" href=\""
                  << anchorPair.first << "\" />\n";

            navigationLinks += "[Previous: <a href=\"" + anchorPair.first + "\">";
            if (linkPair.first == linkPair.second && !anchorPair.second.isEmpty())
                navigationLinks += protect(anchorPair.second);
            else
                navigationLinks += protect(linkPair.second);
            navigationLinks += "</a>]\n";
        }
        if (node->links().contains(Node::ContentsLink)) {
            linkPair = node->links()[Node::ContentsLink];
            linkNode = findNodeForTarget(linkPair.first, node, marker);
            if (!linkNode || linkNode == node)
                anchorPair = linkPair;
            else
                anchorPair = anchorForNode(linkNode);

            out() << "    <link rel=\"contents\" href=\""
                  << anchorPair.first << "\" />\n";

            navigationLinks += "[<a href=\"" + anchorPair.first + "\">";
            if (linkPair.first == linkPair.second && !anchorPair.second.isEmpty())
                navigationLinks += protect(anchorPair.second);
            else
                navigationLinks += protect(linkPair.second);
            navigationLinks += "</a>]\n";
        }
        if (node->links().contains(Node::NextLink)) {
            linkPair = node->links()[Node::NextLink];
            linkNode = findNodeForTarget(linkPair.first, node, marker);
            if (!linkNode || linkNode == node)
                anchorPair = linkPair;
            else
                anchorPair = anchorForNode(linkNode);

            out() << "    <link rel=\"next\" href=\""
                  << anchorPair.first << "\" />\n";

            navigationLinks += "[Next: <a href=\"" + anchorPair.first + "\">";
            if (linkPair.first == linkPair.second && !anchorPair.second.isEmpty())
                navigationLinks += protect(anchorPair.second);
            else
                navigationLinks += protect(linkPair.second);
            navigationLinks += "</a>]\n";
        }
        if (node->links().contains(Node::IndexLink)) {
            linkPair = node->links()[Node::IndexLink];
            linkNode = findNodeForTarget(linkPair.first, node, marker);
            if (!linkNode || linkNode == node)
                anchorPair = linkPair;
            else
                anchorPair = anchorForNode(linkNode);
            out() << "    <link rel=\"index\" href=\""
                  << anchorPair.first << "\" />\n";
        }
        if (node->links().contains(Node::StartLink)) {
            linkPair = node->links()[Node::StartLink];
            linkNode = findNodeForTarget(linkPair.first, node, marker);
            if (!linkNode || linkNode == node)
                anchorPair = linkPair;
            else
                anchorPair = anchorForNode(linkNode);
            out() << "    <link rel=\"start\" href=\""
                  << anchorPair.first << "\" />\n";
        }
    }
    out() << "</head>\n"
             "<body>\n"
	  << QString(postHeader).replace("\\" + COMMAND_VERSION, tre->version());

    if (node && !node->links().empty())
        out() << "<p>\n" << navigationLinks << "</p>\n";
}

void HtmlGenerator::generateTitle(const QString& title, const QString &subTitle)
{
    out() << "<h1 align=\"center\">" << protect( title );
    if (!subTitle.isEmpty())
        out() << "<br /><small><small>" << protect(subTitle) << "</small></small>";
    out() << "</h1>\n";
}

void HtmlGenerator::generateFooter( const Node *node )
{
    if (node && !node->links().empty())
        out() << "<p>\n" << navigationLinks << "</p>\n";

    out() << QString(footer).replace("\\" + COMMAND_VERSION, tre->version())
	  << QString(address).replace("\\" + COMMAND_VERSION, tre->version())
	  << "</body>\n"
	     "</html>\n";
}

void HtmlGenerator::generateBrief(const Node *node, CodeMarker *marker)
{
    Text brief = node->doc().briefText();
    if (!brief.isEmpty()) {
	out() << "<p>";
	generateText(brief, node, marker);
	out() << " <a href=\"#" << registerRef("details") << "\">More...</a></p>\n";
    }
}

void HtmlGenerator::generateIncludes(const InnerNode *inner, CodeMarker *marker)
{
    if (!inner->includes().isEmpty()) {
	QString code = highlightedCode(marker->markedUpIncludes(inner->includes()), marker, inner);
	out() << "<pre>" << trimmedTrailing(code) << "</pre>";
    }
}

void HtmlGenerator::generateTableOfContents(const Node *node, CodeMarker *marker,
                                            Doc::SectioningUnit sectioningUnit,
                                            int numColumns, const Node *relative)

{
    if (!node->doc().hasTableOfContents())
	return;
    QList<Atom *> toc = node->doc().tableOfContents();
    if (toc.isEmpty())
	return;

    QString nodeName = "";
    if (node != relative)
        nodeName = node->name();

    QStringList sectionNumber;
    int columnSize = 0;

    QString tdTag;
    if (numColumns > 1) {
        tdTag = "<td width=\"" + QString::number((100 + numColumns - 1) / numColumns) + "%\">";
        out() << "<table width=\"100%\">\n<tr valign=\"top\">" << tdTag << "\n";
    }

    // disable nested links in table of contents
    inContents = true;
    inLink = true;

    for (int i = 0; i < toc.size(); ++i) {
        Atom *atom = toc.at(i);

	int nextLevel = atom->string().toInt();
        if (nextLevel > (int)sectioningUnit)
            continue;

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
	Text headingText = Text::sectionHeading(atom);

        if (sectionNumber.size() == 1 && columnSize > toc.size() / numColumns) {
            out() << "</ul></td>" << tdTag << "<ul>\n";
            columnSize = 0;
        }
	out() << "<li>";
        out() << "<a href=\"" << nodeName << "#" << Doc::canonicalTitle(headingText.toString())
              << "\">";
	generateAtomList(headingText.firstAtom(), node, marker, true, numAtoms);
        out() << "</a></li>\n";

        ++columnSize;
    }
    while (!sectionNumber.isEmpty()) {
	out() << "</ul>\n";
	sectionNumber.removeLast();
    }

    if (numColumns > 1)
        out() << "</td></tr></table>\n";

    inContents = false;
    inLink = false;
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
	    generateText( Text::sectionHeading(bar.next.begin()), node, marker );
	    out() << "</a>]\n";
	}
	out() << "</p>\n";
    }
}
#endif

QString HtmlGenerator::generateListOfAllMemberFile(const InnerNode *inner, CodeMarker *marker)
{
    QList<Section> sections;
    QList<Section>::ConstIterator s;

    sections = marker->sections(inner, CodeMarker::SeparateList, CodeMarker::Okay);
    if (sections.isEmpty())
        return QString();

    QString fileName = fileBase(inner) + "-members." + fileExtension();
    beginSubPage(inner->location(), fileName);
    QString title = "List of All Members for " + inner->name();
    generateHeader(title, inner);
    generateTitle(title);
    out() << "<p>This is the complete list of members for ";
    generateFullName(inner, 0, marker);
    out() << ", including inherited members.</p>\n";

    Section section = sections.first();
    generateSectionList(section, 0, marker, CodeMarker::SeparateList);

    generateFooter();
    endSubPage();
    return fileName;
}

QString HtmlGenerator::generateLowStatusMemberFile(const InnerNode *inner, CodeMarker *marker,
                                                   CodeMarker::Status status)
{
    QList<Section> sections = marker->sections(inner, CodeMarker::Summary, status);
    QMutableListIterator<Section> j(sections);
    while (j.hasNext()) {
        if (j.next().members.size() == 0)
            j.remove();
    }
    if (sections.isEmpty())
        return QString();

    int i;

    QString title;
    QString fileName;

    if (status == CodeMarker::Compat) {
        title = "Qt 3 Support Members for " + inner->name();
        fileName = fileBase(inner) + "-qt3." + fileExtension();
    } else {
        title = "Obsolete Members for " + inner->name();
        fileName = fileBase(inner) + "-obsolete." + fileExtension();
    }

    beginSubPage(inner->location(), fileName);
    generateHeader(title, inner);
    generateTitle(title);

    if (status == CodeMarker::Compat) {
        out() << "<p><b>The following class members are part of the Qt 3 support layer.</b> "
                 "They are provided to help you port old code to Qt 4. We advise against "
                 "using them in new code.</p>\n";
    } else {
        out() << "<p><b>The following class members are obsolete.</b> They are provided to keep "
                 "old source code working. We strongly advise against using them in new "
                 "code.</p>\n";
    }

    out() << "<p><ul><li><a href=\"" << linkForNode(inner, 0) << "\">" << protect(inner->name())
          << " class reference</a></li></ul></p>\n";

    for (i = 0; i < sections.size(); ++i) {
	out() << "<h3>" << protect(sections.at(i).name) << "</h3>\n";

	generateSectionList(sections.at(i), inner, marker, CodeMarker::Summary);
    }

    sections = marker->sections(inner, CodeMarker::Detailed, status);
    for (i = 0; i < sections.size(); ++i) {
	out() << "<hr />\n";
	out() << "<h2>" << protect( sections.at(i).name ) << "</h2>\n";

	NodeList::ConstIterator m = sections.at(i).members.begin();
	while ( m != sections.at(i).members.end() ) {
	    if ( (*m)->access() != Node::Private )
		generateDetailedMember(*m, inner, marker);
	    ++m;
	}
    }

    generateFooter();
    endSubPage();
    return fileName;
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
            out() << "</li>\n";
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
}

void HtmlGenerator::generateAnnotatedList(const Node *relative, CodeMarker *marker,
				          const QMap<QString, const Node *> &nodeMap)
{
    out() << "<p><table width=\"100%\">\n";
    QMap<QString, const Node *>::ConstIterator n = nodeMap.begin();
    while (n != nodeMap.end()) {
	out() << "<tr valign=\"top\" bgcolor=\"#f0f0f0\">";
	out() << "<td><b>";
        generateFullName(*n, relative, marker);
	out() << "</b></td>";
        Text brief = (*n)->doc().trimmedBriefText((*n)->name());
        if (!brief.isEmpty()) {
	    out() << "<td>";
            generateText(brief, *n, marker);
	    out() << "</td>";
	}
	out() << "</tr>\n";
	++n;
    }
    out() << "</table></p>\n";
}

void HtmlGenerator::generateCompactList(const Node *relative, CodeMarker *marker,
					const QMap<QString, const Node *> &classMap)
{
    const int NumParagraphs = 37; // '0' to '9', 'A' to 'Z', '_'
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

    if (classMap.size() > 1) {
        while (commonPrefixLen < first.length() + 1 && commonPrefixLen < last.length() + 1
	       && first[commonPrefixLen] == last[commonPrefixLen])
	    ++commonPrefixLen;
    }

    /*
      Divide the data into 37 paragraphs: 0, ..., 9, A, ..., Z,
      underscore (_). QAccel will fall in paragraph 10 (A) and
      QXtWidget in paragraph 33 (X). This is the only place where we
      assume that NumParagraphs is 37. Each paragraph is a
      QMap<QString, const Node *>.
    */
    QMap<QString, const Node *> paragraph[NumParagraphs];
    QString paragraphName[NumParagraphs];

    QMap<QString, const Node *>::ConstIterator c = classMap.begin();
    while (c != classMap.end()) {
	QString key = c.key().mid( commonPrefixLen ).toLower();
	int paragraphNo = NumParagraphs - 1;

	if (key[0].digitValue() != -1) {
            paragraphNo = key[0].digitValue();
        } else if (key[0] >= QLatin1Char('a') && key[0] <= QLatin1Char('z')) {
	    paragraphNo = 10 + key[0].unicode() - 'a';
        }

        paragraphName[paragraphNo] = key[0].toUpper();
	paragraph[paragraphNo].insert(key, c.value());
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
	firstOffset[i] = qMin(i * numRows, classMap.size());
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
		out() << "<td>\n</td>\n";
	    } else {
		while (currentOffsetInParagraph[i] == paragraph[currentParagraphNo[i]].count()) {
		    ++currentParagraphNo[i];
		    currentOffsetInParagraph[i] = 0;
		}

		out() << "<td align=\"right\">";
		if ( currentOffsetInParagraph[i] == 0 ) {
		    // start a new paragraph
		    out() << "<b>" << paragraphName[currentParagraphNo[i]] << "&nbsp;</b>";
		}
		out() << "</td>\n";

		// bad loop
		QMap<QString, const Node *>::Iterator it;
		it = paragraph[currentParagraphNo[i]].begin();
		for ( j = 0; j < currentOffsetInParagraph[i]; j++ )
		    ++it;

		out() << "<td>";
                generateFullName(it.value(), relative, marker);
		out() << "</td>\n";

		currentOffset[i]++;
		currentOffsetInParagraph[i]++;
	    }
	}
        out() << "</tr>\n";
    }
    out() << "</table></p>\n";
}

void HtmlGenerator::generateFunctionIndex(const Node *relative, CodeMarker *marker)
{
    out() << "<p><center><font size=\"+1\"><b>";
    for ( int i = 0; i < 26; i++ ) {
	QChar ch( 'a' + i );
	out() << QString("<a href=\"#%1\">%2</a>&nbsp;").arg(ch).arg(ch.toUpper());
    }
    out() << "</b></font></center></p>\n";

    char nextLetter = 'a';
    char currentLetter;

#if 0
    out() << "<ul>\n";
#endif
    QMap<QString, QMap<QString, const Node *> >::ConstIterator f = funcIndex.begin();
    while (f != funcIndex.end()) {
#if 0
	out() << "<li>";
#else
        out() << "<p>";
#endif
	out() << protect(f.key()) << ":";

	currentLetter = f.key()[0].unicode();
	while (islower(currentLetter) && currentLetter >= nextLetter) {
	    out() << QString("<a name=\"%1\"></a>").arg(nextLetter);
	    nextLetter++;
	}

	QMap<QString, const Node *>::ConstIterator s = (*f).begin();
	while ( s != (*f).end() ) {
	    out() << " ";
	    generateFullName((*s)->parent(), relative, marker, *s);
	    ++s;
	}
#if 0
	out() << "</li>";
#else
        out() << "</p>";
#endif
        out() << "\n";
	++f;
    }
#if 0
    out() << "</ul>\n";
#endif
}

void HtmlGenerator::generateLegaleseList(const Node *relative, CodeMarker *marker)
{
    QMap<Text, const Node *>::ConstIterator it = legaleseTexts.begin();
    while (it != legaleseTexts.end()) {
	Text text = it.key();
	out() << "<hr />\n";
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

void HtmlGenerator::generateSynopsis(const Node *node, const Node *relative,
				     CodeMarker *marker, CodeMarker::SynopsisStyle style)
{
    QString marked = marker->markedUpSynopsis( node, relative, style );
    QRegExp templateTag("(<[^@>]*>)");
    if (marked.indexOf(templateTag) != -1) {
        QString contents = protect(marked.mid(templateTag.pos(1),
                                              templateTag.cap(1).length()));
        marked.replace( templateTag.pos(1), templateTag.cap(1).length(),
                        contents );
    }
    marked.replace(QRegExp("<@param>([a-z]+)_([1-9n])</@param>"), "<i>\\1<sub>\\2</sub></i>");
    marked.replace("<@param>", "<i>");
    marked.replace("</@param>", "</i>");

    if (style == CodeMarker::Summary)
        marked.replace("@name>", "b>");

    if (style == CodeMarker::SeparateList) {
	QRegExp extraRegExp("<@extra>.*</@extra>");
        extraRegExp.setMinimal(true);
	marked.replace(extraRegExp, "");
    } else {
        marked.replace( "<@extra>", "&nbsp;&nbsp;<tt>" );
        marked.replace( "</@extra>", "</tt>" );
    }

    if (style != CodeMarker::Detailed) {
        marked.replace("<@type>", "");
        marked.replace("</@type>", "");
    }
    out() << highlightedCode( marked, marker, relative );
}

void HtmlGenerator::generateOverviewList(const Node *relative, CodeMarker * /* marker */)
{
    QMap<QString, FakeNode *> fakeNodeMap;
    QRegExp singleDigit("\\b([0-9])\\b");

    const NodeList children = tre->root()->childNodes();
    foreach (Node *child, children) {
        if (child->type() == Node::Fake && child != relative) {
            FakeNode *fakeNode = static_cast<FakeNode *>(child);

            // there are too many examples; they would clutter the list
            if (fakeNode->subType() == FakeNode::Example)
                continue;

            // not interested either in individial (Qt Designer etc.) manual chapters
            if (fakeNode->links().contains(Node::PreviousLink))
                continue;

            QString sortKey = fakeNode->fullTitle().toLower();
            if (sortKey.startsWith("the "))
                sortKey.remove(0, 4);
            sortKey.replace(singleDigit, "0\\1");
            fakeNodeMap.insert(sortKey, fakeNode);
        }
    }

    if (!fakeNodeMap.isEmpty()) {
        out() << "<ul>\n";
        foreach (FakeNode *fakeNode, fakeNodeMap) {
            QString title = fakeNode->fullTitle();
            if (title.startsWith("The "))
                title.remove(0, 4);
            out() << "<li><a href=\"" << linkForNode(fakeNode, relative) << "\">"
                  << protect(title) << "</li>\n";
        }
        out() << "</ul>\n";
    }
}

void HtmlGenerator::generateSectionList(const Section& section, const Node *relative,
					CodeMarker *marker, CodeMarker::SynopsisStyle style)
{
    if ( !section.members.isEmpty() ) {
	bool twoColumn = false;
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
            if (style == CodeMarker::Accessors)
                out() << "<b>";
	    generateSynopsis( *m, relative, marker, style );
            if (style == CodeMarker::Accessors)
                out() << "</b>";
	    out() << "</li>\n";
	    i++;
	    ++m;
	}
	out() << "</ul>\n";
	if ( twoColumn )
	    out() << "</td></tr>\n</table>\n";
    }

    if (style == CodeMarker::Summary && !section.inherited.isEmpty()) {
	out() << "<ul>\n";
        generateSectionInheritedList(section, relative, marker);
	out() << "</ul>\n";
    }
}

void HtmlGenerator::generateSectionInheritedList(const Section& section, const Node *relative,
					         CodeMarker *marker)
{
    QList<QPair<ClassNode *, int> >::ConstIterator p = section.inherited.begin();
    while ( p != section.inherited.end() ) {
	out() << "<li><div class=\"fn\"/>";
	out() << (*p).second << " ";
	if ( (*p).second == 1 ) {
	    out() << section.singularMember;
	} else {
	    out() << section.pluralMember;
	}
	out() << " inherited from <a href=\"" << fileName((*p).first)
	      << "#" << cleanRef(section.name.toLower()) << "\">"
	      << protect(marker->plainFullName((*p).first, relative))
	      << "</a></li>\n";
	++p;
    }
}

void HtmlGenerator::generateLink(const Atom *atom, const Node * /* relative */, CodeMarker * /* marker */)
{
    if (funcLeftParen.indexIn(atom->string()) != -1) {
	int k = funcLeftParen.pos( 1 );
	out() << protect( atom->string().left(k) );
	if ( link.isEmpty() ) {
            if (showBrokenLinks)
                out() << "</i>";
	} else {
	    out() << "</a>";
	}
        inLink = false;
	out() << protect(atom->string().mid(k));
    } else {
	out() << protect(atom->string());
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
	clean += "dtor.";
    } else if ( ref[0] == '_' ) {
	clean += "underscore.";
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
    if (string.isEmpty())
        return string;

    QString html = string;
    html.replace( "&", "&amp;" );
    html.replace( "<", "&lt;" );
    html.replace( ">", "&gt;" );
    html.replace( "\"", "&quot;" );
    return html;
}

QString HtmlGenerator::protectPreformatted(const QString &string)
{
    if (string.isEmpty())
        return string;

    QString html = protect(string);
    if (html.at(0) == QLatin1Char(' '))
        html.replace(0, 1, "&nbsp;");
    return html;
}

QString HtmlGenerator::highlightedCode(const QString& markedCode, CodeMarker *marker,
                                       const Node *relative)
{
    QRegExp linkTag("(<@link node=\"([^\"]+)\">).*(</@link>)");
    linkTag.setMinimal(true);

    QString html = markedCode;

    int k = 0;
    while ((k = html.indexOf(linkTag, k)) != -1) {
	QString begin;
	QString end;
	QString link = linkForNode(CodeMarker::nodeForString(linkTag.cap(2)), relative);

	if (!link.isEmpty()) {
	    begin = "<a href=\"" + link + "\">";
	    end = "</a>";
	}

	html.replace( linkTag.pos(3), linkTag.cap(3).length(), end );
	html.replace( linkTag.pos(1), linkTag.cap(1).length(), begin );
	++k;
    }

    QRegExp typeTag("(<@type>)(.*)(</@type>)");
    typeTag.setMinimal(true);

    k = 0;
    while ((k = html.indexOf(typeTag, k)) != -1) {
	QString begin;
	QString end;
        QString link = linkForNode(marker->resolveTarget(typeTag.cap(2), tre, relative), relative);

        if (!link.isEmpty()) {
	    begin = "<a href=\"" + link + "\">";
	    end = "</a>";
	}

	html.replace( typeTag.pos(3), typeTag.cap(3).length(), end );
	html.replace( typeTag.pos(1), typeTag.cap(1).length(), begin );
	++k;
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
#endif
    html.replace( QRegExp("</?@[^>]*>"), "" );
    return html;
}

QString HtmlGenerator::fileBase(const Node *node)
{
    QString result = PageGenerator::fileBase(node);
    if (!node->isInnerNode()) {
        switch (node->status()) {
        case Node::Compat:
            result += "-qt3";
            break;
        case Node::Obsolete:
            result += "-obsolete";
            break;
        default:
            ;
        }
    }
    return result;
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
    const TypedefNode *typedeffe;
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
        typedeffe = static_cast<const TypedefNode *>(node);
        if (typedeffe->associatedEnum()) {
            return refForNode(typedeffe->associatedEnum());
        } else {
            ref = node->name() + "-typedef";
        }
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
        break;
    case Node::Variable:
        ref = node->name() + "-var";
	break;
    case Node::Target:
        return protect(node->name());
    }
    return registerRef( ref );
}

QString HtmlGenerator::linkForNode(const Node *node, const Node *relative)
{
    QString link;
    QString fn;
    QString ref;

    if (node == 0 || node == relative || fileBase(node).isEmpty())
	return QString();

    fn = fileName(node);
#if 0
    // ### reintroduce this test, without breaking .dcf files
    if (fn != outFileName())
#endif
        link += fn;

    if (!node->isInnerNode()) {
        ref = refForNode(node);
        if (relative && fn == fileName(relative) && ref == refForNode(relative))
            return QString();

	link += "#";
        link += ref;
    }
    return link;
}

QString HtmlGenerator::refForAtom(Atom *atom, const Node * /* node */)
{
    if (atom->type() == Atom::SectionLeft) {
        return Doc::canonicalTitle(Text::sectionHeading(atom).toString());
    } else if (atom->type() == Atom::Target) {
        return Doc::canonicalTitle(atom->string());
    } else {
        return QString();
    }
}

void HtmlGenerator::generateFullName(const Node *apparentNode, const Node *relative,
				     CodeMarker *marker, const Node *actualNode)
{
    if ( actualNode == 0 )
	actualNode = apparentNode;
    out() << "<a href=\"" << linkForNode(actualNode, relative) << "\">";
    if (apparentNode->type() == Node::Fake)
	out() << protect(static_cast<const FakeNode *>(apparentNode)->title());
    else
	out() << protect(marker->plainFullName(apparentNode, relative));
    out() << "</a>";
}

void HtmlGenerator::generateDetailedMember(const Node *node, const InnerNode *relative,
					   CodeMarker *marker)
{
    const EnumNode *enume;

    if (node->type() == Node::Enum
            && (enume = static_cast<const EnumNode *>(node))->flagsType()) {
        out() << "<h3 class=\"flags\">";
        out() << "<a name=\"" + refForNode( node ) + "\"></a>";
        generateSynopsis(enume, relative, marker, CodeMarker::Detailed);
        out() << "<br />";
        generateSynopsis(enume->flagsType(), relative, marker, CodeMarker::Detailed);
        out() << "</h3>\n";
    } else {
        out() << "<h3 class=\"fn\">";
        out() << "<a name=\"" + refForNode( node ) + "\"></a>";
        generateSynopsis( node, relative, marker, CodeMarker::Detailed );
        out() << "</h3>\n";
    }

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
	    out() << "<p>Access functions:</p>\n";
	    generateSectionList(section, node, marker, CodeMarker::Accessors);
	}
    } else if (node->type() == Node::Enum) {
        const EnumNode *enume = static_cast<const EnumNode *>(node);
        if (enume->flagsType()) {
            out() << "<p>The <tt>" << protect(enume->flagsType()->name())
                  << "</tt> type stores an OR combination of <tt>" << protect(enume->name())
                  << "</tt> values.</p>\n";
        }
    }
    generateAlsoList( node, marker );
}

void HtmlGenerator::findAllClasses(const InnerNode *node)
{
    NodeList::const_iterator c = node->childNodes().constBegin();
    while (c != node->childNodes().constEnd()) {
	if ((*c)->access() != Node::Private) {
	    if ((*c)->type() == Node::Class && !(*c)->doc().isEmpty()) {
                if ((*c)->status() == Node::Compat) {
                    compatClasses.insert((*c)->name(), *c);
                } else {
	            nonCompatClasses.insert((*c)->name(), *c);
                    if ((*c)->status() == Node::Main)
		        mainClasses.insert((*c)->name(), *c);
                }
                QString moduleName = (*c)->moduleName();
                if (!moduleName.isEmpty())
                    moduleClassMap[moduleName].insert((*c)->name(), *c);
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
                if (func->status() > Node::Obsolete && func->metaness() != FunctionNode::Ctor
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

bool HtmlGenerator::isThreeColumnEnumValueTable(const Atom *atom)
{
    while (atom != 0 && !(atom->type() == Atom::ListRight && atom->string() == ATOM_LIST_VALUE)) {
        if (atom->type() == Atom::ListItemLeft && !matchAhead(atom, Atom::ListItemRight))
            return true;
        atom = atom->next();
    }
    return false;
}

#include <qdebug.h>

const Node *HtmlGenerator::findNodeForTarget(const QString &target,
    const Node *relative, CodeMarker *marker, const Atom *atom)
{
    const Node *node = 0;
    

    if (target.isEmpty()) {
        node = relative;
    } else if (target.endsWith(".html")) {
        node = tre->root()->findNode(target, Node::Fake);
    } else if (marker) {
        node = marker->resolveTarget(target, tre, relative);
        if (!node)
            node = tre->findFakeNodeByTitle(target);
        if (!node && atom) {
            node = tre->findUnambiguousTarget(target,
                *const_cast<Atom**>(&atom));
        }
    }

    if (!node)
        relative->doc().location().warning(tr("Cannot link to '%1'").arg(target));

    return node;
}

const QPair<QString,QString> HtmlGenerator::anchorForNode(const Node *node)
{
    QPair<QString,QString> anchorPair;
    const FakeNode *fakeNode = static_cast<const FakeNode*>(node);

    anchorPair.first = PageGenerator::fileName(node);
    if (fakeNode)
        anchorPair.second = fakeNode->title();

    return anchorPair;
}

QString HtmlGenerator::getLink(const Atom *atom, const Node *relative, CodeMarker *marker)
{
    QString link;
    if (atom->string().contains(":") &&
	    (atom->string().startsWith("file:")
	     || atom->string().startsWith("http:")
	     || atom->string().startsWith("https:")
             || atom->string().startsWith("ftp:")
	     || atom->string().startsWith("mailto:"))) {
        link = atom->string();
    } else if (atom->string().count('@') == 1) {
        link = "mailto:" + atom->string();
    } else {
        QStringList path;
        if (atom->string().contains('#')) {
            path = atom->string().split('#');
        } else {
            path.append(atom->string());
        }

        const Node *node = 0;
        Atom *targetAtom = 0;

        QString first = path.first().trimmed();
        if (first.isEmpty()) {
            node = relative;
        } else if (first.endsWith(".html")) {
            node = tre->root()->findNode(first, Node::Fake);
        } else {
            node = marker->resolveTarget(first, tre, relative);
            if (!node)
                node = tre->findFakeNodeByTitle(first);
            if (!node)
                node = tre->findUnambiguousTarget(first, targetAtom);
        }

        if (node) {
            path.removeFirst();
        } else {
            node = relative;
        }

        while (!path.isEmpty()) {
            targetAtom = tre->findTarget(path.first(), node);
            if (targetAtom == 0)
                break;
            path.removeFirst();
        }

        if (path.isEmpty()) {
            link = linkForNode(node, relative);
            if (targetAtom)
                link += "#" + refForAtom(targetAtom, node);
        }
    }
    return link;
}

void HtmlGenerator::generateDcf(const QString &fileBase, const QString &startPage,
                                const QString &title, DcfSection &dcfRoot)
{
    dcfRoot.ref = startPage;
    dcfRoot.title = title;
    generateDcfSections(dcfRoot, outputDir() + "/" + fileBase + ".dcf", fileBase + "/reference");
}
