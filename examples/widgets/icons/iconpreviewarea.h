/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef ICONPREVIEWAREA_H
#define ICONPREVIEWAREA_H

#include <QIcon>
#include <QWidget>

class QLabel;

class IconPreviewArea : public QWidget
{
    Q_OBJECT

public:
    IconPreviewArea(QWidget *parent = 0);

    void setIcon(const QIcon &icon);
    void setSize(const QSize &size);

private:
    QLabel *createHeaderLabel(const QString &text);
    QLabel *createPixmapLabel();
    void updatePixmapLabels();

    enum { NumModes = 4, NumStates = 2 };

    QIcon icon;
    QSize size;
    QLabel *stateLabels[NumStates];
    QLabel *modeLabels[NumModes];
    QLabel *pixmapLabels[NumModes][NumStates];
};

#endif
