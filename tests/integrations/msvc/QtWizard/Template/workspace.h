// $$Root$$Workspace.h: interface for the C$$Root$$Workspace class.
//
//////////////////////////////////////////////////////////////////////

#include <QWorkspace.h>
#include "$$Root$$View.h"
#include <QList.h>

class C$$Root$$Workspace : public QWorkspace
{
public:
	C$$Root$$Workspace( QWidget* pParent );
	virtual ~C$$Root$$Workspace();

	QList<QWidget> m_Views;

	QPixmap* m_pBackground;
};
