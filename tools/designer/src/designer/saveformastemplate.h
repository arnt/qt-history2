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

#ifndef SAVEFORMASTEMPLATE_H
#define SAVEFORMASTEMPLATE_H

#include "ui_saveformastemplate.h"

QT_BEGIN_NAMESPACE

class QDesignerFormWindowInterface;

class SaveFormAsTemplate: public QDialog
{
    Q_OBJECT
public:
    explicit SaveFormAsTemplate(QDesignerFormWindowInterface *formWindow, QWidget *parent = 0);
    virtual ~SaveFormAsTemplate();

private slots:
    void accept();
    void updateOKButton(const QString &str);
    void checkToAddPath(int itemIndex);

private:
    Ui::SaveFormAsTemplate ui;
    QDesignerFormWindowInterface *m_formWindow;
    int m_addPathIndex;
};

QT_END_NAMESPACE

#endif // SAVEFORMASTEMPLATE_H
