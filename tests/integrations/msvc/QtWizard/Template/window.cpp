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
	m_pCentralWidget = new $$QT_CENTRAL_WIDGET_TYPE$$( this );
$$IF(QT_MENUBAR)
	m_pMenu = new C$$Root$$Menu( this, pApp );
$$ENDIF
$$IF(QT_TOOLBAR)
	m_pToolbar = new C$$Root$$Toolbar( this, pApp );
$$ENDIF
$$IF(QT_STATUSBAR)
	m_pStatusBar = new QStatusBar( this );
$$ENDIF

$$IF(QT_BACKGROUND)
	m_pBackground = new QPixmap( QString( "$$QT_BACKGROUND$$" ) );
	m_pCentralWidget->setBackgroundPixmap( *m_pBackground );
	m_pCentralWidget->setBackgroundMode( QWidget::FixedPixmap );
$$ENDIF
	setCentralWidget( m_pCentralWidget );

}

C$$Root$$Window::~C$$Root$$Window()
{

}
