// QActiveXBase.h: interface for the abstract QActiveXBase class.
//
//////////////////////////////////////////////////////////////////////

#ifndef QACTIVEXBASE_H
#define QACTIVEXBASE_H

#include "stdafx.h"

#include <qwidget.h>
#include <qapplication.h>

class QActiveXBase : public QWidget 
{
public:
    QActiveXBase( const char* pName = NULL ) : QWidget( 0, pName, WStyle_Customize )
    {
    }
    ~QActiveXBase()
    {
    }
};

#endif // QACTIVEXBASE_H