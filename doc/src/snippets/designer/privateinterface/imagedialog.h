#ifndef IMAGEDIALOG_H
#define IMAGEDIALOG_H

#include "ui_imagedialog.h"

class ImageDialog : public QDialog
{
    Q_OBJECT

public:
    ImageDialog(QWidget *parent = 0);

private slots:
    void on_cancelButton_clicked();
    void on_okButton_clicked();

private:
    Ui::ImageDialog ui;
};

#endif
