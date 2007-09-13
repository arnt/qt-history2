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

#ifndef PREVIEWLABEL_H
#define PREVIEWLABEL_H

#include <QLabel>
#include <QPixmap>
#include <QSize>

QT_DECLARE_CLASS(QResizeEvent)

class PreviewLabel : public QWidget
{
    Q_OBJECT

public:
    PreviewLabel(QWidget *parent = 0);
    void setPixmap(const QPixmap &pixmap);

protected:
    void paintEvent(QPaintEvent *event);

private:
    QPixmap pixmap;
};

#endif
