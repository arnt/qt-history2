#include "qtextedit.h"
#include "qtexteditintern_h.cpp"

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

static QPixmap *buf_pixmap = 0;

static QPixmap *bufferPixmap( const QSize &s )
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

QTextEdit::QTextEdit( QWidget *parent, QTextEditDocument *d )
    : QScrollView( parent, "", WNorthWestGravity | WRepaintNoErase ), doc( d ), undoRedoInfo( d )
{
    doc->setFormatter( new QTextEditFormatterBreakWords( d ) );
    currentFormat = doc->formatCollection()->defaultFormat();

    viewport()->setBackgroundMode( PaletteBase );
    resizeContents( 0, doc->lastParag() ?
		    ( doc->lastParag()->paragId() + 1 ) * doc->formatCollection()->defaultFormat()->height() : 0 );
    cursor = new QTextEditCursor( doc );
    interval = 0;
    setKeyCompression( TRUE );
    setHScrollBarMode( AlwaysOff );
    setVScrollBarMode( AlwaysOn );
    drawAll = TRUE;
    viewport()->setMouseTracking( TRUE );
    mousePressed = FALSE;
    formatTimer = new QTimer( this );
    connect( formatTimer, SIGNAL( timeout() ),
	     this, SLOT( formatMore() ) );
    scrollTimer = new QTimer( this );
    connect( scrollTimer, SIGNAL( timeout() ),
	     this, SLOT( doAutoScroll() ) );
    changeIntervalTimer = new QTimer( this );
    connect( changeIntervalTimer, SIGNAL( timeout() ),
	     this, SLOT( doChangeInterval() ) );
    lastFormatted = doc->firstParag();
    formatMore();
    viewport()->setCursor( ibeamCursor );
    viewport()->setFocusPolicy( WheelFocus );

    completionPopup = new QVBox( this, 0, WType_Popup );
    completionPopup->setFrameStyle( QFrame::Box | QFrame::Plain );
    completionPopup->setLineWidth( 1 );
    completionListBox = new QListBox( completionPopup );
    completionListBox->setFrameStyle( QFrame::NoFrame );
    completionListBox->installEventFilter( this );
    completionPopup->installEventFilter( this );
    completionPopup->setFocusProxy( completionListBox );
    completionOffset = 0;
}

