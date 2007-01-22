#ifndef ADDBUTTONDIALOG_H
#define ADDBUTTONDIALOG_H

#include <QDialog>
#include <QDialogButtonBox>

#include "ui_addbuttondialog.h"

class AddButtonDialog : public QDialog, Ui::AddButtonDialog
{
    Q_OBJECT

public:
    AddButtonDialog(QDialogButtonBox *box, QWidget *parent = 0);
    
    static QString roleToString(QDialogButtonBox::ButtonRole role);

public slots:
    void addButton();

private slots:
    void standardButtonSelected(const QString &text);

private:
    void fillStandardButtonCombo();
    void fillCustomButtonCombo();
    bool contains(QAbstractButton *button, QList<QAbstractButton *> buttons);

    QDialogButtonBox *dialogBox;
};

#endif
