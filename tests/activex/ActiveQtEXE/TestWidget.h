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

class QActiveX;

class CTestWidget : public QActiveXBase 
{
public:
    CTestWidget();
    ~CTestWidget();

    void resizeEvent( QResizeEvent* pEvent );

private:
    QSlider* m_pSlider;
    QLCDNumber* m_pLCD;
    QMultiLineEdit* m_pEdit;
    QMultiLineEdit* m_pEdit2;

};

#endif // !defined(AFX_TESTWIDGET_H__DA4969C1_5578_42FE_AD7E_8E6924794EBB__INCLUDED_)
