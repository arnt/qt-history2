/****************************************************************************
** $Id: //depot/qt/main/src/opengl/qcolormap.cpp#0 $
**
** Implementation of QColormap class
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
  \class QColormap qcolormap.h
  \brief The QColormap class is used for installing custom colormaps into
  widgets.
  
  QColormap provides a platform independent way of specifying and
  installing custom colormaps into top-level widgets. QColormap was
  originally designed to make it easier to create colormaps for use in
  the OpenGL color-index mode.
  
  Under X11 you will have to use an X server that supports either a
  GrayScale, PseudoColor or DirectColor visual class.  If your X
  server currently only provides a TrueColor, StaticColor or
  StaticGray visual, you will not be able to allocate colorcells for
  writing. If you can't get this to work, then try to set up your X
  server in 8 bit mode. It should then provide a PseudoColor
  visual. You may also experience colormap flashing if your X server
  is running in 8 bit mode.
    
  Under Windows the size of the colormap is always set to 256 colors.
  
  This class uses implicit sharing (see \link shclass.html Shared
  Classes\endlink).
  
  Example of use:
  \code
  #include <qapplication.h>
  #include <qcolormap.h>
  
  int main() 
  {
      QApplication a( argc, argv );
      
      MySuperGLWidget widget( 0 );
          
      // Do not worry about deleting the colormap, as it will
      // be deleted by the widget that it is installed in.
      QColormap * colormap = new QColormap( &widget );
      
      // This will create a colormap with colors ranging from
      // black to white.
      for ( int i = 0; i < colormap->size(); i++ )
          colormap->setRgb( i, qRgb( i, i, i ) );
	  
      widget.show();
      return a.exec();
  }
  \endcode
  
  \sa QGLWidget()
 */

/*!
  \fn QColormap & QColormap::operator=( const QColormap & map )
  Assign a shallow copy of \a map to this QColormap.
*/

/*!
  \fn bool QColormap::isValid() const
  Returns TRUE if the colormap is valid, otherwise FALSE.
  
  The most common reason for a colormap to be invalid under X11, is
  that the X server does not support the visual class that is needed
  for a read/write colormap.
*/

/*!
  \fn void QColormap::setRgb( int base, int count, const QRgb *colors )
  Set an array of cells in this colormap. \a base is the starting
  index, \a count is the number of colors that should be set, and \a
  colors is the array of colors.
*/

/*!
  \fn void QColormap::setRgb( int idx, QRgb color )
  Set cell \a idx in the colormap to \a color.
*/

/*!
  \fn QRgb QColormap::rgb( int idx ) const
  Returns the QRgb value in the colorcell \a idx.
*/

/*!
  \fn void QColormap::setColor( int idx, const QColor & color )
  Set cell \a idx in the colormap to \a color.
*/

/*!
  \fn QRgb QColormap::color( int idx ) const
  Returns the QRgb value in the colorcell \a idx.
*/

/*!
  \fn void QColormap::install( QWidget * w )
  Install this colormap into widget \a w.
*/

/*!
  \fn int QColormap::size() const
  Returns the number of colorcells in the colormap.
*/
