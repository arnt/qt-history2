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

#ifndef QCONTEXT2DCANVAS_H
#define QCONTEXT2DCANVAS_H

#include <qscriptengine.h>
#include <qscriptcontext.h>
#include <qscriptvalue.h>

#include <QWidget>
#include <QTimer>

class QPaintEvent;
class QResizeEvent;
class QMouseEvent;
class Context2D;

class QContext2DCanvas : public QWidget
{
    Q_OBJECT
public:
    QContext2DCanvas(QWidget *parent=0);
    ~QContext2DCanvas();

    QScriptEngine *engine();

public slots:
    void setScriptContents(const QString &txt);
    QObject *getContext(const QString &str) const;
    void setInterval(const QScriptValue &func,
                     qreal interval);
    void resize(int w, int h);
signals:
    void error(const QString &error, int lineno);
protected:
    virtual void paintEvent(QPaintEvent *e);
    virtual void mouseMoveEvent(QMouseEvent *e);
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e);
    virtual void keyPressEvent(QKeyEvent *e);
    virtual void keyReleaseEvent(QKeyEvent *e);
    virtual void resizeEvent(QResizeEvent *e);

private:
    QScriptEngine      m_engine;
    QString            m_script;

    Context2D         *m_context;
    QScriptValue       m_self;
    QScriptValue       m_doc;

    QScriptValue       m_intervalFunc;
    QTimer             m_timer;

    bool               m_firstRun;

    QScriptValue       m_keyDownHandler;
    QScriptValue       m_keyUpHandler;
    QScriptValue       m_mouseDownHandler;
    QScriptValue       m_mouseUpHandler;
    QScriptValue       m_mouseMoveHandler;
};

#endif
