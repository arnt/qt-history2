#include "employee.h"

Employee::Employee()
{
    d = new EmployeeData;
    d->id = 0;
}

Employee::Employee(int id, const QString &name)
{
    d = new EmployeeData;
    d->id = id;
    d->name = name;
}
