#ifndef TABLE_H
#define TABLE_H

#include <qtableview.h>
#include <qarray.h>

class QHeader;

class Table : public QTableView
{
    Q_OBJECT
public:
    Table( QHeader *, int rows, QWidget *parent=0, int flags = -1,
		 const char *name=0 );
    ~Table();

protected slots:
    void rehash();
    void moveCol( int from, int to );

protected:
    void paintCell( QPainter *p, int row, int col );
    int  cellWidth( int );
    /*
protected slots:
    void scrollHorz(int);
    void scrollVert(int);
    void setInputText(QString);
    */
private:
    QHeader *bar;
    QArray<int> numbers;
};

#endif //TABLE_H
