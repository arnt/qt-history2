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

#include "glworkspace.h"
#include "glbox.h"
#include "glgear.h"
#include "gltexobj.h"

#include <qworkspace.h>
#include <qdialog.h>
#include <qtoolbar.h>
#include <qpopupmenu.h>
#include <qmenubar.h>
#include <q3action.h>
#include <qprinter.h>
#include <qpainter.h>
#include <qcheckbox.h>
#include <qvbox.h>
#include <qimage.h>
#include "printpreview.h"
using namespace Qt;

GLWorkspace::GLWorkspace( QWidget *parent, const char *name, WFlags f )
: Q3MainWindow( parent, name, f ), printer( 0 )
{
    setupSceneActions();

    QVBox *vbox = new QVBox( this );
    vbox->setFrameStyle( QFrame::StyledPanel | QFrame::Sunken );
    vbox->setMargin( 1 );
    vbox->setLineWidth( 1 );

    workspace = new QWorkspace( vbox );
    workspace->setBackgroundMode( PaletteMid );
    setCentralWidget( vbox );
}

GLWorkspace::~GLWorkspace()
{
}

void GLWorkspace::setupSceneActions()
{
    Q3ToolBar *tb = new Q3ToolBar( "Scene", this );
    QPopupMenu *menu = new QPopupMenu( this );
    menuBar()->insertItem( tr( "&Scene" ), menu );

    Q3Action *a;
    Q3ActionGroup *newGroup = new Q3ActionGroup( this );
    newGroup->setMenuText( tr( "&New" ) );
    newGroup->setText( tr( "New" ) );
    newGroup->setUsesDropDown( TRUE );
    newGroup->setExclusive( FALSE );
    newGroup->setIconSet( QPixmap( "textdrawing/filenew.png" ) );
    a = new Q3Action( tr( "Wirebox" ), QPixmap( "opengl/wirebox.xpm" ), tr( "&Wirebox" ), 0, newGroup );
    connect( a, SIGNAL( activated() ), this, SLOT( newWirebox() ) );
    a = new Q3Action( tr( "Gear" ), QPixmap( "opengl/gear.xpm" ), tr( "&Gears" ), 0, newGroup );
    connect( a, SIGNAL( activated() ), this, SLOT( newGear() ) );
    a = new Q3Action( tr( "Texture" ), QPixmap( "opengl/texture.xpm" ), tr( "&Texture" ), 0, newGroup );
    connect( a, SIGNAL( activated() ), this, SLOT( newTexture() ) );
/*    a = new Q3Action( tr( "Nurbs" ), QPixmap( "opengl/nurbs.xpm" ), tr( "&Nurbs" ), 0, newGroup );
    connect( a, SIGNAL( activated() ), this, SLOT( newNurbs() ) );*/
    newGroup->addTo( tb );
    newGroup->addTo( menu );

    menu->insertSeparator();
    Q3ActionGroup *printGroup = new Q3ActionGroup( this );
    printGroup->setMenuText( tr( "&Print" ) );
    printGroup->setText( tr( "Print" ) );
    printGroup->setUsesDropDown( TRUE );
    printGroup->setExclusive( FALSE );
    printGroup->setIconSet( QPixmap( "textdrawing/print.png" ) );
    Q3Action *da = new Q3Action( tr( "Window Size" ), QPixmap( "textdrawing/print.png" ), tr( "&Window Size" ), CTRL + Key_P, printGroup );
    connect( da, SIGNAL( activated() ), this, SLOT( filePrintWindowRes() ) );
    a = new Q3Action( tr( "Low Resolution" ), tr( "&Low Resolution" ), 0, printGroup );
    connect( a, SIGNAL( activated() ), this, SLOT( filePrintLowRes() ) );
    a = new Q3Action( tr( "Medium Resolution" ), tr( "&Medium Resolution" ), 0, printGroup );
    connect( a, SIGNAL( activated() ), this, SLOT( filePrintMedRes() ) );
    a = new Q3Action( tr( "High Resolution" ), tr( "&High Resolution" ), 0, printGroup );
    connect( a, SIGNAL( activated() ), this, SLOT( filePrintHighRes() ) );
    printGroup->addSeparator();
    a = new Q3Action( tr( "Setup" ), tr( "&Setup..." ), 0, printGroup );
    connect( a, SIGNAL( activated() ), this, SLOT( filePrintSetup() ) );
    da->addTo( tb );
    printGroup->addTo( menu );

    a = new Q3Action( tr( "Close" ), QPixmap(), tr( "&Close" ), 0, this );
    connect( a, SIGNAL( activated() ), this, SLOT( fileClose() ) );
    a->addTo( menu );
}

