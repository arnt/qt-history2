// QActiveXBase.h: interface for the abstract QActiveXBase class.
//
//////////////////////////////////////////////////////////////////////

#ifndef QACTIVEXBASE_H
#define QACTIVEXBASE_H

#include "stdafx.h"

#include <QWidget.h>
#include <QApplication.h>
#include <QPainter.h>

class QAtlPaintDevice : public QPaintDevice
{
public:
    QAtlPaintDevice( HDC h ) : QPaintDevice(QInternal::System)
	{
		hdc = h;
	}
};

class QActiveXBase : public QWidget 
{
	Q_OBJECT
public:
	QActiveXBase( const char* pName = NULL ) : QWidget( 0, pName, WStyle_Customize )
	{
		debug("QActiveX::QActiveX");
		m_pComControl = NULL;
		m_bIsActive   = FALSE;
		m_hOldParent  = NULL;
		m_nOldStyle   = 0;
	}
	~QActiveXBase()
	{
	}

    bool isActive() const;	// Not implemented in the original source?
    virtual void drawControl( QPainter* pPainter, const QRect& rc )
	{
		debug("QActiveXBase::drawControl()");
	}
    virtual void paintEvent( QPaintEvent* pEvent )
	{
		debug("QActiveXBase::paintEvent()");
	}
    virtual void activateEvent( QEvent* pEvent )
	{
		debug("QActiveXBase::activateEvent()");
	}
    virtual void deactivateEvent( QEvent* pEvent )
	{
		debug("QActiveXBase::deactivateEvent()");
	}
    static void initialize()
	{
		debug("QActiveXBase::initialize()");
	}
    static void terminate()
	{
		debug("QActiveXBase::terminate()");
	}

// These are semi-internal functions and should be called
// from the ATL control only.
    void setComControl( void* pControl )
	{
		m_pComControl = pControl;
	}
    virtual void setActive( bool bActive, HANDLE parentWindow )
	{
		QEvent e( bActive ? QEvent::ActivateControl : QEvent::DeactivateControl );
		debug( "QActiveXBase::setActive()" );
		ASSERT( m_bIsActive != bActive );

		if ( m_bIsActive == bActive )
		{
			return;
		}
		m_bIsActive = bActive;
		if ( bActive )
		{
			m_hOldParent = GetParent( winId() );
			m_nOldStyle  = GetWindowLong( winId(), GWL_STYLE );
			SetParent( winId(), (HWND)parentWindow );
			SetWindowLong( winId(), GWL_STYLE, WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN );
			show();
		}
		else
		{
			hide();
			SetParent( winId(), (HWND)m_hOldParent );
			SetWindowLong( winId(), GWL_STYLE, m_nOldStyle );
			m_hOldParent = 0;
		}
		QApplication::sendEvent( this, &e );
	}
    virtual void drawControlAtl( void *pDrawInfo )
	{
		ATL_DRAWINFO* di = (ATL_DRAWINFO*)pDrawInfo;
		RECT& rect = *(RECT*)di->prcBounds;
		QRect r( QPoint( rect.left, rect.top ), QPoint( rect.right, rect.bottom ) );
		QPainter p;
		QAtlPaintDevice pd( di->hdcDraw );

		p.begin( &pd );
		drawControl( &p, r );
		p.end();
	}

public slots:
    void updateControl()
	{
		if ( m_pComControl )
		{
			debug("QActiveXControl::updateControl()");
		}
	}

protected:
    bool event( QEvent* pEvent )
	{
		switch ( pEvent->type() )
		{
		case QEvent::ActivateControl:
			activateEvent( pEvent );
			break;
		case QEvent::DeactivateControl:
			deactivateEvent( pEvent );
			break;
		default:
			return QWidget::event( pEvent );
			break;
		}
		return true;
	}

private:
    void* m_pComControl;
    bool m_bIsActive;
    HANDLE m_hOldParent;
    int m_nOldStyle;
	
/*
** Hide these until we know that things really work
	void attachToControl( HWND hCtrl )
	{
		create( hCtrl );	// Grab the control window, and get rid of our old window
	}
	virtual void InitWidget() = 0;
	virtual void UnInitWidget() = 0;

	bool m_bWidgetReady;
	HWND m_hWndParent;
*/
};

#endif // QACTIVEXBASE_H