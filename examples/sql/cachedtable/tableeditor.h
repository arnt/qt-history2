#ifndef TABLEEDITOR_H
#define TABLEEDITOR_H

#include <QDialog>

class QPushButton;
class QSqlTableModel;

class TableEditor : public QDialog
{
    Q_OBJECT

public:
    TableEditor(const QString &tableName, QWidget *parent = 0);

private slots:
    void submit();

private:
    QPushButton *submitButton;
    QPushButton *revertButton;
    QPushButton *quitButton;
    QSqlTableModel *model;
};

#endif
