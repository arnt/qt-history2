#include "codemarker.h"
#include "config.h"
#include "node.h"

#include <stdio.h>

QString CodeMarker::defaultLang;
QList<CodeMarker *> CodeMarker::markers;

CodeMarker::CodeMarker()
    : amp( "&" ), lt( "<" ), gt( ">" ), quot( "\"" )
{
    markers.prepend( this );
}

CodeMarker::~CodeMarker()
{
    markers.removeAll( this );
}

void CodeMarker::initializeMarker( const Config& /* config */ )
{
}

void CodeMarker::terminateMarker()
{
}

void CodeMarker::initialize( const Config& config )
{
    defaultLang = config.getString( CONFIG_LANGUAGE );

    QList<CodeMarker *>::ConstIterator m = markers.begin();
    while ( m != markers.end() ) {
	(*m)->initializeMarker( config );
	++m;
    }
}

void CodeMarker::terminate()
{
    QList<CodeMarker *>::ConstIterator m = markers.begin();
    while ( m != markers.end() ) {
	(*m)->terminateMarker();
	++m;
    }
}

CodeMarker *CodeMarker::markerForCode( const QString& code )
{
    CodeMarker *defaultMarker = markerForLanguage( defaultLang );
    if ( defaultMarker != 0 && defaultMarker->recognizeCode(code) )
	return defaultMarker;

    QList<CodeMarker *>::ConstIterator m = markers.begin();
    while ( m != markers.end() ) {
	if ( (*m)->recognizeCode(code) )
	    return *m;
	++m;
    }
    return defaultMarker;
}

CodeMarker *CodeMarker::markerForFileName( const QString& fileName )
{
    CodeMarker *defaultMarker = markerForLanguage( defaultLang );
    int dot = -1;
    while ((dot = fileName.indexOf(".", dot + 1)) != -1) {
	QString ext = fileName.mid( dot + 1 );
	if ( defaultMarker != 0 && defaultMarker->recognizeExtension(ext) )
	    return defaultMarker;
	QList<CodeMarker *>::ConstIterator m = markers.begin();
	while ( m != markers.end() ) {
	    if ( (*m)->recognizeExtension(ext) )
		return *m;
	    ++m;
	}
    }
    return defaultMarker;
}

CodeMarker *CodeMarker::markerForLanguage( const QString& lang )
{
    QList<CodeMarker *>::ConstIterator m = markers.begin();
    while ( m != markers.end() ) {
	if ( (*m)->recognizeLanguage(lang) )
	    return *m;
	++m;
    }
    return 0;
}

const Node *CodeMarker::nodeForString( const QString& string )
{
    void *ptr = 0;
    sscanf( string.toLatin1().data(), "%p", &ptr );
    return (const Node *) ptr;
}

QString CodeMarker::stringForNode( const Node *node )
{
    QString str;
    str.sprintf( "%p", (void *) node );
    return str;
}

QString CodeMarker::protect( const QString& string )
{
    QString marked = string;
    marked.replace( amp, "&amp;" );
    marked.replace( lt, "&lt;" );
    marked.replace( gt, "&gt;" );
    marked.replace( quot, "&quot;" );
    return marked;
}

QString CodeMarker::typified(const QString &string)
{
    QString result;
    QString pendingWord;

    for (int i = 0; i <= string.size(); ++i) {
        QChar ch;
        if (i != string.size())
            ch = string.at(i);

        QChar lower = ch.toLower();
        if ((lower >= 'a' && lower <= 'z') || ch.digitValue() >= 0 || ch == QLatin1Char('_')
                || ch == QLatin1Char(':')) {
            pendingWord += ch;
        } else {
            if (!pendingWord.isEmpty()) {
                bool isProbablyType = (pendingWord != "const");
                if (isProbablyType)
                    result += "<@type>";
                result += pendingWord;
                if (isProbablyType)
                    result += "</@type>";
            }
            pendingWord.clear();

            switch (ch.unicode()) {
            case '\0':
                break;
            case '&':
                result += "&amp;";
                break;
            case '<':
                result += "&lt;";
                break;
            case '>':
                result += "&gt;";
                break;
            default:
                result += ch;
            }
        }
    }
    return result;
}

