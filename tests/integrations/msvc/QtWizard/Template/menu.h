// $$Root$$Menu.h: interface for the C$$Root$$Menu class.
//
//////////////////////////////////////////////////////////////////////

#include <QMenuBar.h>
#include <QPopupMenu.h>

class C$$Root$$Menu : public QMenuBar  
{
public:
	C$$Root$$Menu( QWidget* pParent, QApplication* pApp );
	virtual ~C$$Root$$Menu();

$$IF(QT_COMMENTS)
// When adding more menus, add the corresponding QPopupMenu* entries below
$$ENDIF
	QPopupMenu* m_pFileMenu;

};

