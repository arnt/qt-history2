// $$Root$$Toolbar.h: interface for the C$$Root$$Toolbar class.
//
//////////////////////////////////////////////////////////////////////

#include <QToolBar.h>
#include <QToolButton.h>
#include <QPixmap.h>

class C$$Root$$Toolbar : public QToolBar
{
public:
	C$$Root$$Toolbar( QMainWindow* pParent, QApplication* pApp );
	virtual ~C$$Root$$Toolbar();

$$IF(QT_COMMENTS)
// Add your custom button IDs below.
$$ENDIF
	enum
	{
		BUTTON_NEW_FILE,
		BUTTON_OPEN_FILE,
		BUTTON_SAVE_FILE,
		BUTTON_PRINT_FILE,

		BUTTON_MAX
	};
$$IF(QT_COMMENTS)
// Each tool button will have a QToolButton object that described
// it, and a QPixmap object that contains the image.
// Currently, the prototype appwizard will not generate Icon sets.
$$ENDIF
	QToolButton* m_pButtons[ BUTTON_MAX ];
	QPixmap* m_pPixmaps[ BUTTON_MAX ];

$$IF(QT_COMMENTS)
// Declare the string arrays that describe the buttons to the framework
// The static arrays are defined in $$Root$$Toolbar.cpp
$$ENDIF
	static const QString m_strButtonTexts[ BUTTON_MAX ];
	static const QString m_strButtonNames[ BUTTON_MAX ];
	static const QString m_strButtonImages[ BUTTON_MAX ];
};
