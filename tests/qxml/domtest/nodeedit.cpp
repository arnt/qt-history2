#include <qlayout.h>
#include <qhbox.h>
#include <qgrid.h>
#include <qpushbutton.h>
#include <qlabel.h>

#include "nodeedit.h"

NodeEdit::NodeEdit( QDomNode _node )
    : QDialog( 0, 0, TRUE )
{
    node = _node;

    QBoxLayout * layout = new QVBoxLayout( this );
    layout->setAutoAdd( TRUE );

    QHBox *hb;
    QPushButton *pb;

    QGrid *grid = new QGrid( 2, this );

    new QLabel( "NodeValue: ", grid );
    edits.insert( "NodeValue", new QLineEdit( node.nodeValue(), grid ) );
    new QLabel( "Prefix: ", grid );
    edits.insert( "Prefix", new QLineEdit( node.prefix(), grid ) );
    switch ( node.nodeType() ) {
	case QDomNode::ElementNode:
	    new QLabel( "TagName: ", grid );
	    edits.insert( "TagName", new QLineEdit( node.toElement().tagName(), grid ) );
	    break;
	case QDomNode::AttributeNode:
	    new QLabel( "Value: ", grid );
	    edits.insert( "Value", new QLineEdit( node.toAttr().value(), grid ) );
	    break;
	case QDomNode::TextNode:
	    new QLabel( "Data: ", grid );
	    edits.insert( "Data", new QLineEdit( node.toText().data(), grid ) );
	    break;
	case QDomNode::CDATASectionNode:
	    new QLabel( "Data: ", grid );
	    edits.insert( "Data", new QLineEdit( node.toCDATASection().data(), grid ) );
	    break;
	case QDomNode::ProcessingInstructionNode:
	    new QLabel( "Data: ", grid );
	    edits.insert( "Data", new QLineEdit( node.toProcessingInstruction().data(), grid ) );
	    break;
	case QDomNode::CommentNode:
	    new QLabel( "Data: ", grid );
	    edits.insert( "Data", new QLineEdit( node.toComment().data(), grid ) );
	    break;
	default:
	    break;
    }

    hb = new QHBox( this );
    pb = new QPushButton( "Ok", hb );
    connect( pb, SIGNAL(clicked()),
	    this, SLOT(ok()) );
    pb = new QPushButton( "Cancel", hb );
    connect( pb, SIGNAL(clicked()),
	    this, SLOT(cancel()) );
}

void NodeEdit::ok()
{
    node.setNodeValue( edits["NodeValue"]->text() );
    node.setPrefix( edits["Prefix"]->text() );
    switch ( node.nodeType() ) {
	case QDomNode::ElementNode:
	    node.toElement().setTagName( edits["TagName"]->text() );
	    break;
	case QDomNode::AttributeNode:
	    node.toAttr().setValue( edits["Value"]->text() );
	    break;
	case QDomNode::TextNode:
	    node.toText().setData( edits["Data"]->text() );
	    break;
	case QDomNode::CDATASectionNode:
	    node.toCDATASection().setData( edits["Data"]->text() );
	    break;
	case QDomNode::ProcessingInstructionNode:
	    node.toProcessingInstruction().setData( edits["Data"]->text() );
	    break;
	case QDomNode::CommentNode:
	    node.toComment().setData( edits["Data"]->text() );
	    break;
	default:
	    break;
    }
    done( 1 );
}

void NodeEdit::cancel()
{
    done( 0 );
}
