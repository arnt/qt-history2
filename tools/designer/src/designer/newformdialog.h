#ifndef NEWFORMDIALOG_H
#define NEWFORMDIALOG_H

#include <QDialog>

class NewFormTree;
class QPushButton;
class QTreeWidget;
class QTreeWidgetItem;

class NewFormDialog : public QDialog
{
    Q_OBJECT
public:
    NewFormDialog(QWidget *parent);
    ~NewFormDialog();

signals:
    void needOpen();
    void itemPicked(const QString &widgetClass);

private slots:
    void handleClass(const QString &strClass);
    void createThisOne();
    void handleDoubleClick(QTreeWidgetItem *item);
    void fixButton(QTreeWidgetItem *item);

private:
    QTreeWidget *mWidgetTree;
    QPushButton *btnCreate;
};
#endif
