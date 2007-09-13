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

#ifndef VIEW_H
#define VIEW_H

#include <QFrame>

QT_DECLARE_CLASS(QGraphicsView)
QT_DECLARE_CLASS(QLabel)
QT_DECLARE_CLASS(QSlider)
QT_DECLARE_CLASS(QToolButton)

class View : public QFrame
{
    Q_OBJECT
public:
    View(const QString &name, QWidget *parent = 0);

    QGraphicsView *view() const;

private slots:
    void resetView();
    void setResetButtonEnabled();
    void setupMatrix();
    void toggleOpenGL();
    void toggleAntialiasing();
    void print();

    void zoomIn();
    void zoomOut();
    void rotateLeft();
    void rotateRight();
    
private:
    QGraphicsView *graphicsView;
    QLabel *label;
    QToolButton *openGlButton;
    QToolButton *antialiasButton;
    QToolButton *printButton;
    QToolButton *resetButton;
    QSlider *zoomSlider;
    QSlider *rotateSlider;
};

#endif
