/****************************************************************************
** $Id: //depot/qt/main/examples/application/application.cpp#14 $
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qvaluelist.h>
#include <qstring.h>
#include <stdio.h>


class Employee
{
public:
    Employee(): s(0) {}
    Employee( const QString& name, int salary )
	: n(name), s(salary) {}

    QString name() const { return n; }

    int salary() const { return s; }
    void setSalary( int salary ) { s = salary; }
private:
    QString n;
    int s;
};


void main()
{
    typedef QValueList<Employee> EmployeeList;
    EmployeeList list;

    list.append( Employee("Bill", 50000) );
    list.append( Employee("Steve",80000) );
    list.append( Employee("Ron",  60000) );

    Employee joe( "Joe", 50000 );
    list.append( joe );
    joe.setSalary( 4000 );
	
    EmployeeList::ConstIterator it = list.begin();
    while( it != list.end() ) {
	printf( "%s earns %d\n", (*it).name().latin1(), (*it).salary() );
	++it;
    }
}
