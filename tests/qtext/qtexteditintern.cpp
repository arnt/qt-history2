#include "qtexteditintern_p.h"
#include "qtextedit.h"
#include "qrichtext_p.h"

#include <qstringlist.h>
#include <qfont.h>
#include <qtextstream.h>
#include <qfile.h>
#include <qregexp.h>
#include <qapplication.h>
#include <qclipboard.h>
#include <qmap.h>
#include <qfileinfo.h>
#include <qstylesheet.h>
#include <qmime.h>
#include <qregexp.h>

#include <stdlib.h>

static QMap<QChar, QStringList> *eCompletionMap = 0;

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void QTextEditCommandHistory::addCommand( QTextEditCommand *cmd )
{
    if ( current < (int)history.count() - 1 ) {
	QPtrList<QTextEditCommand> commands;
	commands.setAutoDelete( FALSE );

	for( int i = 0; i <= current; ++i ) {
	    commands.insert( i, history.at( 0 ) );
	    history.take( 0 );
	}

	commands.append( cmd );
	history.clear();
	history = commands;
	history.setAutoDelete( TRUE );
    } else {
	history.append( cmd );
    }

    if ( (int)history.count() > steps )
	history.removeFirst();
    else
	++current;
}

QTextEditCursor *QTextEditCommandHistory::undo( QTextEditCursor *c )
{
    if ( current > -1 ) {
	QTextEditCursor *c2 = history.at( current )->unexecute( c );
	--current;
	return c2;
    }
    return 0;
}

