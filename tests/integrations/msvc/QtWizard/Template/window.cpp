// $$Root$$Window.cpp: implementation of the C$$Root$$Window class.
//
//////////////////////////////////////////////////////////////////////

#include "$$Root$$Window.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

C$$Root$$Window::C$$Root$$Window( QApplication* pApp ) :
	QMainWindow( 0, 0, WDestructiveClose )
{
$$IF(QT_MDI)
	m_pWorkspace = new C$$Root$$Workspace( this );
$$ELSE
	m_pView = new C$$Root$$View( this );
$$ENDIF
$$IF(QT_MENUBAR)
	m_pMenu = new C$$Root$$Menu( this );
$$ENDIF
$$IF(QT_TOOLBAR)
	m_pToolbar = new C$$Root$$Toolbar( this );
$$ENDIF
$$IF(QT_STATUSBAR)
	m_pStatusBar = new QStatusBar( this );
$$ENDIF

$$IF(QT_MDI)
	setCentralWidget( m_pWorkspace );
$$ELSE
	setCentralWidget( m_pView );
$$ENDIF

	connect( m_pMenu, SIGNAL( sigFileQuit() ), (QObject*)pApp, SLOT( closeAllWindows() ) );
$$IF(QT_MDI)
	connect( m_pMenu, SIGNAL( sigWindowCascade() ), m_pWorkspace, SLOT( cascade() ) );
	connect( m_pMenu, SIGNAL( sigWindowTile() ), m_pWorkspace, SLOT( tile() ) );
$$ENDIF
}

C$$Root$$Window::~C$$Root$$Window()
{

}
