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

#ifndef QT_NO_SPLASHSCREEN

#include "qapplication.h"
#include "qpainter.h"
#include "qpixmap.h"

class QSplashScreenPrivate
{
public:
    QPixmap pixmap;
    QString currStatus;
    QColor currColor;
    int currAlign;
};

/*!
   \class QSplashScreen qsplashscreen.h
   \brief The QSplashScreen widget provides a splash screen that can be shown during
   application startup.

    \ingroup misc
    \mainclass

   A splash screen is a widget that is usually displayed when an
   application is being started. Splash screens are often used for
   applications that have long start up times (e.g. database or
   networking applications that take time to establish connections) to
   provide the user with feedback that the application is loading.

   The splash screen will be on top of all the windows and centered on
   the screen. Some X11 window managers do not support the "stays on
   top" flag, in such cases it may be necessary to set up a timer that
   periodically calls raise() on the splash screen to get the "stays
   on top" effect.

   The most common usage is to show a splash screen before the main
   widget is displayed on the screen. This is illustrated in the
   following code snippet.

   \code
   int main( int argc, char **argv )
   {
       QApplication app( argc, argv );
       QPixmap pixmap( "splash.png" );
       QSplashScreen *splash = new QSplashScreen( pixmap );
       splash->show();
       QMainWindow *mainWin = new QMainWindow;
       ...
       app.setMainWidget( mainWin );
       mainWin->show();
       splash->finish( mainWin );
       delete splash;
       return app.exec();
   }
   \endcode

   It is sometimes useful to update the splash screen with messages,
   for example, announcing connections established or modules loaded
   as the application starts up. QSplashScreen supports this with the
   message() function. If you wish to do your own drawing you can
   get a pointer to the pixmap used in the splash screen with pixmap().
   Alternatively, you can subclass QSplashScreen and reimplement
   drawContents().

   The user can hide the splash screen by clicking on it with the
   mouse. Since the splash screen is typically displayed before the
   event loop has started running, it is necessary to periodically
   call QApplication::processEvents() to receive the mouse clicks.

   \code
   QSplashScreen *splash = new QSplashScreen( "splash.png" );
   splash->show();
   ... // Loading some items
   splash->message( "Loaded modules" );
   qApp->processEvents();
   ... // Establishing connections
   splash->message( "Established connections" );
   qApp->processEvents();
   \endcode

*/

/*!
    Construct a splash screen that will display \a pixmap.

    There should be no need to set the widget flags, \a f, other than
    perhaps WDestructiveClose.
*/
QSplashScreen::QSplashScreen( const QPixmap &pixmap, WFlags f )
    : QWidget( 0, 0, WStyle_Customize | WStyle_Splash | f )
{
    d = new QSplashScreenPrivate();
    d->pixmap = pixmap;
    setPixmap( d->pixmap );  // Does an implicit repaint
}

/*!
  Destructor.
*/
QSplashScreen::~QSplashScreen()
{
    delete d;
}

/*!
    \reimp
*/
void QSplashScreen::mousePressEvent( QMouseEvent * )
{
    hide();
}

/*!
    This is an override of QWidget::repaint(). It differs from the
    standard repaint function in that it additionally calls
    QApplication::flush() to ensure the updates are displayed when
    there is no event loop present.
*/
void QSplashScreen::repaint()
{
    drawContents();
    QWidget::repaint();
    QApplication::flush();
}

/*!
    \fn QSplashScreen::messageChanged( const QString &message )

    This signal is emitted when the message on the splash screen
    changes.  \a message is the new message and is a null-string
    when the message has been removed.

    \sa message(), clear()
*/



/*!
    Draws the \a message text onto the splash screen with color \a
    color and aligns the text according to the flags in \a alignment.

    \sa Qt::AlignmentFlags clear()
*/
void QSplashScreen::message( const QString &message, int alignment,
			     const QColor &color )
{
    d->currStatus = message;
    d->currAlign = alignment;
    d->currColor = color;
    emit messageChanged( d->currStatus );
    repaint();
}

/*!
    Removes the message being displayed on the splash screen

    \sa message()
 */
void QSplashScreen::clear()
{
    d->currStatus = QString::null;
    emit messageChanged( d->currStatus );
    repaint();
}

/*!
    Makes the splash screen wait until the widget \a mainWin is
    displayed before calling close() on itself.
*/
void QSplashScreen::finish( QWidget *mainWin )
{
#if defined(Q_WS_X11)
    extern void qt_wait_for_window_manager( QWidget *mainWin );
    qt_wait_for_window_manager( mainWin );
#else
    Q_UNUSED( mainWin );
#endif
    close();
}

/*!
  Set the pixmap that will be used as the image of the splash screen to \a pixmap.
*/
void QSplashScreen::setPixmap( const QPixmap &pixmap )
{
    d->pixmap = pixmap;
    resize( d->pixmap.size() );
    move( QApplication::desktop()->screenGeometry().center()
	  - rect().center() );
    repaint();
}

/*!
  Returns the pixmap that is used in the splash screen, minus any text possibly
  set with message().
*/
QPixmap* QSplashScreen::pixmap() const
{
    return &( d->pixmap );
}

/*!
  \intern
*/
void QSplashScreen::drawContents()
{
    QPixmap textPix = d->pixmap;
    QPainter painter( &textPix, this );
    drawContents( &painter );
    setErasePixmap( textPix );
}

/*!
  Draw the contents of the splash screen using painter \a painter.  The default
  implementation draws the message passed by message().  Reimplement this
  function if you want to do your own drawing on the splash screen.
*/
void QSplashScreen::drawContents( QPainter *painter )
{
    painter->setPen( d->currColor );
    QRect r = rect();
    r.setRect( r.x() + 5, r.y() + 5, r.width() - 10, r.height() - 10 );
    painter->drawText( r, d->currAlign, d->currStatus );
}

#endif //QT_NO_SPLASHSCREEN
