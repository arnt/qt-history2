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
	QWidget* pView;

	pView = new C$$Root$$View( this );
	pView->setGeometry( 0, 0, 200, 200 );
	m_Views.append( pView );

	m_pBackground = new QPixmap( QString( "background.png" ) );
	setBackgroundPixmap( *m_pBackground );
}

C$$Root$$Workspace::~C$$Root$$Workspace()
{

}
