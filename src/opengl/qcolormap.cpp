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
  QGLWidgets.
  
  \module OpenGL
    
  This class is used to install custom colormaps in QGLWidgets that
  use the OpenGL color-index mode. You can only install one colormap
  in each top-level widget. QColormap tries to allocate all the
  colorcells in a colormap for exclusive use. This will most likely
  result in colormap flashing when running in 8 bit color mode.
  
  Under X11 you will have to use an X server that supports either a
  PseudoColor or DirectColor visual class.  If your X server currently
  only provides a TrueColor, StaticColor or StaticGray visual, you
  will not be able to allocate colorcells for writing. The QColormap
  will then be invalid. Hint: Try setting up your X server in 8 bit
  mode, and it will most likely provide a PseudoColor visual.
    
  This class uses implicit sharing (see \link shclass.html Shared
  Classes\endlink).
    
  \sa QGLWidget::setColormap()
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
  \fn Qt::HANDLE QColormap::colormap() const
  Returns the system specific colormap handle.
*/

/*!
  \fn int QColormap::size() const
  Returns the number of colorcells in the colormap.
*/
