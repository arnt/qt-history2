#ifndef SPREADSHEET_H
#define SPREADSHEET_H

#include <qapplication.h>
#include <qtable.h>
#include <qstring.h>
#include <qvaluelist.h>
#include <qinputdialog.h>

#include "spreadsheetwindow.h"

// spreadsheet cell base class
class SpreadsheetCell : public QTableItem
{
 public:
    SpreadsheetCell ( const QString& t, QTable * table, EditType et = OnTyping )
	: QTableItem(table, et, QString::null)
    {
	setText( t );
    }
};

template< class T >
class QValueStack : public QValueList< T >
{
public:
    QValueStack() : QValueList< T >() {}
    void push( T d )
    {
	append( d );
    }
    T pop()
    {
	T d = last();
	remove(fromLast());
	return d;
    }
    T top()
    {
	return operator[](count()-1);
    }
};

typedef QValueStack< double > DoubleStack;
typedef QValueStack< QChar > CharStack;

class EquationCell : public SpreadsheetCell
{
 public:
    EquationCell ( const QString& e, QTable * table )
	: SpreadsheetCell( QString::null, table, Never),
	  eq(e)
    {}
    QString equation() { return eq; }
    QString text() const { return eval( toPostfix( eq.simplifyWhiteSpace() ) ); }
protected:
    QString evalToken( QString& tkn ) const;
    int precedence( QChar c ) const;
    QString toPostfix( QString s ) const;
    QString eval( QString s ) const;
    void paint ( QPainter * p, const QColorGroup & cg, const QRect & cr, bool selected );
    QWidget * createEditor () const { return 0; }
private:
    QString eq;
};

class Spreadsheet : public QTable
{
    Q_OBJECT
public:
    Spreadsheet ( QWidget * parent = 0, const char * name = 0 );
    ~Spreadsheet(){}
protected:
    void createTable();
public slots:
    void clearCurrent();
    virtual void editEquation();
    virtual void newSelection( int row, int col );
signals:
    void newTextSelected( const QString& text);
};


#endif
