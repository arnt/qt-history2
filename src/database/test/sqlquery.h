#include <qtable.h>
#include <qstring.h>
#include <qsql.h>
#include <qdialog.h>
#include <qlayout.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qmultilineedit.h>

#include "browse.h"
#include "databasechooserdialog.h"
#include "sqlquerywindow.h"

class DatabaseDialog : public DatabaseChooserDialog
{
    Q_OBJECT
public:
    DatabaseDialog ( QWidget * parent=0, const char * name=0 )
    : DatabaseChooserDialog(parent, name, TRUE)
    {
    }
    QSqlDatabase* createDatabase()
    {
        QString driver = driverComboBox->currentText();
	QString database = databaseLineEdit->text();
	QString username = usernameLineEdit->text();
	QString password = passwordLineEdit->text();
	QString host = hostLineEdit->text();
	QSqlDatabase* db = new QSqlDatabase( driver );
	db->reset( database,
		   username,
		   password,
		   host );
	return db;
    }
};

class ResultWindow : public SqlQuery
{
    Q_OBJECT
public:
    ResultWindow ( QWidget * parent=0, const char * name=0, WFlags f=0 )
    : SqlQuery(parent, name, f),
      db(0)
    {
    	execButton->setEnabled( FALSE );
	connect( connectButton,SIGNAL(clicked()), this, SLOT(slotConnect()));
	connect( execButton,SIGNAL(clicked()), this, SLOT(slotExec()));
    }
    ~ResultWindow()
    {
	qDebug("start ~ResultWindow()");
    	if (db) {
	    qDebug("freeing browse");
	    sqlBrowse->free();
	    qDebug("closing db");
	    db->close();
	    qDebug("deleting db");
	    delete db;
	}
	qDebug("done ~ResultWindow()");
    }
public slots:
    void slotConnect()
    {
 	DatabaseDialog dlg(this);
 	if ( dlg.exec() == QDialog::Accepted ) {
	    if ( db ) {
	        db->close();
	    	delete db;
	    }
	    execButton->setEnabled(TRUE);
 	    db = dlg.createDatabase();
	    qDebug("after creating db");
	    if ( db->open() ) {
		connectionStatus->setText("Connected");
		sqlQuery->setFocus();
	    }
	    else {
		connectionStatus->setText("Not Connected");
		QSqlError err = db->lastError();
		QMessageBox::information(0,"Connection Error",err.databaseText());
	    }
	}
    }
    void slotExec()
    {
	if ( db && sqlQuery->text().length() && db->isOpen() ) {
	    QString query( sqlQuery->text().simplifyWhiteSpace() );
	    QSql res =  db->query( query );
	    if ( res.isActive() ) {
		sqlBrowse->take(res);
	    } else {
		QSqlError err = res.lastError();
		QMessageBox::information(0,"SQL Error",err.driverText() + err.databaseText());
	    }
	    sqlQuery->setFocus();
	}
    }
private:
    QSqlDatabase* db;
};


















