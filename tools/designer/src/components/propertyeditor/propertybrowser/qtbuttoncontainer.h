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

#ifndef QTBUTTONCONTAINER_H
#define QTBUTTONCONTAINER_H

#include <QWidget>

class QtButtonContainer : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool containerVisible READ isContainerVisible WRITE setContainerVisible)
public:
    QtButtonContainer(QWidget *parent = 0);
    ~QtButtonContainer();

    QWidget *container() const;
    void setContainer(QWidget *container);

    QString title() const;
    void setTitle(const QString &text);

    QIcon icon() const;
    void setIcon(const QIcon &icon);

    bool isContainerVisible() const;

public slots:
    void setContainerVisible(bool visible);

signals:
    void containerVisibilityChanged(bool visible);

private:
    class QtButtonContainerPrivate *d_ptr;
    Q_DECLARE_PRIVATE(QtButtonContainer)
    Q_DISABLE_COPY(QtButtonContainer)
    Q_PRIVATE_SLOT(d_func(), void slotToggled(bool))
};

#endif


