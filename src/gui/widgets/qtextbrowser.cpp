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

#include "qtextbrowser.h"
#include "qtextedit_p.h"

#include <qstack.h>
#include <qapplication.h>
#include <qevent.h>
#include <qdesktopwidget.h>
#include <qdebug.h>
#include <qabstracttextdocumentlayout.h>
#include <qurl.h>
#include "private/qtextdocumentlayout_p.h"
#include "private/qtexthtmlparser_p.h"
#include <qtextcodec.h>
#include <qpainter.h>
#include <qdir.h>
#include <qwhatsthis.h>

#define d d_func()
#define q q_func()

class QTextBrowserPrivate : public QTextEditPrivate
{
    Q_DECLARE_PUBLIC(QTextBrowser)
public:
    QTextBrowserPrivate() : textOrSourceChanged(false) {}

    QStack<QString> stack;
    QStack<QString> forwardStack;
    QString home;
    QString currentURL;
    QString currentAnchor;

    /*flag necessary to give the linkClicked() signal some meaningful
      semantics when somebody connected to it calls setText() or
      setSource() */
    bool textOrSourceChanged;

    QString resolvePath(const QString &name) const;
};

static bool isAbsoluteFileName(const QString &name)
{
    return !name.isEmpty()
           && (name[0] == '/'
#if defined(Q_WS_WIN)
               || (name[0].isLetter() && name[1] == QLatin1Char(':')) || name.startsWith("\\\\")
#endif
              );

}

QString QTextBrowserPrivate::resolvePath(const QString &name) const
{
    if (isAbsoluteFileName(name))
        return name;

    if (d->stack.isEmpty())
        return name;

    QFileInfo path(QFileInfo(currentURL).absolutePath(), name);
    return path.absoluteFilePath();
}

QTextBrowser::QTextBrowser(QWidget *parent)
    : QTextEdit(*new QTextBrowserPrivate, parent)
{
    setReadOnly(true);
    setUndoRedoEnabled(false);
    d->viewport->setMouseTracking(true);
}

#ifdef QT_COMPAT
/*!
    Use one of the constructors that doesn't take the \a name
    argument and then use setObjectName() instead.
*/
QTextBrowser::QTextBrowser(QWidget *parent, const char *name)
    : QTextEdit(*new QTextBrowserPrivate, parent)
{
    setObjectName(name);
    setReadOnly(true);
    setUndoRedoEnabled(false);
    d->viewport->setMouseTracking(true);
}
#endif

QTextBrowser::~QTextBrowser()
{
}

QString QTextBrowser::source() const
{
    if (d->stack.isEmpty())
        return QString::null;
    else
        return d->stack.top();
}

void QTextBrowser::reload()
{
    QString s = d->currentURL;
    d->currentURL = QString::null;
    setSource(s);
}

