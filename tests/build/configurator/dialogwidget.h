/*
** The class definition for the configurator main dialog
*/

#include <qwidget.h>
#include <qlineedit.h>
#include <qcombobox.h>

#include "configview.h"

class CDialogWidget : public QWidget
{
	Q_OBJECT
public:
	CDialogWidget( QWidget* pParent = NULL, const char* pName = NULL, WFlags f = 0 );
	~CDialogWidget();

public slots:
	void generate();
	void clickedLib( int );
	void clickedThread( int );
	void FillCompilers( const QString& strPlatform );
	void toggledDebug( bool );

private:
	CConfigView* m_pConfigView;
	QLineEdit* m_pOutNameEdit;
	QComboBox* m_pCompiler;
	QComboBox* m_pPlatform;
	QLineEdit* m_pOptions;

	bool m_bShared;
	bool m_bThreaded;
	bool m_bDebug;

private:
	void FillPlatforms();
};
