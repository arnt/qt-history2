#ifndef LOCALEMODEL_H
#define LOCALEMODEL_H

#include <QtCore/QAbstractItemModel>
#include <QtCore/QList>
#include <QtCore/QVariant>

class LocaleModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    LocaleModel(QObject *parent = 0);

    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual QModelIndex index(int row, int column,
                                const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &index) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role = Qt::DisplayRole ) const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value,
                            int role = Qt::EditRole);
private:
    QList<QVariant> m_data_list;
};

#endif // LOCALEMODEL_H
