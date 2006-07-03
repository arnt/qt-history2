#include "employee.h"

EmployeeData::EmployeeData()
{
    id = -1;
    name = 0;
}

EmployeeData::EmployeeData(const EmployeeData &other)
    : QSharedData(other)
{
    id = other.id;
    if (other.name) {
        name = new QString(*other.name);
    } else {
        name = 0;
    }
}

EmployeeData::~EmployeeData()
{
    delete name;
}

Employee::Employee()
{
    d = new EmployeeData;
}

Employee::Employee(int id, const QString &name)
{
    d = new EmployeeData;
    setId(id);
    setName(name);
}

void Employee::setName(const QString &name)
{
    if (!d->name)
        d->name = new QString;
    *d->name = name;
}

QString Employee::name() const
{
    if (!d->name)
        return QString();
    return *d->name;
}
