/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qglcolormap.cpp#0 $
**
** Implementation of QGLColormap class
**
** Created : 20010326
**
** Copyright (C) 1992-2001 Trolltech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
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
** Licensees holding valid Qt Enterprise Edition licenses may use this
** file in accordance with the Qt Commercial License Agreement provided
** with the Software.
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

/*!
  \class QGLColormap qglcolormap.h
  \brief The QGLColormap class is used for installing custom colormaps into
  widgets.
  
  QGLColormap provides a platform independent way of specifying and
  installing indexed colormaps into QGLWidgets. QGLColormap is 
  especially useful when using the \link opengl.html OpenGL\endlink
  color-index mode.
  
  Under X11 you will have to use an X server that supports either a
  GrayScale, PseudoColor or DirectColor visual class.  If your X
  server currently only provides a TrueColor, StaticColor or
  StaticGray visual, you will not be able to allocate colorcells for
  writing. If your X server does not provide one of the needed
  visuals, try setting up your X server in 8 bit mode. It should then
  provide a you with at least a PseudoColor visual. Note that you may
  experience colormap flashing if your X server is running in 8 bit
  mode.
    
  Under Windows the size of the colormap is always set to 256
  colors.
  
  This class uses explicit sharing (see \link shclass.html Shared
  Classes\endlink).
  
  Example of use:
  \code
  #include <qapplication.h>
  #include <qglcolormap.h>
  
  int main() 
  {
      QApplication a( argc, argv );
      
      MySuperGLWidget widget( 0 );
      QGLColormap colormap;
      
      // This will fill the colormap with colors ranging from
      // black to white.
      for ( int i = 0; i < colormap->size(); i++ )
          colormap->setRgb( i, qRgb( i, i, i ) );
	  
      widget.setColormap( colormap );
      widget.show();
      return a.exec();
  }
  \endcode
  
  \sa QGLWidget::setColormap(), QGLWidget::colormap()
*/

#include "qglcolormap.h"
#include "qmemarray.h"


/*!
  Construct a QGLColormap.
*/
QGLColormap::QGLColormap()
{
    d = 0;
}


/*!
  Construct a shallow copy of \a map.
*/
QGLColormap::QGLColormap( const QGLColormap & map )
{
    d = map.d;
    if ( d )
	d->ref();
}

/*!
  Dereferences the QGLColormap and deletes it if this was the last reference.
*/
QGLColormap::~QGLColormap()
{
    if ( d && d->deref() ) {
	delete d;
	d = 0;
    }
}

/*!
  Assign a shallow copy of \a map to this QGLColormap.
*/
QGLColormap & QGLColormap::operator=( const QGLColormap & map )
{
    if ( map.d != 0 )
	map.d->ref();
    
    if ( d && d->deref() )
	delete d;
    d = map.d;
    
    return *this;
}

/*!
  Detaches this QGLColormap from the shared block.
 */
void QGLColormap::detach()
{
    if ( d && d->count != 1 ) {
	// ### What about the actual colormap handle?
	Private * newd = new Private();
	newd->cells = d->cells;
	newd->cells.detach();
	if ( d->deref() )
	    delete d;
	d = newd;
    }
}

/*!
  Set cell \a idx in the colormap to \a color.
*/
void QGLColormap::setEntry( int idx, QRgb color )
{
    if ( !d )
	d = new Private();
    
#if defined(QT_CHECK_RANGE)
    if ( idx < 0 || idx > (int) d->cells.size() ) {
	qWarning( "QGLColormap::setRgb: Index out of range." );
	return;
    }
#endif
    d->cells[ idx ] = color;
}

/*!
  Set an array of cells in this colormap. \a base is the starting
  index, \a count is the number of colors that should be set, and \a
  colors is the array of colors.
*/
void QGLColormap::setEntries( int base, int count, const QRgb * colors )
{
    if ( !d )
	d = new Private();
	
    if ( !colors || base < 0 || base >= (int) d->cells.size() )
	return;
    
    for( int i = base; i < base + count; i++ ) {
	if ( i < (int) d->cells.size() )
	    setEntry( i, colors[i] );
	else
	    break;
    }
}

/*!
  Returns the QRgb value in the colorcell with index \a idx.
*/
QRgb QGLColormap::entryRgb( int idx ) const
{
    if ( !d || idx < 0 || idx > (int) d->cells.size() )
	return 0;
    else
	return d->cells[ idx ];
}

/*!
  Set cell with index \a idx in the colormap to color \a color.
*/
void QGLColormap::setEntry( int idx, const QColor & color )
{
    setEntry( idx, color.rgb() );
}

/*!
  Returns the QRgb value in the colorcell with index \a idx.
*/
QColor QGLColormap::entryColor( int idx ) const
{
    if ( !d || idx < 0 || idx > (int) d->cells.size() )
	return QColor();
    else
	return QColor( d->cells[ idx ] );
}

/*!
  Returns TRUE if the colormap is valid, otherwise FALSE.
  
  The most common reason for a colormap to be invalid under X11, is
  that the X server does not support the visual class that is needed
  for a read/write colormap. An empty colormap (no color values set) is
  also considered to be invalid.
*/
bool QGLColormap::isEmpty() const
{
    return (d != 0) && (d->cells.size() > 0);
}


/*!
  Returns the number of colorcells in the colormap.
*/
int QGLColormap::size() const
{
    return d != 0 ? d->cells.size() : 0;
}
