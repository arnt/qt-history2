/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/****************************************************************************
**
** This is a simple QGLWidget displaying an openGL wireframe box
**
** The OpenGL code is mostly borrowed from Brian Pauls "spin" example
** in the Mesa distribution
**
****************************************************************************/

#include "glbox.h"

#include <qaxaggregated.h>
#include <quuid.h>
#include <objsafe.h>

#if defined(Q_CC_MSVC)
#pragma warning(disable:4305) // init: truncation from const double to float
#endif

/*!
  Create a GLBox widget
*/

GLBox::GLBox( QWidget* parent, const char* name )
    : QGLWidget( parent )
{
    xRot = yRot = zRot = 0.0;		// default object rotation
    scale = 1.25;			// default object scale
    object = 0;
}


/*!
  Release allocated resources
*/

GLBox::~GLBox()
{
    makeCurrent();
    glDeleteLists( object, 1 );
}


/*!
  Paint the box. The actual openGL commands for drawing the box are
  performed here.
*/

void GLBox::paintGL()
{
    glClear( GL_COLOR_BUFFER_BIT );

    glLoadIdentity();
    glTranslatef( 0.0, 0.0, -10.0 );
    glScalef( scale, scale, scale );

    glRotatef( xRot, 1.0, 0.0, 0.0 ); 
    glRotatef( yRot, 0.0, 1.0, 0.0 ); 
    glRotatef( zRot, 0.0, 0.0, 1.0 );

    glCallList( object );
}


/*!
  Set up the OpenGL rendering state, and define display list
*/

void GLBox::initializeGL()
{
    qglClearColor(Qt::black); 		// Let OpenGL clear to black
    object = makeObject();		// Generate an OpenGL display list
    glShadeModel( GL_FLAT );
}



/*!
  Set up the OpenGL view port, matrix mode, etc.
*/

void GLBox::resizeGL( int w, int h )
{
    glViewport( 0, 0, (GLint)w, (GLint)h );
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glFrustum( -1.0, 1.0, -1.0, 1.0, 5.0, 15.0 );
    glMatrixMode( GL_MODELVIEW );
}


/*!
  Generate an OpenGL display list for the object to be shown, i.e. the box
*/

GLuint GLBox::makeObject()
{	
    GLuint list;

    list = glGenLists( 1 );

    glNewList( list, GL_COMPILE );

    qglColor(Qt::white);		      // Shorthand for glColor3f or glIndex

    glLineWidth( 2.0 );

    glBegin( GL_LINE_LOOP );
    glVertex3f(  1.0,  0.5, -0.4 );
    glVertex3f(  1.0, -0.5, -0.4 );
    glVertex3f( -1.0, -0.5, -0.4 );
    glVertex3f( -1.0,  0.5, -0.4 );
    glEnd();

    glBegin( GL_LINE_LOOP );
    glVertex3f(  1.0,  0.5, 0.4 );
    glVertex3f(  1.0, -0.5, 0.4 );
    glVertex3f( -1.0, -0.5, 0.4 );
    glVertex3f( -1.0,  0.5, 0.4 );
    glEnd();

    glBegin( GL_LINES );
    glVertex3f(  1.0,  0.5, -0.4 );   glVertex3f(  1.0,  0.5, 0.4 );
    glVertex3f(  1.0, -0.5, -0.4 );   glVertex3f(  1.0, -0.5, 0.4 );
    glVertex3f( -1.0, -0.5, -0.4 );   glVertex3f( -1.0, -0.5, 0.4 );
    glVertex3f( -1.0,  0.5, -0.4 );   glVertex3f( -1.0,  0.5, 0.4 );
    glEnd();

    glEndList();

    return list;
}


/*!
  Set the rotation angle of the object to \e degrees around the X axis.
*/

void GLBox::setXRotation( int degrees )
{
    xRot = (GLfloat)(degrees % 360);
    updateGL();
}


/*!
  Set the rotation angle of the object to \e degrees around the Y axis.
*/

void GLBox::setYRotation( int degrees )
{
    yRot = (GLfloat)(degrees % 360);
    updateGL();
}


/*!
  Set the rotation angle of the object to \e degrees around the Z axis.
*/

void GLBox::setZRotation( int degrees )
{
    zRot = (GLfloat)(degrees % 360);
    updateGL();
}

class ObjectSafetyImpl : public QAxAggregated, 
			 public IObjectSafety
{
public:
    ObjectSafetyImpl() {}

    long queryInterface( const QUuid &iid, void **iface )
    {
	*iface = 0;
	if ( iid == IID_IObjectSafety )
	    *iface = (IObjectSafety*)this;
	else
	    return E_NOINTERFACE;

	AddRef();
	return S_OK;
    }

    QAXAGG_IUNKNOWN;

    HRESULT WINAPI GetInterfaceSafetyOptions( REFIID riid, DWORD *pdwSupportedOptions, DWORD *pdwEnabledOptions )
    {
	*pdwSupportedOptions = INTERFACESAFE_FOR_UNTRUSTED_DATA | INTERFACESAFE_FOR_UNTRUSTED_CALLER;
	*pdwEnabledOptions = INTERFACESAFE_FOR_UNTRUSTED_DATA | INTERFACESAFE_FOR_UNTRUSTED_CALLER;
	return S_OK;
    }
    HRESULT WINAPI SetInterfaceSafetyOptions( REFIID riid, DWORD pdwSupportedOptions, DWORD pdwEnabledOptions )
    {
	return S_OK;
    }
};

QAxAggregated *GLBox::createAggregate()
{
    return new ObjectSafetyImpl();
}

