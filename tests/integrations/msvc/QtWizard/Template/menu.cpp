// $$Root$$Menu.cpp: implementation of the C$$Root$$Menu class.
//
//////////////////////////////////////////////////////////////////////

#include "$$Root$$Menu.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

C$$Root$$Menu::C$$Root$$Menu( QWidget* pParent, QApplication* pApp ) :
	QMenuBar( pParent )
{

$$IF(QT_COMMENTS)
// When adding more menus, create the new QPopupMenu object, fill it with
// menu entries, and then add it to the menubar
$$ENDIF
	m_pFileMenu = new QPopupMenu;
	m_pFileMenu->insertItem( "&Quit", (QObject*)pApp, SLOT( closeAllWindows() ) );
	insertItem( "&File", m_pFileMenu );
}

C$$Root$$Menu::~C$$Root$$Menu()
{

}
