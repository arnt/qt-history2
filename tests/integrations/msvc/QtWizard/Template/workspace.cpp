// $$Root$$Workspace.cpp: implementation of the C$$Root$$Workspace class.
//
//////////////////////////////////////////////////////////////////////

#include "$$Root$$Workspace.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

C$$Root$$Workspace::C$$Root$$Workspace( QWidget* pParent ) :
	QWorkspace( pParent )
{
	m_pBackground = new QPixmap( QString( "background.png" ) );
	setBackgroundPixmap( *m_pBackground );
}

C$$Root$$Workspace::~C$$Root$$Workspace()
{

}

void C$$Root$$Workspace::slotNewDocument( void )
{
	QWidget* pView;

	pView = new C$$Root$$View( this );
	pView->setGeometry( 0, 0, 200, 200 );
	pView->show();

	m_Views.append( pView );
}

void C$$Root$$Workspace::slotOpenDocument( void )
{
}

void C$$Root$$Workspace::slotCloseDocument( void )
{
}

void C$$Root$$Workspace::slotPrintDocument( void )
{
}
