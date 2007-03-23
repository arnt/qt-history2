/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "itemdialog.h"
#include "item.h"

#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QPixmap>
#include <QtGui/QPushButton>
#include <QtGui/QDialogButtonBox>

QList<ItemDialog *> ItemDialog::openDialogs;

ItemDialog::ItemDialog(QWidget *parent, const Item *item)
    : QDialog(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    QGridLayout *layout = new QGridLayout(this);
    QLabel *lblPix = new QLabel();
    const BusinessCard *bcard = static_cast<const BusinessCard *>(item);
    if (bcard) {
        QPixmap pm(bcard->bigPicture());
        if (pm.isNull())
            pm = QPixmap(QLatin1String(":/qthack/images/qt.png"));
        lblPix->setPixmap(pm);
    } else {
        lblPix->setPixmap(item->pixmapName());
    }

    QLabel *lblName = new QLabel(item->name());
    QLabel *lblDesc = new QLabel(item->description());
    lblDesc->setWordWrap(true);
    QFont descFont = lblDesc->font();
    descFont.setItalic(true);
    descFont.setPointSize(descFont.pointSize() - 2);
    lblDesc->setFont(descFont);

    QDialogButtonBox *btnBox = new QDialogButtonBox(QDialogButtonBox::Close);
    connect(btnBox, SIGNAL(rejected()), this, SLOT(close()));

    layout->addWidget(lblPix, 0, 0, 2, 2);
    layout->addWidget(lblName, 0, 2, 1, 1);
    layout->addWidget(lblDesc, 1, 2, 1, 1);
    layout->addWidget(btnBox, 2, 3, 1, 1);
    openDialogs.append(this);
}

void ItemDialog::closeEvent(QCloseEvent *event)
{
    QDialog::closeEvent(event);
    openDialogs.removeAll(this);
    if (openDialogs.isEmpty()) {
        parentWidget()->activateWindow();
    } else {
        ItemDialog *itemDialog = openDialogs.last();
        itemDialog->show();
        itemDialog->raise();
        itemDialog->activateWindow();
    }
}
