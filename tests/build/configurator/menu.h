/*
** Definitions for the menu bar
*/

#include <qmenubar.h>
#include <qpopupmenu.h>

class CConfiguratorMenu : public QMenuBar
{
	Q_OBJECT
public:
	CConfiguratorMenu( QWidget* pParent = NULL, const char* pName = NULL );
	~CConfiguratorMenu();

signals:
	void fileOpen( void );
	void fileSave( void );
};
