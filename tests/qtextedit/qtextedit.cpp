#include "qtextedit.h"
#include "qtexteditintern_p.h"

#include <qpainter.h>
#include <qpen.h>
#include <qbrush.h>
#include <qpixmap.h>
#include <qfont.h>
#include <qcolor.h>
#include <qsize.h>
#include <qevent.h>
#include <qtimer.h>
#include <qapplication.h>
#include <qlistbox.h>
#include <qvbox.h>
#include <qapplication.h>
#include <qclipboard.h>
#include <qcolordialog.h>
#include <qfontdialog.h>

QPixmap *QTextEdit::bufferPixmap( const QSize &s )
{
    if ( !buf_pixmap ) {
	buf_pixmap = new QPixmap( s );
    } else {
	if ( buf_pixmap->width() < s.width() ||
	     buf_pixmap->height() < s.height() ) {
	    buf_pixmap->resize( QMAX( s.width(), buf_pixmap->width() ),
				QMAX( s.height(), buf_pixmap->height() ) );
	}
    }
    return buf_pixmap;
}

QTextEdit::QTextEdit( QWidget *parent, const QString &fn, bool tabify )
    : QScrollView( parent, "", WNorthWestGravity | WRepaintNoErase ),
      doc( new QTextEditDocument( fn, tabify ) ), undoRedoInfo( doc )
{
    init();
}

QTextEdit::QTextEdit( QWidget *parent, const QString &text )
    : QScrollView( parent, "", WNorthWestGravity | WRepaintNoErase ),
      doc( new QTextEditDocument( QString::null, FALSE ) ), undoRedoInfo( doc )
{
    setText( text );
    init();
}

QTextEdit::~QTextEdit()
{
    if ( painter.isActive() )
	painter.end();
    delete buf_pixmap;
}

void QTextEdit::init()
{
    buf_pixmap = 0;
    doubleBuffer = 0;
    drawAll = TRUE;
    mousePressed = FALSE;
    inDoubleClick = FALSE;

    doc->setFormatter( new QTextEditFormatterBreakWords( doc ) );
    currentFormat = doc->formatCollection()->defaultFormat();
    currentAlignment = Qt::AlignLeft;
    currentParagType = QTextEditParag::Normal;

    viewport()->setBackgroundMode( PaletteBase );
    resizeContents( 0, doc->lastParag() ?
		    ( doc->lastParag()->paragId() + 1 ) * doc->formatCollection()->defaultFormat()->height() : 0 );
    
    setKeyCompression( TRUE );
    setHScrollBarMode( AlwaysOff );
    setVScrollBarMode( AlwaysOn );
    viewport()->setMouseTracking( TRUE );
    viewport()->setCursor( ibeamCursor );
    viewport()->setFocusPolicy( WheelFocus );

    cursor = new QTextEditCursor( doc );
    
    formatTimer = new QTimer( this );
    connect( formatTimer, SIGNAL( timeout() ),
	     this, SLOT( formatMore() ) );
    lastFormatted = doc->firstParag();
    
    scrollTimer = new QTimer( this );
    connect( scrollTimer, SIGNAL( timeout() ),
	     this, SLOT( doAutoScroll() ) );
    
    interval = 0;
    changeIntervalTimer = new QTimer( this );
    connect( changeIntervalTimer, SIGNAL( timeout() ),
	     this, SLOT( doChangeInterval() ) );

    cursorVisible = TRUE;
    blinkTimer = new QTimer( this );
    connect( blinkTimer, SIGNAL( timeout() ),
	     this, SLOT( blinkCursor() ) );
    
    formatMore();

    completionPopup = new QVBox( this, 0, WType_Popup );
    completionPopup->setFrameStyle( QFrame::Box | QFrame::Plain );
    completionPopup->setLineWidth( 1 );
    completionListBox = new QListBox( completionPopup );
    completionListBox->setFrameStyle( QFrame::NoFrame );
    completionListBox->installEventFilter( this );
    completionPopup->installEventFilter( this );
    completionPopup->setFocusProxy( completionListBox );
    completionOffset = 0;

    blinkCursorVisible = FALSE;
    blinkTimer->start( QApplication::cursorFlashTime() / 2 );
}

