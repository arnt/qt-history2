#include "helpwindow.h"
#include <qurl.h>
#include <qmessagebox.h>
#include <qdragobject.h>
#include <qdir.h>
#include <qfile.h>
#include <qsettings.h>
#include <qprocess.h>
#include <qpopupmenu.h>
#include <qaction.h>

#include "mainwindow.h"

HelpWindow::HelpWindow( MainWindow *w, QWidget *parent, const char *name )
    : QTextBrowser( parent, name ), mw( w ), shiftPressed( FALSE )
{
}

void HelpWindow::setSource( const QString &name )
{
    if ( name.isEmpty() )
	return;

    if ( shiftPressed ) {
	removeSelection();
	mw->saveSettings();
	mw->saveToolbarSettings();
	MainWindow *nmw = new MainWindow;
	nmw->showLink( name, "" );
	nmw->move( mw->geometry().topLeft() );
	if ( mw->isMaximized() )
	    nmw->showMaximized();
	else
	    nmw->show();
	return;
    }

    mw->setCaption( tr( "Qt Assistant by Trolltech - %1" ).arg( name ) );
    
    if ( name.left( 7 ) == "http://" || name.left( 6 ) == "ftp://" ) {
	QSettings settings;
	QString webbrowser = settings.readEntry( "/Qt Assistant/3.1/Webbrowser/" );
	if ( webbrowser.isEmpty() ) {
	    QMessageBox::information( this, tr( "Help" ), tr( "Currently no webbrowser is selected.\nPlease use the settingsdialog to specify one!\n" ) );
	    return;
	}
	QProcess *proc = new QProcess();
	proc->addArgument( webbrowser );
	proc->addArgument( name );
	proc->launch( "" );
	return;
    }
    
    if ( name.left( 2 ) != "p:" ) {
	QUrl u( context(), name );
	if ( !u.isLocalFile() ) {
	    QMessageBox::information( this, tr( "Help" ), tr( "Can't load and display non-local file\n"
							      "%1" ).arg( name ) );
	    return;	
	}

	QTextBrowser::setSource( name );
    } else {
#if 1
	QUrl u( context(), name.mid( 2 ) );
	if ( !u.isLocalFile() ) {
	    QMessageBox::information( this, tr( "Help" ), tr( "Can't load and display non-local file\n"
							      "%1" ).arg( name.mid( 2 ) ) );
	    return;	
	}

	QTextBrowser::setSource( name.mid( 2 ) );
#else
	QString txt;
	const QMimeSource* m = mimeSourceFactory()->data( name.mid( 2 ).left( name.mid( 2 ).find( '#' ) ), context() );
	if ( !m || !QTextDrag::decode( m, txt ) ) {
	    qWarning("QTextBrowser: cannot decode %s", name.mid( 2 ).latin1() );
	    return;
	}	
	
	int i = txt.find( "name=\"" + name.mid( name.find( '#' ) + 1 ) );
	i = txt.findRev( "<h3 class", i );
	QString s( "<a name=\"" + name.mid( name.find( '#' ) + 1 ) + "\"><p><table><tr><td bgcolor=gray>" );
	txt.insert( i, s );
	int j = txt.find( "<h3 class", i + 1 + s.length() );
	if ( j == -1 ) {
	    j = txt.find( "<!-- eo", i + 1 + s.length() );
	} else {
	    int k = txt.find( "<hr>", i + 1 + s.length() );
	    j = QMIN( j, k );
	}
	txt.insert( j, "</td></tr></table>" );
	QTextBrowser::setText( txt, context() );
	QUrl u( name.mid( 2 ) );
	if ( !u.ref().isEmpty() )
	    scrollToAnchor( u.ref() );
#endif
    }
}


void HelpWindow::openLinkInNewWindow()
{
    if ( lastAnchor.isEmpty() )
	return;
    bool oldShiftPressed = shiftPressed;
    shiftPressed = TRUE;
    setSource( lastAnchor );
    shiftPressed = oldShiftPressed;

}


QPopupMenu *HelpWindow::createPopupMenu( const QPoint& pos )
{
    QPopupMenu *m = new QPopupMenu( this );
    lastAnchor = anchorAt( pos );
    if ( !lastAnchor.isEmpty() ) {
	m->insertItem( tr("Open Link in New Window\tShift+LMB"), this, SLOT( openLinkInNewWindow() ) );
    }
    mw->actionNewWindow->addTo( m );
    m->insertSeparator();
    mw->actionGoPrev->addTo( m );
    mw->actionGoNext->addTo( m );
    mw->actionGoHome->addTo( m );
    m->insertSeparator();
    mw->actionZoomIn->addTo( m );
    mw->actionZoomOut->addTo( m );
    m->insertSeparator();
    mw->actionEditCopy->addTo( m );
    mw->actionEditFind->addTo( m );
    return m;
}

void HelpWindow::keyPressEvent( QKeyEvent *e )
{
    shiftPressed = e->key() == Key_Shift;
    QTextBrowser::keyPressEvent( e );
}

void HelpWindow::keyReleaseEvent( QKeyEvent *e )
{
    shiftPressed = FALSE;
    QTextBrowser::keyReleaseEvent( e );
}
