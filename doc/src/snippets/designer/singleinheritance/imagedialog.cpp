#include <QtGui>

#include "imagedialog.h"

ImageDialog::ImageDialog(QWidget *parent)
    : QDialog(parent)
{
    ui.setupUi(this);

    ui.colorDepthCombo->insertItem(tr("2 colors (1 bit per pixel)"));
    ui.colorDepthCombo->insertItem(tr("4 colors (2 bits per pixel)"));
    ui.colorDepthCombo->insertItem(tr("16 colors (4 bits per pixel)"));
    ui.colorDepthCombo->insertItem(tr("256 colors (8 bits per pixel)"));
    ui.colorDepthCombo->insertItem(tr("65536 colors (16 bits per pixel)"));
    ui.colorDepthCombo->insertItem(tr("16 million colors (24 bits per pixel)"));
}

void ImageDialog::on_cancelButton_clicked()
{
    reject();
}

void ImageDialog::on_okButton_clicked()
{
    accept();
}
