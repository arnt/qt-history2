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
    void editRequestChanged(QTreeWidgetItem *item);

protected:
    void updateProperties();

protected slots:
    void propertySelected();
    void setValue();

private:
    QAxWidget *activex;
};

#endif // CHANGEPROPERTIES_H
