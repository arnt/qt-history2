#ifndef PIMMODEL_H
#define PIMMODEL_H

#include <qabstractitemmodel.h>
#include <qpixmap.h>
#include <qstring.h>

struct PimEntry
{
    PimEntry() {}

    QPixmap photo;

    QString firstName;
    QString lastName;
    QString middleName;

    QString company;
    QString department;
    QString jobTitle;

    // and so on ...
};

class PimModel : public QAbstractListModel
{
    Q_OBJECT

public:

    enum Roles {
        PhotoRole = Qt::DecorationRole,
        FirstNameRole = Qt::DisplayRole,
        LastNameRole = Qt::UserRole + 1,
        MiddleNameRole,
        CompanyRole,
        DepartmentRole,
        JobTitleRole
    };

    PimModel(QObject *parent = 0);
    ~PimModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index,  const QVariant &value, int role = Qt::EditRole);

    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());

    // specialized interface
    inline const PimEntry &entry(const QModelIndex &index) const { return entry(index.row()); }
    const PimEntry &entry(int row) const;
    bool appendEntry(const PimEntry &entry);
    bool setEntry(int row, const PimEntry &entry);
    bool removeEntry(int row);

private:
    QList<PimEntry> entries;
};

#endif // PIMMODEL_H
