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

#ifndef QSVGVIEW_H
#define QSVGVIEW_H

#include <QWidget>
#include <QImage>
#include <QGLWidget>

class QPaintEvent;
class QSvgRenderer;
class QWheelEvent;

class QSvgRasterView : public QWidget
{
    Q_OBJECT
public:
    QSvgRasterView(const QString &file, QWidget *parent=0);

    virtual QSize sizeHint() const;
protected:
    virtual void paintEvent(QPaintEvent *e);
    virtual void wheelEvent(QWheelEvent *e);

private:
    QSvgRenderer *doc;
    QImage buffer;
};

class QSvgNativeView : public QWidget
{
    Q_OBJECT
public:
    QSvgNativeView(const QString &file, QWidget *parent=0);

    virtual QSize sizeHint() const;
protected:
    virtual void paintEvent(QPaintEvent *e);
    virtual void wheelEvent(QWheelEvent *e);

private:
    QSvgRenderer *doc;
};

#ifndef QT_NO_OPENGL
class QSvgGLView : public QGLWidget
{
    Q_OBJECT
public:
    QSvgGLView(const QString &file, QWidget *parent=0);

    virtual QSize sizeHint() const;
protected:
    virtual void paintEvent(QPaintEvent *e);
    virtual void wheelEvent(QWheelEvent *e);

private:
    QSvgRenderer *doc;
};
#endif

#endif
