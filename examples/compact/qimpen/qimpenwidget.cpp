/****************************************************************************
** $Id: //depot/qt/qws/util/qws/qws.h#4 $
**
** Definition of pen input method widget
**
** Created : 20000414
**
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qapplication.h>
#include <qinputdialog.h>
#include <qpainter.h>
#include <qfile.h>
#include <qdatastream.h>
#include <qtimer.h>
#include "qimpenchar.h"
#include "qimpenwidget.h"

#define TITLE_WIDTH	30  // ### magic

/*!
  \class QIMPenWidget qimpenwidget.h

  Draws characters and allows input of characters.
*/

QIMPenWidget::QIMPenWidget( QWidget *parent )
 : QWidget( parent )
{
    inputStroke = 0;
    outputChar = 0;
    outputStroke = 0;
    mode = Waiting;
    currCharSet = 0;
    strokes.setAutoDelete( TRUE );

    timer = new QTimer(this);
    connect( timer, SIGNAL(timeout()), SLOT(timeout()));

    setBackgroundColor( qApp->palette().color( QPalette::Normal,
                                               QColorGroup::Base ) );
}

void QIMPenWidget::clear()
{
    outputChar = 0;
    outputStroke = 0;
    strokes.clear();
    repaint();
}

void QIMPenWidget::removeStroke()
{
    strokes.removeFirst();
    update();
}

/*!
  Add a character set to the list.
*/
void QIMPenWidget::addCharSet( QIMPenCharSet *cs )
{
    if ( !currCharSet )
        currCharSet = cs;
    charSets.append( cs );
    update();
}

/*!
  Remove a character set from the list.
*/
void QIMPenWidget::removeCharSet( QIMPenCharSet *cs )
{
    charSets.remove( cs );
    if ( currCharSet == cs ) {
        currCharSet = charSets.first();
    }
    update();
}

/*!
  Display a character. \a speed determines how quickly the character is
  drawn.
*/
void QIMPenWidget::showCharacter( QIMPenChar *ch, int speed )
{
    outputChar = 0;
    outputStroke = 0;
    mode = Output;
    repaint();
    if ( !ch || ch->isEmpty() ) {
        mode = Waiting;
        return;
    }

    outputChar = ch;
    outputStroke = outputChar->penStrokes().getFirst();
    if ( speed < 0 ) speed = 0;
    if ( speed > 20 ) speed = 20;
    speed = 50 - speed;
    pointIndex = 0;
    strokeIndex = 0;
    lastPoint = outputStroke->startingPoint();
    QRect br( outputChar->boundingRect() );
    lastPoint.setX( (width() - br.width()) / 2 + (lastPoint.x () - br.left()) );
    timer->start( speed );
}

/*!
  Handle drawing/clearing of characters.
*/
void QIMPenWidget::timeout()
{
    if ( mode == Output ) {
        const QArray<QIMPenGlyphLink> &chain = outputStroke->chain();
        if ( pointIndex < chain.count() ) {
            QPainter paint( this );
            paint.setBrush( Qt::black );
            for ( unsigned i = 0; i < 3 && pointIndex < chain.count(); i++ ) {
                lastPoint.rx() += chain[pointIndex].dx;
                lastPoint.ry() += chain[pointIndex].dy;
                pointIndex++;
                paint.drawRect( lastPoint.x()-1, lastPoint.y()-1, 2, 2 );
            }
        }
        if ( pointIndex >= chain.count() ) {
            QList<QIMPenStroke> strokes = outputChar->penStrokes();
            if ( strokeIndex < (int)strokes.count() - 1 ) {
                pointIndex = 0;
                strokeIndex++;
                outputStroke = strokes.at( strokeIndex );
                lastPoint = outputChar->startingPoint();
                QRect br( outputChar->boundingRect() );
                lastPoint.setX( (width() - br.width()) / 2
                                + (lastPoint.x () - br.left()) );
                QPoint off = lastPoint - outputChar->startingPoint();
                lastPoint = outputStroke->startingPoint() + off;
            } else {
                timer->stop();
                mode = Waiting;
            }
        }
    }
}

/*!
  If the point \a p is over one of the character set titles, switch
  to the set and return TRUE.
*/
bool QIMPenWidget::selectSet( QPoint p )
{
    if ( p.x() > TITLE_WIDTH && p.x() < TITLE_WIDTH * (int)(charSets.count()+1) && p.y() < 12 ) {
        int idx = p.x() / TITLE_WIDTH - 1;
        if ( charSets.at( idx ) != currCharSet ) {
            currCharSet = charSets.at( idx );
            update();
            emit changeCharSet( currCharSet );
        }
        return TRUE;
    }

    return FALSE;
}

