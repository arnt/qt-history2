/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "topicchooser.h"

#include <qlabel.h>
#include <qlistbox.h>
#include <qpushbutton.h>

TopicChooser::TopicChooser(QWidget *parent, const QStringList &lnkNames,
                            const QStringList &lnks, const QString &title)
    : QDialog(parent), links(lnks), linkNames(lnkNames)
{
    ui.setupUi(this);

    ui.label->setText(tr("Choose a topic for <b>%1</b>").arg(title));
    ui.listbox->insertStringList(linkNames);
    ui.listbox->setCurrentItem(ui.listbox->firstItem());
    ui.listbox->setFocus();
}

QString TopicChooser::link() const
{
    if (ui.listbox->currentItem() == -1)
        return QString::null;
    QString s = ui.listbox->currentText();
    if (s.isEmpty())
        return s;
    int i = linkNames.indexOf(s);
    return links[i];
}

QString TopicChooser::getLink(QWidget *parent, const QStringList &lnkNames,
                              const QStringList &lnks, const QString &title)
{
    TopicChooser *dlg = new TopicChooser(parent, lnkNames, lnks, title);
    QString lnk;
    if (dlg->exec() == QDialog::Accepted)
        lnk = dlg->link();
    delete dlg;
    return lnk;
}

void TopicChooser::on_buttonDisplay_clicked()
{
    accept();
}

void TopicChooser::on_buttonCancel_clicked()
{
    reject();
}

void TopicChooser::on_listbox_doubleClicked(QListBoxItem *item)
{
    Q_UNUSED(item);
    accept();
}

void TopicChooser::on_listbox_returnPressed(QListBoxItem *item)
{
    Q_UNUSED(item);
    accept();
}

