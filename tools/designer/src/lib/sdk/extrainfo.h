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

#include <extension.h>

class DomUI;
class DomWidget;
// ### ... more ??

class QWidget;
class AbstractFormWindow;

struct IExtraInfo
{
    virtual ~IExtraInfo() {}

    virtual void saveExtraInfo(DomUI *ui, AbstractFormWindow *formWindow) = 0;
    virtual void saveExtraInfo(DomWidget *ui_widget, QWidget *widget) = 0;

    virtual void loadExtraInfo(AbstractFormWindow *formWindow, DomUI *ui) = 0;
    virtual void loadExtraInfo(QWidget *widget, DomWidget *ui_widget) = 0;
};
Q_DECLARE_EXTENSION(IExtraInfo, "http://trolltech.com/Qt/IDE/ExtraInfo")


#endif // EXTRAINFO_H
