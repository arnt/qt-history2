// TestWidget.h: interface for the CTestWidget class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TESTWIDGET_H__DA4969C1_5578_42FE_AD7E_8E6924794EBB__INCLUDED_)
#define AFX_TESTWIDGET_H__DA4969C1_5578_42FE_AD7E_8E6924794EBB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "QActiveXBase.h"
#include <QSlider.h>
#include <QLCDNumber.h>
#include <QMultiLineEdit.h>

class CTestWidget : public QActiveXBase 
{
public:
	CTestWidget();
	virtual ~CTestWidget();

private:
	QSlider* m_pSlider;
	QLCDNumber* m_pLCD;
	QMultiLineEdit* m_pEdit;

// These are pure virtual functions in the baseclass
protected:
	virtual void InitWidget();
	virtual void UnInitWidget();

// Event handlers
	virtual void resizeEvent( QResizeEvent* pEvent );
	virtual void closeEvent( QCloseEvent* pEvent );
};

#endif // !defined(AFX_TESTWIDGET_H__DA4969C1_5578_42FE_AD7E_8E6924794EBB__INCLUDED_)
