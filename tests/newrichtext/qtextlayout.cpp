#include <qtextlayout.h>
#include <qtextengine.h>

#include <malloc.h>


int QTextItem::width() const
{
    return engine->items[item].width;
}

int QTextItem::ascent() const
{
    return engine->items[item].ascent;
}

int QTextItem::descent() const
{
    return engine->items[item].descent;
}

int QTextItem::baselineAdjustment() const
{
    return engine->items[item].baselineAdjustment;
}


void QTextItem::setWidth( int w )
{
    engine->items[item].width = w;
}

void QTextItem::setAscent( int a )
{
    engine->items[item].ascent = a;
}

void QTextItem::setDescent( int d )
{
    engine->items[item].descent = d;
}


void QTextItem::setBaselineAdjustment( int adjust )
{
    engine->items[item].baselineAdjustment = adjust;
}


int QTextItem::cursorToX( int *cPos, Edge edge = Leading )
{

}

int QTextItem::xToCursor( int x )
{

}


bool QTextItem::isRightToLeft() const
{
    return (engine->items[item].analysis.bidiLevel % 2);
}

bool QTextItem::isObject() const
{
    return (engine->string.at(engine->items[item].position).unicode() == 0xfffc);
}




QTextLayout::QTextLayout( const QString &string, QPainter * = 0 )
{
    // ### fix 0 font
    d = new QTextEngine( string,  0 );
}

QTextLayout::~QTextLayout()
{
    delete d;
}

/* add an additional item boundary eg. for style change */
void QTextLayout::setBoundary( int strPos )
{
    d->items.split( strPos );
}


bool QTextLayout::validCursorPosition( int strPos )
{

}


int QTextLayout::numItems() const
{
    return d->items.size();
}

QTextItem QTextLayout::itemAt( int i )
{
    return QTextItem( i,  d );
}


void QTextLayout::beginLayout()
{

}

void QTextLayout::beginLine( int width )
{

}


QTextItem QTextLayout::nextItem()
{

}

/* ## maybe also currentItem() */
void QTextLayout::setLineWidth( int newWidth )
{

}

int QTextLayout::availableWidth() const
{

}


/* returns true if completely added */
bool QTextLayout::addCurrentItem()
{

}


void QTextLayout::endLine( int x, int y, Qt::AlignmentFlags alignment )
{

}

void QTextLayout::endLayout()
{

}


