#ifndef DIALOGS_H
#define DIALOGS_H

#include <qdialog.h>
#include <qsqltable.h>
#include <qframe.h>

class QSqlForm;

class GenericDialog : public QDialog
{
    Q_OBJECT

public:
    typedef enum Mode { Insert, Update, Delete };

    GenericDialog( QSqlRecord* buf, Mode mode, QWidget * parent = 0,
		   const char * name = 0 );
public slots:
    void close();
    void execute();

private:
    Mode mMode;
    QSqlForm * form;
};

#endif // DIALOGS_H

