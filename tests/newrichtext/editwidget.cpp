#include "editwidget.h"

#include <qfont.h>
#include <qpainter.h>
#include <private/qfontdata_p.h>
#include <private/qtextlayout_p.h>

#include <qmemarray.h>
#include <qdatetime.h>
#include <qevent.h>

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
	d->cursorPos = d->layout->previousCursorPosition( d->cursorPos );
	break;
    case Key_Right:
	d->cursorPos = d->layout->nextCursorPosition( d->cursorPos );
	break;
    case Key_Backspace:
	if ( d->cursorPos )
	    d->text.remove( --d->cursorPos, 1 );
	break;
    case Key_Delete: {
	int nextChar = d->layout->nextCursorPosition( d->cursorPos );
	d->text.remove( d->cursorPos, nextChar - d->cursorPos );
	break;
    }
    default:
	if ( !e->text().isEmpty() ) {
	    d->text.insert( d->cursorPos, e->text() );
	    d->cursorPos += e->text().length();
	}
	break;
    }
    qDebug("cursorPos at %d",  d->cursorPos );
    recalculate();
    repaint();
}

void EditWidget::resizeEvent( QResizeEvent * )
{
    recalculate();
}


void EditWidget::paintEvent( QPaintEvent * )
{
    QPainter p( this );
    p.drawRect( 10, 10, width()-20, height()-20 );
    for ( int i = 0; i < d->layout->numLines(); i++ ) {
	QTextLine line = d->layout->lineAt(i);
	line.draw(&p, 0, 0);
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

    int leading = QFontMetrics(d->font).leading();
    int x = 10;
    int y = 10 - leading;
    int lw = width() - 20;

    int from = 0;
    while (from < d->text.length()) {
	y += leading;
	QTextLine l = d->layout->createLine(from, y, x, x+lw);
	y += l.ascent() + l.descent();
	from += l.length();
    }

    qDebug("layout took %dms (%dus/char)", t.elapsed(), t.elapsed()*1000/qMax(d->text.length(),1) );
}
