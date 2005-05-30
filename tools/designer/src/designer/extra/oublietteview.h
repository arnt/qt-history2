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

#ifndef OUBLIETTEVIEW_H
#define OUBLIETTEVIEW_H

#include <QtGui/QScrollArea>
class Oubliette;

class OublietteView : public QScrollArea
{
    Q_OBJECT
public:
    OublietteView();
    ~OublietteView();

public slots:
    void scrollToCharacter(const QPoint &pt);

private:
    Oubliette *m_oubliette;
};

#endif
