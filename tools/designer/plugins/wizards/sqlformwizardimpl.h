#ifndef SQLFORMWIZARD_H
#define SQLFORMWIZARD_H
#include "sqlformwizard.h"
#include "../../shared/templatewizardiface.h"
#include <qvaluelist.h>

class SqlFormWizard : public SqlFormWizardBase
{
    Q_OBJECT

public:
    SqlFormWizard( QWidget *w, const QValueList<TemplateWizardInterface::DatabaseConnection> &conns,
		   QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~SqlFormWizard();

protected slots:
    void connectionSelected( const QString & );
    void tableSelected( const QString & );
    void autoPopulate( bool populate );
    void fieldDown();
    void fieldUp();
    void removeField();
    void addField();

private:
    QWidget *widget;
    QValueList<TemplateWizardInterface::DatabaseConnection> dbConnections;

};

#endif // SQLFORMWIZARD_H
