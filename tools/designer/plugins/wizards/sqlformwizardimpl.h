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
    void databaseSelected( const QString & );
    void tableSelected( const QString & );

private:
    QWidget *widget;
    QValueList<TemplateWizardInterface::DatabaseConnection> dbConnections;

};

#endif // SQLFORMWIZARD_H