void QTextEdit::drawContents( QPainter *p, int cx, int cy, int cw, int ch )
{
    if ( !doc->firstParag() )
	return;

    QTextEditParag *parag = doc->firstParag();
    QSize s( doc->firstParag()->rect().size() );

    p->fillRect( contentsX(), contentsY(), visibleWidth(), doc->y(),
		 colorGroup().color( QColorGroup::Base ) );

    if ( !doubleBuffer ) {
	doubleBuffer = bufferPixmap( s );
	if ( painter.isActive() )
	    painter.end();
	painter.begin( doubleBuffer );
    }

    if ( !painter.isActive() )
	painter.begin( doubleBuffer );

    while ( parag ) {
	lastFormatted = parag;
	if ( !parag->isValid() )
	    parag->format();
	
	if ( !parag->rect().intersects( QRect( cx, cy, cw, ch ) ) ) {
	    if ( parag->rect().y() > cy + ch )
		return;
	    parag = parag->next();
	    continue;
	}
	
	if ( !parag->hasChanged() && !drawAll ) {
	    parag = parag->next();
	    continue;
	}
	
	parag->setChanged( FALSE );
 	QSize s( parag->rect().size() );
	if ( s.width() > doubleBuffer->width() ||
	     s.height() > doubleBuffer->height() ) {
	    if ( painter.isActive() )
		painter.end();
	    doubleBuffer = bufferPixmap( s );
	    painter.begin( doubleBuffer );
	}
	painter.fillRect( QRect( 0, 0, s.width(), s.height() ),
			  colorGroup().color( QColorGroup::Base ) );
	
	parag->paint( painter, colorGroup(), cursor, TRUE );
	
	p->drawPixmap( parag->rect().topLeft(), *doubleBuffer, QRect( QPoint( 0, 0 ), s ) );
	if ( parag->rect().x() + parag->rect().width() < contentsX() + contentsWidth() )
	    p->fillRect( parag->rect().x() + parag->rect().width(), parag->rect().y(),
			 ( contentsX() + contentsWidth() ) - ( parag->rect().x() + parag->rect().width() ),
			 parag->rect().height(), colorGroup().brush( QColorGroup::Base ) );
	parag = parag->next();
    }

    parag = doc->lastParag();
    if ( parag->rect().y() + parag->rect().height() - contentsY() < visibleHeight() )
	p->fillRect( 0, parag->rect().y() + parag->rect().height(), contentsWidth(),
		     visibleHeight() - ( parag->rect().y() + parag->rect().height() ),
		     colorGroup().brush( QColorGroup::Base ) );
}

void QTextEdit::keyPressEvent( QKeyEvent *e )
{
    changeIntervalTimer->stop();
    interval = 10;

    bool selChanged = FALSE;
    for ( int i = 1; i < doc->numSelections; ++i ) // start with 1 as we don't want to remove the Standard-Selection
	selChanged = doc->removeSelection( i ) || selChanged;

    if ( selChanged ) {
	repaintChanged();
    }

    bool clearUndoRedoInfo = TRUE;

    switch ( e->key() ) {
    case Key_Left:
	moveCursor( MoveLeft, e->state() & ShiftButton, e->state() & ControlButton );
	break;
    case Key_Right:
	moveCursor( MoveRight, e->state() & ShiftButton, e->state() & ControlButton );
	break;
    case Key_Up:
	moveCursor( MoveUp, e->state() & ShiftButton, e->state() & ControlButton );
	break;
    case Key_Down:
	moveCursor( MoveDown, e->state() & ShiftButton, e->state() & ControlButton );
	break;
    case Key_Home:
	moveCursor( MoveHome, e->state() & ShiftButton, e->state() & ControlButton );
	break;
    case Key_End:
	moveCursor( MoveEnd, e->state() & ShiftButton, e->state() & ControlButton );
	break;
    case Key_Prior:
	moveCursor( MovePgUp, e->state() & ShiftButton, e->state() & ControlButton );
	break;
    case Key_Next:
	moveCursor( MovePgDown, e->state() & ShiftButton, e->state() & ControlButton );
	break;
    case Key_Return: case Key_Enter:
	doc->removeSelection( QTextEditDocument::Standard );
	// ########### should be optimized to use QWidget::scroll here
	// ########### this will make it MUCH faster!
	clearUndoRedoInfo = FALSE;
	doKeyboardAction( ActionReturn );
	break;
    case Key_Delete:
	if ( doc->hasSelection( QTextEditDocument::Standard ) ) {
	    removeSelectedText();
	    break;
	}

	doKeyboardAction( ActionDelete );
	clearUndoRedoInfo = FALSE;
	
	break;
    case Key_Backspace:
	if ( doc->hasSelection( QTextEditDocument::Standard ) ) {
	    removeSelectedText();
	    break;
	}

	if ( !cursor->parag()->prev() &&
	     cursor->atParagStart() )
	    break;
	
	doKeyboardAction( ActionBackspace );
	clearUndoRedoInfo = FALSE;
	
	break;
    default: {
	    if ( e->text().length() &&
		   ( e->ascii() >= 32 || ( e->text() == "\t" &&
					   !( e->state() & ControlButton && e->key() == Key_I ) ) ) &&
					   !( e->state() & AltButton ) ) {
		clearUndoRedoInfo = FALSE;
		if ( e->key() == Key_Tab ) {
		    if ( cursor->index() == 0 && cursor->parag()->type() != QTextEditParag::Normal ) {
			cursor->parag()->setListDepth( cursor->parag()->listDepth() + 1 );
			drawCursor( FALSE );
			repaintChanged();
			drawCursor( TRUE );
			break;
		    }
		    if ( doCompletion() )
			break;
		}
		
		if ( cursor->parag()->type() != QTextEditParag::BulletList &&
		     cursor->index() == 0 && ( e->text() == "-" || e->text() == "*" ) ) {
		    setParagType( (int)QTextEditParag::BulletList );
		} else {
		    insert( e->text(), TRUE );
		}
		break;
	    }
	    if ( e->state() & ControlButton ) {
		switch ( e->key() ) {
		case Key_C:
		    copy();
		    break;
		case Key_V:
		    paste();
		    break;
		case Key_X: {
		    cut();
		} break;
		case Key_I: case Key_T: case Key_Tab:
		    indent();
		    break;
		case Key_A:
		    moveCursor( MoveHome, e->state() & ShiftButton, FALSE );
		    break;
		case Key_E:
		    moveCursor( MoveEnd, e->state() & ShiftButton, FALSE );
		    break;
		case Key_Z:
		    undo();
		    break;
		case Key_Y:
		    redo();
		    break;
		}
		break;
	    } else if ( e->state() & AltButton && !doc->syntaxHighlighter() ) { // ############ just for testing
		switch ( e->key() ) {
		case Key_B: {
		    setBold( !currentFormat->font().bold() );
		} break;
		case Key_I: {
		    setItalic( !currentFormat->font().italic() );
		} break;
		case Key_U: {
		    setUnderline( !currentFormat->font().underline() );
		} break;
		case Key_F: {
		    QFont font = currentFormat->font();
		    bool ok;
		    font = QFontDialog::getFont( &ok, font, this );
		    if ( ok ) {
			setFont( font );
		    }
		} break;
		case Key_C: {
		    QColor col = currentFormat->color();
		    col = QColorDialog::getColor( col, this );
		    if ( col.isValid() ) {
			setColor( col );
		    }
		} break;
		case Key_L: {
		    if ( cursor->parag()->type() != QTextEditParag::BulletList )
			setParagType( (int)QTextEditParag::BulletList );
		    else
			setParagType( (int)QTextEditParag::Normal );
		} break;
		case Key_Q:
		    setAlignment( Qt::AlignLeft );
		    break;
		case Key_W:
		    setAlignment( Qt::AlignHCenter );
		    break;
		case Key_E:
		    setAlignment( Qt::AlignRight );
		    break;
		case Key_S:
		    if ( !find( "QString", FALSE, FALSE ) )
			QApplication::beep();
		    break;
		}
	    }
    }
    }

    if ( clearUndoRedoInfo )
	undoRedoInfo.clear();

    changeIntervalTimer->start( 100, TRUE );
}

