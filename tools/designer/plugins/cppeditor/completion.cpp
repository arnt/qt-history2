#include "completion.h"
#include "paragdata.h"
#include "editor.h"
#include "qlistbox.h"
#include "qvbox.h"
#include "qmap.h"
#include <qrichtext_p.h>
#include "qapplication.h"
#include "qregexp.h"
#include "syntaxhighliter_cpp.h"
#include "qlabel.h"

static QMap<QChar, QStringList> *completionMap = 0;

EditorCompletion::EditorCompletion( Editor *e )
{
    completionPopup = new QVBox( 0, 0, WType_Popup );
    completionPopup->setFrameStyle( QFrame::Box | QFrame::Plain );
    completionPopup->setLineWidth( 1 );
    functionLabel = new QLabel( 0, 0, WType_Popup );
    functionLabel->setBackgroundMode( QWidget::PaletteBase );
    functionLabel->setFrameStyle( QFrame::Box | QFrame::Plain );
    functionLabel->setLineWidth( 1 );
    functionLabel->hide();
    completionListBox = new QListBox( completionPopup );
    completionListBox->setFrameStyle( QFrame::NoFrame );
    completionListBox->installEventFilter( this );
    completionPopup->installEventFilter( this );
    functionLabel->installEventFilter( this );
    completionPopup->setFocusProxy( completionListBox );
    completionOffset = 0;
    curEditor = e;
    curEditor->installEventFilter( this );
    if ( !completionMap ) {
	completionMap = new QMap<QChar, QStringList >();
	int i = 0;
	while ( SyntaxHighlighter_CPP::keywords[ i ] != QString::null )
	    addCompletionEntry( SyntaxHighlighter_CPP::keywords[ i++ ], 0 );
    }
}

void EditorCompletion::addCompletionEntry( const QString &s, QTextDocument * )
{
    if ( !completionMap )
	completionMap = new QMap<QChar, QStringList >();
    QChar key( s[ 0 ] );
    QMap<QChar, QStringList>::Iterator it = completionMap->find( key );
    if ( it == completionMap->end() )
	completionMap->insert( key, QStringList( s ) );
    else
	( *it ).append( s );
}

QStringList EditorCompletion::completionList( const QString &s, QTextDocument *doc ) const
{
    if ( doc )
	( (EditorCompletion*)this )->updateCompletionMap( doc );

    QChar key( s[ 0 ] );
    QMap<QChar, QStringList>::ConstIterator it = completionMap->find( key );
    if ( it == completionMap->end() )
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

void EditorCompletion::updateCompletionMap( QTextDocument *doc )
{
    QTextParag *s = doc->firstParag();
    if ( !s->extraData() )
	s->setExtraData( (void*)new ParagData );
    while ( s ) {
	if ( s->length() == ( (ParagData*)s->extraData() )->lastLengthForCompletion ) {
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
		addCompletionEntry( buffer, doc );
		buffer = QString::null;
	    }
	}
	if ( !buffer.isEmpty() )
	    addCompletionEntry( buffer, doc );

	( (ParagData*)s->extraData() )->lastLengthForCompletion = s->length();
	s = s->next();
    }
}

bool EditorCompletion::doCompletion()
{
    searchString = "";
    if ( !curEditor )
	return FALSE;

    QTextCursor *cursor = curEditor->textCursor();
    QTextDocument *doc = curEditor->document();

    if ( cursor->index() > 0 && cursor->parag()->at( cursor->index() - 1 )->c == '.' )
	return doObjectCompletion();

    int idx = cursor->index();
    if ( idx == 0 )
	return FALSE;
    QChar c = cursor->parag()->at( idx - 1 )->c;
    if ( !c.isLetter() && !c.isNumber() && c != '_' && c != '#' )
	return FALSE;

    QString s;
    idx--;
    completionOffset = 1;
    while ( TRUE ) {
	s.prepend( QString( cursor->parag()->at( idx )->c ) );
	idx--;
	if ( idx < 0 )
	    break;
	if ( !cursor->parag()->at( idx )->c.isLetter() &&
	     !cursor->parag()->at( idx )->c.isNumber() &&
	     cursor->parag()->at( idx )->c != '_' &&
	     cursor->parag()->at( idx )->c != '#' )
	    break;
	completionOffset++;
    }

    searchString = s;

    QStringList lst( completionList( s, doc ) );
    if ( lst.count() > 1 ) {
	QTextStringChar *chr = cursor->parag()->at( cursor->index() );
	int h = cursor->parag()->lineHeightOfChar( cursor->index() );
	int x = cursor->parag()->rect().x() + chr->x;
	int y, dummy;
	cursor->parag()->lineHeightOfChar( cursor->index(), &dummy, &y );
	y += cursor->parag()->rect().y();
	completionListBox->clear();
	completionListBox->insertStringList( lst );
	cList = lst;
	completionPopup->resize( completionListBox->sizeHint() + QSize( 4, 4 ) );
	completionListBox->setCurrentItem( 0 );
	completionListBox->setFocus();
	completionPopup->move( curEditor->mapToGlobal( curEditor->contentsToViewport( QPoint( x, y + h ) ) ) );
	completionPopup->show();
    } else if ( lst.count() == 1 ) {
	curEditor->insert( lst.first().mid( completionOffset, 0xFFFFFF ), TRUE );
    } else {
	return FALSE;
    }

    return TRUE;
}

