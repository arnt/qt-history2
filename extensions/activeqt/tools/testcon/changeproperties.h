#ifndef CHANGEPROPERTIES_H
#define CHANGEPROPERTIES_H

#include "ui_changeproperties.h"

class QAxWidget;

class ChangeProperties : public QDialog, Ui::ChangeProperties
{
    Q_OBJECT
public:
    ChangeProperties(QWidget *parent);

    void setControl(QAxWidget *control);

public slots:
    void updateProperties();

protected slots:
    void on_listProperties_currentItemChanged(QTreeWidgetItem *current);
    void on_listEditRequests_itemChanged(QTreeWidgetItem *item);
    void on_buttonSet_clicked();

private:

    QAxWidget *activex;
};

#endif // CHANGEPROPERTIES_H
