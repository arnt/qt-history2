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

#ifndef TOPICCHOOSER_H
#define TOPICCHOOSER_H

#include "ui_topicchooser.h"

#include <qdialog.h>
#include <qstringlist.h>

class TopicChooser : public QDialog
{
    Q_OBJECT
public:
    TopicChooser(QWidget *parent, const QStringList &lnkNames,
                  const QStringList &lnks, const QString &title);

    QString link() const;

    static QString getLink(QWidget *parent, const QStringList &lnkNames,
                            const QStringList &lnks, const QString &title);

private slots:
    void on_buttonDisplay_clicked();
    void on_buttonCancel_clicked();
    void on_listbox_doubleClicked(QListBoxItem *item);
    void on_listbox_returnPressed(QListBoxItem *item);

private:
    Ui::TopicChooser gui;
    QString theLink;
    QStringList links, linkNames;
};

#endif // TOPICCHOOSER_H
