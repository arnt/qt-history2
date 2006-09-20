/****************************************************************************
**
** Copyright (C) 2006-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef STYLESHEETEDITOR_H
#define STYLESHEETEDITOR_H

#include <QDialog>

#include "ui_stylesheeteditor.h"

class StyleSheetEditor : public QDialog
{
    Q_OBJECT

public:
    StyleSheetEditor(QWidget *parent = 0);

private slots:
    void on_styleCombo_activated(const QString &styleName);
    void on_styleSheetCombo_activated(const QString &styleSheetName);
    void on_styleTextEdit_textChanged();
    void on_applyButton_clicked();

private:
    void loadStyleSheet(const QString &sheetName);

    Ui::StyleSheetEditor ui;
};

#endif
