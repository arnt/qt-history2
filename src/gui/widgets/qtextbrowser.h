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
#include <qurl.h>

class QTextBrowserPrivate;

class Q_GUI_EXPORT QTextBrowser : public QTextEdit
{
    Q_OBJECT

    Q_PROPERTY(QUrl source READ source WRITE setSource)
    Q_OVERRIDE(bool modified SCRIPTABLE false)
    Q_OVERRIDE(bool readOnly DESIGNABLE false SCRIPTABLE false)
    Q_OVERRIDE(bool undoRedoEnabled DESIGNABLE false SCRIPTABLE false)
    Q_PROPERTY(QStringList searchPaths READ searchPaths WRITE setSearchPaths)

public:
    QTextBrowser(QWidget* parent = 0);
    virtual ~QTextBrowser();

    QUrl source() const;

    QStringList searchPaths() const;
    void setSearchPaths(const QStringList &paths);

    enum ResourceType {
        HtmlResource,
        ImageResource
    };
    virtual QVariant loadResource(ResourceType type, const QUrl &name);

public slots:
    virtual void setSource(const QUrl &name);
    virtual void backward();
    virtual void forward();
    virtual void home();
    virtual void reload();

signals:
    void backwardAvailable(bool);
    void forwardAvailable(bool);
    void sourceChanged(const QUrl &);
    void highlighted(const QUrl &);
    void highlighted(const QString &);
    void anchorClicked(const QUrl &);

protected:
    void keyPressEvent(QKeyEvent *ev);
    void mouseMoveEvent(QMouseEvent *ev);
    void mouseReleaseEvent(QMouseEvent *ev);

#if defined(QT_COMPAT)
public:
    QT_COMPAT_CONSTRUCTOR QTextBrowser(QWidget *parent, const char *name);
#endif

private:
    Q_DISABLE_COPY(QTextBrowser)
    Q_DECLARE_PRIVATE(QTextBrowser)
    Q_PRIVATE_SLOT(d, void documentModified())
};

#endif // QTEXTBROWSER_H
