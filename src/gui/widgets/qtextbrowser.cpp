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
#include <qdebug.h>
#include <qabstracttextdocumentlayout.h>
#include <qurl.h>
#include "private/qtextdocumentlayout_p.h"
#include "private/qtexthtmlparser_p.h"
#include <qtextcodec.h>
#include <qpainter.h>
#include <qdir.h>

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

class QTextDetailPopup : public QWidget
{
public:
    QTextDetailPopup()
        : QWidget (0, Qt::WType_Popup | Qt::WDestructiveClose)
        {}

    virtual void mousePressEvent(QMouseEvent*)
    { close(); }
};


static void popupDetail(const QString& contents, const QPoint& pos)
{
    const int shadowWidth = 6;   // also used as '5' and '6' and even '8' below
    const int vMargin = 8;
    const int hMargin = 12;

    QWidget* popup = new QTextDetailPopup;
    popup->setAttribute(Qt::WA_NoSystemBackground, true);

    QTextDocument doc;
    doc.setHtml(contents); // ### popup->font()
    QTextDocumentLayout *layout = qt_cast<QTextDocumentLayout *>(doc.documentLayout());
    layout->adjustSize();

    QRect r(QPoint(0, 0), layout->sizeUsed());

    int w = r.width() + 2*hMargin;
    int h = r.height() + 2*vMargin;

    popup->resize(w + shadowWidth, h + shadowWidth);

    // okay, now to find a suitable location
    //###### we need a global fancy popup positioning somewhere
    popup->move(pos - popup->rect().center());
    if (popup->geometry().right() > QApplication::desktop()->width())
        popup->move(QApplication::desktop()->width() - popup->width(),
                     popup->y());
    if (popup->geometry().bottom() > QApplication::desktop()->height())
        popup->move(popup->x(),
                     QApplication::desktop()->height() - popup->height());
    if (popup->x() < 0)
        popup->move(0, popup->y());
    if (popup->y() < 0)
        popup->move(popup->x(), 0);


    popup->show();

    // now for super-clever shadow stuff.  super-clever mostly in
    // how many window system problems it skirts around.

    QPainter p(popup);
    p.setPen(QApplication::palette().color(QPalette::Active, QPalette::Foreground));
    p.drawRect(0, 0, w, h);
    p.setPen(QApplication::palette().color(QPalette::Active, QPalette::Mid));
    p.setBrush(QColor(255, 255, 240));
    p.drawRect(1, 1, w-2, h-2);
    p.setPen(Qt::black);

    QAbstractTextDocumentLayout::PaintContext context;
    context.textColorFromPalette = true;
    context.palette = popup->palette();
    p.save();
    p.setClipRect(r);
    layout->draw(&p, context);
    p.restore();

    p.drawPoint(w + 5, 6);
    p.drawLine(w + 3, 6,
                w + 5, 8);
    p.drawLine(w + 1, 6,
                w + 5, 10);
    int i;
    for(i=7; i < h; i += 2)
        p.drawLine(w, i,
                    w + 5, i + 5);
    for(i = w - i + h; i > 6; i -= 2)
        p.drawLine(i, h,
                    i + 5, h + 5);
    for(; i > 0 ; i -= 2)
        p.drawLine(6, h + 6 - i,
                    i + 5, h + 5);
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
                popupDetail(txt, QCursor::pos());
                qApp->restoreOverrideCursor();
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
        QTextEdit::setText(txt);

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

// #### temporary
QImage QTextBrowser::image(const QString &name)
{
    return loadImage(name);
}

