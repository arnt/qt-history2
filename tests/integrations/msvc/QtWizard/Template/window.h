// $$Root$$Window.h: interface for the C$$Root$$Window class.
//
//////////////////////////////////////////////////////////////////////

#include <QMainWindow.h>
$$IF(QT_MENUBAR)
#include "$$Root$$Menu.h"
$$ENDIF
$$IF(QT_TOOLBAR)
#include "$$Root$$Toolbar.h"
$$ENDIF
$$IF(QT_STATUSBAR)
#include <QStatusbar.h>
$$ENDIF
$$IF(QT_MDI)
#include "$$Root$$Workspace.h"
$$ELSE
#include "$$Root$$View.h"
$$ENDIF

class C$$Root$$Window : public QMainWindow
{
public:
	C$$Root$$Window( QApplication* pApp );
	virtual ~C$$Root$$Window();

$$IF(QT_MENUBAR)
	C$$Root$$Menu* m_pMenu;
$$ENDIF
$$IF(QT_TOOLBAR)
	C$$Root$$Toolbar* m_pToolbar;
$$ENDIF
$$IF(QT_STATUSBAR)
	QStatusBar* m_pStatusBar;
$$ENDIF

$$IF(QT_MDI)
	C$$Root$$Workspace* m_pWorkspace;
$$ELSE
	C$$Root$$View* m_pView;
$$ENDIF
};