void QTextEdit::doKeyboardAction( int action )
{
    lastFormatted = cursor->parag();
    drawCursor( FALSE );
	
    switch ( action ) {
    case ActionDelete:
	checkUndoRedoInfo( UndoRedoInfo::Delete );
	if ( !undoRedoInfo.valid() ) {
	    undoRedoInfo.id = cursor->parag()->paragId();
	    undoRedoInfo.index = cursor->index();
	    undoRedoInfo.text = QString::null;
	}
	undoRedoInfo.text += cursor->parag()->at( cursor->index() )->c;
	if ( cursor->remove() )
	    undoRedoInfo.text += "\n";
	break;
    case ActionBackspace:
	if ( cursor->parag()->type() != QTextEditParag::Normal && cursor->index() == 0 ) {
	    if ( cursor->parag()->listDepth() > 0 )
		cursor->parag()->setListDepth( cursor->parag()->listDepth() - 1 );
	    else
		cursor->parag()->setType( QTextEditParag::Normal );
	    lastFormatted = cursor->parag();
	    repaintChanged();
	    drawCursor( TRUE );
	    return;
	}
	checkUndoRedoInfo( UndoRedoInfo::Delete );
	if ( !undoRedoInfo.valid() ) {
	    undoRedoInfo.id = cursor->parag()->paragId();
	    undoRedoInfo.index = cursor->index();
	    undoRedoInfo.text = QString::null;
	}
	cursor->gotoLeft();
	undoRedoInfo.text.prepend( QString( cursor->parag()->at( cursor->index() )->c ) );
	undoRedoInfo.index = cursor->index();
	if ( cursor->remove() ) {
	    undoRedoInfo.text.remove( 0, 1 );
	    undoRedoInfo.text.prepend( "\n" );
	    undoRedoInfo.index = cursor->index();
	    undoRedoInfo.id = cursor->parag()->paragId();
	}
	lastFormatted = cursor->parag();
	break;
    case ActionReturn:
	checkUndoRedoInfo( UndoRedoInfo::Return );
	if ( !undoRedoInfo.valid() ) {
	    undoRedoInfo.id = cursor->parag()->paragId();
	    undoRedoInfo.index = cursor->index();
	    undoRedoInfo.text = QString::null;
	}
	undoRedoInfo.text += "\n";
	cursor->splitAndInsertEmtyParag();
 	if ( cursor->parag()->prev() )
 	    lastFormatted = cursor->parag()->prev();
	break;
    }

    formatMore();
    repaintChanged();
    ensureCursorVisible();
    drawCursor( TRUE );

    updateCurrentFormat();
}

