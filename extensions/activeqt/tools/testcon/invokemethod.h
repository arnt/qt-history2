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
    void invoke();
    void setValue();
    void methodSelected(const QString &method);
    void parameterSelected(QTreeWidgetItem *item);

private:
    QAxBase *activex;
};

#endif // INVOKEMETHOD_H
