// $$Root$$Workspace.h: interface for the C$$Root$$Workspace class.
//
//////////////////////////////////////////////////////////////////////

#include <QWorkspace.h>
#include "$$Root$$View.h"
#include <QList.h>
#include <QPixmap.h>

class C$$Root$$Workspace : public QWorkspace
{
	Q_OBJECT
public:
	C$$Root$$Workspace( QWidget* pParent );
	virtual ~C$$Root$$Workspace();

	QList<QWidget> m_Views;

	QPixmap* m_pBackground;

public slots:
	void slotNewDocument( void );
	void slotOpenDocument( void );
	void slotCloseDocument( void );
	void slotPrintDocument( void );
};