void QTextEdit::removeSelectedText()
{
    drawCursor( FALSE );
    checkUndoRedoInfo( UndoRedoInfo::RemoveSelected );
    if ( !undoRedoInfo.valid() ) {
	doc->selectionStart( QTextEditDocument::Standard, undoRedoInfo.id, undoRedoInfo.index );
	undoRedoInfo.text = QString::null;
    }
    undoRedoInfo.text = doc->selectedText( QTextEditDocument::Standard );
    doc->removeSelectedText( QTextEditDocument::Standard, cursor );
    ensureCursorVisible();
    lastFormatted = cursor->parag();
    formatMore();
    repaintChanged();
    ensureCursorVisible();
    drawCursor( TRUE );
    undoRedoInfo.clear();
}

void QTextEdit::moveCursor( int direction, bool shift, bool control )
{
    drawCursor( FALSE );
    if ( shift ) {
	if ( !doc->hasSelection( QTextEditDocument::Standard ) )
	    doc->setSelectionStart( QTextEditDocument::Standard, cursor );
	moveCursor( direction, control );
	if ( doc->setSelectionEnd( QTextEditDocument::Standard, cursor ) ) {
	    repaintChanged();
	} else {
	    drawCursor( TRUE );
	}
	ensureCursorVisible();
    } else {
	bool redraw = doc->removeSelection( QTextEditDocument::Standard );
	moveCursor( direction, control );
	if ( !redraw ) {
	    ensureCursorVisible();
	    drawCursor( TRUE );
	} else {
	    repaintChanged();
	    ensureCursorVisible();
	    drawCursor( TRUE );
	}
    }

    updateCurrentFormat();
}

void QTextEdit::moveCursor( int direction, bool control )
{
    switch ( direction ) {
    case MoveLeft: {
	if ( !control )
	    cursor->gotoLeft();
	else
	    cursor->gotoWordLeft();
    } break;
    case MoveRight: {
	if ( !control )
	    cursor->gotoRight();
	else
	    cursor->gotoWordRight();
    } break;
    case MoveUp: {
	if ( !control )
	    cursor->gotoUp();
	else
	    cursor->gotoPageUp( this );
    } break;
    case MoveDown: {
	if ( !control )
	    cursor->gotoDown();
	else
	    cursor->gotoPageDown( this );
    } break;
    case MoveHome: {
	if ( !control )
	    cursor->gotoLineStart();
	else
	    cursor->gotoHome();
    } break;
    case MoveEnd: {
	if ( !control )
	    cursor->gotoLineEnd();
	else
	    cursor->gotoEnd();
    } break;
    case MovePgUp:
	cursor->gotoPageUp( this );
	break;
    case MovePgDown:
	cursor->gotoPageDown( this );
	break;
    }

    updateCurrentFormat();
}

void QTextEdit::resizeEvent( QResizeEvent *e )
{
    QScrollView::resizeEvent( e );
    doResize();
}

void QTextEdit::ensureCursorVisible()
{
    lastFormatted = cursor->parag();
    formatMore();
    QTextEditString::Char *chr = cursor->parag()->at( cursor->index() );
    int h = cursor->parag()->lineHeightOfChar( cursor->index() );
    int x = cursor->parag()->rect().x() + chr->x;
    int y = 0; int dummy;
    cursor->parag()->lineHeightOfChar( cursor->index(), &dummy, &y );
    y += cursor->parag()->rect().y();
    int w = 2;
    ensureVisible( x, y + h / 2, w, h / 2 + 2 );
}

void QTextEdit::drawCursor( bool visible )
{
    if ( !cursor->parag()->isValid() )
	return;
    
    cursorVisible = visible;
    
    QPainter p;
    p.begin( viewport() );
    p.translate( -contentsX(), -contentsY() );

    cursor->parag()->format();
    QSize s( cursor->parag()->rect().size() );

    if ( !doubleBuffer ) {
	doubleBuffer = bufferPixmap( s );
	if ( painter.isActive() )
	    painter.end();
	painter.begin( doubleBuffer );
    }

    if ( !painter.isActive() )
	painter.begin( doubleBuffer );

    QTextEditString::Char *chr = cursor->parag()->at( cursor->index() );

    painter.setPen( QPen( chr->format->color() ) );
    painter.setFont( chr->format->font() );
    int h = 0; int y = 0;
    int bl = 0;
    int cw = chr->format->width( chr->c );
    h = cursor->parag()->lineHeightOfChar( cursor->index(), &bl, &y );

    bool fill = TRUE;
    if ( cursor->parag()->hasSelection( QTextEditDocument::Standard ) ) {
	if ( cursor->parag()->selectionStart( QTextEditDocument::Standard ) <= cursor->index() &&
	     cursor->parag()->selectionEnd( QTextEditDocument::Standard ) > cursor->index() ) {
	    painter.setPen( QPen( colorGroup().color( QColorGroup::HighlightedText ) ) );
	    painter.fillRect( chr->x, y, cw, h,
			      doc->selectionColor( QTextEditDocument::Standard ) );
	    fill = FALSE;
	}
    }

    if ( fill )
	painter.fillRect( chr->x, y, cw, h,
			  colorGroup().color( QColorGroup::Base ) );

    if ( chr->c != '\t' )
	painter.drawText( chr->x, y + bl, chr->c );

    if ( visible ) {
	int x = chr->x;
	int w = 2;
	painter.fillRect( QRect( x, y, w, h ), black );
    }
	
    p.drawPixmap( cursor->parag()->rect().topLeft() + QPoint( chr->x, y ), *doubleBuffer,
		  QRect( chr->x, y, cw, h ) );
    p.end();
}

