/**********************************************************************
**
** Copyright (C) 2000-2001 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <qgl.h>

class GLWidget : public QGLWidget
{
    Q_OBJECT
    Q_PROPERTY( double xRot READ xRot WRITE setXRot )
    Q_PROPERTY( double yRot READ yRot WRITE setYRot )
    Q_PROPERTY( double zRot READ zRot WRITE setZRot )
    Q_PROPERTY( double scale READ scale WRITE setScale )

public:
    GLWidget( QWidget* parent, const char* name );
    ~GLWidget();

    double xRot() const { return xrot; }
    double yRot() const { return yrot; }
    double zRot() const { return zrot; }
    double scale() const { return scale_; }

public slots:
    void setXRot( double );
    void setYRot( double );
    void setZRot( double );
    void setScale( double );

protected:
    void		initializeGL();
    void		paintGL();
    void		resizeGL( int w, int h );

    virtual GLuint 	makeObject();

private:
    GLuint object;
    GLfloat xrot, yrot, zrot, scale_;
};

#endif
