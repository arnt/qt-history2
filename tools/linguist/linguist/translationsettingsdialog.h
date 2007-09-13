/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef TRANSLATIONSETTINGSDIALOG_H
#define TRANSLATIONSETTINGSDIALOG_H

#include "ui_translationsettings.h"
#include <QDialog>
#include <QtCore/QLocale>

QT_BEGIN_NAMESPACE
class MessageModel;
class TranslationSettingsDialog : public QDialog
{
    Q_OBJECT
public:
    TranslationSettingsDialog(QWidget *w = 0);
    void setMessageModel(MessageModel *model);

private:
    virtual void showEvent(QShowEvent *e);
private slots:
    void on_buttonBox_accepted();

private:
    Ui::TranslationSettings m_ui;
    MessageModel *m_messageModel;

};

QT_END_NAMESPACE

#endif /* TRANSLATIONSETTINGSDIALOG_H */
