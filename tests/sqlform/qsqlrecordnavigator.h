#ifndef QSQLRECORDNAVIGATOR_H
#define QSQLRECORDNAVIGATOR_H

#include "qsqlrecordnavigatorbase.h"
#include <qlineedit.h>

class QSqlRecordNavigator : public QSqlRecordNavigatorBase
{
    Q_OBJECT
    
public:
    QSqlRecordNavigator( QWidget * parent = 0, const char * name = 0,
			 int f = 0 );
public slots:
    void first(){ emit firstClicked(); }
    void previous(){ emit previousClicked(); }
    void next(){ emit nextClicked(); }
    void last(){ emit lastClicked(); }
    void update(){ emit updateClicked(); }
    void insert(){ emit insertClicked(); }
    void del(){ emit delClicked(); }
    void gotoRecord(){ emit seekRecord( recordNumber->text().toInt() ); }
 
    void updateRecordNum( int i ){ 
	recordNumber->setText( QString().setNum(i) ); }
    
signals:
    void firstClicked();
    void previousClicked();
    void nextClicked();
    void lastClicked();
    void updateClicked();
    void insertClicked();
    void delClicked();

    void seekRecord( int i );
};

#endif // QSQLRECORDNAVIGATOR_H
