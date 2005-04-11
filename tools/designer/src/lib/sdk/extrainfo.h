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

#ifndef EXTRAINFO_H
#define EXTRAINFO_H

#include <QtDesigner/extension.h>

class DomUI;
class DomWidget;
class DomLayout;
// ### ... more ??

class QWidget;
class QDesignerFormWindowInterface;

class QDesignerExtraInfoExtension
{
public:
    virtual ~QDesignerExtraInfoExtension() {}

    virtual void saveExtraInfo(DomUI *ui, QDesignerFormWindowInterface *formWindow) = 0;
    virtual void saveExtraInfo(DomWidget *ui_widget, QWidget *widget) = 0;
    virtual void saveExtraInfo(DomLayout *ui_layout, QLayout *layout) = 0;

    virtual void loadExtraInfo(QDesignerFormWindowInterface *formWindow, DomUI *ui) = 0;
    virtual void loadExtraInfo(QWidget *widget, DomWidget *ui_widget) = 0;
    virtual void loadExtraInfo(QLayout *layout, DomLayout *ui_layout) = 0;
};
Q_DECLARE_EXTENSION_INTERFACE(QDesignerExtraInfoExtension, "http://trolltech.com/Qt/IDE/ExtraInfo")


#endif // EXTRAINFO_H