QString CodeMarker::taggedNode( const Node *node )
{
    QString tag;

    switch ( node->type() ) {
    case Node::Namespace:
	tag = "@namespace";
	break;
    case Node::Class:
	tag = "@class";
	break;
    case Node::Enum:
	tag = "@enum";
	break;
    case Node::Typedef:
	tag = "@typedef";
	break;
    case Node::Function:
	tag = "@function";
	break;
    case Node::Property:
	tag = "@property";
	break;
    default:
	tag = "@unknown";
    }
    return "<" + tag + ">" + protect( node->name() ) + "</" + tag + ">";
}

QString CodeMarker::linkTag( const Node *node, const QString& body )
{
    return "<@link node=\"" + stringForNode( node ) + "\">" + body + "</@link>";
}

QString CodeMarker::sortName( const Node *node )
{
    QString nodeName = node->name();
    int numDigits = 0;
    for (int i = nodeName.size() - 1; i > 0; --i) {
        if (nodeName.at(i).digitValue() == -1)
            break;
        ++numDigits;
    }

    // we want 'qint8' to appear before 'qint16'
    if (numDigits > 0) {
        for (int i = 0; i < 4 - numDigits; ++i)
            nodeName.insert(nodeName.size() - numDigits - 1, "0");
    }

    if ( node->type() == Node::Function ) {
	const FunctionNode *func = static_cast<const FunctionNode *>(node);
	QString sortNo;
	if ( func->metaness() == FunctionNode::Ctor ) {
	    sortNo = "C";
	} else if ( func->metaness() == FunctionNode::Dtor ) {
	    sortNo = "D";
	} else {
	    if (nodeName.startsWith("operator") && nodeName.length() > 8
		    && !nodeName[8].isLetterOrNumber())
		sortNo = "F";
	    else
		sortNo = "E";
	}
	return sortNo + nodeName + " " + QString::number(func->overloadNumber(), 36);
    } else if (node->type() == Node::Class) {
	return "A" + nodeName;
    } else if (node->type() == Node::Property || node->type() == Node::Variable) {
        return "E" + nodeName;
    } else {
	return "B" + nodeName;
    }
}

void CodeMarker::insert(FastSection &fastSection, Node *node, SynopsisStyle style, Status status)
{
    bool inheritedMember = (!node->relates() &&
			    (node->parent() != (const InnerNode *)fastSection.innerNode));
    bool irrelevant = false;

    if (node->access() == Node::Private) {
	irrelevant = true;
    } else if ( node->type() == Node::Function ) {
	FunctionNode *func = (FunctionNode *) node;
	irrelevant = (inheritedMember
		      && (func->metaness() == FunctionNode::Ctor ||
			  func->metaness() == FunctionNode::Dtor));
    } else if ( node->type() == Node::Class || node->type() == Node::Enum
		    || node->type() == Node::Typedef ) {
	irrelevant = ( inheritedMember && style != SeparateList );
        if (!irrelevant && style == Detailed && node->type() == Node::Typedef) {
            const TypedefNode *typedeffe = static_cast<const TypedefNode *>(node);
            if (typedeffe->associatedEnum())
                irrelevant = true;
        }
    }

    if (!irrelevant) {
        if (status == Compat) {
            irrelevant = (node->status() != Node::Compat);
	} else if (status == Obsolete) {
            irrelevant = (node->status() != Node::Obsolete);
	} else {
            irrelevant = (node->status() == Node::Compat || node->status() == Node::Obsolete);
	}
    }

    if ( !irrelevant ) {
	if ( !inheritedMember || style == SeparateList ) {
	    QString key = sortName(node);
            if (!fastSection.memberMap.contains(key))
		fastSection.memberMap.insert(key, node);
	} else {
	    if ( node->parent()->type() == Node::Class ) {
		if (fastSection.inherited.isEmpty()
			|| fastSection.inherited.last().first != node->parent()) {
		    QPair<ClassNode *, int> p((ClassNode *)node->parent(), 0);
		    fastSection.inherited.append(p);
		}
		fastSection.inherited.last().second++;
	    }
	}
    }
}

void CodeMarker::append(QList<Section>& sectionList, const FastSection& fastSection)
{
    if ( !fastSection.memberMap.isEmpty() ||
	 !fastSection.inherited.isEmpty() ) {
	Section section(fastSection.name, fastSection.singularMember, fastSection.pluralMember);
	section.members = fastSection.memberMap.values();
	section.inherited = fastSection.inherited;
	sectionList.append( section );
    }
}
