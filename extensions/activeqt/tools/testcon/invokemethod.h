#ifndef INVOKEMETHOD_H
#define INVOKEMETHOD_H

#include "ui_invokemethod.h"

class QAxBase;

class InvokeMethod : public QDialog, Ui::InvokeMethod
{
    Q_OBJECT
public:
    InvokeMethod(QWidget *parent);

    void setControl(QAxBase *ax);

protected slots:
    void on_buttonInvoke_clicked();
    void on_buttonSet_clicked();

    void on_comboMethods_activated(const QString &method);
    void on_listParameters_currentItemChanged(QTreeWidgetItem *item);

private:
    QAxBase *activex;
};

#endif // INVOKEMETHOD_H
