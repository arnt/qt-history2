// $$Root$$App.h: interface for the C$$Root$$App class.
//
//////////////////////////////////////////////////////////////////////

#include <QApplication.h>
#include "$$Root$$Window.h"

class C$$Root$$App : public QApplication
{
public:
	C$$Root$$App( int argc, char** argv );
	virtual ~C$$Root$$App();

private:
$$IF(QT_COMMENTS)
// This is the applications main window.
$$ENDIF
	C$$Root$$Window* m_pMainWindow;
};
