// QActiveXBase.h: interface for the abstract QActiveXBase class.
//
//////////////////////////////////////////////////////////////////////

#include <QWidget.h>

class QActiveXBase : public QWidget 
{
public:
	QActiveXBase( QWidget* pParent = NULL, const char* pName = NULL, WFlags f = 0 ) : QWidget( pParent, pName, f ) {};

protected:
	virtual void InitWidget() = 0;
	virtual void UnInitWidget() = 0;

	bool m_bWidgetReady;
	HWND m_hWndParent;
};
