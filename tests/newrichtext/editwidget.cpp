#include "qfont.h"
#include "qpainter.h"
#include "editwidget.h"
#include "fontengine.h"

#include "qtextlayout_p.h"
#include <qmemarray.h>

#include <qdatetime.h>

class EditWidgetPrivate
{
public:
    QString text;
    QFont font;
    QScriptItemArray items;
    QShapedItem shaped;
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
    QPainter painter( this );
    const TextLayout *layout = TextLayout::instance();
    int start = 0;
    int y = 5;
    for ( int j = 0; j < d->lineBreaks.size(); j++ ) {
	int end = d->lineBreaks[j].pos;
	y += d->lineBreaks[j].ascent;
	int vo[256];
	int *visualOrder = vo;
	if ( end-start > 255 )
	    visualOrder = (int *)malloc( (end-start)*sizeof(int) );
	{
	    unsigned char lv[256];
	    unsigned char *levels = lv;
	    if ( end-start > 255 )
		levels = (unsigned char *)malloc( (end-start)*sizeof(unsigned char) );
	    for ( int i = 0; i < end-start; i++ )
		levels[i] = d->items[start+i].analysis.bidiLevel;
	    layout->bidiReorder( end-start, (unsigned char *)levels, (int *)visualOrder );
	    if ( end-start > 255 )
		free( levels );
	}

	int x = 5;
	for ( int i = 0; i < end-start; i++ ) {
	    int current = visualOrder[i];
	    const QScriptItem &it = d->items[ start+current ];
	    QShapedItem shaped;
	    layout->shape( shaped, d->font, d->text, d->items, current+start );
	    if ( it.position <= d->cursorPos &&
		 (current == d->items.size()-1 || d->items[ start+current+1 ].position > d->cursorPos) ) {
		// draw cursor
		int xp = layout->cursorToX( shaped, d->cursorPos - it.position );
		qDebug("cursor inside run %d at pos %d (absolute=%d)", start+current, xp, x+xp );
		painter.drawLine( x+xp, y-30, x+xp, y+10 );
	    }

	    int swidth = layout->width( shaped );

	    QFont::Script script = (QFont::Script)it.analysis.script;
	    QFontEngineIface *fe = d->font.engineForScript( script );
// 	    qDebug("drawing item %d (pos=%d), script=%d, fe=%p", current, d->items[current].position, script, fe );
	    if ( fe && fe != (QFontEngineIface *)-1 ) {
		fe->draw( &painter, x,  y, shaped.glyphs(), shaped.advances(), shaped.offsets(), shaped.count(),
			  (shaped.d->analysis.bidiLevel%2) );
		x += swidth;
		// 	    drawLine( x, y-20, x, y+20 );
	    }
	}
	if ( end-start > 255 )
	    free( visualOrder );
	start = end;
	y += d->lineBreaks[j].descent;
// 	painter.drawLine( 0,  y+1,  1000, y+1 );
// 	y+=3;
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
    QShapedItem shaped;
    int lw = 0;
    int line = 0;
    int allocLines = 10;
    d->lineBreaks.resize(allocLines);
    int i = 0;
    int ascent = 0;
    int descent = 0;
    while ( i < d->items.size() ) {
	layout->shape( shaped, d->font, d->text, d->items, i );
	int cw = layout->width( shaped );
// 	qDebug("width(%d)=%d", i, cw );
	if ( 0 && lw + cw > w ) {
	    // need to split the current item
	    CharAttributesArray attrs;
	    layout->attributes( attrs, d->text, d->items, i );
	    if ( layout->split( d->items, i, shaped, attrs, w - lw ) ) {
		layout->shape( shaped, d->font, d->text, d->items, i );
		// dummy call to initialize the ascent and descent
		layout->width( shaped );
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
    qDebug("recalulate took %d ms (%dus/char)",  t.elapsed(), t.elapsed()*1000/d->text.length() );
}