void QTextEdit::contentsMousePressEvent( QMouseEvent *e )
{
    undoRedoInfo.clear();
    QTextEditCursor c = *cursor;
    mousePos = e->pos();

    if ( e->button() == LeftButton ) {
	mousePressed = TRUE;
	drawCursor( FALSE );
	placeCursor( e->pos() );
	ensureCursorVisible();

	bool redraw = FALSE;
	if ( doc->hasSelection( QTextEditDocument::Standard ) ) {
	    if ( !( e->state() & ShiftButton ) ) {
		redraw = doc->removeSelection( QTextEditDocument::Standard );
		doc->setSelectionStart( QTextEditDocument::Standard, cursor );
	    } else {
		redraw = doc->setSelectionEnd( QTextEditDocument::Standard, cursor ) || redraw;
	    }
	} else {
	    if ( !( e->state() & ShiftButton ) ) {
		doc->setSelectionStart( QTextEditDocument::Standard, cursor );
	    } else {
		doc->setSelectionStart( QTextEditDocument::Standard, &c );
		redraw = doc->setSelectionEnd( QTextEditDocument::Standard, cursor ) || redraw;
	    }
	}

	for ( int i = 1; i < doc->numSelections; ++i ) // start with 1 as we don't want to remove the Standard-Selection
	    redraw = doc->removeSelection( i ) || redraw;

	if ( !redraw ) {
	    drawCursor( TRUE );
	} else {
	    repaintChanged();
	}
    } else if ( e->button() == MidButton ) {
	paste();
    }
    updateCurrentFormat();
}

void QTextEdit::contentsMouseMoveEvent( QMouseEvent *e )
{
    if ( mousePressed ) {
	mousePos = e->pos();
	doAutoScroll();
	oldMousePos = mousePos;
    }
}

void QTextEdit::contentsMouseReleaseEvent( QMouseEvent * )
{
    if ( scrollTimer->isActive() )
	scrollTimer->stop();
    if ( mousePressed ) {
	if ( !doc->selectedText( QTextEditDocument::Standard ).isEmpty() )
	    doc->copySelectedText( QTextEditDocument::Standard );
	mousePressed = FALSE;
    }
    if ( cursor->checkParens() ) {
	repaintChanged();
    }
    updateCurrentFormat();
}

void QTextEdit::contentsMouseDoubleClickEvent( QMouseEvent * )
{
    QTextEditCursor c1 = *cursor;
    QTextEditCursor c2 = *cursor;
    c1.gotoWordLeft();
    c2.gotoWordRight();

    doc->setSelectionStart( QTextEditDocument::Standard, &c1 );
    doc->setSelectionEnd( QTextEditDocument::Standard, &c2 );

    *cursor = c2;

    repaintChanged();

    inDoubleClick = TRUE;
    mousePressed = TRUE;
}

void QTextEdit::doAutoScroll()
{
    if ( !mousePressed )
	return;

    QPoint pos( mapFromGlobal( QCursor::pos() ) );
    drawCursor( FALSE );
    QTextEditCursor oldCursor = *cursor;
    placeCursor( viewportToContents( pos ) );
    if ( inDoubleClick ) {
	QTextEditCursor cl = *cursor;
	cl.gotoWordLeft();
	QTextEditCursor cr = *cursor;
	cr.gotoWordRight();
	
	int diff = QABS( oldCursor.parag()->at( oldCursor.index() )->x - mousePos.x() );
	int ldiff = QABS( cl.parag()->at( cl.index() )->x - mousePos.x() );
	int rdiff = QABS( cr.parag()->at( cr.index() )->x - mousePos.x() );
	
	
	if ( cursor->parag()->lineStartOfChar( cursor->index() ) !=
	     oldCursor.parag()->lineStartOfChar( oldCursor.index() ) )
	    diff = 0xFFFFFF;
	
	if ( rdiff < diff && rdiff < ldiff )
	    *cursor = cr;
	else if ( ldiff < diff && ldiff < rdiff )
	    *cursor = cl;
	else
	    *cursor = oldCursor;
	
    }
    ensureCursorVisible();

    bool redraw = FALSE;
    if ( doc->hasSelection( QTextEditDocument::Standard ) ) {
	redraw = doc->setSelectionEnd( QTextEditDocument::Standard, cursor ) || redraw;
    }

    if ( !redraw ) {
	drawCursor( TRUE );
    } else {
	repaintChanged();
	drawCursor( TRUE );
    }

    if ( !scrollTimer->isActive() && pos.y() < 0 || pos.y() > height() )
	scrollTimer->start( 100, FALSE );
    else if ( scrollTimer->isActive() && pos.y() >= 0 && pos.y() <= height() )
	scrollTimer->stop();
}

