/****************************************************************************
** $Id$
**
** Implementation of QTestMenuBar class
**
** Created : 010301
**
** Copyright (C) 1992-2001 Trolltech AS.  All rights reserved.
**
** This file is part of the Qt TestFramework of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qtestmenubar_p.h"

/*!
  \class QTestMenuBar qtestmenubar.h
  \brief The QTestMenuBar class is an extension to QMenuBar and introduces
  functionality for testing a QMenuBar.
  
	Detailed description

   <strong>Groups of functions:</strong>
  <ul>

  <li> Construction:
	QTestApplication(),
	~QTestApplication().

  </ul>
*/

/*!
	Converts a menuBar item referenced by \a item_index into a \a mouse_pos.
	Returns TRUE if the conversion was valid.
*/

bool QTestMenuBar::index2MousePos(int item_index, QPoint &mouse_pos)
{
/*
    QRect* item_rects = 0;
    calculateItemRects (item_rects);
    if (item_rects) {
        mouse_pos = item_rects[item_index].topLeft () + QPoint(4, 4);
	return TRUE;
    }
*/
    QRect rect;
    rect = itemRect(item_index);
    if (rect != QRect(0,0,0,0)) {
    mouse_pos = rect.topLeft() + QPoint(4, 4);
	return TRUE;
    }
    return FALSE;
}

/*!
    Converts a mouse position \a pos into a menuBar item \a index that is 
    visible in that region.
    Returns TRUE if the conversion is valid.
*/

bool QTestMenuBar::mousePos2Index(const QPoint &pos, int &index)
{
    index = itemAtPos(pos);
    return (index >= 0);
/*
    QRect* item_rects = 0;
    calculateItemRects (item_rects);
    if ( ! item_rects)
	return FALSE;

    int i = 0;
    while ( i < (int)mitems->count() ) {
	if ( item_rects[i].contains( pos ) ) {
	    QMenuItem *mi = mitems->at(i);
	    index = ( mi->isSeparator() ? -1 : i);
	    return (index >= 0);
	}
	++i;
    }
    return FALSE;					// no match
*/
}

/*
// Motif style parameters

static const int motifBarFrame		= 2;	// menu bar frame width
static const int motifBarHMargin	= 2;	// menu bar hor margin to item
static const int motifBarVMargin	= 1;	// menu bar ver margin to item
static const int motifItemFrame		= 2;	// menu item frame width
static const int motifItemHMargin	= 5;	// menu item hor text margin
static const int motifItemVMargin	= 4;	// menu item ver text margin

int QTestMenuBar::calculateItemRects (QRect* &item_rects)
{
# define irects item_rects
    polish();
    bool update = TRUE;   // keep 'update' for easier updates
    int max_width = -1;

    if ( update ) {
		//rightSide = 0;
		//if ( !badSize )				// size was not changed
			//return 0;
		if ( irects )				// Avoid purify complaint.
			delete [] irects;

		if ( mitems->isEmpty() ) {
			irects = 0;
			return 0;
		}
		
		int i = mitems->count();		// workaround for gcc 2.95.2
		irects = new QRect[ i ];		// create rectangle array
		Q_CHECK_PTR( irects );
		max_width = width();
    }

    QFontMetrics fm = fontMetrics();
    int max_height = 0;
    int max_item_height = 0;
    int nlitems = 0;				// number on items on cur line
    GUIStyle gs = style().guiStyle ();
    int x = motifBarFrame + motifBarHMargin;
    int y = motifBarFrame + motifBarVMargin;
    int i = 0;
    int separator = -1;
    if ( gs == Qt::WindowsStyle )	//###
	x = y = 2;
    while ( i < (int)mitems->count() ) {	// for each menu item...
		QMenuItem *mi = mitems->at(i);
		int w=0, h=0;

		if ( mi->widget() ) {
			if ( mi->widget()->parentWidget() != this ) {
				mi->widget()->reparent( this, QPoint(0,0), TRUE );
			}
		
			w = mi->widget()->sizeHint().width()+2;
			h = mi->widget()->sizeHint().height()+2;
			if ( separator < 0 )
			separator = i;
		} else if ( mi->pixmap() ) {			// pixmap item
			w = mi->pixmap()->width() + 4;
			h = mi->pixmap()->height() + 4;
		} else if ( !mi->text().isNull() ) {	// text item
			QString s = mi->text();
			w = fm.boundingRect( s ).width()
			+ 2*motifItemHMargin;
			w -= s.contains('&')*fm.width('&');
			w += s.contains("&&")*fm.width('&');
			h = fm.height() + motifItemVMargin;
		} else if ( mi->isSeparator() ) {	// separator item
				if ( gs == Qt::MotifStyle )
			separator = i; //### only motif?
		}

		if ( !mi->isSeparator() || mi->widget() ) {
            if ( gs == Qt::MotifStyle ) {
				w += 2*motifItemFrame;
				h += 2*motifItemFrame;
			}

			if ( x + w + motifBarFrame - max_width > 0 && nlitems > 0 ) {
				nlitems = 0;
				x = motifBarFrame + motifBarHMargin;
				y += h + motifBarHMargin;
				separator = -1;
			}

			if ( y + h + 2*motifBarFrame > max_height )
				max_height = y + h + 2*motifBarFrame;
	    
			if ( h > max_item_height )
				max_item_height = h;
		}

		if ( update ) {
			irects[i].setRect( x, y, w, h );
		}
	
		x += w;
		nlitems++;
		i++;
    }

    if ( update ) {
		if ( separator >= 0 ) {
		    int moveBy = max_width - x;
			//rightSide = x;
			while( --i >= separator ) {
				irects[i].moveBy( moveBy, 0 );
		    }
		} else {
		    //rightSide = width()-frameWidth();
		}
	
		if ( max_height != height() )
			resize( max_width, max_height );

 		for ( i = 0; i < (int)mitems->count(); i++ ) {
 			irects[i].setHeight( max_item_height  );
			QMenuItem *mi = mitems->at(i);
			if ( mi->widget() ) {
				QRect r ( QPoint(0,0), mi->widget()->sizeHint() );
				r.moveCenter( irects[i].center() );
				mi->widget()->setGeometry( r );
			}
		}
	
		badSize = FALSE;
    }

    return max_height;
# undef irects
}
*/