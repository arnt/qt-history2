// $$Root$$Menu.h: interface for the C$$Root$$Menu class.
//
//////////////////////////////////////////////////////////////////////

#include <QMenuBar.h>
#include <QPopupMenu.h>

class C$$Root$$Menu : public QMenuBar  
{
	Q_OBJECT
public:
	C$$Root$$Menu( QWidget* pParent );
	virtual ~C$$Root$$Menu();

$$IF(QT_COMMENTS)
// When adding more menus, add the corresponding QPopupMenu* entries below
$$ENDIF
	QPopupMenu* m_pFileMenu;
$$IF(QT_MDI)
	QPopupMenu* m_pWindowMenu;
$$ENDIF

signals:
	void sigFileQuit( void );
	void sigFileNew( void );
	void sigFileOpen( void );
	void sigFileClose( void );
	void sigFilePrint( void );
	void sigWindowTile( void );
	void sigWindowCascade( void );
	void sigWindowMaximize( void );
	void sigWindowMinimize( void );
};

