// $$Root$$View.cpp: implementation of the C$$Root$$View class.
//
//////////////////////////////////////////////////////////////////////

#include "$$Root$$View.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

$$IF(QT_COMMENTS)
// Accept these parameters, even though the view class may not support
// all of them.
$$ENDIF
C$$Root$$View::C$$Root$$View( QWidget* pParent, const char* pName, WFlags f ) :
$$IF(QT_CUSTOMWIDGET)
	C$$Root$$Widget( pParent )
$$ELSE
	$$QT_CENTRAL_WIDGET_TYPE$$( pParent )
$$ENDIF
{
}

C$$Root$$View::~C$$Root$$View()
{

}
