// $$Root$$View.h: interface for the C$$Root$$View class.
//
//////////////////////////////////////////////////////////////////////

$$IF(QT_CUSTOMWIDGET)
#include "$$Root$$Widget.h"
$$ELSE
#include <$$QT_CENTRAL_WIDGET_TYPE$$.h>
$$ENDIF

$$IF(QT_CUSTOMWIDGET)
class C$$Root$$View : public C$$Root$$Widget
$$ELSE
class C$$Root$$View : public $$QT_CENTRAL_WIDGET_TYPE$$
$$ENDIF
{
public:
	C$$Root$$View( QWidget* pParent, const char* pName = NULL, WFlags f = 0 );
	virtual ~C$$Root$$View();
};
