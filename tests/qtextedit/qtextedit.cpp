#include "qtextedit.h"
#include "qrichtext_p.h"

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
#include <qstylesheet.h>
#include <qdragobject.h>

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

QTextEdit::QTextEdit( QWidget *parent, const char *name )
    : QScrollView( parent, name, WNorthWestGravity | WRepaintNoErase ),
      doc( new QTextDocument( 0 ) ), undoRedoInfo( doc )
{
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
    firstResize = TRUE;
    buf_pixmap = 0;
    doubleBuffer = 0;
    drawAll = TRUE;
    mousePressed = FALSE;
    inDoubleClick = FALSE;
    readOnly = FALSE;
    modified = FALSE;

    doc->setFormatter( new QTextFormatterBreakWords( doc ) );
    currentFormat = doc->formatCollection()->defaultFormat();
    currentAlignment = Qt::AlignLeft;

    viewport()->setBackgroundMode( PaletteBase );
    viewport()->setAcceptDrops( TRUE );
    resizeContents( 0, doc->lastParag() ?
		    ( doc->lastParag()->paragId() + 1 ) * doc->formatCollection()->defaultFormat()->height() : 0 );

    setKeyCompression( TRUE );
    setHScrollBarMode( AlwaysOff );
    setVScrollBarMode( AlwaysOn );
    viewport()->setMouseTracking( TRUE );
    viewport()->setCursor( ibeamCursor );
    viewport()->setFocusPolicy( WheelFocus );

    cursor = new QTextCursor( doc );

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

    dragStartTimer = new QTimer( this );
    connect( dragStartTimer, SIGNAL( timeout() ),
	     this, SLOT( startDrag() ) );

    resizeTimer = new QTimer( this );
    connect( resizeTimer, SIGNAL( timeout() ),
	     this, SLOT( doResize() ) );

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

    mLines = -1;

    connect( this, SIGNAL( textChanged() ),
	     this, SLOT( setModified() ) );
}

