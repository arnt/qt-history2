#ifndef CONFIG_H
#define CONFIG_H

#include "ui_config.h"

class Config : public QDialog, public Ui::Config
{
    Q_OBJECT
public:
    Config(QWidget *parent = 0, Qt::WFlags flags = 0)
        : QDialog(parent, flags)
    {
        setupUi(this);
    }

private slots:
    void on_buttonOk_clicked();
    void on_buttonCancel_clicked();
};

#endif // CONFIG_H
