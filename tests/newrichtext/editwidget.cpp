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
#if 0
	p.drawText( ti.x(), ti.y(), d->text, ti.from(), ti.length() );
#else
	p.drawTextItem( 0,  0, ti );
#endif
	int pos = d->cursorPos - ti.from();
	if ( pos >= 0 && pos <= ti.length() ) {
	    int x = ti.cursorToX( &pos ) + ti.x();
	    p.drawLine( x, ti.y() - ti.ascent(), x, ti.y() + ti.descent() );
	}
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

    int add = 0;
//     qDebug("\n\nbeginLayout: lw = %d", lw );
    while ( !d->layout->atEnd() ) {
	d->layout->beginLine( lw + add );
// 	qDebug("beginLine( %d )",  lw+add );
	while ( d->layout->addCurrentItem() == QTextLayout::Ok && !d->layout->atEnd() );

	int ascent, descent;
	int state = d->layout->endLine( x, y, Qt::AlignLeft, &ascent, &descent );

	if ( state != QTextLayout::LineEmpty ) {
//  	    qDebug("finalizing line: ascent = %d, descent=%d", ascent, descent );
	    y += ascent + descent + 2;
	    add = 0;
	} else {
	    add += 10;
	}
    }
    d->layout->endLayout();
    qDebug("layout took %dms (%dus/char)", t.elapsed(), t.elapsed()*1000/d->text.length() );
}
