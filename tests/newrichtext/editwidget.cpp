#include "qfont.h"
#include "qpainter.h"
#include "editwidget.h"
#include <private/qfontdata_p.h>

#include "qtextlayout.h"
#include <qmemarray.h>

#include <qdatetime.h>

class EditWidgetPrivate
{
public:
    QString text;
    QFont font;
    int cursorPos;
    bool cursorOn;
    QTextLayout *layout;
};



EditWidget::EditWidget( QWidget *parent,  const char * name )
    : QWidget( parent, name )
{
    d = new EditWidgetPrivate();
    startTimer( 1000 );
    d->cursorOn = FALSE;
    d->cursorPos = 0;
    d->layout = 0;
}

EditWidget::~EditWidget()
{
    delete d->layout;
    delete d;
}


void EditWidget::setText( const QString &t )
{
    d->text = t;
    recalculate();
}

void EditWidget::setFont( const QFont &f )
{
    d->font = f;
    recalculate();
}

void EditWidget::timerEvent( QTimerEvent * )
{
    d->cursorOn = !d->cursorOn;
//     update();
}

void EditWidget::mousePressEvent( QMouseEvent *e )
{



}


void EditWidget::keyPressEvent ( QKeyEvent *e )
{
    switch( e->key() ) {
    case Key_Left:
	if ( d->cursorPos )
	    d->cursorPos--;
	break;
    case Key_Right:
	if ( d->cursorPos < d->text.length()-1 )
	    d->cursorPos++;
	break;
    case Key_Backspace:
	if ( d->cursorPos )
	    d->text.remove( --d->cursorPos, 1 );
	break;
    case Key_Delete:
	d->text.remove( d->cursorPos, 1 );
	break;
    default:
	if ( !e->text().isEmpty() ) {
	    d->text.insert( d->cursorPos, e->text() );
	    d->cursorPos += e->text().length();
	}
	break;
    }
    qDebug("cursorPos at %d",  d->cursorPos );
    recalculate();
    repaint( TRUE );
}

void EditWidget::resizeEvent( QResizeEvent * )
{
    recalculate();
}


void EditWidget::paintEvent( QPaintEvent * )
{
    QPainter p( this );
    p.drawRect( 10, 10, width()-20, height()-20 );
    for ( int i = 0; i < d->layout->numItems(); i++ ) {
	QTextItem ti = d->layout->itemAt( i );
#if 1
	p.drawText( ti.x(), ti.y(), d->text, ti.from(), ti.length() );
#else
	p.drawTextItem( 0,  0, ti );
#endif
    }
}


void EditWidget::recalculate()
{
    delete d->layout;
    d->layout = 0;

    QTime t;
    t.start();

    d->layout = new QTextLayout( d->text, d->font );

    d->layout->beginLayout();

    int x = 10;
    int y = 10;
    int lw = width() - 20;

    int state = QTextLayout::Ok;
    int add = 0;
    while ( !d->layout->atEnd() ) {
	d->layout->beginLine( lw + add );
	int ascent = 0;
	int descent = 0;
	int state = QTextLayout::Ok;
	do {
	    QTextItem ti = d->layout->currentItem();
	    ascent = QMAX( ascent, ti.ascent() );
	    descent = QMAX( descent, ti.descent() );
	    state = d->layout->addCurrentItem();
	} while ( state == QTextLayout::Ok && !d->layout->atEnd() );
	if ( !d->layout->lineIsEmpty() ) {
// 	    qDebug("finalizing line: ascent = %d, descent=%d", ascent, descent );
	    d->layout->endLine( x, y+ascent, Qt::AlignLeft );
	    y += ascent + descent + 2;
	    add = 0;
	} else {
	    add += 10;
	}
    }
    d->layout->endLayout();
    qDebug("layout took %dms (%dus/char)", t.elapsed(), t.elapsed()*1000/d->text.length() );
}
