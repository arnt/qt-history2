#include "employee.h"

Employee::Employee()
{
    d = new EmployeeData;
    d->id = 0;
}

Employee::Employee(int id, const QString &firstName,
                   const QString &lastName)
{
    d = new EmployeeData;
    d->id = id;
    d->firstName = firstName;
    d->lastName = lastName;
}

Employee::Employee(const Employee &other)
{
    d = other.d;
}

Employee &Employee::operator=(const Employee &other)
{
    d = other.d;
    return *this;
}