void QTextEdit::placeCursor( const QPoint &pos, QTextEditCursor *c )
{
    if ( !c )
	c = cursor;

    QTextEditParag *s = doc->firstParag();
    QRect r;
    while ( s ) {
	r = s->rect();
	r.setWidth( contentsWidth() );
	if ( r.contains( pos ) )
	    break;
	s = s->next();
    }

    if ( !s )
	return;

    c->setParag( s );
    int y = s->rect().y();
    int lines = s->lines();
    QTextEditString::Char *chr = 0, *c2;
    int index;
    int i = 0;
    int cy;
    int ch;
    for ( ; i < lines; ++i ) {
	chr = s->lineStartOfLine( i, &index );
	cy = s->lineY( i );
	ch = s->lineHeight( i );
 	if ( !chr )
 	    return;
	if ( pos.y() >= y + cy && pos.y() <= y + cy + ch )
	    break;
    }

    c2 = chr;
    i = index;
    int x = s->rect().x(), last = index;
    int lastw = 0;
    int h = ch;
    int bl;
    int cw;
    while ( TRUE ) {
	if ( c2->lineStart )
	    h = s->lineHeightOfChar( i, &bl, &cy );
	last = i;
	cw = c2->format->width( c2->c );
	if ( pos.x() >= x + c2->x - lastw && pos.x() <= x + c2->x + cw / 2 &&
	     pos.y() >= y + cy && pos.y() <= y + cy + h )
	    break;
	lastw = cw / 2;
	i++;
	if ( i < s->length() )
	    c2 = s->at( i );
	else
	    break;
    }

    cursor->setIndex( last );
}

void QTextEdit::formatMore()
{
    if ( !lastFormatted ) {
	return;
    }

    int bottom = contentsHeight();
    int lastBottom = -1;
    int to = !sender() ? 2 : 20;
    bool firstVisible = FALSE;
    QRect cr( contentsX(), contentsY(), visibleWidth(), visibleHeight() );
    for ( int i = 0; ( i < to || firstVisible ) && lastFormatted; ++i ) {
	lastFormatted->format();
	if ( i == 0 )
	    firstVisible = lastFormatted->rect().intersects( cr );
	else if ( firstVisible )
	    firstVisible = lastFormatted->rect().intersects( cr );
	bottom = QMAX( bottom, lastFormatted->rect().top() +
		       lastFormatted->rect().height() );
	lastBottom = lastFormatted->rect().top() + lastFormatted->rect().height();
	lastFormatted = lastFormatted->next();
	if ( lastFormatted )
	    lastBottom = -1;
    }

    if ( bottom > contentsHeight() )
	resizeContents( contentsWidth(), bottom );
    else if ( lastBottom != -1 && lastBottom < contentsHeight() )
	resizeContents( contentsWidth(), lastBottom );

    if ( lastFormatted ) {
	formatTimer->start( interval, TRUE );
    } else {
	interval = QMAX( 0, interval );
    }
}

void QTextEdit::doResize()
{	
    resizeContents( width() - verticalScrollBar()->width(), contentsHeight() );
    doc->setWidth( visibleWidth() - 1 );
    doc->invalidate();
    viewport()->repaint( FALSE );
    lastFormatted = doc->firstParag();
    interval = 0;
    formatMore();
}

void QTextEdit::doChangeInterval()
{
    if ( cursor->checkParens() ) {
	repaintChanged();
    }
	
    interval = 0;
}

bool QTextEdit::doCompletion()
{
    if ( !doc->isCompletionEnabled() )
	return FALSE;

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

    QStringList lst( doc->completionList( s ) );
    if ( lst.count() > 1 ) {
	QTextEditString::Char *chr = cursor->parag()->at( cursor->index() );
	int h = cursor->parag()->lineHeightOfChar( cursor->index() );
	int x = cursor->parag()->rect().x() + chr->x;
	int y, dummy;
	cursor->parag()->lineHeightOfChar( cursor->index(), &dummy, &y );
	y += cursor->parag()->rect().y();
	completionListBox->clear();
	completionListBox->insertStringList( lst );
	completionListBox->resize( completionListBox->sizeHint() );
	completionPopup->resize( completionListBox->size() );
	completionListBox->setCurrentItem( 0 );
	completionListBox->setFocus();
	completionPopup->move( mapToGlobal( contentsToViewport( QPoint( x, y + h ) ) ) );
	completionPopup->show();
    } else if ( lst.count() == 1 ) {
	insert( lst.first().mid( completionOffset, 0xFFFFFF ), TRUE );
    } else {
	return FALSE;
    }

    return TRUE;
}

