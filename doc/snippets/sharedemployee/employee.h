#ifndef EMPLOYEE_H
#define EMPLOYEE_H

#include <qshareddatapointer.h>
#include <qstring.h>

struct EmployeeData : public QSharedObject
{
    int id;
    QString firstName;
    QString lastName;
};

class Employee
{
public:
    Employee();
    Employee(int id, const QString &firstName, const QString &lastName);
    Employee(const Employee &other);

    Employee &operator=(const Employee &other);

    void setId(int id) { d->id = id; }
    void setFirstName(const QString &name) { d->firstName = name; }
    void setLastName(const QString &name) { d->lastName = name; }

    int id() const { return d->id; }
    QString firstName() const { return d->firstName; }
    QString lastName() const { return d->lastName; }

private:
    QSharedDataPointer<EmployeeData> d;
};

#endif
