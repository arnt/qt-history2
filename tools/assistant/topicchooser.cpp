/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of Qt Assistant.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
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
    : QDialog(parent, 0, true), links(lnks), linkNames(lnkNames)
{
    gui.setupUI(this);

    gui.label->setText(tr("Choose a topic for <b>%1</b>").arg(title));
    gui.listbox->insertStringList(linkNames);
    gui.listbox->setCurrentItem(gui.listbox->firstItem());
    gui.listbox->setFocus();
}

QString TopicChooser::link() const
{
    if (gui.listbox->currentItem() == -1)
        return QString::null;
    QString s = gui.listbox->currentText();
    if (s.isEmpty())
        return s;
    int i = linkNames.findIndex(s);
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