bool QTextEdit::eventFilter( QObject *o, QEvent *e )
{
    if ( !o || !e )
	return TRUE;

    if ( o == completionPopup || o == completionListBox ||
	o == completionListBox->viewport() ) {
	if ( e->type() == QEvent::KeyPress ) {
	    QKeyEvent *ke = (QKeyEvent*)e;
	    if ( ke->key() == Key_Enter || ke->key() == Key_Return ||
		 ke->key() == Key_Tab ) {
		insert( completionListBox->currentText().
			mid( completionOffset, 0xFFFFFF ), TRUE );
		completionPopup->close();
		return TRUE;
	    } else if ( ke->key() == Key_Left || ke->key() == Key_Right ||
			ke->key() == Key_Up || ke->key() == Key_Down ||
			ke->key() == Key_Home || ke->key() == Key_End ||
			ke->key() == Key_Prior || ke->key() == Key_Next ) {
		return FALSE;
	    } else if ( ke->key() != Key_Shift && ke->key() != Key_Control &&
			ke->key() != Key_Alt ) {
		completionPopup->close();
		QApplication::sendEvent( this, e );
		return TRUE;
	    }
	}
    }

    return QScrollView::eventFilter( o, e );
}

void QTextEdit::insert( const QString &text, bool indent, bool checkNewLine )
{
    drawCursor( FALSE );
    if ( doc->hasSelection( QTextEditDocument::Standard ) ) {
	checkUndoRedoInfo( UndoRedoInfo::RemoveSelected );
	if ( !undoRedoInfo.valid() ) {
	    doc->selectionStart( QTextEditDocument::Standard, undoRedoInfo.id, undoRedoInfo.index );
	    undoRedoInfo.text = QString::null;
	}
	undoRedoInfo.text = doc->selectedText( QTextEditDocument::Standard );
	doc->removeSelectedText( QTextEditDocument::Standard, cursor );
    }
    checkUndoRedoInfo( UndoRedoInfo::Insert );
    if ( !undoRedoInfo.valid() ) {
	undoRedoInfo.id = cursor->parag()->paragId();
	undoRedoInfo.index = cursor->index();
	undoRedoInfo.text = QString::null;
    }
    lastFormatted = checkNewLine && cursor->parag()->prev() ?
		    cursor->parag()->prev() : cursor->parag();
    int idx = cursor->index();
    cursor->insert( text, checkNewLine );
    if ( !doc->syntaxHighlighter() )
	cursor->parag()->setFormat( idx, text.length(), currentFormat, TRUE );
		
    if ( indent && text == "{" || text == "}" )
	cursor->indent();
    formatMore();
    repaintChanged();
    ensureCursorVisible();
    drawCursor( TRUE );
    undoRedoInfo.text += text;
}

void QTextEdit::undo()
{
    undoRedoInfo.clear();
    drawCursor( FALSE );
    QTextEditCursor *c = doc->undo( cursor );
    if ( !c ) {
	drawCursor( TRUE );
	return;
    }
    ensureCursorVisible();
    repaintChanged();
    drawCursor( TRUE );
}

void QTextEdit::redo()
{
    undoRedoInfo.clear();
    drawCursor( FALSE );
    QTextEditCursor *c = doc->redo( cursor );
    if ( !c ) {
	drawCursor( TRUE );
	return;
    }
    ensureCursorVisible();
    repaintChanged();
    ensureCursorVisible();
    drawCursor( TRUE );
}

void QTextEdit::paste()
{
    QString s = QApplication::clipboard()->text();
    if ( !s.isEmpty() )
	insert( s, FALSE, TRUE );
}

void QTextEdit::checkUndoRedoInfo( UndoRedoInfo::Type t )
{
    if ( undoRedoInfo.valid() && t != undoRedoInfo.type )
	undoRedoInfo.clear();
    undoRedoInfo.type = t;
}

void QTextEdit::repaintChanged()
{
    drawAll = FALSE;
    viewport()->repaint( FALSE );
    drawAll = TRUE;
}

void QTextEdit::cut()
{
    if ( doc->hasSelection( QTextEditDocument::Standard ) ) {
	doc->copySelectedText( QTextEditDocument::Standard );
	removeSelectedText();
    }
}

void QTextEdit::copy()
{
    if ( !doc->selectedText( QTextEditDocument::Standard ).isEmpty() )
	doc->copySelectedText( QTextEditDocument::Standard );
}

void QTextEdit::indent()
{
    drawCursor( FALSE );
    cursor->indent();
    repaintChanged();
    drawCursor( TRUE );
}

bool QTextEdit::focusNextPrevChild( bool )
{
    return FALSE;
}

void QTextEdit::setFormat( QTextEditFormat *f, int flags )
{
    if ( doc->hasSelection( QTextEditDocument::Standard ) ) {
	drawCursor( FALSE );
	doc->setFormat( QTextEditDocument::Standard, f, flags );
	repaintChanged();
	formatMore();
	drawCursor( TRUE );
    }
    if ( currentFormat && currentFormat->key() != f->key() ) {
	currentFormat->removeRef();
	currentFormat = doc->formatCollection()->format( f );
	emit currentFontChanged( currentFormat->font() );
	emit currentColorChanged( currentFormat->color() );
    }
}