void QTextEdit::drawContents( QPainter *p, int cx, int cy, int cw, int ch )
{
    bool drawCur = hasFocus() || viewport()->hasFocus();
    if ( !doc->firstParag() )
	return;

    QTextParag *parag = doc->firstParag();
    QSize s( doc->firstParag()->rect().size() );

    if ( contentsY() == 0 ) {
	p->fillRect( contentsX(), contentsY(), visibleWidth(), doc->y(),
		     colorGroup().color( QColorGroup::Base ) );
    }

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
	    if ( parag->rect().y() > cy + ch ) {
		cursorVisible = TRUE;
		return;
	    }
	    parag = parag->next();
	    continue;
	}
	
	if ( !parag->hasChanged() && !drawAll ) {
	    parag = parag->next();
	    continue;
	}
	
	parag->setChanged( FALSE );
	QRect ir( parag->rect() );
	ir = ir.intersect( QRect( cx, cy, cw, ch ) );
	if ( ir.width() > doubleBuffer->width() ||
	     ir.height() > doubleBuffer->height() ) {
	    if ( painter.isActive() )
		painter.end();
	    doubleBuffer = bufferPixmap( ir.size() );
	    painter.begin( doubleBuffer );
	}
	painter.fillRect( QRect( 0, 0, ir.width(), ir.height() ),
			  colorGroup().color( QColorGroup::Base ) );
	
	painter.translate( -( ir.x() - parag->rect().x() ),
			   -( ir.y() - parag->rect().y() ) );
	parag->paint( painter, colorGroup(), drawCur ? cursor : 0, TRUE, cx, cy, cw, ch );
	if ( !doc->flow()->isEmpty() ) {
	    painter.saveWorldMatrix();
	    painter.translate( 0, -parag->rect().y() );
	    QRect cr( cx, cy, cw, ch );
	    cr = cr.intersect( QRect( 0, parag->rect().y(), parag->rect().width(), parag->rect().height() ) );
	    doc->flow()->drawFloatingItems( &painter, cr.x(), cr.y(), cr.width(), cr.height(), colorGroup() );
	    painter.restoreWorldMatrix();
	}
	
	p->drawPixmap( ir.topLeft(), *doubleBuffer, QRect( QPoint( 0, 0 ), ir.size() ) );
	if ( parag->rect().x() + parag->rect().width() < contentsX() + contentsWidth() )
	    p->fillRect( parag->rect().x() + parag->rect().width(), parag->rect().y(),
			 ( contentsX() + contentsWidth() ) - ( parag->rect().x() + parag->rect().width() ),
			 parag->rect().height(), colorGroup().brush( QColorGroup::Base ) );
	painter.translate( ( ir.x() - parag->rect().x() ),
			   ( ir.y() - parag->rect().y() ) );
	parag = parag->next();
    }

    parag = doc->lastParag();
    if ( parag->rect().y() + parag->rect().height() - contentsY() < visibleHeight() ) {
	p->fillRect( 0, parag->rect().y() + parag->rect().height(), contentsWidth(),
		     visibleHeight() - ( parag->rect().y() + parag->rect().height() ),
		     colorGroup().brush( QColorGroup::Base ) );
	if ( !doc->flow()->isEmpty() ) {
	    QRect cr( cx, cy, cw, ch );
	    cr = cr.intersect( QRect( 0, parag->rect().y() + parag->rect().height(), contentsWidth(),
				      visibleHeight() - ( parag->rect().y() + parag->rect().height() ) ) );
	    doc->flow()->drawFloatingItems( p, cr.x(), cr.y(), cr.width(), cr.height(), colorGroup() );
	}
    }

    cursorVisible = TRUE;
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
	if ( mLines == 1 )
	    break;
	doc->removeSelection( QTextDocument::Standard );
	// ########### should be optimized to use QWidget::scroll here
	// ########### this will make it MUCH faster!
	clearUndoRedoInfo = FALSE;
	doKeyboardAction( ActionReturn );
	break;
    case Key_Delete:
	if ( doc->hasSelection( QTextDocument::Standard ) ) {
	    removeSelectedText();
	    break;
	}

	doKeyboardAction( ActionDelete );
	clearUndoRedoInfo = FALSE;
	
	break;
    case Key_Backspace:
	if ( doc->hasSelection( QTextDocument::Standard ) ) {
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
		    if ( cursor->index() == 0 && cursor->parag()->style() &&
			 cursor->parag()->style()->displayMode() == QStyleSheetItem::DisplayListItem ) {
			cursor->parag()->incDepth();
			drawCursor( FALSE );
			repaintChanged();
			drawCursor( TRUE );
			break;
		    }
		    if ( doCompletion() )
			break;
		}
		if ( cursor->parag()->style() &&
		     cursor->parag()->style()->displayMode() == QStyleSheetItem::DisplayBlock &&
		     cursor->index() == 0 && ( e->text() == "-" || e->text() == "*" ) ) {
		    setParagType( QStyleSheetItem::DisplayListItem, QStyleSheetItem::ListDisc );
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
	    }
	}
    }

    if ( clearUndoRedoInfo )
	undoRedoInfo.clear();

    changeIntervalTimer->start( 100, TRUE );
}

