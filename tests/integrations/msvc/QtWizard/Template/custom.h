// $$Root$$Widget.h: interface for the C$$Root$$Widget class.
//
//////////////////////////////////////////////////////////////////////

#include <QWidget.h>

$$IF(QT_COMMENTS)
// This is a default implementation of a QWidget derived widget
// To make it do anything useful, you would need to customize
// this class
$$ENDIF
class C$$Root$$Widget : public QWidget
{
public:
	C$$Root$$Widget( QWidget* pParent = NULL, const char* pName = NULL, WFlags f = 0 );
	virtual ~C$$Root$$Widget();
};
