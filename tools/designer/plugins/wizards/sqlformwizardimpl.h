#ifndef SQLFORMWIZARD_H
#define SQLFORMWIZARD_H
#include "sqlformwizard.h"
#include <templatewizardiface.h>
#include <designerinterface.h>
#include <qvaluelist.h>

class SqlFormWizard : public SqlFormWizardBase
{
    Q_OBJECT

public:
    SqlFormWizard( QUnknownInterface *aIface, QWidget *w, QWidget* parent = 0, DesignerFormWindow *fw = 0,
		   const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~SqlFormWizard();

    void accept() const;

protected slots:
    void connectionSelected( const QString & );
    void tableSelected( const QString & );
    void autoPopulate( bool populate );
    void fieldDown();
    void fieldUp();
    void removeField();
    void addField();
    void setupDatabaseConnections();
    void accept();
    void addSortField();
    void reSortSortField();
    void removeSortField();
    void sortFieldUp();
    void sortFieldDown();

private:
    void setupPage1();

private:
    QWidget *widget;
    QUnknownInterface *appIface;
    DesignerFormWindow *formWindow;
    enum Mode {
	None,
	Form,
	Table
    };
    Mode mode;

};

#endif // SQLFORMWIZARD_H
