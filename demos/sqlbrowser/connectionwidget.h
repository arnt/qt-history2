#ifndef CONNECTIONWIDGET_H
#define CONNECTIONWIDGET_H

#include <qabstractitemmodel.h>
#include <qvector.h>
#include <qstringlist.h>
#include <qwidget.h>

class QGenericTreeView;
class QSqlDatabase;

class ConnectionModel: public QAbstractItemModel
{
    Q_OBJECT
public:
    ConnectionModel(QObject *parent = 0);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Role_Display) const;
    bool hasChildren(const QModelIndex &parent) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex(),
                      QModelIndex::Type type = QModelIndex::View) const;
    QModelIndex parent(const QModelIndex &child) const;

public slots:
    void refresh();

private:
    struct CData { QString cname, label; };
    QVector<CData> connections;
    QList<QStringList> tableNames;
};

class ConnectionWidget: public QWidget
{
    Q_OBJECT
public:
    ConnectionWidget(QWidget *parent = 0);
    virtual ~ConnectionWidget();

    QSqlDatabase currentDatabase() const;

public slots:
    void refresh();

private:
    QGenericTreeView *tree;
    ConnectionModel *model;
};

#endif

