#include "playparser.h"

PlayParser::PlayParser( QListView *lv, QString *err )
{
    playTree = lv;
    errorProtocol = err;
}


PlayParser::~PlayParser()
{
}


bool PlayParser::startDocument()
{
    playTreeItem = 0;
    playTreeItemAfter = 0;

    treeText = "";
    itemText = "";
    rootElement = TRUE;
    level = 0;
    act = 1;
    scene = 1;

    return TRUE;
}


bool PlayParser::endDocument()
{
    add();
    return TRUE;
}


bool PlayParser::startElement( const QString&, const QString&, const QString& qName, const QXmlAttributes& )
{
    if ( rootElement ) {
	// root element
	rootElement = FALSE;
	if ( qName != "PLAY" ) {
	    return FALSE;
	}
    }

    if ( qName == "PLAY" ) {
	add();
	treeText = "Title Page";
	add();
    } else if ( qName == "TITLE" ) {
	itemText += "<center><h1>";
    } else if ( qName == "FM" ) {
	itemText += "<br/><br/><br/><center>";
    } else if ( qName == "P" ) {
	itemText += "<p>";
    } else if ( qName == "PERSONAE" ) {
	add();
	treeText = "Personae";
	itemText += "<ul>";
	add();
    } else if ( qName == "PGROUP" ) {
	itemText += "<ul>";
    } else if ( qName == "PERSONA" ) {
	itemText += "<li>";
    } else if ( qName == "GRPDESCR" ) {
	// ???
    } else if ( qName == "SCNDESCR" ) {
	itemText += "<p>";
    } else if ( qName == "PLAYSUBT" ) {
	itemText += "<p>";
    } else if ( qName == "INDUCT" ) {
	add();
	treeText = "Introduction";
	add();
    } else if ( qName == "ACT" ) {
	add();
	treeText = QString( "Act %1" ).arg( act );
	act++;
	scene = 1;
	add();
    } else if ( qName == "SCENE" ) {
	add();
	treeText = QString( "Scene %1" ).arg( scene );
	scene++;
	add( TRUE );
    } else if ( qName == "PROLOGUE" ) {
	add();
	treeText = "Prologue";
	add();
    } else if ( qName == "EPILOGUE" ) {
	add();
	treeText = "Epilogue";
	add();
    } else if ( qName == "SPEECH" ) {
	itemText += "<p>";
    } else if ( qName == "SPEAKER" ) {
	itemText += "<b>";
    } else if ( qName == "LINE" ) {
    } else if ( qName == "STAGEDIR" ) {
	itemText += "<p><i>";
    } else if ( qName == "SUBTITLE" ) {
    } else if ( qName == "SUBHEAD" ) {
    }

    return TRUE;
}


bool PlayParser::endElement( const QString&, const QString&, const QString& qName )
{
    if ( qName == "PLAY" ) {
    } else if ( qName == "TITLE" ) {
	itemText += "</h1></center>";
    } else if ( qName == "FM" ) {
	itemText += "</center>";
    } else if ( qName == "P" ) {
	itemText += "</p>";
    } else if ( qName == "PERSONAE" ) {
	itemText += "</ul>";
    } else if ( qName == "PGROUP" ) {
	itemText += "</ul>";
    } else if ( qName == "PERSONA" ) {
	itemText += "</li>";
    } else if ( qName == "GRPDESCR" ) {
	// ???
    } else if ( qName == "SCNDESCR" ) {
	itemText += "</p>";
    } else if ( qName == "PLAYSUBT" ) {
	itemText += "</p>";
    } else if ( qName == "INDUCT" ) {
    } else if ( qName == "ACT" ) {
    } else if ( qName == "SCENE" ) {
	add( TRUE );
    } else if ( qName == "PROLOGUE" ) {
    } else if ( qName == "EPILOGUE" ) {
    } else if ( qName == "SPEECH" ) {
	itemText += "</p>";
    } else if ( qName == "SPEAKER" ) {
	itemText += "</b><br/>";
    } else if ( qName == "LINE" ) {
	itemText += "<br/>";
    } else if ( qName == "STAGEDIR" ) {
	itemText += "</i></p>";
    } else if ( qName == "SUBTITLE" ) {
    } else if ( qName == "SUBHEAD" ) {
    }

    return TRUE;
}


bool PlayParser::characters( const QString& ch )
{
    itemText += ch;

    return TRUE;
}


bool PlayParser::fatalError( const QXmlParseException& exception )
{
    QString errorString = QString( "fatal parsing error: %1 in line %2, column %3\n" )
	.arg( exception.message() )
	.arg( exception.lineNumber() )
	.arg( exception.columnNumber() );
    *errorProtocol += errorString;

    return QXmlDefaultHandler::fatalError( exception );
}


void PlayParser::add( bool sub )
{
    if ( treeText.isEmpty() ) {
	if ( !itemText.isEmpty() ) {
	    playTreeItemAfter->setText( 1,
		    playTreeItemAfter->text( 1 ) + itemText );
	    itemText = "";
	}
	return;
    }

    QListViewItem* tmp;
    if ( sub ) {
	if ( level == 0 ) {
	    // go down
	    level++;
	    QListViewItem* tmp;
	    if ( playTreeItem == 0 )
		return;
	    tmp = new QListViewItem( playTreeItem, 0,
		    treeText, itemText );
	    playTreeItemAfter = tmp;
	} else {
	    // keep same level
	    tmp = new QListViewItem( playTreeItem, playTreeItemAfter,
		    treeText, itemText );
	    playTreeItemAfter = tmp;
	}
    } else {
	if ( level == 1 ) {
	    // go up
	    level--;
	    playTreeItemAfter = playTreeItemAfter->parent();
	}
	tmp = new QListViewItem( playTree, playTreeItemAfter,
		treeText, itemText );
	playTreeItem = tmp;
	playTreeItemAfter = tmp;
    }

    treeText = "";
    itemText = "";
}
