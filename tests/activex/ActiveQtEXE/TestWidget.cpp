// TestWidget.cpp: implementation of the CTestWidget class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TestWidget.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTestWidget::CTestWidget()
{
	m_bWidgetReady = false;
	m_pLCD = NULL;
	m_pSlider = NULL;
	m_pEdit = NULL;
}

CTestWidget::~CTestWidget()
{

}

void CTestWidget::InitWidget()
{
	// Create our child widgets
	m_pSlider = new QSlider( 0, 100, 1, 50, QSlider::Horizontal, this );
	m_pLCD = new QLCDNumber( 3, this );
	m_pEdit = new QMultiLineEdit( this );
	m_pEdit2 = new QMultiLineEdit( this );
	// Initialize the state
	m_pLCD->display( 50 );

	// Make sure things get updated properly
	QObject::connect( m_pSlider, SIGNAL( valueChanged( int ) ), m_pLCD, SLOT( display( int ) ) );

	// Show the darn thing
	show();

	// And tell the world that we're done.
	m_bWidgetReady = true;
}

void CTestWidget::UnInitWidget()
{
	// In this case, nothing needs to be done, because our children
	// will be destroyed along with us.
}

void CTestWidget::resizeEvent( QResizeEvent* pEvent )
{
	m_pSlider->setGeometry( 0, 0, pEvent->size().width(), pEvent->size().height() >> 2 );
	m_pLCD->setGeometry( 0, pEvent->size().height() >> 2, pEvent->size().width(), pEvent->size().height() >> 2 );
	m_pEdit->setGeometry( 0, pEvent->size().height() >> 1, pEvent->size().width(), pEvent->size().height() >> 2 );
	m_pEdit2->setGeometry( 0, ( pEvent->size().height() >> 2 ) + ( pEvent->size().height() >> 1 ), pEvent->size().width(), pEvent->size().height() >> 2 );
}