void QTextEdit::drawContents( QPainter *p, int cx, int cy, int cw, int ch )
{
    if ( !doc->firstParag() )
	return;

    QTextEditParag *parag = doc->firstParag();
    QPixmap *db = 0;
    QTextEditString::Char *chr = 0;
    QPainter painter;
    QSize s( doc->firstParag()->rect().size() );
    db = bufferPixmap( s );
    painter.begin( db );
    while ( parag ) {
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
	if ( s.width() > db->width() ||
	     s.height() > db->height() ) {
	    if ( painter.isActive() )
		painter.end();
	    db = bufferPixmap( s );
	    painter.begin( db );
	}
	painter.fillRect( QRect( 0, 0, s.width(), s.height() ),
			  colorGroup().color( QColorGroup::Base ) );
	chr = parag->at( 0 );
	int i = 0;
	int h = 0;
	int baseLine = 0, lastBaseLine = 0;
	QTextEditFormat *lastFormat = 0;
	int lastY = -1;
	QString buffer;
	int startX = 0;
	int bw = 0;
	int cy;
	int curx = -1, cury, curh;
	
	// #### draw other selections too here!!!!!!!
	int selStart = -1, selEnd = -1;
	int matchStart = -1, matchEnd = -1;
	int mismatchStart = -1, mismatchEnd = -1;
	if ( parag->hasSelection( QTextEditDocument::Standard ) ) {
	    selStart = parag->selectionStart( QTextEditDocument::Standard );
	    selEnd = parag->selectionEnd( QTextEditDocument::Standard );
	}
	if ( parag->hasSelection( QTextEditDocument::ParenMatch ) ) {
	    matchStart = parag->selectionStart( QTextEditDocument::ParenMatch );
	    matchEnd = parag->selectionEnd( QTextEditDocument::ParenMatch );
	}
	if ( parag->hasSelection( QTextEditDocument::ParenMismatch ) ) {
	    mismatchStart = parag->selectionStart( QTextEditDocument::ParenMismatch );
	    mismatchEnd = parag->selectionEnd( QTextEditDocument::ParenMismatch );
	}
	
	int line = -1;
	int cw;
	for ( ; i < parag->length(); i++ ) {
	    chr = parag->at( i );
	    cw = chr->format->width( chr->c );
	    if ( chr->lineStart ) {
		++line;
		parag->lineInfo( line, cy, h, baseLine );
		if ( lastBaseLine == 0 )
		    lastBaseLine = baseLine;
	    }
	    
	    if ( line == 0 && parag->type() == QTextEditParag::BulletList ) {
		painter.save();
		int ext = QMIN( doc->listIndent( 0 ), h );
		ext -= 8;
		switch ( doc->bullet( parag->listDepth() ) ) {
		case QTextEditDocument::FilledCircle: {
		    painter.setPen( NoPen );
		    painter.setBrush( colorGroup().brush( QColorGroup::Foreground ) );
		    painter.drawEllipse( parag->leftIndent() - ext - 4, cy + ( h - ext ) / 2, ext, ext );
		} break;
		case QTextEditDocument::FilledSquare: {
		    painter.fillRect( parag->leftIndent() - ext - 4, cy + ( h - ext ) / 2, ext, ext, 
				      colorGroup().brush( QColorGroup::Foreground ) );
		} break;
		case QTextEditDocument::OutlinedCircle: {
		    painter.setPen( QPen( colorGroup().color( QColorGroup::Foreground ) ) );
		    painter.setBrush( NoBrush );
		    painter.drawEllipse( parag->leftIndent() - ext - 4, cy + ( h - ext ) / 2, ext, ext );
		} break;
		case QTextEditDocument::OutlinedSquare: {
		    painter.setPen( QPen( colorGroup().color( QColorGroup::Foreground ) ) );
		    painter.setBrush( NoBrush );
		    painter.drawRect( parag->leftIndent() - ext - 4, cy + ( h - ext ) / 2, ext, ext );
		} break;
		}
		painter.restore();
	    }
	    
	    if ( parag == cursor->parag() && i == cursor->index() ) {
		curx = chr->x;
		curh = h;
		cury = cy;
	    }
	
	    if ( !lastFormat || lastY == -1 ) {
		lastFormat = chr->format;
		lastY = cy;
		startX = chr->x;
		buffer += chr->c;
		bw = cw;
		continue;
	    }
	
	    if ( lastY != cy || chr->format != lastFormat ||
		 buffer == "\t" || chr->c == '\t' || i == selStart || i == selEnd || i ==matchStart ||
		i == matchEnd || i == mismatchStart || i == mismatchEnd ) {
		painter.setPen( QPen( lastFormat->color() ) );
		painter.setFont( lastFormat->font() );
		if ( i > selStart && i <= selEnd ) {
		    painter.setPen( QPen( colorGroup().color( QColorGroup::HighlightedText ) ) );
		    painter.fillRect( startX, lastY, bw, h, doc->selectionColor( QTextEditDocument::Standard ) );
		}
		if ( i > matchStart && i <= matchEnd )
		    painter.fillRect( startX, lastY, bw, h, doc->selectionColor( QTextEditDocument::ParenMatch ) );
		if ( i > mismatchStart && i <= mismatchEnd )
		    painter.fillRect( startX, lastY, bw, h, doc->selectionColor( QTextEditDocument::ParenMismatch ) );
		if ( buffer != "\t" )
		    painter.drawText( startX, lastY + lastBaseLine, buffer );
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
	
	if ( !buffer.isEmpty() ) {
	    painter.setPen( QPen( lastFormat->color() ) );
	    painter.setFont( lastFormat->font() );
	    if ( i > selStart && i <= selEnd ) {
		painter.setPen( QPen( colorGroup().color( QColorGroup::HighlightedText ) ) );
		painter.fillRect( startX, lastY, bw, h, doc->selectionColor( QTextEditDocument::Standard ) );
	    }
	    if ( i > matchStart && i <= matchEnd )
		painter.fillRect( startX, lastY, bw, h, doc->selectionColor( QTextEditDocument::ParenMatch ) );
	    if ( i > mismatchStart && i <= mismatchEnd )
		painter.fillRect( startX, lastY, bw, h, doc->selectionColor( QTextEditDocument::ParenMismatch ) );
	    if ( buffer != "\t" )
		painter.drawText( startX, lastY + lastBaseLine, buffer );
	}
	
	if ( curx != -1 )
	    painter.fillRect( QRect( curx, cury, 2, curh ), blue );
	
	p->drawPixmap( parag->rect().topLeft(), *db, QRect( QPoint( 0, 0 ), s ) );
	if ( parag->rect().x() + parag->rect().width() < contentsX() + contentsWidth() )
	    p->fillRect( parag->rect().x() + parag->rect().width(), parag->rect().y(),
			 ( contentsX() + contentsWidth() ) - ( parag->rect().x() + parag->rect().width() ),
			 parag->rect().height(), colorGroup().brush( QColorGroup::Base ) );
	parag = parag->next();
    }
    if ( painter.isActive() )
	painter.end();
}

void QTextEdit::keyPressEvent( QKeyEvent *e )
{
    changeIntervalTimer->stop();
    interval = 10;

    bool selChanged = FALSE;
    selChanged = doc->removeSelection( QTextEditDocument::ParenMatch );
    selChanged = doc->removeSelection( QTextEditDocument::ParenMismatch ) || selChanged;

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
		insert( e->text(), TRUE );
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
		    QFont font = currentFormat->font();
		    QColor col = currentFormat->color();
		    font.setBold( !font.bold() );
		    setFormat( font, col );
		} break;
		case Key_I: {
		    QFont font = currentFormat->font();
		    QColor col = currentFormat->color();
		    font.setItalic( !font.italic() );
		    setFormat( font, col );
		} break;
		case Key_U: {
		    QFont font = currentFormat->font();
		    QColor col = currentFormat->color();
		    font.setUnderline( !font.underline() );
		    setFormat( font, col );
		} break;
		case Key_F: {
		    QFont font = currentFormat->font();
		    bool ok;
		    font = QFontDialog::getFont( &ok, font, this );
		    if ( ok ) {
			QColor col = currentFormat->color();
			setFormat( font, col );
		    }
		} break;
		case Key_C: {
		    QColor col = currentFormat->color();
		    col = QColorDialog::getColor( col, this );
		    if ( col.isValid() ) {
			QFont font = currentFormat->font();
			setFormat( font, col );
		    }
		} break;
		case Key_L: {
		    if ( cursor->parag()->type() != QTextEditParag::BulletList )
			setParagType( (int)QTextEditParag::BulletList );
		    else
			setParagType( (int)QTextEditParag::Normal );
		} break;
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
	if ( cursor->parag()->type() != QTextEditParag::Normal ) {
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
    QPainter p, painter;
    p.begin( viewport() );
    p.translate( -contentsX(), -contentsY() );

    cursor->parag()->format();
    QSize s( cursor->parag()->rect().size() );
    QPixmap *db = bufferPixmap( s );
    painter.begin( db );
    painter.fillRect( QRect( 0, 0, s.width(), s.height() ),
		      colorGroup().color( QColorGroup::Base ) );
    QTextEditString::Char *chr = cursor->parag()->at( cursor->index() );

    painter.setPen( QPen( chr->format->color() ) );
    painter.setFont( chr->format->font() );
    int h = 0; int y = 0;
    int bl = 0;
    int cw = chr->format->width( chr->c );
    h = cursor->parag()->lineHeightOfChar( cursor->index(), &bl, &y );
    if ( cursor->parag()->hasSelection( QTextEditDocument::Standard ) ) {
	if ( cursor->parag()->selectionStart( QTextEditDocument::Standard ) <= cursor->index() &&
	     cursor->parag()->selectionEnd( QTextEditDocument::Standard ) > cursor->index() ) {
	    painter.setPen( QPen( colorGroup().color( QColorGroup::HighlightedText ) ) );
	    painter.fillRect( chr->x, y, cw, h,
			      doc->selectionColor( QTextEditDocument::Standard ) );
	}
    }
    if ( chr->c != '\t' )
	painter.drawText( chr->x, y + bl, chr->c );

    if ( visible ) {
	int x = chr->x;
	int w = 2;
	painter.fillRect( QRect( x, y, w, h ), blue );
    }
    painter.end();
	
    p.drawPixmap( cursor->parag()->rect().topLeft() + QPoint( chr->x, y ), *db,
		  QRect( chr->x, y, cw, h ) );
    p.end();
}

void QTextEdit::contentsMousePressEvent( QMouseEvent *e )
{
    undoRedoInfo.clear();
    QTextEditCursor c = *cursor;

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

	redraw = doc->removeSelection( QTextEditDocument::ParenMatch ) || redraw;
	redraw = doc->removeSelection( QTextEditDocument::ParenMismatch ) || redraw;

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

void QTextEdit::contentsMouseMoveEvent( QMouseEvent * )
{
    if ( mousePressed ) {
	doAutoScroll();
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

void QTextEdit::doAutoScroll()
{
    if ( !mousePressed )
	return;

    QPoint pos( mapFromGlobal( QCursor::pos() ) );
    drawCursor( FALSE );
    placeCursor( viewportToContents( pos ) );
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

void QTextEdit::setFormat( const QFont &font, const QColor &color )
{
    currentFormat = doc->formatCollection()->format( font, color );
    if ( doc->hasSelection( QTextEditDocument::Standard ) ) {
	drawCursor( FALSE );
	doc->setFormat( QTextEditDocument::Standard, currentFormat );

	QTextEditParag *start = doc->selectionStart( QTextEditDocument::Standard );
	QTextEditParag *end = doc->selectionEnd( QTextEditDocument::Standard );

	lastFormatted = start;
	int dy = 0;
	while ( lastFormatted ) {
	    int h = lastFormatted->rect().height();
	    lastFormatted->move( dy );
	    lastFormatted->format( 0, FALSE );
	    dy += lastFormatted->rect().height() - h;
	    if ( lastFormatted == end )
		break;
	    lastFormatted = lastFormatted->next();
	}

	if ( lastFormatted )
	    lastFormatted = lastFormatted->next();
	while ( lastFormatted ) {
	    lastFormatted->move( dy );
	    lastFormatted = lastFormatted->next();
	}
	
	repaintChanged();
	drawCursor( TRUE );
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
	while ( start ) {
	    start->setType( type );
	    start->setListDepth( cursor->parag()->listDepth() );
	    if ( start == end )
		break;
	    start = start->next();
	}
	repaintChanged();
    }
    drawCursor( TRUE );
}

void QTextEdit::updateCurrentFormat()
{
    currentFormat = cursor->parag()->at( cursor->index() )->format;
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

