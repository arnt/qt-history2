// $$Root$$Toolbar.cpp: implementation of the C$$Root$$Toolbar class.
//
//////////////////////////////////////////////////////////////////////

#include "$$Root$$Toolbar.h"
#include <stdio.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

C$$Root$$Toolbar::C$$Root$$Toolbar( QMainWindow* pParent, QApplication* pApp ) :
	QToolBar( pParent )
{
	int i;

$$IF(QT_COMMENTS)
// Loop through the button descriptions and generate the
// tool buttons.
$$ENDIF
	for( i = 0; i < BUTTON_MAX; i++ )
	{
		m_pPixmaps[ i ] = new QPixmap( m_strButtonImages[ i ] );
		m_pButtons[ i ] = new QToolButton( *m_pPixmaps[ i ], m_strButtonNames[ i ], m_strButtonTexts[ i ], NULL, NULL, this );
	}
}

C$$Root$$Toolbar::~C$$Root$$Toolbar()
{

}

$$IF(QT_COMMENTS)
// Define the button descriptions.  These are tables for the grouptext,
// button name and button image respectively.
// When defining the buttons, please make sure that the sequence of
// strings match the IDs that are defined in $$Root$$Toolbar.h.
// Although the IDs are not used at the moment, they may be used when
// future versions of this wizard implement signal/slot handling.
$$ENDIF
const QString C$$Root$$Toolbar::m_strButtonTexts[ BUTTON_MAX ] =
{
	"Create a new file",
	"Open a file from disk",
	"Save file to disk",
	"Print the file"
};

const QString C$$Root$$Toolbar::m_strButtonNames[ BUTTON_MAX ] =
{
	"New",
	"Open",
	"Save",
	"Print"
};

const QString C$$Root$$Toolbar::m_strButtonImages[ BUTTON_MAX ] =
{
	"filenew.bmp",
	"fileopen.bmp",
	"filesave.bmp",
	"fileprint.bmp"
};