void QTextEdit::doKeyboardAction( int action )
{
    if ( readOnly )
	return;

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
	if ( cursor->parag()->style() && cursor->parag()->style()->displayMode() == QStyleSheetItem::DisplayListItem &&
	     cursor->index() == 0 ) {
	    cursor->parag()->decDepth();
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
    emit textChanged();
}

void QTextEdit::removeSelectedText()
{
    if ( readOnly )
	return;

    drawCursor( FALSE );
    checkUndoRedoInfo( UndoRedoInfo::RemoveSelected );
    if ( !undoRedoInfo.valid() ) {
	doc->selectionStart( QTextDocument::Standard, undoRedoInfo.id, undoRedoInfo.index );
	undoRedoInfo.text = QString::null;
    }
    undoRedoInfo.text = doc->selectedText( QTextDocument::Standard );
    doc->removeSelectedText( QTextDocument::Standard, cursor );
    ensureCursorVisible();
    lastFormatted = cursor->parag();
    formatMore();
    repaintChanged();
    ensureCursorVisible();
    drawCursor( TRUE );
    undoRedoInfo.clear();
    emit textChanged();
}

void QTextEdit::moveCursor( int direction, bool shift, bool control )
{
    drawCursor( FALSE );
    if ( shift ) {
	if ( !doc->hasSelection( QTextDocument::Standard ) )
	    doc->setSelectionStart( QTextDocument::Standard, cursor );
	moveCursor( direction, control );
	if ( doc->setSelectionEnd( QTextDocument::Standard, cursor ) ) {
	    repaintChanged();
	} else {
	    drawCursor( TRUE );
	}
	ensureCursorVisible();
    } else {
	bool redraw = doc->removeSelection( QTextDocument::Standard );
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
    if ( !firstResize )
	resizeTimer->stop();
    QScrollView::resizeEvent( e );
    if ( !firstResize ) {
	if ( e->oldSize().width() != e->size().width() )
	    resizeTimer->start( 0, TRUE );
    } else {
	if ( e->oldSize().width() != e->size().width() )
	    doResize();
    }
	
    firstResize = FALSE;
}

void QTextEdit::ensureCursorVisible()
{
    if ( cursor->document()->parent() ) // ####### (should do something for nested stuff)
	return;

    lastFormatted = cursor->parag();
    formatMore();
    QTextString::Char *chr = cursor->parag()->at( cursor->index() );
    int h = cursor->parag()->lineHeightOfChar( cursor->index() );
    int x = cursor->parag()->rect().x() + chr->x;
    int y = 0; int dummy;
    cursor->parag()->lineHeightOfChar( cursor->index(), &dummy, &y );
    y += cursor->parag()->rect().y();
    int w = 1;
    ensureVisible( x, y + h / 2, w, h / 2 + 2 );
}

void QTextEdit::drawCursor( bool visible )
{
    if ( !cursor->parag()->isValid() ||
	 ( !hasFocus() && !viewport()->hasFocus() ) )
	return;

    cursorVisible = visible;

    QPainter p;
    p.begin( viewport() );
    p.translate( -contentsX(), -contentsY() );
    p.translate( cursor->offsetX(), cursor->offsetY() );

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

    QTextString::Char *chr = cursor->parag()->at( cursor->index() );

    painter.setPen( QPen( chr->format()->color() ) );
    painter.setFont( chr->format()->font() );
    int h = 0; int y = 0;
    int bl = 0;
    int cw = chr->width();
    h = cursor->parag()->lineHeightOfChar( cursor->index(), &bl, &y );

    bool fill = TRUE;
    for ( int i = 0; i < doc->numSelections; ++i ) {
	if ( cursor->parag()->hasSelection( i ) ) {
	    int start = cursor->parag()->selectionStart( i );
	    int end = cursor->parag()->selectionEnd( i );
	    if ( end == cursor->parag()->length() - 1 && cursor->parag()->next() &&
		 cursor->parag()->next()->hasSelection( i ) )
		end++;
	    if ( start <= cursor->index() && end > cursor->index() ) {
		if ( doc->invertSelectionText( i ) )
		    painter.setPen( QPen( colorGroup().color( QColorGroup::HighlightedText ) ) );
		painter.fillRect( chr->x, y, cw, h, doc->selectionColor( i ) );
		fill = FALSE;
	    }
	}
    }
	
    if ( fill ) {
	if ( cursor->parag()->tableCell() && cursor->parag()->tableCell()->backGround() )
	    painter.fillRect( chr->x, y, cw, h,
			      *cursor->parag()->tableCell()->backGround() );
	else
	    painter.fillRect( chr->x, y, cw, h,
			      colorGroup().color( QColorGroup::Base ) );
    }

    if ( chr->c != '\t' ) {
	QRegion reg;
	if ( !chr->isCustom ) {
	    painter.drawText( chr->x, y + bl, chr->c );
	    if ( chr->format()->isMisspelled() ) {
		painter.save();
		painter.setPen( QPen( Qt::red, 1, Qt::DotLine ) );
		painter.drawLine( chr->x, y + bl + 1, chr->x + chr->format()->width( chr->c ), y + bl + 1 );
		painter.restore();
	    }
	} else {
	    if ( chr->customItem()->placement() == QTextCustomItem::PlaceInline )
		chr->customItem()->draw( &painter, chr->x, y, -1, -1, -1, -1, colorGroup() );
	}
    }

    if ( visible ) {
	int x = chr->x;
	int w = 1;
	painter.fillRect( QRect( x, y, w, h ), black );
    }
	
    p.drawPixmap( cursor->parag()->rect().topLeft() + QPoint( chr->x, y ), *doubleBuffer,
		  QRect( chr->x, y, cw, h ) );
    p.end();
}

void QTextEdit::contentsMousePressEvent( QMouseEvent *e )
{
    undoRedoInfo.clear();
    QTextCursor c = *cursor;
    mousePos = e->pos();
    mightStartDrag = FALSE;

    if ( e->button() == LeftButton ) {
	mousePressed = TRUE;
	drawCursor( FALSE );
	placeCursor( e->pos() );
	ensureCursorVisible();

	if ( doc->inSelection( QTextDocument::Standard, e->pos() ) ) {
	    mightStartDrag = TRUE;
	    drawCursor( TRUE );
	    dragStartTimer->start( QApplication::startDragTime(), TRUE );
	    dragStartPos = e->pos();
	    return;
	}
	
	bool redraw = FALSE;
	if ( doc->hasSelection( QTextDocument::Standard ) ) {
	    if ( !( e->state() & ShiftButton ) ) {
		redraw = doc->removeSelection( QTextDocument::Standard );
		doc->setSelectionStart( QTextDocument::Standard, cursor );
	    } else {
		redraw = doc->setSelectionEnd( QTextDocument::Standard, cursor ) || redraw;
	    }
	} else {
	    if ( !( e->state() & ShiftButton ) ) {
		doc->setSelectionStart( QTextDocument::Standard, cursor );
	    } else {
		doc->setSelectionStart( QTextDocument::Standard, &c );
		redraw = doc->setSelectionEnd( QTextDocument::Standard, cursor ) || redraw;
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
	if ( mightStartDrag ) {
	    dragStartTimer->stop();
	    if ( ( e->pos() - dragStartPos ).manhattanLength() > QApplication::startDragDistance() )
		startDrag();
	    return;
	}
	mousePos = e->pos();
	doAutoScroll();
	oldMousePos = mousePos;
    }
}

void QTextEdit::contentsMouseReleaseEvent( QMouseEvent * )
{
    if ( dragStartTimer->isActive() )
	dragStartTimer->stop();
    if ( scrollTimer->isActive() )
	scrollTimer->stop();
    if ( mightStartDrag ) {
	selectAll( FALSE );
	mousePressed = FALSE;
    }
    if ( mousePressed ) {
	if ( !doc->selectedText( QTextDocument::Standard ).isEmpty() )
	    doc->copySelectedText( QTextDocument::Standard );
	mousePressed = FALSE;
    }
    if ( cursor->checkParens() ) {
	repaintChanged();
    }
    updateCurrentFormat();
    inDoubleClick = FALSE;
}

void QTextEdit::contentsMouseDoubleClickEvent( QMouseEvent * )
{
    QTextCursor c1 = *cursor;
    QTextCursor c2 = *cursor;
    c1.gotoWordLeft();
    c2.gotoWordRight();

    doc->setSelectionStart( QTextDocument::Standard, &c1 );
    doc->setSelectionEnd( QTextDocument::Standard, &c2 );

    *cursor = c2;

    repaintChanged();

    inDoubleClick = TRUE;
    mousePressed = TRUE;
}

void QTextEdit::contentsDragEnterEvent( QDragEnterEvent *e )
{
    e->acceptAction();
}

void QTextEdit::contentsDragMoveEvent( QDragMoveEvent *e )
{
    drawCursor( FALSE );
    placeCursor( e->pos(),  cursor );
    drawCursor( TRUE );
    e->acceptAction();
}

void QTextEdit::contentsDragLeaveEvent( QDragLeaveEvent * )
{
}

void QTextEdit::contentsDropEvent( QDropEvent *e )
{
    e->acceptAction();
    QString text;
    int i = -1;
    while ( ( i = text.find( '\r' ) ) != -1 )
	text.replace( i, 1, "" );
    if ( QTextDrag::decode( e, text ) ) {
	if ( ( e->source() == this ||
	       e->source() == viewport() ) &&
	     e->action() == QDropEvent::Move ) {
	    removeSelectedText();
	}
	insert( text, FALSE, TRUE );
    }
}

void QTextEdit::doAutoScroll()
{
    if ( !mousePressed )
	return;

    QPoint pos( mapFromGlobal( QCursor::pos() ) );
    drawCursor( FALSE );
    QTextCursor oldCursor = *cursor;
    placeCursor( viewportToContents( pos ) );
    if ( inDoubleClick ) {
	QTextCursor cl = *cursor;
	cl.gotoWordLeft();
	QTextCursor cr = *cursor;
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
    if ( doc->hasSelection( QTextDocument::Standard ) ) {
	redraw = doc->setSelectionEnd( QTextDocument::Standard, cursor ) || redraw;
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

void QTextEdit::placeCursor( const QPoint &pos, QTextCursor *c )
{
    if ( !c )
	c = cursor;

    QTextParag *s = doc->firstParag();
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
    QTextString::Char *chr = 0, *c2;
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
	cw = c2->width();
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
    if ( !lastFormatted )
	return;

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

    if ( bottom > contentsHeight() && !cursor->document()->parent() ) // ####### (should do something for nested stuff)
	resizeContents( contentsWidth(), QMAX( doc->flow()->height, bottom ) );
    else if ( lastBottom != -1 && lastBottom < contentsHeight() && !cursor->document()->parent() ) // ####### (should do something for nested stuff)
	resizeContents( contentsWidth(), QMAX( doc->flow()->height, lastBottom ) );

    if ( lastFormatted )
	formatTimer->start( interval, TRUE );
    else
	interval = QMAX( 0, interval );
}

void QTextEdit::doResize()
{	
    resizeContents( width() - verticalScrollBar()->width(), contentsHeight() );
    doc->setWidth( visibleWidth() - 4 );
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
	QTextString::Char *chr = cursor->parag()->at( cursor->index() );
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
    } else if ( ( o == this || o == viewport() ) ) {
	if ( e->type() == QEvent::FocusIn ) {
	    blinkTimer->start( QApplication::cursorFlashTime() / 2 );
	    return FALSE;
	} else if ( e->type() == QEvent::FocusOut ) {
	    blinkTimer->stop();
	    drawCursor( FALSE );
	    return TRUE;
	}
    }
		

    return QScrollView::eventFilter( o, e );
}

void QTextEdit::insert( const QString &text, bool indent, bool checkNewLine )
{
    if ( readOnly )
	return;

    QString txt( text );
    if ( mLines == 1 )
	txt = txt.replace( QRegExp( "\n" ), " " );

    drawCursor( FALSE );
    if ( doc->hasSelection( QTextDocument::Standard ) ) {
	checkUndoRedoInfo( UndoRedoInfo::RemoveSelected );
	if ( !undoRedoInfo.valid() ) {
	    doc->selectionStart( QTextDocument::Standard, undoRedoInfo.id, undoRedoInfo.index );
	    undoRedoInfo.text = QString::null;
	}
	undoRedoInfo.text = doc->selectedText( QTextDocument::Standard );
	doc->removeSelectedText( QTextDocument::Standard, cursor );
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
    cursor->insert( txt, checkNewLine );
    if ( doc->useFormatCollection() )
	cursor->parag()->setFormat( idx, txt.length(), currentFormat, TRUE );
		
    if ( indent && ( txt == "{" || txt == "}" ) )
	cursor->indent();
    formatMore();
    repaintChanged();
    ensureCursorVisible();
    drawCursor( TRUE );
    undoRedoInfo.text += txt;

    emit textChanged();
}

void QTextEdit::undo()
{
    if ( readOnly )
	return;

    undoRedoInfo.clear();
    drawCursor( FALSE );
    QTextCursor *c = doc->undo( cursor );
    if ( !c ) {
	drawCursor( TRUE );
	return;
    }
    ensureCursorVisible();
    repaintChanged();
    drawCursor( TRUE );
    emit textChanged();
}

void QTextEdit::redo()
{
    if ( readOnly )
	return;

    undoRedoInfo.clear();
    drawCursor( FALSE );
    QTextCursor *c = doc->redo( cursor );
    if ( !c ) {
	drawCursor( TRUE );
	return;
    }
    ensureCursorVisible();
    repaintChanged();
    ensureCursorVisible();
    drawCursor( TRUE );
    emit textChanged();
}

void QTextEdit::paste()
{
    if ( readOnly )
	return;

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
    if ( readOnly )
	return;

    if ( doc->hasSelection( QTextDocument::Standard ) ) {
	doc->copySelectedText( QTextDocument::Standard );
	removeSelectedText();
    }
}

void QTextEdit::copy()
{
    if ( !doc->selectedText( QTextDocument::Standard ).isEmpty() )
	doc->copySelectedText( QTextDocument::Standard );
}

void QTextEdit::indent()
{
    if ( readOnly )
	return;

    drawCursor( FALSE );
    if ( !doc->hasSelection( QTextDocument::Standard ) )
	cursor->indent();
    else
	doc->indentSelection( QTextDocument::Standard );
    repaintChanged();
    drawCursor( TRUE );
    emit textChanged();
}

bool QTextEdit::focusNextPrevChild( bool )
{
    return FALSE;
}

void QTextEdit::setFormat( QTextFormat *f, int flags )
{
    if ( readOnly )
	return;

    if ( doc->hasSelection( QTextDocument::Standard ) ) {
	drawCursor( FALSE );
	doc->setFormat( QTextDocument::Standard, f, flags );
	repaintChanged();
	formatMore();
	drawCursor( TRUE );
	emit textChanged();
    }
    if ( currentFormat && currentFormat->key() != f->key() ) {
	currentFormat->removeRef();
	currentFormat = doc->formatCollection()->format( f );
	if ( currentFormat->isMisspelled() ) {
	    currentFormat->removeRef();
	    currentFormat = doc->formatCollection()->format( currentFormat->font(), currentFormat->color() );
	}
	emit currentFontChanged( currentFormat->font() );
	emit currentColorChanged( currentFormat->color() );
    }
}

void QTextEdit::setParagType( QStyleSheetItem::DisplayMode dm, int listStyle )
{
    if ( readOnly )
	return;

    drawCursor( FALSE );
    if ( !doc->hasSelection( QTextDocument::Standard ) ) {
	cursor->parag()->setList( dm == QStyleSheetItem::DisplayListItem, listStyle );
	repaintChanged();
    } else {
	QTextParag *start = doc->selectionStart( QTextDocument::Standard );
	QTextParag *end = doc->selectionEnd( QTextDocument::Standard );
	lastFormatted = start;
	while ( start ) {
	    start->setList( dm == QStyleSheetItem::DisplayListItem, listStyle );
	    if ( start == end )
		break;
	    start = start->next();
	}
	repaintChanged();
	formatMore();
    }
    drawCursor( TRUE );
    emit textChanged();
}

void QTextEdit::setAlignment( int a )
{
    if ( readOnly )
	return;

    drawCursor( FALSE );
    if ( !doc->hasSelection( QTextDocument::Standard ) ) {
	cursor->parag()->setAlignment( a );
	repaintChanged();
    } else {
	QTextParag *start = doc->selectionStart( QTextDocument::Standard );
	QTextParag *end = doc->selectionEnd( QTextDocument::Standard );
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
    emit textChanged();
}

void QTextEdit::updateCurrentFormat()
{
    int i = cursor->index();
    if ( i > 0 )
	--i;
    if ( currentFormat->key() != cursor->parag()->at( i )->format()->key() && doc->useFormatCollection() ) {
	if ( currentFormat )
	    currentFormat->removeRef();
	currentFormat = doc->formatCollection()->format( cursor->parag()->at( i )->format() );
	if ( currentFormat->isMisspelled() ) {
	    currentFormat->removeRef();
	    currentFormat = doc->formatCollection()->format( currentFormat->font(), currentFormat->color() );
	}
	emit currentFontChanged( currentFormat->font() );
	emit currentColorChanged( currentFormat->color() );
    }

    if ( currentAlignment != cursor->parag()->alignment() ) {
	currentAlignment = cursor->parag()->alignment();
	emit currentAlignmentChanged( currentAlignment );
    }
}

void QTextEdit::setItalic( bool b )
{
    QTextFormat f( *currentFormat );
    f.setItalic( b );
    setFormat( &f, QTextFormat::Italic );
}

void QTextEdit::setBold( bool b )
{
    QTextFormat f( *currentFormat );
    f.setBold( b );
    setFormat( &f, QTextFormat::Bold );
}

void QTextEdit::setUnderline( bool b )
{
    QTextFormat f( *currentFormat );
    f.setUnderline( b );
    setFormat( &f, QTextFormat::Underline );
}

void QTextEdit::setFamily( const QString &f_ )
{
    QTextFormat f( *currentFormat );
    f.setFamily( f_ );
    setFormat( &f, QTextFormat::Family );
}

void QTextEdit::setPointSize( int s )
{
    QTextFormat f( *currentFormat );
    f.setPointSize( s );
    setFormat( &f, QTextFormat::Size );
}

void QTextEdit::setColor( const QColor &c )
{
    QTextFormat f( *currentFormat );
    f.setColor( c );
    setFormat( &f, QTextFormat::Color );
}

void QTextEdit::setFont( const QFont &f_ )
{
    QTextFormat f( *currentFormat );
    f.setFont( f_ );
    setFormat( &f, QTextFormat::Font );
}

QString QTextEdit::text() const
{
    return doc->text();
}

QString QTextEdit::text( int parag, bool formatted ) const
{
    return doc->text( parag, formatted );
}

void QTextEdit::setText( const QString &txt, bool tabify )
{
    doc->setText( txt, tabify );
    cursor->setParag( doc->firstParag() );
    cursor->setIndex( 0 );
    viewport()->repaint( FALSE );
    emit textChanged();
}

QString QTextEdit::fileName() const
{
    return doc->fileName();
}

void QTextEdit::load( const QString &fn, bool tabify )
{
    resizeContents( 0, 0 );
    doc->load( fn, tabify );
    cursor->setParag( doc->firstParag() );
    cursor->setIndex( 0 );
    viewport()->repaint( FALSE );
    emit textChanged();
    doResize();
}

void QTextEdit::save( const QString &fn, bool untabify )
{
    doc->save( fn, untabify );
}

bool QTextEdit::find( const QString &expr, bool cs, bool wo, bool forward,
		      int *parag, int *index )
{
    drawCursor( FALSE );
    doc->removeSelection( QTextDocument::Search );
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

void QTextEdit::setCursorPosition( int parag, int index )
{
    QTextParag *p = doc->paragAt( parag );
    if ( !p )
	return;

    if ( index > p->length() - 1 )
	index = p->length() - 1;

    drawCursor( FALSE );
    cursor->setParag( p );
    cursor->setIndex( index );
    ensureCursorVisible();
    drawCursor( TRUE );
}

void QTextEdit::cursorPosition( int &parag, int &index )
{
    parag = cursor->parag()->paragId();
    index = cursor->index();
}

void QTextEdit::setSelection( int parag_from, int index_from,
			      int parag_to, int index_to )
{
    QTextParag *p1 = doc->paragAt( parag_from );
    if ( !p1 )
	return;
    QTextParag *p2 = doc->paragAt( parag_to );
    if ( !p2 )
	return;

    if ( index_from > p1->length() - 1 )
	index_from = p1->length() - 1;
    if ( index_to > p2->length() - 1 )
	index_to = p2->length() - 1;

    drawCursor( FALSE );
    QTextCursor c = *cursor;
    c.setParag( p1 );
    c.setIndex( index_from );
    cursor->setParag( p2 );
    cursor->setIndex( index_to );
    doc->setSelectionStart( QTextDocument::Standard, &c );
    doc->setSelectionEnd( QTextDocument::Standard, cursor );
    repaintChanged();
    ensureCursorVisible();
    drawCursor( TRUE );
}

void QTextEdit::selection( int &parag_from, int &index_from,
			   int &parag_to, int &index_to )
{
    if ( !doc->hasSelection( QTextDocument::Standard ) ) {
	parag_from = -1;
	index_from = -1;
	parag_to = -1;
	index_to = -1;
	return;
    }

    doc->selectionStart( QTextDocument::Standard, parag_from, index_from );
    doc->selectionEnd( QTextDocument::Standard, parag_from, index_from );
}

void QTextEdit::setTextFormat( Qt::TextFormat f )
{
    doc->setTextFormat( f );
}

Qt::TextFormat QTextEdit::textFormat() const
{
    return doc->textFormat();
}

int QTextEdit::paragraphs() const
{
    return doc->lastParag()->paragId() + 1;
}

int QTextEdit::linesOfParagraph( int parag ) const
{
    QTextParag *p = doc->paragAt( parag );
    if ( !p )
	return -1;
    return p->lines();
}

int QTextEdit::lines() const
{
    qWarning( "WARNING: QTextEdit::lines() is slow - will be improved later..." );
    QTextParag *p = doc->firstParag();
    int l = 0;
    while ( p ) {
	l += p->lines();
	p = p->next();
    }

    return l;
}

int QTextEdit::lineOfChar( int parag, int chr )
{
    QTextParag *p = doc->paragAt( parag );
    if ( !p )
	return -1;

    int idx, line;
    QTextString::Char *c = p->lineStartOfChar( chr, &idx, &line );
    if ( !c )
	return -1;

    return line;
}

void QTextEdit::setReadOnly( bool ro )
{
    if ( ro == readOnly )
	return;
    readOnly = ro;
}

void QTextEdit::setModified( bool m )
{
    modified = m;
    if ( modified ) {
	disconnect( this, SIGNAL( textChanged() ),
		    this, SLOT( setModified() ) );
    } else {
	connect( this, SIGNAL( textChanged() ),
		 this, SLOT( setModified() ) );
    }
}

bool QTextEdit::isModified() const
{
    return modified;
}

void QTextEdit::setModified()
{
    setModified( TRUE );
}

bool QTextEdit::italic() const
{
    return currentFormat->font().italic();
}

bool QTextEdit::bold() const
{
    return currentFormat->font().bold();
}

bool QTextEdit::underline() const
{
    return currentFormat->font().underline();
}

QString QTextEdit::family() const
{
    return currentFormat->font().family();
}

int QTextEdit::pointSize() const
{
    return currentFormat->font().pointSize();
}

QColor QTextEdit::color() const
{
    return currentFormat->color();
}

QFont QTextEdit::font() const
{
    return currentFormat->font();
}

int QTextEdit::alignment() const
{
    return currentAlignment;
}

void QTextEdit::startDrag()
{
    mousePressed = FALSE;
    inDoubleClick = FALSE;
    QDragObject *drag = new QTextDrag( doc->selectedText( QTextDocument::Standard ), viewport() );
    if ( readOnly ) {
	drag->dragCopy();
    } else {
	if ( drag->drag() && QDragObject::target() != this ) {
	    doc->removeSelectedText( QTextDocument::Standard, cursor );
	    repaintChanged();
	}
    }
}

void QTextEdit::selectAll( bool select )
{
    // ############## Implement that!!!
    if ( !select ) {
	doc->removeSelection( QTextDocument::Standard );
	repaintChanged();
    }
}

void QTextEdit::UndoRedoInfo::clear()
{
    if ( valid() ) {
	if ( type == Insert || type == Return )
	    doc->addCommand( new QTextInsertCommand( doc, id, index, text ) );
	else if ( type != Invalid )
	    doc->addCommand( new QTextDeleteCommand( doc, id, index, text ) );
    }
    text = QString::null;
    id = -1;
    index = -1;
}

void QTextEdit::setMaxLines( int l )
{
    mLines = l;
}

int QTextEdit::maxLines() const
{
    return mLines;
}

void QTextEdit::resetFormat()
{
    setAlignment( Qt::AlignLeft );
    setParagType( QStyleSheetItem::DisplayBlock, -1 );
    setFormat( doc->formatCollection()->defaultFormat(), QTextFormat::Format );
}

