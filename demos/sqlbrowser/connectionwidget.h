#ifndef CONNECTIONWIDGET_H
#define CONNECTIONWIDGET_H

#include <qabstractitemmodel.h>
#include <qhash.h>
#include <qstringlist.h>
#include <qwidget.h>

class QGenericTreeView;


class ConnectionModel: public QAbstractItemModel
{
    Q_OBJECT
public:
    ConnectionModel(QObject *parent = 0);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Role_Display) const;

public slots:
    void refresh();

private:
    QHash<QString, QString> connections;
    QList<QStringList> tableNames;
};

class ConnectionWidget: public QWidget
{
    Q_OBJECT
public:
    ConnectionWidget(QWidget *parent = 0);
    virtual ~ConnectionWidget();

public slots:
    void refresh();

private:
    QGenericTreeView *tree;
    ConnectionModel *model;
};

#endif

