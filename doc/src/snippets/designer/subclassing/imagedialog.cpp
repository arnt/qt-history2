#include <QtGui>

#include "imagedialog.h"

ImageDialog::ImageDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    colorDepthCombo->insertItem(tr("2 colors (1 bit per pixel)"));
    colorDepthCombo->insertItem(tr("4 colors (2 bits per pixel)"));
    colorDepthCombo->insertItem(tr("16 colors (4 bits per pixel)"));
    colorDepthCombo->insertItem(tr("256 colors (8 bits per pixel)"));
    colorDepthCombo->insertItem(tr("65536 colors (16 bits per pixel)"));
    colorDepthCombo->insertItem(tr("16 million colors (24 bits per pixel)"));

    connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}
