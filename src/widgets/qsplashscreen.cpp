/****************************************************************************
** $Id:$
**
** Definition of QSplashScreen class
**
** Copyright (C) 2003 Trolltech AS.  All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
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

#include "qsplashscreen.h"

#include "qapplication.h"
#include "qlabel.h"
#include "qpainter.h"
#include "qpixmap.h"

/*!
   \class QSplashScreen qsplashscreen.h
   \brief The QSplashScreen widget provides a splash screen that can be shown during
   application startup.

   \mainclass

   A splash screen is a widget that is typically displayed when an application is being
   started.  This provides feedback to the user and potential advertising space to the
   software makers.  The splash screen will be on top of all the windows and centered
   on the screen.  Note: some X11 window managers do not support the "stays on top" flag,
   in such cases it may be necessary to set up a timer that periodically calls raise()
   on the splash screen to enforce such behavior.

   The most common case is to show a splash screen before the main widget is
   displayed on the screen.  This is illustrated in the following code snippet.

   \code
   int main( int argc, char **argv )
   {
       QApplication app( argc, argv );
       QPixmap pixmap( "splash.png" );
       QSplashScreen *splash = new QSplashScreen( pixmap );
       QMainWindow *mainWin = new QMainWindow;
       ...
       app.setMainWidget( mainWin );
       mainWin->show();
       splash->finish( mainWin );
       delete splash;
       return app.exec();
   }
   \endcode

   Additionally, it is also possible to display messages on the splash screen
   informing users what is going on with the setStatus() function.

   The user can hide the splash screen by clicking on it with the mouse.  Since
   the splash screen is typically displayed when there is not an event loop running,
   it is necessary to periodically call QApplication::processEvents() to receive the
   mouse button press.

   \code
   QSplashScreen *splash = new QSplashScreen( "splash.png" );
   ... // Loading some items
   qApp->processEvents();
   ... // Reading settings
   qApp->processEvents();
   \endcode

   It is always possible to have other behavior by reimplementing mousePressEvent() or
   any other QWidget event functions.
*/

/*!
  Construct a splash screen that will display \a pixmap.
*/

QSplashScreen::QSplashScreen( const QPixmap &pixmap, WFlags f )
    : QWidget( 0, 0, WStyle_Customize | WStyle_Splash | f ), pix( pixmap )
{
    setErasePixmap( pix );
    resize( pixmap.size() );
    move( QApplication::desktop()->screenGeometry().center()
	  - rect().center() );
    show();
    repaint();
}

/*!
  \reimp
*/

void QSplashScreen::mousePressEvent( QMouseEvent * )
{
    hide();
}

/*!
  This is an override of QWidget::repaint().  It differs from the
  standard repaint function in that it additionally calls QApplication::flush()
  to ensure the updates are displayed when there is no event loop present.
*/
void QSplashScreen::repaint()
{
    QWidget::repaint();
    QApplication::flush();
}

/*!
  Paints \a message onto the splash screen with color \a color and aligns the text
  according to the flags in \a alignment.
  \sa Qt::AlignmentFlags
*/
void QSplashScreen::setStatus( const QString &message, int alignment,
			       const QColor &color )
{
    QPixmap textPix = pix;
    QPainter painter( &textPix, this );
    painter.setPen( color );
    QRect r = rect();
    r.setRect( r.x() + 10, r.y() + 10, r.width() - 20, r.height() - 20 );
    painter.drawText( r, alignment, message );
    setErasePixmap( textPix );
    repaint();
}

/*!
  Makes the splash screen wait until the widget \a mainWin is displayed before
  calling close() on itself.
*/
void QSplashScreen::finish( QWidget *mainWin )
{
#if defined(Q_WS_X11)
    extern void qt_wait_for_window_manager( QWidget *mainWin );
    qt_wait_for_window_manager( mainWin );
#endif
    close();
}
