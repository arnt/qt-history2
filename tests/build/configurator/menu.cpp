/*
** Implementation of the menu bar
*/

#include "menu.h"

CConfiguratorMenu::CConfiguratorMenu( QWidget* pParent, const char* pName ) : QMenuBar( pParent, pName )
{
	QPopupMenu* pFileMenu;

	pFileMenu = new QPopupMenu;

	pFileMenu->insertItem( "&Open", this, SIGNAL( fileOpen( void ) ) );
	pFileMenu->insertItem( "&Save", this, SIGNAL( fileSave( void ) ) );

	insertItem( "&File", pFileMenu );
}

CConfiguratorMenu::~CConfiguratorMenu()
{
}
