/****************************************************************************
**
** Definition of the QTextBrowser class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QTEXTBROWSER_H
#define QTEXTBROWSER_H

#ifndef QT_H
#include "qpixmap.h"
#include "qcolor.h"
#include "qtextedit.h"
#endif // QT_H

#ifndef QT_NO_TEXTBROWSER

class QTextBrowserData;

class Q_GUI_EXPORT QTextBrowser : public QTextEdit
{
    Q_OBJECT
    Q_PROPERTY(QString source READ source WRITE setSource)
    Q_OVERRIDE(int undoDepth DESIGNABLE false SCRIPTABLE false)
    Q_OVERRIDE(bool overwriteMode DESIGNABLE false SCRIPTABLE false)
    Q_OVERRIDE(bool modified SCRIPTABLE false)
    Q_OVERRIDE(bool readOnly DESIGNABLE false SCRIPTABLE false)
    Q_OVERRIDE(bool undoRedoEnabled DESIGNABLE false SCRIPTABLE false)

    friend class QTextEdit;

public:
    QTextBrowser(QWidget* parent=0, const char* name=0);
    ~QTextBrowser();

    QString source() const;

public slots:
    virtual void setSource(const QString& name);
    virtual void backward();
    virtual void forward();
    virtual void home();
    virtual void reload();
    void setText(const QString &txt) { setText(txt, QString::null); }
    virtual void setText(const QString &txt, const QString &context);

signals:
    void backwardAvailable(bool);
    void forwardAvailable(bool);
    void sourceChanged(const QString&);
    void highlighted(const QString&);
    void linkClicked(const QString&);
    void anchorClicked(const QString&, const QString&);

protected:
    void keyPressEvent(QKeyEvent * e);

private:
    void popupDetail(const QString& contents, const QPoint& pos);
    bool linksEnabled() const { return true; }
    void emitHighlighted(const QString &s);
    void emitLinkClicked(const QString &s);
    QTextBrowserData *d;

private:        // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QTextBrowser(const QTextBrowser &);
    QTextBrowser& operator=(const QTextBrowser &);
#endif
};

#endif // QT_NO_TEXTBROWSER

#endif // QTEXTBROWSER_H
