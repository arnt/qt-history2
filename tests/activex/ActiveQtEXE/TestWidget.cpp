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
    // Create our child widgets
    m_pSlider = new QSlider( 0, 100, 1, 50, QSlider::Horizontal, this );
    m_pLCD = new QLCDNumber( 3, this );
    m_pEdit = new QMultiLineEdit( this );
    m_pEdit2 = new QMultiLineEdit( this );
    // Initialize the state
    m_pLCD->display( 50 );

    // Make sure things get updated properly
    QObject::connect( m_pSlider, SIGNAL( valueChanged( int ) ), m_pLCD, SLOT( display( int ) ) );
}

CTestWidget::~CTestWidget()
{
}

void CTestWidget::resizeEvent( QResizeEvent* pEvent )
{
    QSize newSize = pEvent->size();

    m_pSlider->move( 0, 0 );
    m_pSlider->resize( newSize.width(), newSize.height() >> 2 );
    m_pLCD->move( 0, newSize.height() >> 2 );
    m_pLCD->resize( newSize.width(), newSize.height() >> 2 );
    m_pEdit->move( 0, newSize.height() >> 1 );
    m_pEdit->resize( newSize.width(), newSize.height() >> 2 );
    m_pEdit2->move( 0, newSize.height() - ( newSize.height() >> 2 ) );
    m_pEdit2->resize( newSize.width(), newSize.height() >> 2 );
}