void QTextEdit::setParagType( int t )
{
    QTextEditParag::Type type = (QTextEditParag::Type)t;
    drawCursor( FALSE );
    if ( !doc->hasSelection( QTextEditDocument::Standard ) ) {
	cursor->parag()->setType( type );
	cursor->parag()->setListDepth( cursor->parag()->listDepth() );
	repaintChanged();
    } else {
	QTextEditParag *start = doc->selectionStart( QTextEditDocument::Standard );
	QTextEditParag *end = doc->selectionEnd( QTextEditDocument::Standard );
	lastFormatted = start;
	while ( start ) {
	    start->setType( type );
	    start->setListDepth( cursor->parag()->listDepth() );
	    if ( start == end )
		break;
	    start = start->next();
	}
	repaintChanged();
	formatMore();
    }
    drawCursor( TRUE );
    if ( currentParagType != t ) {
	currentParagType = t;
	emit currentParagTypeChanged( currentParagType );
    }
}

void QTextEdit::setAlignment( int a )
{
    drawCursor( FALSE );
    if ( !doc->hasSelection( QTextEditDocument::Standard ) ) {
	cursor->parag()->setAlignment( a );
	repaintChanged();
    } else {
	QTextEditParag *start = doc->selectionStart( QTextEditDocument::Standard );
	QTextEditParag *end = doc->selectionEnd( QTextEditDocument::Standard );
	lastFormatted = start;
	while ( start ) {
	    start->setAlignment( a );
	    if ( start == end )
		break;
	    start = start->next();
	}
	repaintChanged();
	formatMore();
    }
    drawCursor( TRUE );
    if ( currentAlignment != a ) {
	currentAlignment = a;
	emit currentAlignmentChanged( currentAlignment );
    }
}

void QTextEdit::updateCurrentFormat()
{
    int i = cursor->index();
    if ( i > 0 )
	--i;
    if ( currentFormat->key() != cursor->parag()->at( i )->format->key() && !doc->syntaxHighlighter() ) {
	if ( currentFormat )
	    currentFormat->removeRef();
	currentFormat = doc->formatCollection()->format( cursor->parag()->at( i )->format );
	emit currentFontChanged( currentFormat->font() );
	emit currentColorChanged( currentFormat->color() );
    }

    if ( currentAlignment != cursor->parag()->alignment() ) {
	currentAlignment = cursor->parag()->alignment();
	emit currentAlignmentChanged( currentAlignment );
    }

    if ( currentParagType != (int)cursor->parag()->type() ) {
	currentParagType = (int)cursor->parag()->type();
	emit currentParagTypeChanged( currentParagType );
    }
}

void QTextEdit::setItalic( bool b )
{
    QTextEditFormat f( *currentFormat );
    f.setItalic( b );
    setFormat( &f, QTextEditFormat::Italic );
}

void QTextEdit::setBold( bool b )
{
    QTextEditFormat f( *currentFormat );
    f.setBold( b );
    setFormat( &f, QTextEditFormat::Bold );
}

void QTextEdit::setUnderline( bool b )
{
    QTextEditFormat f( *currentFormat );
    f.setUnderline( b );
    setFormat( &f, QTextEditFormat::Underline );
}

void QTextEdit::setFamily( const QString &f_ )
{
    QTextEditFormat f( *currentFormat );
    f.setFamily( f_ );
    setFormat( &f, QTextEditFormat::Family );
}

void QTextEdit::setPointSize( int s )
{
    QTextEditFormat f( *currentFormat );
    f.setPointSize( s );
    setFormat( &f, QTextEditFormat::Size );
}

void QTextEdit::setColor( const QColor &c )
{
    QTextEditFormat f( *currentFormat );
    f.setColor( c );
    setFormat( &f, QTextEditFormat::Color );
}

void QTextEdit::setFont( const QFont &f_ )
{
    QTextEditFormat f( *currentFormat );
    f.setFont( f_ );
    setFormat( &f, QTextEditFormat::Font );
}

QString QTextEdit::text() const
{
    return doc->text();
}

QString QTextEdit::text( int parag, bool formatted ) const
{
    return doc->text( parag, formatted );
}

void QTextEdit::setText( const QString &txt )
{
    doc->setText( txt );
    cursor->setParag( doc->firstParag() );
    cursor->setIndex( 0 );
    viewport()->repaint( FALSE );
}

bool QTextEdit::find( const QString &expr, bool cs, bool wo, bool forward,
			   int *parag, int *index )
{
    drawCursor( FALSE );
    for ( int i = 0; i < doc->numSelections; ++i )
	doc->removeSelection( i );
    bool found = doc->find( expr, cs, wo, forward, parag, index, cursor );
    ensureCursorVisible();
    drawCursor( TRUE );
    repaintChanged();
    return found;
}

void QTextEdit::blinkCursor()
{
    if ( !cursorVisible )
	return;
    bool cv = cursorVisible;
    blinkCursorVisible = !blinkCursorVisible;
    drawCursor( blinkCursorVisible );
    cursorVisible = cv;
}

void QTextEdit::UndoRedoInfo::clear()
{
    if ( valid() ) {
	if ( type == Insert || type == Return )
	    doc->addCommand( new QTextEditInsertCommand( doc, id, index, text ) );
	else if ( type != Invalid )
	    doc->addCommand( new QTextEditDeleteCommand( doc, id, index, text ) );
    }
    text = QString::null;
    id = -1;
    index = -1;
}

