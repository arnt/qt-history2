// QActiveXBase.h: interface for the abstract QActiveXBase class.
//
//////////////////////////////////////////////////////////////////////

#include <QWidget.h>

class QActiveXBase : public QWidget 
{
public:
	QActiveXBase( QWidget* pParent = NULL, const char* pName = NULL, WFlags f = WStyle_Customize | WStyle_NoBorderEx ) : QWidget( pParent, pName, f )
	{
		setFocusPolicy( QWidget::StrongFocus );
	};

	void attachToControl( HWND hCtrl )
	{
		create( hCtrl );	// Grab the control window, and get rid of our old window
	}
	virtual void InitWidget() = 0;
	virtual void UnInitWidget() = 0;

	bool m_bWidgetReady;
	HWND m_hWndParent;
};
