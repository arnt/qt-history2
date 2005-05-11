#ifndef EMPLOYEE_H
#define EMPLOYEE_H

#include <QSharedData>
#include <QString>

class EmployeeData : public QSharedData
{
public:
    int id;
    QString name;
};

class Employee
{
public:
    Employee();
    Employee(int id, const QString &name);

    void setId(int id) { d->id = id; }
    void setName(const QString &name) { d->name = name; }

    int id() const { return d->id; }
    QString name() const { return d->name; }

private:
    QSharedDataPointer<EmployeeData> d;
};

#endif
