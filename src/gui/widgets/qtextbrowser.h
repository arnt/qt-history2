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

#ifndef QTEXTBROWSER_H
#define QTEXTBROWSER_H

#include "qtextedit.h"

class QTextBrowserPrivate;

class Q_GUI_EXPORT QTextBrowser : public QTextEdit
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QTextBrowser)
    Q_PROPERTY(QString source READ source WRITE setSource)
    Q_OVERRIDE(bool modified SCRIPTABLE false)
    Q_OVERRIDE(bool readOnly DESIGNABLE false SCRIPTABLE false)
    Q_OVERRIDE(bool undoRedoEnabled DESIGNABLE false SCRIPTABLE false)
public:
    QTextBrowser(QWidget* parent = 0);
    virtual ~QTextBrowser();

    QString source() const;

    virtual QImage loadImage(const QString &name);
public slots:
    virtual void setSource(const QString& name);
    virtual void backward();
    virtual void forward();
    virtual void home();
    virtual void reload();

signals:
    void backwardAvailable(bool);
    void forwardAvailable(bool);
    void sourceChanged(const QString&);
    void highlighted(const QString&);
    void linkClicked(const QString&);
    void anchorClicked(const QString &href);

protected:
    void keyPressEvent(QKeyEvent *ev);
    void mouseMoveEvent(QMouseEvent *ev);
    void mouseReleaseEvent(QMouseEvent *ev);

#if defined(QT_COMPAT)
signals:
    QT_MOC_COMPAT void anchorClicked(const QString&, const QString&);
public:
    QT_COMPAT_CONSTRUCTOR QTextBrowser(QWidget *parent, const char *name);
#endif

private:        // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QTextBrowser(const QTextBrowser &);
    QTextBrowser& operator=(const QTextBrowser &);
#endif
};

#endif // QTEXTBROWSER_H