QTextEditCursor *QTextEditCommandHistory::redo( QTextEditCursor *c )
{
    if ( current > -1 ) {
	if ( current < (int)history.count() - 1 ) {
	    ++current;
	    return history.at( current )->execute( c );
	}
    } else {
	if ( history.count() > 0 ) {
	    ++current;
	    return history.at( current )->execute( c );
	}
    }
    return 0;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QTextEditCursor *QTextEditDeleteCommand::execute( QTextEditCursor *c )
{
    QTextEditParag *s = doc->paragAt( id );
    if ( !s ) {
	qWarning( "can't locate parag at %d, last parag: %d", id, doc->lastParag()->paragId() );
	return 0;
    }

    cursor.setParag( s );
    cursor.setIndex( index );
    int len = text.length();
    doc->setSelectionStart( QTextEditDocument::Temp, &cursor );
    for ( int i = 0; i < len; ++i )
	cursor.gotoRight();
    doc->setSelectionEnd( QTextEditDocument::Temp, &cursor );
    doc->removeSelectedText( QTextEditDocument::Temp, &cursor );

    if ( c ) {
	c->setParag( s );
	c->setIndex( index );
    }

    return c;
}

QTextEditCursor *QTextEditDeleteCommand::unexecute( QTextEditCursor *c )
{
    QTextEditParag *s = doc->paragAt( id );
    if ( !s ) {
	qWarning( "can't locate parag at %d, last parag: %d", id, doc->lastParag()->paragId() );
	return 0;
    }

    cursor.setParag( s );
    cursor.setIndex( index );
    cursor.insert( text, TRUE );
    cursor.setParag( s );
    cursor.setIndex( index );
    if ( c ) {
	c->setParag( s );
	c->setIndex( index );
	for ( int i = 0; i < (int)text.length(); ++i )
	    c->gotoRight();
    }

    s = cursor.parag();
    while ( s ) {
	s->format();
	s->setChanged( TRUE );
	if ( s == c->parag() )
	    break;
	s = s->next();
    }

    return &cursor;
}

QTextEditFormatCommand::QTextEditFormatCommand( QTextEditDocument *d, int selId, QTextEditFormat *f, int flgs )
    : QTextEditCommand( d ), selection( selId ),  flags( flgs )
{
    format = d->formatCollection()->format( f );
}

QTextEditFormatCommand::~QTextEditFormatCommand()
{
    format->removeRef();
}

QTextEditCursor *QTextEditFormatCommand::execute( QTextEditCursor *c )
{
    doc->setFormat( selection, format, flags );
    return c;
}

QTextEditCursor *QTextEditFormatCommand::unexecute( QTextEditCursor *c )
{
    return c;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QTextEditCursor::QTextEditCursor( QTextEditDocument *d )
    : doc( d )
{
    idx = 0;
    string = doc->firstParag();
    tmpIndex = -1;
}

void QTextEditCursor::insert( const QString &s, bool checkNewLine )
{
    tmpIndex = -1;
    bool justInsert = TRUE;
    if ( checkNewLine )
	justInsert = s.find( '\n' ) == -1;
    if ( justInsert ) {
	string->insert( idx, s );
	idx += s.length();
    } else {
	QStringList lst = QStringList::split( '\n', s, TRUE );
	QStringList::Iterator it = lst.begin();
	int y = string->rect().y() + string->rect().height();
	for ( ; it != lst.end(); ++it ) {
	    if ( it != lst.begin() ) {
		splitAndInsertEmtyParag( FALSE, FALSE );
		string->setEndState( -1 );
		string->prev()->format( -1, FALSE );
	    }
	    QString s = *it;
	    if ( s.isEmpty() )
		continue;
	    string->insert( idx, s );
	    idx += s.length();
	}
	string->format( -1, FALSE );
	int dy = string->rect().y() + string->rect().height() - y;
	QTextEditParag *p = string->next();
	while ( p ) {
	    p->setParagId( p->prev()->paragId() + 1 );
	    p->move( dy );
	    p->invalidate( 0 );
	    p->setEndState( -1 );
	    p = p->next();
	}
    }
}

void QTextEditCursor::gotoLeft()
{
    tmpIndex = -1;
    if ( idx > 0 ) {
	idx--;
    } else if ( string->prev() ) {
	string = string->prev();
	idx = string->length() - 1;
    }
}

void QTextEditCursor::gotoRight()
{
    tmpIndex = -1;
    if ( idx < string->length() - 1 ) {
	idx++;
    } else if ( string->next() ) {
	string = string->next();
	idx = 0;
    }
}

void QTextEditCursor::gotoUp()
{
    int indexOfLineStart;
    int line;
    QTextEditString::Char *c = string->lineStartOfChar( idx, &indexOfLineStart, &line );
    if ( !c )
	return;

    tmpIndex = QMAX( tmpIndex, idx - indexOfLineStart );
    if ( indexOfLineStart == 0 ) {
	if ( !string->prev() )
	    return;
	string = string->prev();
	int lastLine = string->lines() - 1;
	if ( !string->lineStartOfLine( lastLine, &indexOfLineStart ) )
	    return;
	if ( indexOfLineStart + tmpIndex < string->length() )
	    idx = indexOfLineStart + tmpIndex;
	else
	    idx = string->length() - 1;
    } else {
	--line;
	int oldIndexOfLineStart = indexOfLineStart;
	if ( !string->lineStartOfLine( line, &indexOfLineStart ) )
	    return;
	if ( indexOfLineStart + tmpIndex < oldIndexOfLineStart )
	    idx = indexOfLineStart + tmpIndex;
	else
	    idx = oldIndexOfLineStart - 1;
    }
}

void QTextEditCursor::gotoDown()
{
    int indexOfLineStart;
    int line;
    QTextEditString::Char *c = string->lineStartOfChar( idx, &indexOfLineStart, &line );
    if ( !c )
	return;

    tmpIndex = QMAX( tmpIndex, idx - indexOfLineStart );
    if ( line == string->lines() - 1 ) {
	if ( !string->next() )
	    return;
	string = string->next();
	if ( !string->lineStartOfLine( 0, &indexOfLineStart ) )
	    return;
	int end;
	if ( string->lines() == 1 )
	    end = string->length();
	else
	    string->lineStartOfLine( 1, &end );
	if ( indexOfLineStart + tmpIndex < end )
	    idx = indexOfLineStart + tmpIndex;
	else
	    idx = end - 1;
    } else {
	++line;
	int end;
	if ( line == string->lines() - 1 )
	    end = string->length();
	else
	    string->lineStartOfLine( line + 1, &end );
	if ( !string->lineStartOfLine( line, &indexOfLineStart ) )
	    return;
	if ( indexOfLineStart + tmpIndex < end )
	    idx = indexOfLineStart + tmpIndex;
	else
	    idx = end - 1;
    }
}

void QTextEditCursor::gotoLineEnd()
{
    int indexOfLineStart;
    int line;
    QTextEditString::Char *c = string->lineStartOfChar( idx, &indexOfLineStart, &line );
    if ( !c )
	return;

    if ( line == string->lines() - 1 ) {
	idx = string->length() - 1;
    } else {
	c = string->lineStartOfLine( ++line, &indexOfLineStart );
	indexOfLineStart--;
	idx = indexOfLineStart;
    }
}

void QTextEditCursor::gotoLineStart()
{
    int indexOfLineStart;
    int line;
    QTextEditString::Char *c = string->lineStartOfChar( idx, &indexOfLineStart, &line );
    if ( !c )
	return;

    idx = indexOfLineStart;
}

void QTextEditCursor::gotoHome()
{
    tmpIndex = -1;
    string = doc->firstParag();
    idx = 0;
}

void QTextEditCursor::gotoEnd()
{
    if ( !doc->lastParag()->isValid() )
	return;

    tmpIndex = -1;
    string = doc->lastParag();
    idx = string->length() - 1;
}

void QTextEditCursor::gotoPageUp( QTextEdit *view )
{
    tmpIndex = -1;
    QTextEditParag *s = string;
    int h = view->visibleHeight();
    int y = s->rect().y();
    while ( s ) {
	if ( y - s->rect().y() >= h )
	    break;
	s = s->prev();
    }

    if ( !s )
	s = doc->firstParag();

    string = s;
    idx = 0;
}

void QTextEditCursor::gotoPageDown( QTextEdit *view )
{
    tmpIndex = -1;
    QTextEditParag *s = string;
    int h = view->visibleHeight();
    int y = s->rect().y();
    while ( s ) {
	if ( s->rect().y() - y >= h )
	    break;
	s = s->next();
    }

    if ( !s )
	s = doc->lastParag();

    if ( !s->isValid() )
	return;

    string = s;
    idx = 0;
}

void QTextEditCursor::gotoWordLeft()
{
    gotoLeft();
    tmpIndex = -1;
    QTextEditString *s = string->string();
    bool allowSame = FALSE;
    for ( int i = idx - 1; i >= 0; --i ) {
	if ( s->at( i ).c.isSpace() || s->at( i ).c == '\t' ) {
	    if ( !allowSame && s->at( i ).c == s->at( idx ).c )
		continue;
	    idx = i + 1;
	    return;
	}
	if ( !allowSame && s->at( i ).c != s->at( idx ).c )
	    allowSame = TRUE;
    }

    if ( string->prev() ) {
	string = string->prev();
	idx = string->length() - 1;
    } else {
	gotoLineStart();
    }
}

void QTextEditCursor::gotoWordRight()
{
    tmpIndex = -1;
    QTextEditString *s = string->string();
    bool allowSame = FALSE;
    for ( int i = idx + 1; i < (int)s->length(); ++i ) {
	if ( s->at( i ).c.isSpace() || s->at( i ).c == '\t' ) {
	    if ( !allowSame &&  s->at( i ).c == s->at( idx ).c )
		continue;
	    idx = i;
	    return;
	}
	if ( !allowSame && s->at( i ).c != s->at( idx ).c )
	    allowSame = TRUE;
    }

    if ( string->next() ) {
	string = string->next();
	idx = 0;
    } else {
	gotoLineEnd();
    }
}

bool QTextEditCursor::atParagStart()
{
    return idx == 0;
}

bool QTextEditCursor::atParagEnd()
{
    return idx == string->length() - 1;
}

void QTextEditCursor::splitAndInsertEmtyParag( bool ind, bool updateIds )
{
    tmpIndex = -1;
    QTextEditFormat *f = 0;
    if ( !doc->syntaxHighlighter() )
	f = string->at( idx )->format;

    if ( atParagStart() ) {
	QTextEditParag *p = string->prev();
	QTextEditParag *s = new QTextEditParag( doc, p, string, updateIds );
	s->append( " " );
	if ( f )
	    s->setFormat( 0, 1, f, TRUE );
	s->setType( string->type() );
	s->setListDepth( string->listDepth() );
	s->setAlignment( string->alignment() );
	if ( ind ) {
	    s->indent();
	    s->format();
	    indent();
	    string->format();
	}
    } else if ( atParagEnd() ) {
	QTextEditParag *n = string->next();
	QTextEditParag *s = new QTextEditParag( doc, string, n, updateIds );
	s->append( " " );
	if ( f )
	    s->setFormat( 0, 1, f, TRUE );
	s->setType( string->type() );
	s->setListDepth( string->listDepth() );
	s->setAlignment( string->alignment() );
	if ( ind ) {
	    int oi, ni;
	    s->indent( &oi, &ni );
	    string = s;
	    idx = ni;
	} else {
	    string = s;
	    idx = 0;
	}
    } else {
	QString str = string->string()->toString().mid( idx, 0xFFFFFF );
	string->truncate( idx );
	QTextEditParag *n = string->next();
	QTextEditParag *s = new QTextEditParag( doc, string, n, updateIds );
	s->setType( string->type() );
	s->setListDepth( string->listDepth() );
	s->setAlignment( string->alignment() );
	s->append( str );
	if ( f )
	    s->setFormat( 0, str.length(), f, TRUE );
	if ( ind ) {
	    int oi, ni;
	    s->indent( &oi, &ni );
	    string = s;
	    idx = ni;
	} else {
	    string = s;
	    idx = 0;
	}
    }
}

bool QTextEditCursor::remove()
{
    tmpIndex = -1;
    if ( !atParagEnd() ) {
	string->remove( idx, 1 );
	return FALSE;
    } else if ( string->next() ) {
	string->join( string->next() );
	return TRUE;
    }
    return FALSE;
}

void QTextEditCursor::indent()
{
    int oi = 0, ni = 0;
    string->indent( &oi, &ni );
    if ( oi == ni )
	return;

    if ( idx >= oi )
	idx += ni - oi;
    else
	idx = ni;
}

bool QTextEditCursor::checkOpenParen()
{
    if ( !doc->isParenCheckingEnabled() )
	return FALSE;

    QTextEditParag::ParenList parenList = string->parenList();

    QTextEditParag::Paren openParen, closedParen;
    QTextEditParag *closedParenParag = string;

    int i = 0;
    int ignore = 0;
    bool foundOpen = FALSE;
    QChar c = string->at( idx )->c;
    while ( TRUE ) {
	if ( !foundOpen ) {
	    if ( i >= (int)parenList.count() )
		goto aussi;
	    openParen = parenList[ i ];
	    if ( openParen.pos != idx ) {
		++i;
		continue;
	    } else {
		foundOpen = TRUE;
		++i;
	    }
	}
	
	if ( i >= (int)parenList.count() ) {
	    while ( TRUE ) {
		closedParenParag = closedParenParag->next();
		if ( !closedParenParag )
		    goto aussi;
		if ( closedParenParag->parenList().count() > 0 ) {
		    parenList = closedParenParag->parenList();
		    break;
		}
	    }
	    i = 0;
	}
	
	closedParen = parenList[ i ];
	if ( closedParen.type == QTextEditParag::Paren::Open ) {
	    ignore++;
	    ++i;
	    continue;
	} else {
	    if ( ignore > 0 ) {
		ignore--;
		++i;
		continue;
	    }
	
	    int id = QTextEditDocument::ParenMatch;
	    if ( c == '{' && closedParen.chr != '}' ||
		 c == '(' && closedParen.chr != ')' ||
		 c == '[' && closedParen.chr != ']' )
		id = QTextEditDocument::ParenMismatch;
	    doc->setSelectionStart( id, this );
	    int tidx = idx;
	    QTextEditParag *tstring = string;
	    idx = closedParen.pos + 1;
	    string = closedParenParag;
	    doc->setSelectionEnd( id, this );
	    string = tstring;
	    idx = tidx;
	    return TRUE;
	}
	
	++i;
    }

 aussi:
    return FALSE;
}

bool QTextEditCursor::checkClosedParen()
{
    if ( !doc->isParenCheckingEnabled() )
	return FALSE;

    QTextEditParag::ParenList parenList = string->parenList();

    QTextEditParag::Paren openParen, closedParen;
    QTextEditParag *openParenParag = string;

    int i = parenList.count() - 1;
    int ignore = 0;
    bool foundClosed = FALSE;
    QChar c = string->at( idx - 1 )->c;
    while ( TRUE ) {
	if ( !foundClosed ) {
	    if ( i < 0 )
		goto aussi;
	    closedParen = parenList[ i ];
	    if ( closedParen.pos != idx - 1 ) {
		--i;
		continue;
	    } else {
		foundClosed = TRUE;
		--i;
	    }
	}
	
	if ( i < 0 ) {
	    while ( TRUE ) {
		openParenParag = openParenParag->prev();
		if ( !openParenParag )
		    goto aussi;
		if ( openParenParag->parenList().count() > 0 ) {
		    parenList = openParenParag->parenList();
		    break;
		}
	    }
	    i = parenList.count() - 1;
	}
	
	openParen = parenList[ i ];
	if ( openParen.type == QTextEditParag::Paren::Closed ) {
	    ignore++;
	    --i;
	    continue;
	} else {
	    if ( ignore > 0 ) {
		ignore--;
		--i;
		continue;
	    }
	
	    int id = QTextEditDocument::ParenMatch;
	    if ( c == '}' && openParen.chr != '{' ||
		 c == ')' && openParen.chr != '(' ||
		 c == ']' && openParen.chr != '[' )
		id = QTextEditDocument::ParenMismatch;
	    doc->setSelectionStart( id, this );
	    int tidx = idx;
	    QTextEditParag *tstring = string;
	    idx = openParen.pos;
	    string = openParenParag;
	    doc->setSelectionEnd( id, this );
	    string = tstring;
	    idx = tidx;
	    return TRUE;
	}
	
	--i;
    }

 aussi:
    return FALSE;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

const int QTextEditDocument::numSelections = 4; // Don't count the Temp one!

QTextEditDocument::QTextEditDocument()
{
    syntaxHighlighte = 0;
    pFormatter = 0;
    indenter = 0;
    parenCheck = FALSE;
    completion = FALSE;
    fCollection = new QTextEditFormatCollection;
    fParag = 0;
    txtFormat = Qt::AutoText;
    preferRichText = FALSE;
    filename = QString::null;
    ls = ps = 0;

    lParag = fParag = new QTextEditParag( this, 0, 0 );
    lParag->append( " " );
				
    cx = 2;
    cy = 2;
    cw = 600;

    selectionColors[ Standard ] = QApplication::palette().color( QPalette::Normal, QColorGroup::Highlight );
    selectionColors[ ParenMismatch ] = Qt::magenta;
    selectionColors[ ParenMatch ] = Qt::green;
    selectionColors[ Search ] = Qt::yellow;
    selectionText[ Standard ] = TRUE;
    selectionText[ ParenMismatch ] = FALSE;
    selectionText[ ParenMatch ] = FALSE;
    selectionText[ Search ] = FALSE;
    commandHistory = new QTextEditCommandHistory( 100 ); // ### max undo/redo steps should be configurable
}

void QTextEditDocument::setPlainText( const QString &text, bool tabify )
{
    if ( fParag ) {
	QTextEditParag *p = 0;
	while ( fParag ) {
	    p = fParag->next();
	    delete fParag;
	    fParag = p;
	}
	fParag = 0;
    }
    preferRichText = FALSE;

    ls = ps = 0;

    QString s;
    lParag = 0;
    QStringList lst = QStringList::split( '\n', text, TRUE );
    for ( QStringList::Iterator it = lst.begin(); it != lst.end(); ++it ) {
	lParag = new QTextEditParag( this, lParag, 0 );
	if ( !fParag )
	    fParag = lParag;
	s = *it;
	if ( !s.isEmpty() ) {
	    QChar c;
	    int i = 0;
	    int spaces = 0;
	    if ( tabify ) {
		for ( ; i < (int)s.length(); ++i ) {
		    c = s[ i ];
		    if ( c != ' ' && c != '\t' )
			break;
		    if ( c == '\t' ) {
			spaces = 0;
			s.replace( i, 1, "\t\t" );
			++i;
		    } else if ( c == ' ' )
			++spaces;
		    if ( spaces == 4 ) {
			s.replace( i  - 3, 4, "\t" );
			i-= 2;
			spaces = 0;
		    }
		}
	    }
	    if ( s.right( 1 ) != " " )
		s += " ";
	    lParag->append( s );
	} else {
	    lParag->append( " " );
	}
    }

    if ( !lParag ) {
	lParag = fParag = new QTextEditParag( this, 0, 0 );
	lParag->append( " " );
    }
}

void QTextEditDocument::setRichText( const QString &text )
{
    if ( fParag ) {
	QTextEditParag *p = 0;
	while ( fParag ) {
	    p = fParag->next();
	    delete fParag;
	    fParag = p;
	}
	fParag = 0;
    }
    preferRichText = TRUE;

    ps = 8;
    ls = 1;

    QRichText *rt = new QRichText( text, fCollection->defaultFormat()->font(),
				   QFileInfo( filename ).absFilePath(), 8,
				   QMimeSourceFactory::defaultFactory(),
				   QStyleSheet::defaultSheet() );
    QRichTextIterator it( *rt );
    lParag = 0;
    QTextEditFormat *fm = 0;
    bool nextNl;
    bool empty = TRUE;
    do {
	if ( !it.format()->customItem() &&
	     ( !empty || ( empty && !it.text().simplifyWhiteSpace().isEmpty() ) ) ) {
	    empty = FALSE;
	    if ( !lParag || nextNl ) {
		if ( lParag ) {
		    if ( lParag->at( lParag->length() - 1 )->c != ' ' )
			lParag->append( " " );
		}
		lParag = new QTextEditParag( this, lParag, 0 );
		if ( it.outmostParagraph() &&
		     it.outmostParagraph()->style ) {
		    if ( it.outmostParagraph()->alignment() != -1 ) {
			if ( it.outmostParagraph()->alignment() & Qt::AlignRight )
			    lParag->setAlignment( Qt::AlignRight );
			else if ( it.outmostParagraph()->alignment() & Qt::AlignCenter )
			    lParag->setAlignment( Qt::AlignCenter );
		    }
		    if ( it.outmostParagraph()->style->displayMode() == QStyleSheetItem::DisplayListItem ) {
			lParag->setType( QTextEditParag::BulletList );
			lParag->setListDepth( 0 );
		    }
		}
		if ( !fParag )
		    fParag = lParag;
	    }
	    int i = lParag->length();
	    int len = it.text().length();
	    QString t = it.text();
	    nextNl = t.find( '\n' ) != -1;
	    if ( nextNl )
		t.replace( QRegExp( "\n" ), " " );
	    lParag->append( t );
	    fm = fCollection->format( it.format()->font(), it.format()->color() );
	    lParag->setFormat( i, len, fm, TRUE );
	    fm->removeRef();
	    fm = 0;
	}
    } while ( it.right( FALSE ) );

#ifdef DEBUG_COLLECTION
    fCollection->debug();
#endif

    delete rt;

    if ( !lParag ) {
	setPlainText( text );
    }
}

void QTextEditDocument::load( const QString &fn, bool tabify )
{
    filename = fn;
    QFile file( fn );
    file.open( IO_ReadOnly );
    QTextStream ts( &file );
    QString txt = ts.read();
    file.close();
    setText( txt, tabify );
}

void QTextEditDocument::setText( const QString &text, bool tabify )
{
    if ( txtFormat == Qt::AutoText && QStyleSheet::mightBeRichText( text ) ||
	 txtFormat == Qt::RichText )
	setRichText( text );
    else
	setPlainText( text, tabify );
}

static void do_untabify( QString &s )
{
    int numTabs = 0;
    int i = 0;
    while ( s[ i++ ] == '\t' )
	numTabs++;
    if ( !numTabs )
	return;

    int realTabs = ( numTabs / 2 ) * 2;
    if ( realTabs != numTabs )
	s = s.replace( numTabs - 1, 1, "    " );
    QString tabs;
    tabs.fill( '\t', realTabs / 2 );
    s = s.replace( 0, realTabs, tabs );
}

QString QTextEditDocument::plainText( QTextEditParag *p, bool formatted, bool untabify ) const
{
    if ( !p ) {
	QString buffer;
	QString s;
	QTextEditParag *p = fParag;
	while ( p ) {
	    s = p->string()->toString();
	    if ( untabify )
		do_untabify( s );
	    s += "\n";
	    buffer += s;
	    p = p->next();
	}
	return buffer;
    } else {
	if ( !formatted )
	    return p->string()->toString();

	// ##### TODO: return formatted string
	return p->string()->toString();
    }
}

QString QTextEditDocument::richText( QTextEditParag *p, bool formatted ) const
{
    if ( !p ) {
	// #### very poor implementation!
	QString text;
	p = fParag;
	QTextEditParag *lastParag = 0;
	QTextEditFormat *lastFormat = 0;
	QTextEditString::Char *c = 0;
	bool listDepth = 0;
	bool inList = FALSE;
	while ( p ) {
	    QString s;
	    if ( inList && p->type() != QTextEditParag::BulletList ) {
		text += "</ul>\n";
		inList = FALSE;
	    }
	    if ( !inList && p->type() == QTextEditParag::BulletList ) {
		text += "<ul>\n";
		listDepth = p->listDepth();
		inList = TRUE;
	    }

	    if ( inList ) {
		s = "<li>";
	    } else if ( !lastParag || lastParag->alignment() != p->alignment() ) {
		s = "<p align=\"";
		if ( p->alignment() & Qt::AlignRight )
		    s += "right";
		else if ( p->alignment() & Qt::AlignCenter )
		    s += "center";
		else
		    s += "left";
		s += "\">";
	    } else {
		s = "<p>";
	    }
	
	    int len = 0;
	    for ( int i = 0; i < p->length(); ++i ) {
		c = &p->string()->at( i );
		if ( !lastFormat || ( lastFormat->key() != c->format->key() && c->c != ' ' ) ) {
		    s += c->format->makeFormatChangeTags( lastFormat );
		    lastFormat = c->format;
		}
		if ( c->c == '<' )
		    s += "&lt;";
		else if ( c->c == '>' )
		    s += "&gt;";
		else
		    s += c->c;
		len += c->c != ' ' ? 1 : 0;
	    }
	    if ( !inList )
		text += s + lastFormat->makeFormatEndTags() + "</p>\n";
	    else if ( len > 0 )
		text += s + lastFormat->makeFormatEndTags() + "\n";
	    lastFormat = 0;
	    lastParag = p;
	    p = p->next();
	}
	text += "\n";
	return text;
    } else {
	// #### TODO return really rich text
	return plainText( p, formatted );
    }
}

QString QTextEditDocument::text( bool untabify ) const
{
    if ( plainText().simplifyWhiteSpace().isEmpty() )
	return QString::null;
    if ( txtFormat == Qt::AutoText && preferRichText || txtFormat == Qt::RichText )
	return richText();
    return plainText( 0, FALSE, untabify );
}

QString QTextEditDocument::text( int parag, bool formatted ) const
{
    QTextEditParag *p = paragAt( parag );
    if ( !p )
	return QString::null;

    if ( txtFormat == Qt::AutoText && preferRichText || txtFormat == Qt::RichText )
	return richText( p, formatted );
    else
	return plainText( p, formatted );
}

void QTextEditDocument::invalidate()
{
    QTextEditParag *s = fParag;
    while ( s ) {
	s->invalidate( 0 );
	s = s->next();
    }
}

void QTextEditDocument::save( const QString &fn, bool untabify )
{
    if ( !fn.isEmpty() )
	filename = fn;
    if ( !filename.isEmpty() ) {
	QFile file( filename );
	if ( file.open( IO_WriteOnly ) ) {
	    QTextStream ts( &file );
	    ts << text( untabify );;
	    file.close();
	} else {
	    qWarning( "couldn't open file %s", filename.latin1() );
	}
    } else {
	qWarning( "QTextEditDocument::save(): couldn't save - no filename specified!" );
    }
}

QString QTextEditDocument::fileName() const
{
    return filename;
}

void QTextEditDocument::selectionStart( int id, int &paragId, int &index )
{
    QMap<int, Selection>::Iterator it = selections.find( id );
    if ( it == selections.end() )
	return;
    Selection &sel = *it;
    paragId = QMIN( sel.startParag->paragId(), sel.endParag->paragId() );
    index = sel.startIndex;
}

void QTextEditDocument::selectionEnd( int id, int &paragId, int &index )
{
    QMap<int, Selection>::Iterator it = selections.find( id );
    if ( it == selections.end() )
	return;
    Selection &sel = *it;
    paragId = QMAX( sel.startParag->paragId(), sel.endParag->paragId() );
    if ( paragId == sel.startParag->paragId() )
	index = sel.startParag->selectionEnd( id );
    else
	index = sel.endParag->selectionEnd( id );
}

QTextEditParag *QTextEditDocument::selectionStart( int id )
{
    QMap<int, Selection>::Iterator it = selections.find( id );
    if ( it == selections.end() )
	return 0;
    Selection &sel = *it;
    if ( sel.startParag->paragId() <  sel.endParag->paragId() )
	return sel.startParag;
    return sel.endParag;
}

QTextEditParag *QTextEditDocument::selectionEnd( int id )
{
    QMap<int, Selection>::Iterator it = selections.find( id );
    if ( it == selections.end() )
	return 0;
    Selection &sel = *it;
    if ( sel.startParag->paragId() >  sel.endParag->paragId() )
	return sel.startParag;
    return sel.endParag;
}

bool QTextEditDocument::setSelectionEnd( int id, QTextEditCursor *cursor )
{
    QMap<int, Selection>::Iterator it = selections.find( id );
    if ( it == selections.end() )
	return FALSE;

    Selection &sel = *it;
    QTextEditParag *oldEndParag = sel.endParag;
    QTextEditParag *oldStartParag = sel.startParag;
    if ( sel.endParag->paragId() < sel.startParag->paragId() ) {
	oldStartParag = sel.endParag;
	oldEndParag = sel.startParag;
    }
    sel.endParag = cursor->parag();
    int start = sel.startIndex;
    int end = cursor->index();
    bool swapped = FALSE;
    if ( sel.endParag->paragId() < sel.startParag->paragId() ) {
	sel.endParag = sel.startParag;
	sel.startParag = cursor->parag();
	end = sel.startIndex;
	start = cursor->index();
	swapped = TRUE;
    }

    if ( sel.startParag == sel.endParag ) {
	if ( end < start) {
	    end = sel.startIndex;
	    start = cursor->index();
	}
	sel.endParag->setSelection( id, start, end );

	QTextEditParag *p = 0;
	if ( sel.endParag->paragId() < oldEndParag->paragId() ) {
	    p = sel.endParag;
	    p = p->next();
	    while ( p ) {
		p->removeSelection( id );
		if ( p == oldEndParag )
		    break;
		p = p->next();
	    }
	}
	
	if ( sel.startParag->paragId() > oldStartParag->paragId() ) {
	    p = sel.startParag;
	    p = p->prev();
	    while ( p ) {
		p->removeSelection( id );
		if ( p == oldStartParag )
		    break;
		p = p->prev();
	    }
	}
    } else {
	QTextEditParag *p = sel.startParag;
	p->setSelection( id, start, p->length() - 1 );
	p->setChanged( TRUE );
	p = p->next();
	if ( p )
	    p->setChanged( TRUE );
	while ( p && p != sel.endParag ) {
	    p->setSelection( id, 0, p->length() - 1 );
	    p->setChanged( TRUE );
	    p = p->next();
	}
	sel.endParag->setSelection( id, 0, end );
	sel.endParag->setChanged( TRUE );

	if ( sel.endParag->paragId() < oldEndParag->paragId() ) {
	    p = sel.endParag;
	    p = p->next();
	    while ( p ) {
		p->removeSelection( id );
		if ( p == oldEndParag )
		    break;
		p = p->next();
	    }
	}
	
	if ( sel.startParag->paragId() > oldStartParag->paragId() ) {
	    p = sel.startParag;
	    p = p->prev();
	    while ( p ) {
		p->removeSelection( id );
		if ( p == oldStartParag )
		    break;
		p = p->prev();
	    }
	}
	
	if ( swapped ) {
	    p = sel.startParag;
	    sel.startParag = sel.endParag;
	    sel.endParag = p;
	}
    }

    return TRUE;
}

bool QTextEditDocument::removeSelection( int id )
{
    QMap<int, Selection>::Iterator it = selections.find( id );
    if ( it == selections.end() )
	return FALSE;

    QTextEditParag *start = ( *it ).startParag;
    QTextEditParag *end = ( *it ).endParag;
    if ( end->paragId() < start->paragId() ) {
	end = ( *it ).startParag;
	start = ( *it ).endParag;
    }

    QTextEditParag *p = start;
    while ( p ) {
	p->removeSelection( id );
	if ( p == end )
	    break;
	p = p->next();
    }

    selections.remove( id );
    return TRUE;
}

QString QTextEditDocument::selectedText( int id ) const
{
    // ######## TODO: look at textFormat() and return rich text or plain text (like the text() method!)
    QMap<int, Selection>::ConstIterator it = selections.find( id );
    if ( it == selections.end() )
	return QString::null;

    Selection sel = *it;

    QTextEditParag *endParag = sel.endParag;
    QTextEditParag *startParag = sel.startParag;
    if ( sel.endParag->paragId() < sel.startParag->paragId() ) {
	startParag = sel.endParag;
	endParag = sel.startParag;
    }

    QString buffer;
    QString s;
    QTextEditParag *p = startParag;
    while ( p ) {
	s = p->string()->toString().mid( p->selectionStart( id ),
					 p->selectionEnd( id ) - p->selectionStart( id ) );
	if ( p->selectionEnd( id ) == p->length() - 1 && p != endParag )
	    s += "\n";
	buffer += s;
	if ( p == endParag )
	    break;
	p = p->next();
    }

    return buffer;
}

void QTextEditDocument::setFormat( int id, QTextEditFormat *f, int flags )
{
    QMap<int, Selection>::ConstIterator it = selections.find( id );
    if ( it == selections.end() )
	return;

    Selection sel = *it;

    QTextEditParag *endParag = sel.endParag;
    QTextEditParag *startParag = sel.startParag;
    if ( sel.endParag->paragId() < sel.startParag->paragId() ) {
	startParag = sel.endParag;
	endParag = sel.startParag;
    }

    QTextEditParag *p = startParag;
    while ( p ) {
	int end = p->selectionEnd( id );
	if ( end == p->length() - 1 )
	    end++;
	p->setFormat( p->selectionStart( id ), end - p->selectionStart( id ),
		      f, TRUE, flags );
	if ( p == endParag )
	    break;
	p = p->next();
    }
}

void QTextEditDocument::copySelectedText( int id )
{
    if ( !hasSelection( id ) )
	return;

    QApplication::clipboard()->setText( selectedText( id ) );
}

void QTextEditDocument::removeSelectedText( int id, QTextEditCursor *cursor )
{
    QMap<int, Selection>::Iterator it = selections.find( id );
    if ( it == selections.end() )
	return;

    Selection sel = *it;
    QTextEditParag *startParag = sel.startParag;
    QTextEditParag *endParag = sel.endParag;
    if ( sel.endParag->paragId() < sel.startParag->paragId() ) {
	endParag = sel.startParag;
	startParag = sel.endParag;
    }

    if ( startParag == endParag ) {
	int idx = -1;
	if ( cursor->parag() == startParag &&
	     cursor->index() > startParag->selectionStart( id ) )
	    idx = startParag->selectionStart( id );
	startParag->remove( startParag->selectionStart( id ),
			    startParag->selectionEnd( id ) - startParag->selectionStart( id ) );
	if ( idx != -1 )
	    cursor->setIndex( idx );
    } else {
	int idx = -1;
	QTextEditParag *cp = 0;
	
	if ( cursor->parag() == startParag &&
	     cursor->index() > startParag->selectionStart( id ) )
	    idx = startParag->selectionStart( id );
	else if ( cursor->parag()->paragId() > startParag->paragId() &&
		  cursor->parag()->paragId() <= endParag->paragId() ) {
	    cp = startParag;
	    idx = startParag->selectionStart( id );
	}
	
	startParag->remove( startParag->selectionStart( id ),
			    startParag->selectionEnd( id ) - startParag->selectionStart( id ) );
	endParag->remove( 0, endParag->selectionEnd( id ) );
	QTextEditParag *p = startParag, *tmp;
	p = p->next();
	int dy = 0;
	while ( p ) {
	    if ( p == endParag )
		break;
	    tmp = p->next();
	    dy += p->rect().height();
	    delete p;
	    p = tmp;
	}
	
	while ( p ) {
	    p->move( -dy );
	    p->invalidate( 0 );
	    p->setEndState( -1 );
	    p = p->next();
	}
	
	startParag->join( endParag );
	
	if ( cp )
	    cursor->setParag( cp );
	if ( idx != -1 )
	    cursor->setIndex( idx );
    }

    removeSelection( id );
}

void QTextEditDocument::indentSelection( int id )
{
    QMap<int, Selection>::Iterator it = selections.find( id );
    if ( it == selections.end() )
	return;

    Selection sel = *it;
    QTextEditParag *startParag = sel.startParag;
    QTextEditParag *endParag = sel.endParag;
    if ( sel.endParag->paragId() < sel.startParag->paragId() ) {
	endParag = sel.startParag;
	startParag = sel.endParag;
    }

    QTextEditParag *p = startParag;
    while ( p && p != endParag ) {
	p->indent();
	p = p->next();
    }
}

void QTextEditDocument::addCompletionEntry( const QString &s )
{
    if ( !eCompletionMap )
	eCompletionMap = new QMap<QChar, QStringList >();
    QChar key( s[ 0 ] );
    QMap<QChar, QStringList>::Iterator it = eCompletionMap->find( key );
    if ( it == eCompletionMap->end() )
	eCompletionMap->insert( key, QStringList( s ) );
    else
	( *it ).append( s );
}

QStringList QTextEditDocument::completionList( const QString &s ) const
{
    ( (QTextEditDocument*)this )->updateCompletionMap();

    QChar key( s[ 0 ] );
    QMap<QChar, QStringList>::ConstIterator it = eCompletionMap->find( key );
    if ( it == eCompletionMap->end() )
	return QStringList();
    QStringList::ConstIterator it2 = ( *it ).begin();
    QStringList lst;
    int len = s.length();
    for ( ; it2 != ( *it ).end(); ++it2 ) {
	if ( (int)( *it2 ).length() > len && ( *it2 ).left( len ) == s &&
	     lst.find( *it2 ) == lst.end() )
	    lst << *it2;
    }

    return lst;
}

void QTextEditDocument::updateCompletionMap()
{
    // #############
    // Quite slow the first time the completion list is setup
    // It should be imprved somehow
    // #############
    QTextEditParag *s = fParag;
    while ( s ) {
	if ( s->length() == s->lastLengthForCompletion() ) {
	    s = s->next();
	    continue;
	}
	
	QChar c;
	QString buffer;
	for ( int i = 0; i < s->length(); ++i ) {
	    c = s->at( i )->c;
	    if ( c.isLetter() || c.isNumber() || c == '_' || c == '#' ) {
		buffer += c;
	    } else {
		addCompletionEntry( buffer );
		buffer = QString::null;
	    }
	}
	if ( !buffer.isEmpty() )
	    addCompletionEntry( buffer );
	
	s->setLastLengthFotCompletion( s->length() );
	s = s->next();
    }
}

void QTextEditDocument::addCommand( QTextEditCommand *cmd )
{
    commandHistory->addCommand( cmd );
}

QTextEditCursor *QTextEditDocument::undo( QTextEditCursor *c )
{
    return commandHistory->undo( c );
}

QTextEditCursor *QTextEditDocument::redo( QTextEditCursor *c )
{
    return commandHistory->redo( c );
}

bool QTextEditDocument::find( const QString &expr, bool cs, bool wo, bool forward,
			      int *parag, int *index, QTextEditCursor *cursor )
{
    // #### wo and forward is ignored at the moment
    QTextEditParag *p = fParag;
    if ( parag )
	p = paragAt( *parag );
    else if ( cursor )
	p = cursor->parag();
    bool first = TRUE;

    while ( p ) {
	QString s = p->string()->toString();
	int start = 0;
	if ( first && index )
	    start = *index;
	else if ( first )
	    start = cursor->index();
	first = FALSE;
	int res = s.find( expr, start, cs );
	if ( res != -1 ) {
	    cursor->setParag( p );
	    cursor->setIndex( res );
	    setSelectionStart( Search, cursor );
	    cursor->setIndex( res + expr.length() );
	    setSelectionEnd( Search, cursor );
	    if ( parag )
		*parag = p->paragId();
	    if ( index )
		*index = res;
	    return TRUE;
	}
	p = p->next();
    }

    return FALSE;
}

void QTextEditDocument::setTextFormat( Qt::TextFormat f )
{
    txtFormat = f;
}

Qt::TextFormat QTextEditDocument::textFormat() const
{
    return txtFormat;
}

void QTextEditDocument::setParagSpacing( int s )
{
    ps = s;
}

void QTextEditDocument::setLineSpacing( int s )
{
    ls = s;
}

bool QTextEditDocument::inSelection( int selId, const QPoint &pos ) const
{
    QMap<int, Selection>::ConstIterator it = selections.find( selId );
    if ( it == selections.end() )
	return FALSE;

    Selection sel = *it;
    QTextEditParag *startParag = sel.startParag;
    QTextEditParag *endParag = sel.endParag;
    if ( sel.endParag->paragId() < sel.startParag->paragId() ) {
	endParag = sel.startParag;
	startParag = sel.endParag;
    }

    // ######### Real implementation needed!!!!!!!

    QRect r = startParag->rect();
    r = r.unite( endParag->rect() );

    return r.contains( pos );
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QTextEditString::QTextEditString( )
{
}

void QTextEditString::insert( int index, const QString &s, QTextEditFormat *f )
{
    int os = data.size();
    data.resize( data.size() + s.length() );
    if ( index < os ) {
	memmove( data.data() + index + s.length(), data.data() + index,
		 sizeof( Char ) * ( os - index ) );
    }
    for ( int i = 0; i < (int)s.length(); ++i ) {
	data[ (int)index + i ].x = 0;
	data[ (int)index + i ].lineStart = 0;
#if defined(Q_WS_X11)
	//### workaround for broken courier fonts on X11
	if ( s[ i ] == QChar( 0x00a0U ) )
	    data[ (int)index + i ].c = ' ';
	else
	    data[ (int)index + i ].c = s[ i ];
#else
	data[ (int)index + i ].c = s[ i ];
#endif
	data[ (int)index + i ].format = f;
    }
    cache.insert( index, s );
}

void QTextEditString::truncate( int index )
{
    data.truncate( index );
    cache.truncate( index );
}

void QTextEditString::remove( int index, int len )
{
    memmove( data.data() + index, data.data() + index + len,
	     sizeof( Char ) * ( data.size() - index - len ) );
    data.resize( data.size() - len );
    cache.remove( index, len );
}

void QTextEditString::setFormat( int index, QTextEditFormat *f, bool useCollection )
{
    if ( useCollection && data[ index ].format )
	data[ index ].format->removeRef();
    data[ index ].format = f;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QTextEditParag::QTextEditParag( QTextEditDocument *d, QTextEditParag *pr, QTextEditParag *nx, bool updateIds )
    : invalid( -1 ), p( pr ), n( nx ), doc( d ), typ( Normal ), align( Qt::AlignLeft )
{
    if ( p )
	p->n = this;
    if ( n )
	n->p = this;
    if ( !p )
	doc->setFirstParag( this );
    if ( !n )
	doc->setLastParag( this );

    changed = FALSE;
    firstFormat = TRUE;
    state = -1;
    needHighlighte = FALSE;

    if ( p )
	id = p->id + 1;
    else
	id = 0;
    if ( n && updateIds ) {
	QTextEditParag *s = n;
	while ( s ) {
	    s->id = s->p->id + 1;
	    s = s->n;
	}
    }
    firstHilite = TRUE;
    lastLenForCompletion = -1;

    str = new QTextEditString( this );

    left = depth = 0;
}

void QTextEditParag::setNext( QTextEditParag *s )
{
    n = s;
    if ( !n )
	doc->setLastParag( this );
}

void QTextEditParag::setPrev( QTextEditParag *s )
{
    p = s;
    if ( !p )
	doc->setFirstParag( this );
}

void QTextEditParag::invalidate( int chr )
{
    if ( invalid < 0 )
	invalid = chr;
    else
	invalid = QMIN( invalid, chr );
}

void QTextEditParag::insert( int index, const QString &s )
{
    if ( doc->syntaxHighlighter() )
	str->insert( index, s,
		     doc->syntaxHighlighter()->format( QTextEditSyntaxHighlighter::Standard ) );
    else
	str->insert( index, s, doc->formatCollection()->defaultFormat() );
    invalidate( index );
    needHighlighte = TRUE;
}

void QTextEditParag::truncate( int index )
{
    str->truncate( index );
    append( " " );
    needHighlighte = TRUE;
}

void QTextEditParag::remove( int index, int len )
{
    str->remove( index, len );
    invalidate( 0 );
    needHighlighte = TRUE;
}

void QTextEditParag::join( QTextEditParag *s )
{
    int oh = r.height() + s->r.height();
    n = s->n;
    if ( n )
	n->p = this;
    else
	doc->setLastParag( this );

    if ( str->at( str->length() -1 ).c == ' ' ) // #### check this
	str->truncate( str->length() - 1 );
    int start = str->length();
    append( s->str->toString() );
    if ( !doc->syntaxHighlighter() ) {
	for ( int i = 0; i < s->length(); ++i ) {
	    s->str->at( i ).format->addRef();
	    str->setFormat( i + start, s->str->at( i ).format, TRUE );
	}
    }
    delete s;
    invalidate( 0 );
    r.setHeight( oh );
    format();
    needHighlighte = TRUE;
    if ( n ) {
	QTextEditParag *s = n;
	while ( s ) {
	    s->id = s->p->id + 1;
	    s->state = -1;
	    s->needHighlighte = TRUE;
	    s->changed = TRUE;
	    s = s->n;
	}
    }
    state = -1;
}

void QTextEditParag::move( int dy )
{
    if ( dy == 0 )
	return;
    changed = TRUE;
    r.moveBy( 0, dy );
}

void QTextEditParag::format( int start, bool doMove )
{
    if ( str->length() == 0 || !doc->formatter() )
	return;

    if ( doc->syntaxHighlighter() &&
	 ( needHighlighte || state == -1 ) )
	doc->syntaxHighlighter()->highlighte( this, 0 );
    needHighlighte = FALSE;

    if ( invalid == -1 )
	return;

    r.moveTopLeft( QPoint( doc->x(), p ? p->r.y() + p->r.height() : doc->y() ) );
    r.setWidth( doc->width() );
    QMap<int, LineStart*>::Iterator it = lineStarts.begin();
    for ( ; it != lineStarts.end(); ++it )
	delete *it;
    lineStarts.clear();
    int y = doc->formatter()->format( this, start );

    QTextEditString::Char *c = 0;
    if ( lineStarts.count() == 1 ) {
	c = &str->at( str->length() - 1 );
	r.setWidth( c->x + c->format->width( c->c ) );
    }

    if ( y != r.height() )
	r.setHeight( y );

    if ( n && doMove && n->invalid == -1 && r.y() + r.height() != n->r.y() ) {
	int dy = ( r.y() + r.height() ) - n->r.y();
	QTextEditParag *s = n;
	while ( s ) {
	    s->move( dy );
	    s = s->n;
	}
    }

    firstFormat = FALSE;
    changed = TRUE;
    invalid = -1;
}

int QTextEditParag::lineHeightOfChar( int i, int *bl, int *y ) const
{
    if ( !isValid() )
	( (QTextEditParag*)this )->format();

    QMap<int, LineStart*>::ConstIterator it = lineStarts.end();
    --it;
    for ( ;; ) {
	if ( i >= it.key() ) {
	    if ( bl )
		*bl = ( *it )->baseLine;
	    if ( y )
		*y = ( *it )->y;
	    return ( *it )->h;
	}
	if ( it == lineStarts.begin() )
	    break;
	--it;
    }
	
    qWarning( "QTextEditParag::lineHeightOfChar: couldn't find lh for %d", i );
    return 15;
}

QTextEditString::Char *QTextEditParag::lineStartOfChar( int i, int *index, int *line ) const
{
    if ( !isValid() )
	( (QTextEditParag*)this )->format();

    int l = lineStarts.count() - 1;
    QMap<int, LineStart*>::ConstIterator it = lineStarts.end();
    --it;
    for ( ;; ) {
	if ( i >= it.key() ) {
	    if ( index )
		*index = it.key();
	    if ( line )
		*line = l;
	    return &str->at( it.key() );
	}
	if ( it == lineStarts.begin() )
	    break;
	--it;
	--l;
    }

    qWarning( "QTextEditParag::lineStartOfChar: couldn't find %d", i );
    return 0;
}

int QTextEditParag::lines() const
{
    if ( !isValid() )
	( (QTextEditParag*)this )->format();

    return lineStarts.count();
}

QTextEditString::Char *QTextEditParag::lineStartOfLine( int line, int *index ) const
{
    if ( !isValid() )
	( (QTextEditParag*)this )->format();

    if ( line >= 0 && line < (int)lineStarts.count() ) {
	QMap<int, LineStart*>::ConstIterator it = lineStarts.begin();
	while ( line-- > 0 )
	    ++it;
	int i = it.key();
	if ( index )
	    *index = i;
	return &str->at( i );
    }

    qWarning( "QTextEditParag::lineStartOfLine: couldn't find %d", line );
    return 0;
}

void QTextEditParag::setFormat( int index, int len, QTextEditFormat *f, bool useCollection, int flags )
{
    if ( index < 0 )
	index = 0;
    if ( index > str->length() - 1 )
	index = str->length() - 1;
    if ( index + len > str->length() )
	len = str->length() - 1 - index;

    QTextEditFormatCollection *fc = 0;
    if ( useCollection )
	fc = doc->formatCollection();
    QTextEditFormat *of;
    for ( int i = 0; i < len; ++i ) {
	of = str->at( i + index ).format;
	if ( !changed && f->key() != of->key() )
	    changed = TRUE;
	if ( invalid == -1 &&
	     ( f->font().family() != of->font().family() ||
	       f->font().pointSize() != of->font().pointSize() ||
	       f->font().weight() != of->font().weight() ||
	       f->font().italic() != of->font().italic() ) ) {
	    invalidate( 0 );
	}
	if ( flags == -1 || flags == QTextEditFormat::Format || !fc ) {
	    if ( fc )
		f = fc->format( f );
	    str->setFormat( i + index, f, useCollection );
	} else {
	    QTextEditFormat *fm = fc->format( of, f, flags );
	    str->setFormat( i + index, fm, useCollection );
	}
    }
}

void QTextEditParag::indent( int *oldIndent, int *newIndent )
{
    if ( !doc->indent() || typ != Normal ) {
	if ( oldIndent )
	    *oldIndent = 0;
	if ( newIndent )
	    *newIndent = 0;
	if ( oldIndent && newIndent )
	    *newIndent = *oldIndent;
	return;
    }
    doc->indent()->indent( this, oldIndent, newIndent );
}

void QTextEditParag::setListDepth( int d )
{
    if ( typ == Normal ) {
	depth = d;
	left = 0;
	return;
    }
    left = doc->listIndent( d );
    depth = d;
    invalidate( 0 );
}

void QTextEditParag::paint( QPainter &painter, const QColorGroup &cg, QTextEditCursor *cursor, bool drawSelections )
{
    QTextEditString::Char *chr = at( 0 );
    int i = 0;
    int h = 0;
    int baseLine = 0, lastBaseLine = 0;
    QTextEditFormat *lastFormat = 0;
    int lastY = -1;
    QString buffer;
    int startX = 0;
    int bw = 0;
    int cy = 0;
    int curx = -1, cury, curh;
	
    // #### draw other selections too here!!!!!!!
    int selectionStarts[ doc->numSelections ];
    int selectionEnds[ doc->numSelections ];
    if ( drawSelections ) {
	bool hasASelection = FALSE;
	for ( i = 0; i < doc->numSelections; ++i ) {
	    if ( !hasSelection( i ) ) {
		selectionStarts[ i ] = -1;
		selectionEnds[ i ] = -1;
	    } else {
		hasASelection = TRUE;
		selectionStarts[ i ] = selectionStart( i );
		int end = selectionEnd( i );
		if ( end == length() - 1 && n && n->hasSelection( i ) )
		    end++;
		selectionEnds[ i ] = end;
	    }
	}
	if ( !hasASelection )
	    drawSelections = FALSE;
    }
	
    int line = -1;
    int cw;
    for ( i = 0; i < length(); i++ ) {
	chr = at( i );
	cw = chr->format->width( chr->c );

	// init a new line
	if ( chr->lineStart ) {
	    ++line;
	    lineInfo( line, cy, h, baseLine );
	    if ( lastBaseLine == 0 )
		lastBaseLine = baseLine;
	}
	
	// draw bullet list items
	if ( line == 0 && type() == QTextEditParag::BulletList ) {
	    int ext = QMIN( doc->listIndent( 0 ), h );
	    ext -= 10;
	
	    // ######## use pixmaps for drawing that stuff - this way it's very slow when having long lists!
	    switch ( doc->bullet( listDepth() ) ) {
	    case QTextEditDocument::FilledCircle: {
		painter.setPen( Qt::NoPen );
		painter.setBrush( cg.brush( QColorGroup::Foreground ) );
		painter.drawEllipse( leftIndent() - ext - 4, cy + ( h - ext ) / 2, ext, ext );
	    } break;
	    case QTextEditDocument::FilledSquare: {
		painter.fillRect( leftIndent() - ext - 4, cy + ( h - ext ) / 2, ext, ext,
				  cg.brush( QColorGroup::Foreground ) );
	    } break;
	    case QTextEditDocument::OutlinedCircle: {
		painter.setPen( QPen( cg.color( QColorGroup::Foreground ) ) );
		painter.setBrush( Qt::NoBrush );
		painter.drawEllipse( leftIndent() - ext - 4, cy + ( h - ext ) / 2, ext, ext );
	    } break;
	    case QTextEditDocument::OutlinedSquare: {
		painter.setPen( QPen( cg.color( QColorGroup::Foreground ) ) );
		painter.setBrush( Qt::NoBrush );
		painter.drawRect( leftIndent() - ext - 4, cy + ( h - ext ) / 2, ext, ext );
	    } break;
	    }
	}
	
	// check for cursor mark
	if ( cursor && this == cursor->parag() && i == cursor->index() ) {
	    curx = chr->x;
	    curh = h;
	    cury = cy;
	}
	
	// first time - start again...
	if ( !lastFormat || lastY == -1 ) {
	    lastFormat = chr->format;
	    lastY = cy;
	    startX = chr->x;
	    buffer += chr->c;
	    bw = cw;
	    continue;
	}
	
	// check if selection state changed
	bool selectionChange = FALSE;
	if ( drawSelections ) {
	    for ( int j = 0; j < doc->numSelections; ++j ) {
		selectionChange = selectionStarts[ j ] == i || selectionEnds[ j ] == i;
		if ( selectionChange )
		    break;
	    }
	}
	
	// if something (format, etc.) changed, draw what we have so far
	if ( lastY != cy || chr->format != lastFormat || buffer == "\t" || chr->c == '\t' || selectionChange ) {
	    drawParagBuffer( painter, buffer, startX, lastY, lastBaseLine, bw, h, drawSelections,
			     lastFormat, i, selectionStarts, selectionEnds, cg );
	    buffer = chr->c;
	    lastFormat = chr->format;
	    lastY = cy;
	    startX = chr->x;
	    bw = cw;
	} else {
	    buffer += chr->c;
	    bw += cw;
	}
	lastBaseLine = baseLine;
    }
	
    // if we are through thg parag, but still have some stuff left to draw, draw it now
    if ( !buffer.isEmpty() ) {
	bool selectionChange = FALSE;
	if ( drawSelections ) {
	    for ( int j = 0; j < doc->numSelections; ++j ) {
		selectionChange = selectionStarts[ j ] == i || selectionEnds[ j ] == i;
		if ( selectionChange )
		    break;
	    }
	}
	drawParagBuffer( painter, buffer, startX, lastY, lastBaseLine, bw, h, drawSelections,
			 lastFormat, i, selectionStarts, selectionEnds, cg );
    }
	
    // if we should draw a cursor, draw it now
    if ( curx != -1 && cursor )
	painter.fillRect( QRect( curx, cury, 1, curh ), Qt::black );
}

void QTextEditParag::drawParagBuffer( QPainter &painter, const QString &buffer, int startX,
				      int lastY, int baseLine, int bw, int h, bool drawSelections,
				      QTextEditFormat *lastFormat, int i, int *selectionStarts,
				      int *selectionEnds, const QColorGroup &cg )
{
    painter.setPen( QPen( lastFormat->color() ) );
    painter.setFont( lastFormat->font() );
    if ( drawSelections ) {
	for ( int j = 0; j < doc->numSelections; ++j ) {
	    if ( i > selectionStarts[ j ] && i <= selectionEnds[ j ] ) {
		if ( doc->invertSelectionText( j ) )
		    painter.setPen( QPen( cg.color( QColorGroup::HighlightedText ) ) );
		painter.fillRect( startX, lastY, bw, h, doc->selectionColor( j ) );
	    }
	}
    }
    if ( buffer != "\t" )
	painter.drawText( startX, lastY + baseLine, buffer );
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


QTextEditSyntaxHighlighter::QTextEditSyntaxHighlighter( QTextEditDocument *d )
    : doc( d )
{
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QTextEditFormatter::QTextEditFormatter( QTextEditDocument *d )
    : doc( d )
{
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QTextEditFormatterBreakInWords::QTextEditFormatterBreakInWords( QTextEditDocument *d )
    : QTextEditFormatter( d )
{
}

int QTextEditFormatterBreakInWords::format( QTextEditParag *parag, int start )
{
    QTextEditString::Char *c = 0;
    int left = parag->leftIndent();
    int x = left;
    int w = doc->width() - x;
    int y = 0;
    int h = 0;

    // #########################################
    // Should be optimized so that we start formatting
    // really at start (this means the last line begin before start)
    // and not always at the beginnin of the parag!
    start = 0;
    if ( start == 0 ) {
	c = &parag->string()->at( 0 );
    }
    // #########################################

    int i = start;
    QTextEditParag::LineStart *lineStart = new QTextEditParag::LineStart( 0, 0, 0 );
    parag->lineStartList().insert( 0, lineStart );

    for ( ; i < parag->string()->length(); ++i ) {
	c = &parag->string()->at( i );
	if ( i > 0 ) {
	    c->lineStart = 0;
	} else {
	    c->lineStart = 1;
	}
	int ww = 0;
	if ( c->c.unicode() >= 32 || c->c == '\t' ) {
	    ww = c->format->width( c->c );
	} else {
	    ww = c->format->width( ' ' );
	}
	
	if ( x + ww > left + w ) {
	    x = left;
	    y += h + doc->lineSpacing();
	    h = c->format->height();
	    lineStart = new QTextEditParag::LineStart( y, 0, 0 );
	    parag->lineStartList().insert( i, lineStart );
	    lineStart->baseLine = c->format->ascent();
	    lineStart->h = c->format->height();
	    c->lineStart = 1;
	} else if ( lineStart ) {
	    lineStart->baseLine = QMAX( lineStart->baseLine, c->format->ascent() );
	    h = QMAX( h, c->format->height() );
	    lineStart->h = h;
	}
	
	c->x = x;
	x += ww;
    }

    y += h + doc->paragSpacing( parag );
    return y;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QTextEditFormatterBreakWords::QTextEditFormatterBreakWords( QTextEditDocument *d )
    : QTextEditFormatter( d )
{
}

int QTextEditFormatterBreakWords::format( QTextEditParag *parag, int start )
{
    QTextEditString::Char *c = 0;
    int left = parag->leftIndent();
    int x = left;
    int w = doc->width() - x;
    int y = 0;
    int h = 0;

    // #########################################
    // Should be optimized so that we start formatting
    // really at start (this means the last line begin before start)
    // and not always at the beginnin of the parag!
    start = 0;
    if ( start == 0 ) {
	c = &parag->string()->at( 0 );
    }
    // #########################################

    int i = start;
    QTextEditParag::LineStart *lineStart = new QTextEditParag::LineStart( 0, 0, 0 );
    parag->lineStartList().insert( 0, lineStart );
    int lastSpace = -1;
    int tmpBaseLine = 0, tmph = 0;

    for ( ; i < parag->string()->length(); ++i ) {
	c = &parag->string()->at( i );
	if ( i > 0 && x > left ) {
	    c->lineStart = 0;
	} else {
	    c->lineStart = 1;
	}
	int ww = 0;
	if ( c->c.unicode() >= 32 || c->c == '\t' ) {
	    ww = c->format->width( c->c );
	} else {
	    ww = c->format->width( ' ' );
	}
	
	if ( x + ww > left + w ) {
	    if ( lastSpace == -1 ) {
		if ( lineStart ) {
		    lineStart->baseLine = QMAX( lineStart->baseLine, tmpBaseLine );
		    h = QMAX( h, tmph );
		    lineStart->h = h;
		}
		x = left;
		y += h + doc->lineSpacing();
		tmph = c->format->height();
		h = 0;
		lineStart = new QTextEditParag::LineStart( y, 0, 0 );
		parag->lineStartList().insert( i, lineStart );
		lineStart->baseLine = c->format->ascent();
		lineStart->h = c->format->height();
		c->lineStart = 1;
		tmpBaseLine = lineStart->baseLine;
		lastSpace = -1;
	    } else {
		i = lastSpace;
		x = left;
		y += h + doc->lineSpacing();
		tmph = c->format->height();
		h = tmph;
		lineStart = new QTextEditParag::LineStart( y, 0, 0 );
		parag->lineStartList().insert( i + 1, lineStart );
		lineStart->baseLine = c->format->ascent();
		lineStart->h = c->format->height();
		c->lineStart = 1;
		tmpBaseLine = lineStart->baseLine;
		lastSpace = -1;
		continue;
	    }
	} else if ( lineStart && c->c == ' ' ) {
	    tmpBaseLine = QMAX( tmpBaseLine, c->format->ascent() );
	    tmph = QMAX( tmph, c->format->height() );
	    lineStart->baseLine = QMAX( lineStart->baseLine, tmpBaseLine );
	    h = QMAX( h, tmph );
	    lineStart->h = h;
	    lastSpace = i;
	} else {
	    tmpBaseLine = QMAX( tmpBaseLine, c->format->ascent() );
	    tmph = QMAX( tmph, c->format->height() );
	}
	
	c->x = x;
	x += ww;
    }

    // ############## unefficient!!!!!!!!!!!!!!!!!!!!!! - rewrite that!!!!
    if ( parag->alignment() & Qt::AlignHCenter || parag->alignment() & Qt::AlignRight ) {
	int last = 0;
	QMap<int, QTextEditParag::LineStart*>::Iterator it = parag->lineStartList().begin();
	while ( TRUE ) {
	    it++;
	    int i = 0;
	    if ( it == parag->lineStartList().end() )
		i = parag->length() - 1;
	    else
		i = it.key() - 1;
	    c = &parag->string()->at( i );
	    int lw = c->x + c->format->width( c->c );
	    int diff = w - lw;
	    if ( parag->alignment() & Qt::AlignHCenter )
		diff /= 2;
	    for ( int j = last; j <= i; ++j )
		parag->string()->at( j ).x += diff;
	    last = i + 1;
	    if ( it == parag->lineStartList().end() )
		break;
	}
    }

    y += h + doc->paragSpacing( parag );
    return y;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QTextEditIndent::QTextEditIndent( QTextEditDocument *d )
    : doc( d )
{
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QTextEditFormatCollection::QTextEditFormatCollection()
{
    defFormat = new QTextEditFormat( QApplication::font(),
				     QApplication::palette().color( QPalette::Normal, QColorGroup::Text ) );
    lastFormat = cres = 0;
    cflags = -1;
    cKey.setAutoDelete( TRUE );
    cachedFormat = 0;
}

QTextEditFormat *QTextEditFormatCollection::format( QTextEditFormat *f )
{
    if ( f->parent() == this ) {
#ifdef DEBUG_COLLECTION
	qDebug( "need '%s', best case!", f->key().latin1() );
#endif
	lastFormat = f;
	lastFormat->addRef();
	return lastFormat;
    }

    if ( f == lastFormat || ( lastFormat && f->key() == lastFormat->key() ) ) {
#ifdef DEBUG_COLLECTION
	qDebug( "need '%s', good case!", f->key().latin1() );
#endif
	lastFormat->addRef();
	return lastFormat;
    }

    QTextEditFormat *fm = cKey.find( f->key() );
    if ( fm ) {
#ifdef DEBUG_COLLECTION
	qDebug( "need '%s', normal case!", f->key().latin1() );
#endif
	lastFormat = fm;
	lastFormat->addRef();
	return lastFormat;
    }

#ifdef DEBUG_COLLECTION
    qDebug( "need '%s', worst case!", f->key().latin1() );
#endif
    lastFormat = new QTextEditFormat( *f );
    lastFormat->collection = this;
    cKey.insert( lastFormat->key(), lastFormat );
    return lastFormat;
}

QTextEditFormat *QTextEditFormatCollection::format( QTextEditFormat *of, QTextEditFormat *nf, int flags )
{
    if ( cres && kof == of->key() && knf == nf->key() && cflags == flags ) {
#ifdef DEBUG_COLLECTION
	qDebug( "mix of '%s' and '%s, best case!", of->key().latin1(), nf->key().latin1() );
#endif
	cres->addRef();
	return cres;
    }

    cres = new QTextEditFormat( *of );
    kof = of->key();
    knf = nf->key();
    cflags = flags;
    if ( flags & QTextEditFormat::Bold )
	cres->fn.setBold( nf->fn.bold() );
    if ( flags & QTextEditFormat::Italic )
	cres->fn.setItalic( nf->fn.italic() );
    if ( flags & QTextEditFormat::Underline )
	cres->fn.setUnderline( nf->fn.underline() );
    if ( flags & QTextEditFormat::Family )
	cres->fn.setFamily( nf->fn.family() );
    if ( flags & QTextEditFormat::Size )
	cres->fn.setPointSize( nf->fn.pointSize() );
    if ( flags & QTextEditFormat::Color )
	cres->col = nf->col;
    cres->update();

    QTextEditFormat *fm = cKey.find( cres->key() );
    if ( !fm ) {
#ifdef DEBUG_COLLECTION
	qDebug( "mix of '%s' and '%s, worst case!", of->key().latin1(), nf->key().latin1() );
#endif
	cres->collection = this;
	cKey.insert( cres->key(), cres );
    } else {
#ifdef DEBUG_COLLECTION
	qDebug( "mix of '%s' and '%s, good case!", of->key().latin1(), nf->key().latin1() );
#endif
	delete cres;
	cres = fm;
	cres->addRef();
    }
					
    return cres;
}

QTextEditFormat *QTextEditFormatCollection::format( const QFont &f, const QColor &c )
{
    if ( cachedFormat && cfont == f && ccol == c ) {
#ifdef DEBUG_COLLECTION
	qDebug( "format of font and col '%s' - best case", cachedFormat->key().latin1() );
#endif
	cachedFormat->addRef();
	return cachedFormat;
    }

    QString key = QTextEditFormat::getKey( f, c );
    cachedFormat = cKey.find( key );
    cfont = f;
    ccol = c;

    if ( cachedFormat ) {
#ifdef DEBUG_COLLECTION
	qDebug( "format of font and col '%s' - good case", cachedFormat->key().latin1() );
#endif
	cachedFormat->addRef();
	return cachedFormat;
    }

    cachedFormat = new QTextEditFormat( f, c );
    cachedFormat->collection = this;
    cKey.insert( cachedFormat->key(), cachedFormat );
#ifdef DEBUG_COLLECTION
    qDebug( "format of font and col '%s' - worst case", cachedFormat->key().latin1() );
#endif
    return cachedFormat;
}

void QTextEditFormatCollection::remove( QTextEditFormat *f )
{
    if ( lastFormat == f )
	lastFormat = 0;
    if ( cres == f )
	cres = 0;
    if ( cachedFormat == f )
	cachedFormat = 0;
    cKey.remove( f->key() );
}

void QTextEditFormatCollection::debug()
{
#ifdef DEBUG_COLLECTION
    qDebug( "------------ QTextEditFormatCollection: debug --------------- BEGIN" );
    QDictIterator<QTextEditFormat> it( cKey );
    for ( ; it.current(); ++it ) {
	qDebug( "format '%s' (%p): refcount: %d", it.current()->key().latin1(),
		it.current(), it.current()->ref );
    }
    qDebug( "------------ QTextEditFormatCollection: debug --------------- END" );
#endif
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void QTextEditFormat::setBold( bool b )
{
    if ( b == fn.bold() )
	return;
    fn.setBold( b );
    update();
}

void QTextEditFormat::setItalic( bool b )
{
    if ( b == fn.italic() )
	return;
    fn.setItalic( b );
    update();
}

void QTextEditFormat::setUnderline( bool b )
{
    if ( b == fn.underline() )
	return;
    fn.setUnderline( b );
    update();
}

void QTextEditFormat::setFamily( const QString &f )
{
    if ( f == fn.family() )
	return;
    fn.setFamily( f );
    update();
}

void QTextEditFormat::setPointSize( int s )
{
    if ( s == fn.pointSize() )
	return;
    fn.setPointSize( s );
    update();
}

void QTextEditFormat::setFont( const QFont &f )
{
    if ( f == fn )
	return;
    fn = f;
    update();
}

void QTextEditFormat::setColor( const QColor &c )
{
    if ( c == col )
	return;
    col = c;
}

static int makeLogicFontSize( int s )
{
    int defSize = QApplication::font().pointSize();
    if ( s < defSize - 4 )
	return 1;
    if ( s < defSize )
	return 2;
    if ( s < defSize + 4 )
	return 3;
    if ( s < defSize + 8 )
	return 4;
    if ( s < defSize + 12 )
	return 5;
    if (s < defSize + 16 )
	return 6;
    return 7;
}

static QTextEditFormat *defaultFormat = 0;

QString QTextEditFormat::makeFormatChangeTags( QTextEditFormat *f ) const
{
    if ( !defaultFormat )
	defaultFormat = new QTextEditFormat( QApplication::font(),
					     QApplication::palette().color( QPalette::Normal, QColorGroup::Text ) );

    QString tag;
    if ( f ) {
	if ( f->font() != defaultFormat->font() ||
	     f->color().rgb() != defaultFormat->color().rgb() )
	    tag += "</font>";
	if ( f->font() != defaultFormat->font() ) {
	    if ( f->font().underline() && f->font().underline() != defaultFormat->font().underline() )
		tag += "</u>";
	    if ( f->font().italic() && f->font().italic() != defaultFormat->font().italic() )
		tag += "</i>";
	    if ( f->font().bold() && f->font().bold() != defaultFormat->font().bold() )
		tag += "</b>";
	}
    }

    if ( font() != defaultFormat->font() ) {
	if ( font().bold() && font().bold() != defaultFormat->font().bold() )
	    tag += "<b>";
	if ( font().italic() && font().italic() != defaultFormat->font().italic() )
	    tag += "<i>";
	if ( font().underline() && font().underline() != defaultFormat->font().underline() )
	    tag += "<u>";
    }
    if ( font() != defaultFormat->font() ||
	 color().rgb() != defaultFormat->color().rgb() ) {
	tag += "<font ";
	if ( font().family() != defaultFormat->font().family() )
	    tag +="face=\"" + fn.family() + "\" ";
	if ( font().pointSize() != defaultFormat->font().pointSize() )
	    tag +="size=\"" + QString::number( makeLogicFontSize( fn.pointSize() ) ) + "\" ";
	if ( color().rgb() != defaultFormat->color().rgb() )
	    tag +="color=\"" + col.name() + "\" ";
	tag += ">";
    }

    return tag;
}

QString QTextEditFormat::makeFormatEndTags() const
{
    if ( !defaultFormat )
	defaultFormat = new QTextEditFormat( QApplication::font(),
					     QApplication::palette().color( QPalette::Normal, QColorGroup::Text ) );

    QString tag;
    if ( font() != defaultFormat->font() ||
	 color().rgb() != defaultFormat->color().rgb() )
	tag += "</font>";
    if ( font() != defaultFormat->font() ) {
	if ( font().underline() && font().underline() != defaultFormat->font().underline() )
	    tag += "</u>";
	if ( font().italic() && font().italic() != defaultFormat->font().italic() )
	    tag += "</i>";
	if ( font().bold() && font().bold() != defaultFormat->font().bold() )
	    tag += "</b>";
    }
    return tag;
}


