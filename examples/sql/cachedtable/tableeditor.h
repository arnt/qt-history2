#ifndef TABLEEDITOR_H
#define TABLEEDITOR_H

#include <QWidget>

class QSqlTableModel;

class TableEditor : public QWidget
{
    Q_OBJECT

public:
    TableEditor(const QString &tableName, QWidget *parent = 0);

private slots:
    void submit();

private:
    QSqlTableModel *model;
};

#endif