void GLWorkspace::newWirebox()
{
    GLBox *gl = new GLBox( workspace, 0, WDestructiveClose );
    gl->setIcon( QPixmap( "opengl/wirebox.xpm" ) );
    gl->setWindowTitle( tr( "Wirebox" ) );
    gl->resize( 320, 240 );
    gl->show();
}

void GLWorkspace::newGear()
{
    GLGear *gl = new GLGear( workspace, 0, WDestructiveClose );
    gl->setIcon( QPixmap( "opengl/gear.xpm" ) );
    gl->setWindowTitle( tr( "Gear" ) );
    gl->resize( 320, 240 );
    gl->show();
}

void GLWorkspace::newTexture()
{
    GLTexobj *gl = new GLTexobj( workspace, 0, WDestructiveClose );
    gl->setIcon( QPixmap( "opengl/texture.xpm" ) );
    gl->setWindowTitle( tr( "Texture" ) );
    gl->resize( 320, 240 );
    gl->show();
}

void GLWorkspace::newNurbs()
{
    GLGear *gl = new GLGear ( workspace, 0, WDestructiveClose );
    gl->setIcon( QPixmap( "opengl/nurbs.xpm" ) );
    gl->setWindowTitle( tr( "Nurbs" ) );
    gl->resize( 320, 240 );
    gl->show();
}

void GLWorkspace::filePrint( int x, int y )
{
    bool print = printer || filePrintSetup();
    if ( !print || !printer )
	return;

    QWidget *widget = workspace->activeWindow();
    if ( !widget || !widget->inherits( "QGLWidget" ) )
	return;
    QGLWidget *gl = (QGLWidget *)widget;
    QPixmap pm = gl->renderPixmap( x, y );

    PrintPreview view( this );
    QImage temp = pm.convertToImage();
    temp = temp.smoothScale( 400, 300 );
    QPixmap temppix;
    temppix.convertFromImage( temp );
    view.setPixmap( temppix );
    view.setIcon( QPixmap( "opengl/snapshot.xpm" ) );
    view.setWindowTitle( gl->caption() + " - Print preview" );
    if ( view.exec() ) {
	QImage img = pm.convertToImage();
	if ( view.checkInvert->isChecked() ) {
	    img.invertPixels();
	}
	if ( view.checkMirror->isChecked() ) {
	    img = img.mirror( TRUE, FALSE );
	}
	if ( view.checkFlip->isChecked() ) {
	    img = img.mirror( FALSE, TRUE );
	}
	if ( view.checkLeft->isEnabled() && view.checkLeft->isChecked() ) {
	}
	if ( view.checkRight->isEnabled() && view.checkRight->isChecked() ) {
	}
	pm.convertFromImage( img );

	QPainter painter;
	if ( !painter.begin( printer ) )
	    return;

	painter.drawPixmap( QPoint( 0, 0 ), pm );

	painter.end();
    }
}

void GLWorkspace::filePrintWindowRes()
{
    filePrint( 0, 0 );
}

void GLWorkspace::filePrintLowRes()
{
    filePrint( 640, 480 );
}

void GLWorkspace::filePrintMedRes()
{
    filePrint( 1024, 768 );
}

void GLWorkspace::filePrintHighRes()
{
    filePrint( 2048, 1536 );
}

bool GLWorkspace::filePrintSetup()
{
    bool newPrinter = !printer;

    if ( !printer )
	printer = new QPrinter;
    if ( printer->setup() ) {
	return TRUE;
    } else {
	if ( newPrinter ) {
	    delete printer;
	    printer = 0;
	}
	return FALSE;
    }
}

void GLWorkspace::fileClose()
{
    workspace->closeActiveWindow();
}
