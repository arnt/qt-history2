#ifndef DIALOGS_H
#define DIALOGS_H

#include <qdialog.h>
#include "pingpongapp.h"

class QSqlForm;
class QSqlRecord;
class TeamPicker;
class QSpinBox;
class QLineEdit;

class MatchDialog : public QDialog
{
    Q_OBJECT

public:
    typedef enum Mode { Insert, Update, Delete };

    MatchDialog( QSqlRecord* buf, Mode mode, QWidget * parent = 0,
		 const char * name = 0 );
public slots:
    void close();
    void execute();

private slots:
    void updateSets();

private:
    QSqlForm   * form;
    QSqlRecord * matchRecord;
    TeamPicker * wteam;
    TeamPicker * lteam;
    QSpinBox   * wins;
    QSpinBox   * losses;
    QLineEdit  * sets;
    Mode mMode;
};

#endif // DIALOGS_H

