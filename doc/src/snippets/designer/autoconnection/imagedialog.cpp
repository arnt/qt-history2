#include <QtGui>

#include "imagedialog.h"

ImageDialog::ImageDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);
    okButton->setAutoDefault(false);
    cancelButton->setAutoDefault(false);

    colorDepthCombo->addItem(tr("2 colors (1 bit per pixel)"));
    colorDepthCombo->addItem(tr("4 colors (2 bits per pixel)"));
    colorDepthCombo->addItem(tr("16 colors (4 bits per pixel)"));
    colorDepthCombo->addItem(tr("256 colors (8 bits per pixel)"));
    colorDepthCombo->addItem(tr("65536 colors (16 bits per pixel)"));
    colorDepthCombo->addItem(tr("16 million colors (24 bits per pixel)"));

    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

void ImageDialog::on_okButton_clicked()
{
    if (nameLineEdit->text().isEmpty())
        (void) QMessageBox::information(this, tr("No Image Name"),
            tr("Please supply a name for the image."), QMessageBox::Cancel);
    else
        accept();
}
