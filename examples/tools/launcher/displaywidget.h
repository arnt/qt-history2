/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef DISPLAYWIDGET_H
#define DISPLAYWIDGET_H

#include <QWidget>

class DisplayShape;

class DisplayWidget : public QWidget
{
    Q_OBJECT

public:
    DisplayWidget(QWidget *parent = 0);
    QSize minimumSizeHint() const;
    DisplayShape *shape(int index) const;
    int shapesCount() const;
    void appendShape(DisplayShape *shape);
    void enableUpdates();
    void insertShape(int position, DisplayShape *shape);
    void reset();

protected:
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);

signals:
    void displayEmpty();
    void menuRequested(const QString &name);
    void categoryRequested(const QString &name);
    void documentationRequested(const QString &name);
    void exampleRequested(const QString &name);
    void launchRequested(const QString &name);

private slots:
    void updateShapes();

private:
    bool empty;
    bool emptying;
    QList<DisplayShape*> shapes;
    QTimer *timer;
};

#endif
