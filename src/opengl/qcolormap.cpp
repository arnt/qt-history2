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
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

/*!
  \class QColormap qcolormap.h
  \brief The QColormap class is used for installing custom colormaps into
  QGLWidgets.
  
  \module OpenGL
    
  This class is used to install custom colormaps in QGLWidgets that
  use the OpenGL color-index mode.
    
  This class uses implicit sharing.
    
  \sa QGLWidget::setColormap()
 */

/*!
  \fn QColormap &operator=( const QColormap & map )
  Assign a shallow copy of \a map to this QColormap.
*/

/*!
  \fn bool isValid() const
  Returns TRUE if the colormap is valid. All attempts to use an invalid
  colormap will fail.
  
  The most common reason for a colormap to be invalid under X11 is
  that the X server does not support the visual class that is need for
  a read/write colormap. 
*/

/*!
  \fn void setEntry( int idx, QRgb color )
  Set entry \a idx in the colormap to \a color.
*/

/*!
  \fn void setEntry( int idx, QColor & color )
  Set entry \a idx in the colormap to \a color.
*/