/*!
  Hopefully returns a sensible size.
*/
QSize QIMPenWidget::sizeHint()
{
    return QSize( TITLE_WIDTH * charSets.count(), 75 );
}

void QIMPenWidget::mousePressEvent( QMouseEvent *e )
{
    if ( e->button() == LeftButton && mode == Waiting ) {
        // if selectSet returns false the click was not over the
        // char set selectors.
        if ( !selectSet( e->pos() ) ) {
            // start of character input
            timer->stop();
            if ( outputChar ) {
                outputChar = 0;
                outputStroke = 0;
                repaint();
            }
            mode = Input;
            lastPoint = e->pos();
            emit beginStroke();
            inputStroke = new QIMPenStroke;
            strokes.append( inputStroke );
            inputStroke->beginInput( e->pos() );
            QPainter paint( this );
            paint.setBrush( Qt::black );
            paint.drawRect( lastPoint.x()-1, lastPoint.y()-1, 2, 2 );
        }
    }
}

void QIMPenWidget::mouseReleaseEvent( QMouseEvent *e )
{
    if ( e->button() == LeftButton && mode == Input ) {
        mode = Waiting;
        inputStroke->endInput();
        if ( currCharSet )
            emit stroke( inputStroke );
    }
}

void QIMPenWidget::mouseMoveEvent( QMouseEvent *e )
{
    if ( mode == Input ) {
        int dx = QABS( e->pos().x() - lastPoint.x() );
        int dy = QABS( e->pos().y() - lastPoint.y() );
        if ( dx + dy > 1 ) {
            if ( inputStroke->addPoint( e->pos() ) ) {
		QPainter paint( this );
		paint.setPen( Qt::black );
		paint.setBrush( Qt::black );
		const QArray<QIMPenGlyphLink> &chain = inputStroke->chain();
		QPoint p( e->pos() );
		for ( int i = (int)chain.count()-1; i >= 0; i-- ) {
		    paint.drawRect( p.x()-1, p.y()-1, 2, 2 );
		    p.rx() -= chain[i].dx;
		    p.ry() -= chain[i].dy;
		    if ( p == lastPoint )
			break;
		}

		/* ### use this when thick lines work properly on all devices
		paint.setPen( QPen( Qt::black, 2 ) );
		paint.drawLine( lastPoint, e->pos() );
		*/
	    }
	    lastPoint = e->pos();
        }
    }
}

void QIMPenWidget::paintEvent( QPaintEvent * )
{
    QPainter paint( this );

    // draw guidelines
    paint.setPen( Qt::gray );
    int dy = ( height() - 15 ) / 3;
    for ( int y = 12; y < height(); y += dy ) {
        paint.drawLine( 0, y, width(), y );
    }

    // draw the character set titles
    QFont selFont( "helvetica", 8, QFont::Bold );
    QFont font( "helvetica", 8 );
    QListIterator<QIMPenCharSet> it( charSets );
    int pos = TITLE_WIDTH;
    for ( ; it.current(); ++it ) {
        if ( currCharSet == it.current() ) {
            paint.setPen( Qt::black );
            paint.setFont( selFont );
        } else {
            paint.setPen( Qt::gray );
            paint.setFont( font );
        }
        paint.drawText( pos, 0, TITLE_WIDTH, 12, QPainter::AlignCenter,
                        it.current()->title() );
        pos += TITLE_WIDTH;
    }

    // draw any character that should be displayed when repainted.
    QPoint off;
    const QList<QIMPenStroke> *stk = 0;
    if ( outputChar && mode == Waiting ) {
        stk = &outputChar->penStrokes();
        QPoint p( outputChar->startingPoint() );
        QRect br( outputChar->boundingRect() );
        p.setX( (width() - br.width()) / 2 + (p.x () - br.left()) );
        off = p - outputChar->startingPoint();
    } else if ( mode == Waiting ) {
        stk = &strokes;
    }

    if ( stk && !stk->isEmpty() ) {
        paint.setPen( Qt::black );
        paint.setBrush( Qt::black );
        QListIterator<QIMPenStroke> it( *stk );
        while ( it.current() ) {
            QPoint p = it.current()->startingPoint() + off;
            paint.drawRect( p.x()-1, p.y()-1, 2, 2 );
            const QArray<QIMPenGlyphLink> &chain = it.current()->chain();
            for ( unsigned i = 0; i < chain.count(); i++ ) {
                    p.rx() += chain[i].dx;
                    p.ry() += chain[i].dy;
                    paint.drawRect( p.x()-1, p.y()-1, 2, 2 );
            }
            ++it;
        }
    }

    // debug
/*
    if ( input ) {
        QArray<int> sig = input->sig();
        for ( unsigned i = 0; i < sig.count(); i++ ) {
            paint.drawPoint( 200 + i, height()/2 - sig[i] / 8 );
        }
    }
*/
}

