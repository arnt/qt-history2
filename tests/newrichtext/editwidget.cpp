#include "qfont.h"
#include "qpainter.h"
#include "editwidget.h"
#include "fontengine.h"

#include "qtextlayout.h"
#include <qmemarray.h>

#include <qdatetime.h>

class EditWidgetPrivate
{
public:
    QString text;
    QFont font;
    ScriptItemArray items;
    ShapedItem shaped;
    int cursorPos;
    struct Line {
	int pos;
	int ascent;
	int descent;
    };
    QMemArray<Line> lineBreaks;
    bool cursorOn;
};



EditWidget::EditWidget( QWidget *parent,  const char * name )
    : QWidget( parent, name )
{
    d = new EditWidgetPrivate();
    startTimer( 1000 );
    d->cursorOn = FALSE;
    d->cursorPos = 0;
}

EditWidget::~EditWidget()
{
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
    default:
	d->text.insert( d->cursorPos, e->text() );
	break;
    }
    qDebug("cursorPos at %d",  d->cursorPos );
    repaint( TRUE );
}

void EditWidget::resizeEvent( QResizeEvent * )
{
    recalculate();
}


void EditWidget::paintEvent( QPaintEvent * )
{
    QPainter painter( this );
    const TextLayout *layout = TextLayout::instance();
    int start = 0;
    int y = 5;
    for ( int j = 0; j < d->lineBreaks.size(); j++ ) {
	int end = d->lineBreaks[j].pos;
	y += d->lineBreaks[j].ascent;
	unsigned char levels[256];
	int visualOrder[256];
	int x = 5;
	for ( int i = 0; i < end-start; i++ )
	    levels[i] = d->items[start+i].analysis.bidiLevel;
	layout->bidiReorder( end-start, (unsigned char *)levels, (int *)visualOrder );

	for ( int i = 0; i < end-start; i++ ) {
	    int current = visualOrder[i];
	    const ScriptItem &it = d->items[ start+current ];
	    ShapedItem shaped;
	    layout->shape( shaped, d->font, d->text, d->items, current+start );
	    layout->position( shaped );
	    if ( it.position <= d->cursorPos && d->items[ start+current+1 ].position > d->cursorPos ) {
		// draw cursor
		int xp = layout->cursorToX( shaped, d->cursorPos - it.position );
		qDebug("cursor inside run %d at pos %d (absolute=%d)", start+current, xp, x+xp );
		painter.drawLine( x+xp, y-30, x+xp, y+10 );
	    }

	    QFont::Script script = (QFont::Script)it.analysis.script;
	    FontEngineIface *fe = d->font.engineForScript( script );
// 	    qDebug("drawing item %d (pos=%d), script=%d, fe=%p", current, d->items[current].position, script, fe );
	    if ( fe && fe != (FontEngineIface *)-1 ) {
		fe->draw( &painter, x,  y, shaped.glyphs(), shaped.advances(), shaped.offsets(), shaped.count(),
			  (shaped.d->analysis.bidiLevel%2) );
		int xoff = 0;
		int yoff = 0;
		const Offset *advances = shaped.advances();
		int i = shaped.count();
		while ( i-- ) {
		    xoff += advances->x;
		    yoff += advances->y;
		    ++advances;
		}
		// 	    qDebug("width = %d", xoff );
		x += xoff;
		y += yoff;
		// 	    drawLine( x, y-20, x, y+20 );
	    }
	}
	start = end;
	y += d->lineBreaks[j].descent;
    }

}


void EditWidget::recalculate()
{
    QTime t;
    t.start();
    const TextLayout *layout = TextLayout::instance();

    layout->itemize( d->items, d->text );

    int w = width() - 10; // we leave 5 px margin
//     qDebug("recalulate: width=%d, numItems=%d", w, d->items.size() );
    // layout text
    ShapedItem shaped;
    int lw = 0;
    int line = 0;
    int allocLines = 10;
    d->lineBreaks.resize(allocLines);
    int i = 0;
    int ascent = 0;
    int descent = 0;
    while ( i < d->items.size() ) {
	layout->shape( shaped, d->font, d->text, d->items, i );
	layout->position( shaped );
	int cw = layout->width( shaped );
// 	qDebug("width(%d)=%d", i, cw );
	if ( lw + cw > w ) {
	    // need to split the current item
	    CharAttributesArray attrs;
	    layout->attributes( attrs, d->text, d->items, i );
	    if ( layout->split( d->items, i, shaped, attrs, w - lw ) ) {
		layout->shape( shaped, d->font, d->text, d->items, i );
		layout->position( shaped );
		ascent = QMAX( ascent, shaped.ascent() );
		descent = QMAX( descent, shaped.descent() );
		i++;
	    }
	    // ensure we process at least one word
	    if ( line && d->lineBreaks[line-1].pos == i ) {
		ascent = QMAX( ascent, shaped.ascent() );
		descent = QMAX( descent, shaped.descent() );
		i++;
	    }
	    d->lineBreaks[line].pos = i;
	    d->lineBreaks[line].ascent = ascent;
	    d->lineBreaks[line].descent = descent;
	    line++;
	    ascent = descent = 0;
	    if ( line >= allocLines ) {
		allocLines += 10;
		d->lineBreaks.resize( allocLines );
	    }
	    lw = 0;
// 	    qDebug("line break at item %d (position %d)", i, d->items[i].position );
	} else {
	    i++;
	    lw += cw;
	    ascent = QMAX( ascent, shaped.ascent() );
	    descent = QMAX( descent, shaped.descent() );
	}
    }
    d->lineBreaks[line].pos = d->items.size();
    d->lineBreaks[line].ascent = ascent;
    d->lineBreaks[line].descent = descent;
    d->lineBreaks.resize( line+1 );
    qDebug("recalulate took %d ms",  t.elapsed() );
}
