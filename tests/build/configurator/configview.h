/*
** Definitions for the config selection widget
*/

#include <QScrollView.h>
#include <QStringList.h>

class CConfigView : public QScrollView
{
	Q_OBJECT
public:
	CConfigView( QWidget* pParent = NULL, const char* pName = NULL, WFlags f = 0 );
	~CConfigView();

	QStringList* activeModules();

	enum { NUM_MODULES = 13 };
private:
	static QString m_Modules[ NUM_MODULES ];
	QStringList m_activeModules;

public slots:
	void configToggled( const QString& modulename );
};