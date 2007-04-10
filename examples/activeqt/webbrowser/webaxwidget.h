/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_COMMERCIAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef WEBAXWIDGET_H
#define WEBAXWIDGET_H

#include <ActiveQt/QAxWidget>
#include "windows.h"

class WebAxWidget : public QAxWidget
{
public:
    
    WebAxWidget(QWidget* parent = 0, Qt::WindowFlags f = 0) 
        : QAxWidget(parent, f)
    {
    }
protected:
    virtual bool translateKeyEvent(int message, int keycode) const
    {
        if (message >= WM_KEYFIRST && message <= WM_KEYLAST)
            return true;
        else
            return QAxWidget::translateKeyEvent(message, keycode);
    }

};

#endif // WEBAXWIDGET_H
