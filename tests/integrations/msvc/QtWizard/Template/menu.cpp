// $$Root$$Menu.cpp: implementation of the C$$Root$$Menu class.
//
//////////////////////////////////////////////////////////////////////

#include "$$Root$$Menu.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

C$$Root$$Menu::C$$Root$$Menu( QWidget* pParent ) :
	QMenuBar( pParent )
{

$$IF(QT_COMMENTS)
// When adding more menus, create the new QPopupMenu object, fill it with
// menu entries, and then add it to the menubar
$$ENDIF
	m_pFileMenu = new QPopupMenu;
	m_pFileMenu->insertItem( "&Quit", this, SIGNAL( sigFileQuit() ) );
	m_pFileMenu->insertItem( "&New", this, SIGNAL( sigFileNew() ) );
	m_pFileMenu->insertItem( "&Open", this, SIGNAL( sigFileOpen() ) );
	m_pFileMenu->insertItem( "&Close", this, SIGNAL( sigFileClose() ) );
	m_pFileMenu->insertItem( "&Print", this, SIGNAL( sigFilePrint() ) );
	insertItem( "&File", m_pFileMenu );

$$IF(QT_MDI)
	m_pWindowMenu = new QPopupMenu;
	m_pWindowMenu->insertItem( "&Cascade", this, SIGNAL( sigWindowCascade() ) );
	m_pWindowMenu->insertItem( "&Tile", this, SIGNAL( sigWindowTile() ) );
	m_pWindowMenu->insertItem( "Ma&ximize", this, SIGNAL( sigWindowMaximize() ) );
	m_pWindowMenu->insertItem( "&Minimize", this, SIGNAL( sigWindowMinimize() ) );
	insertItem( "&Window", m_pWindowMenu );
$$ENDIF
}

C$$Root$$Menu::~C$$Root$$Menu()
{

}