bool EditorCompletion::eventFilter( QObject *o, QEvent *e )
{
    if ( o->inherits( "Editor" ) && e->type() == QEvent::KeyPress ) {
	curEditor = (Editor*)o;
	QKeyEvent *ke = (QKeyEvent*)e;
	if ( ke->text().length() && !( ke->state() & AltButton ) &&
	     ( !ke->ascii() || ke->ascii() >= 32 ) ||
	     ( ke->text() == "\t" && !( ke->state() & ControlButton ) ) ) {
	    if ( ke->key() == Key_Tab ) {
		if ( curEditor->textCursor()->index() == 0 &&
		     curEditor->textCursor()->parag()->style() &&
		     curEditor->textCursor()->parag()->style()->displayMode() == QStyleSheetItem::DisplayListItem )
		    return FALSE;
		if ( doCompletion() )
			return TRUE;
	    } else if ( ke->key() == Key_Period ) {
		doObjectCompletion();
	    } else {
		if ( !doArgumentHint( ke->text() == "(" ) )
		    functionLabel->hide();
	    }
	}
    } else if ( o == completionPopup || o == completionListBox ||
	 o == completionListBox->viewport() ) {
	if ( e->type() == QEvent::KeyPress ) {
	    QKeyEvent *ke = (QKeyEvent*)e;
	    if ( ke->key() == Key_Enter || ke->key() == Key_Return ||
		 ke->key() == Key_Tab ) {
		int idx = curEditor->textCursor()->index();
		QString s = completionListBox->currentText().mid( searchString.length() );
		curEditor->insert( s, TRUE );
		int i = s.find( '(' );
		completionPopup->close();
		curEditor->setFocus();
		if ( i != -1 && i < (int)s.length() ) {
		    curEditor->setCursorPosition( curEditor->textCursor()->parag()->paragId(), idx + i + 1 );
		    doArgumentHint( FALSE );
		}
		return TRUE;
	    } else if ( ke->key() == Key_Left || ke->key() == Key_Right ||
			ke->key() == Key_Up || ke->key() == Key_Down ||
			ke->key() == Key_Home || ke->key() == Key_End ||
			ke->key() == Key_Prior || ke->key() == Key_Next ) {
		return FALSE;
	    } else if ( ke->key() != Key_Shift && ke->key() != Key_Control &&
			ke->key() != Key_Alt ) {
		int l = searchString.length();
		if ( ke->key() == Key_Backspace ) {
		    searchString.remove( searchString.length() - 1, 1 );
		} else {
		    searchString += ke->text();
		    l = 1;
		}
		if ( !l || !continueComplete() ) {
		    completionPopup->close();
		    curEditor->setFocus();
		}
		QApplication::sendEvent( curEditor, e );
		return TRUE;
	    }
	}
    }
    if ( o == functionLabel || o->inherits( "Editor" ) && functionLabel->isVisible() ) {
	if ( e->type() == QEvent::KeyPress ) {
	    QKeyEvent *ke = (QKeyEvent*)e;
	    if ( ke->key() == Key_Escape ) {
		functionLabel->hide();
	    } else {
		if ( !doArgumentHint( ke->text() == "(" ) )
		    functionLabel->hide();
		if ( o == functionLabel ) {
		    QApplication::sendEvent( curEditor, e );
		    return TRUE;
		}
	    }
	}
    }
    return FALSE;
}

void EditorCompletion::setCurrentEdior( Editor *e )
{
    curEditor = e;
    curEditor->installEventFilter( this );
}

void EditorCompletion::addEditor( Editor *e )
{
    e->installEventFilter( this );
}

bool EditorCompletion::doObjectCompletion()
{
    return FALSE;
}

static void strip( QString &txt )
{
    int i = txt.find( "(" );
    if ( i == -1 )
	return;
    txt = txt.left( i );
}

bool EditorCompletion::continueComplete()
{
    if ( searchString.isEmpty() ) {
	completionListBox->clear();
	completionListBox->insertStringList( cList );
	completionListBox->setCurrentItem( 0 );
	completionListBox->setSelected( completionListBox->currentItem(), TRUE );
	return TRUE;
    }

    QListBoxItem *i = completionListBox->findItem( searchString );
    if ( !i )
	return FALSE;

    QString txt1 = i->text();
    QString txt2 = searchString;
    strip( txt1 );
    strip( txt2 );
    if ( txt1 == txt2 )
	return FALSE;

    QStringList res = cList.grep( QRegExp( "^" + searchString ) );
    completionListBox->clear();
    completionListBox->insertStringList( res );
    completionListBox->setCurrentItem( 0 );
    completionListBox->setSelected( completionListBox->currentItem(), TRUE );
    return TRUE;
}

bool EditorCompletion::doArgumentHint( bool )
{
    return FALSE;
}

QStringList EditorCompletion::functionParameters( const QString &, QChar & )
{
    return QStringList();
}

void EditorCompletion::setContext( QObjectList *, QObject * )
{
}
