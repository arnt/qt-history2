// $$Root$$App.cpp: implementation of the C$$Root$$App class.
//
//////////////////////////////////////////////////////////////////////

#include "$$Root$$App.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

C$$Root$$App::C$$Root$$App( int argc, char** argv ) :
	QApplication( argc, argv )
{
$$IF(QT_COMMENT)
// Create the applications main window, and register
// it as the main widget.
$$ENDIF
	m_pMainWindow = new C$$Root$$Window( this );
	setMainWidget( m_pMainWindow );
	m_pMainWindow->show();

$$IF(QT_COMMENT)
// The main window will make sure that the destructors of
// our derived framework classes will be called.  When this
// is done, the signal lastWindowClosed() is emitted, which
// is our cue to quit the application
$$ENDIF
	connect( this, SIGNAL( lastWindowClosed() ), this, SLOT( quit() ) );
}

C$$Root$$App::~C$$Root$$App()
{

}
