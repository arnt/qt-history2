#ifndef DIALOGS_H
#define DIALOGS_H

#include <qdialog.h>
#include <qsqltable.h>
#include <qframe.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qlineedit.h>

class QSqlForm;

class TeamPicker : public QComboBox
{
    Q_OBJECT
    Q_PROPERTY( int teamid READ teamId WRITE setTeamId )

public:
    TeamPicker( QWidget * parent = 0, const char * name = 0 );
    int teamId() const;
    void setTeamId( int id );

private:
    QMap< int, int > index2Id;
};

class UpdateMatchDialog : public QDialog
{
    Q_OBJECT

public:
    typedef enum Mode { Insert, Update, Delete };    
    
    UpdateMatchDialog( QSqlRecord* buf, Mode mode, QWidget * parent = 0,
		       const char * name = 0 );
public slots:
    void close();
    void execute();
private slots:
    void updateSets();

private:
    QSqlForm * form;
    QSqlRecord* matchRecord;
    TeamPicker * wteam;
    TeamPicker * lteam;
    QSpinBox * wins;
    QSpinBox * losses;
    QLineEdit * sets;
    Mode mMode;        
};
#endif // DIALOGS_H

