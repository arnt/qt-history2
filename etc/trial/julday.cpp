#include <qdatetime.h>
#include <qtextstream.h>
#include <stdio.h>
#include <stdlib.h>

class MyDate : public QDate
{
public:
    MyDate(int y,int m, int d) : QDate(y,m,d) {}
    int julianDay() const { return greg2jul(year(),month(),day()); }
};


int main()
{
    QTextStream cout(stdout,IO_WriteOnly);
    QTextStream cin(stdin,IO_ReadOnly);
    cout << "Enter date (yy/mm/dd): ";
    int  y, m, d;
    char c;
    cin >> y >> c >> m >> c >> d;
    if ( y < 100 )
	y += 1900;
    MyDate date(y,m,d);
    cout << "The julian day for " << date.toString() << " is " << date.julianDay()
	 << "\n";
    return 0;
}
