#ifndef CONFIG_H
#define CONFIG_H

#include "ui_config.h"

class Config : public QDialog, public Ui::Config
{
public:
    Config(QWidget *parent = 0)
        : QDialog(parent)
    {
        setupUi(this);
    }
};

#endif // CONFIG_H