void QTextBrowser::setSource(const QString& name)
{
    qDebug() << "QTextBrowser::setSource(" << name << ")";
    if (isVisible())
        qApp->setOverrideCursor(Qt::WaitCursor);

    d->textOrSourceChanged = true;
    QString source = name;
    QString anchor;
    int hash = name.indexOf('#');
    if (hash != -1) {
        source = name.left(hash);
        anchor = name.mid(hash+1);
    }

    if (source.startsWith("file:"))
        source = source.mid(6);

    QString url = d->resolvePath(source);
    QString txt;

    bool doSetText = false;

    if (!source.isEmpty() && url != d->currentURL) {
        QFile f(url);
        if (f.open(IO_ReadOnly)) {
            QByteArray data = f.readAll();
            QTextCodec *codec = QTextHtmlParser::codecForStream(data);
            txt = codec->toUnicode(data);

            if (txt.isEmpty())
                qWarning("QTextBrowser: no document for %s", source.latin1());
        } else {
            qWarning("QTextBrowser: cannot open '%s' for reading", url.toLocal8Bit().data());
        }

        if (isVisible()) {
            QString firstTag = txt.left(txt.indexOf('>') + 1);
            if (firstTag.left(3) == "<qt" && firstTag.contains("type") && firstTag.contains("detail")) {
                qApp->restoreOverrideCursor();
                QWhatsThis::showText(QCursor::pos(), txt, this);
                return;
            }
        }

        d->currentURL = url;
        doSetText = true;
    }

    d->currentAnchor = anchor;

    if (!anchor.isEmpty()) {
        url += '#';
        url += anchor;
    }

    if (d->home.isEmpty())
        d->home = url;

    if (d->stack.isEmpty() || d->stack.top() != url)
        d->stack.push(name);

    int stackCount = d->stack.count();
    if (d->stack.top() == name)
        stackCount--;
    emit backwardAvailable(stackCount > 0);

    stackCount = d->forwardStack.count();
    if (d->forwardStack.isEmpty() || d->forwardStack.top() == name)
        stackCount--;
    emit forwardAvailable(stackCount > 0);

    if (doSetText)
        QTextEdit::setHtml(txt);

    if (!anchor.isEmpty()) {
        scrollToAnchor(anchor);
    } else {
        d->hbar->setValue(0);
        d->vbar->setValue(0);
    }

    if (isVisible())
        qApp->restoreOverrideCursor();

    emit sourceChanged(url);
}

void QTextBrowser::backward()
{
    if (d->stack.count() <= 1)
        return;
    d->forwardStack.push(d->stack.pop());
    setSource(d->stack.pop());
    emit forwardAvailable(true);
}

void QTextBrowser::forward()
{
    if (d->forwardStack.isEmpty())
        return;
    setSource(d->forwardStack.pop());
    emit forwardAvailable(!d->forwardStack.isEmpty());
}

void QTextBrowser::home()
{
    if (!d->home.isNull())
        setSource(d->home);
}

void QTextBrowser::keyPressEvent(QKeyEvent *ev)
{
    if (ev->state() & Qt::AltButton) {
        switch (ev->key()) {
        case Qt::Key_Right:
            forward();
            ev->accept();
            return;
        case Qt::Key_Left:
            backward();
            ev->accept();
            return;
        case Qt::Key_Up:
            home();
            ev->accept();
            return;
        }
    }
    QTextEdit::keyPressEvent(ev);
}

void QTextBrowser::mouseMoveEvent(QMouseEvent *ev)
{
    QTextEdit::mouseMoveEvent(ev);

    QString anchor = d->doc->documentLayout()->anchorAt(d->translateCoordinates(ev->pos()));
    if (anchor.isEmpty()) {
        d->viewport->setCursor(Qt::ArrowCursor);
        emit highlighted(QString::null);
    } else {
        d->viewport->setCursor(Qt::PointingHandCursor);

        QUrl url = QUrl(d->currentURL).resolved(anchor);
        emit highlighted(url.toString());
    }

}

void QTextBrowser::mouseReleaseEvent(QMouseEvent *ev)
{
    QTextEdit::mouseReleaseEvent(ev);

    QString anchor = d->doc->documentLayout()->anchorAt(d->translateCoordinates(ev->pos()));
    if (!anchor.isEmpty()) {
        d->textOrSourceChanged = false;

        QUrl url = QUrl(d->currentURL).resolved(anchor);
        emit linkClicked(url.toString());

        // compat signal. the name is set to null. the 'name' makes no sense as it is
        // an attribute for specifying a destination.
        emit anchorClicked(QString::null, anchor);
        emit anchorClicked(anchor);

        if (!d->textOrSourceChanged)
            setSource(anchor);
    }
}

QImage QTextBrowser::loadImage(const QString &name)
{
    QImage img;
    img.load(d->resolvePath(name));
    return img;
}

/*
void QTextBrowser::setText(const QString &txt, const QString &context)
{
    d->textOrSourceChanged = true;
    d->curmark = "";
    d->curmain = "";
    Q3TextEdit::setText(txt, context);
}
*/

