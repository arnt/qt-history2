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
#include "private/qtextdocumentlayout_p.h"
#include <qtextcodec.h>
#include <qpainter.h>
#include <qdir.h>
#include <qwhatsthis.h>
#include <qtextobject.h>

class QTextBrowserPrivate : public QTextEditPrivate
{
    Q_DECLARE_PUBLIC(QTextBrowser)
public:
    QTextBrowserPrivate() : textOrSourceChanged(false), forceLoadOnSourceChange(false) {}

    void init();

    QStack<QUrl> stack;
    QStack<QUrl> forwardStack;
    QUrl home;
    QUrl currentURL;

    QStringList searchPaths;

    /*flag necessary to give the linkClicked() signal some meaningful
      semantics when somebody connected to it calls setText() or
      setSource() */
    bool textOrSourceChanged;

    bool forceLoadOnSourceChange;

    QString findFile(const QUrl &name) const;

    inline void documentModified()
    {
        textOrSourceChanged = true;
        forceLoadOnSourceChange = true;
    }

    void activateAnchor(const QString &href);
};

static bool isAbsoluteFileName(const QString &name)
{
    return !name.isEmpty()
           && (name[0] == '/'
#if defined(Q_WS_WIN)
               || (name[0].isLetter() && name[1] == QLatin1Char(':')) || name.startsWith("\\\\")
#endif
               || (name[0]  == QLatin1Char(':') && name[1] == QLatin1Char('/'))
              );

}

QString QTextBrowserPrivate::findFile(const QUrl &name) const
{
    QString fileName;
    if (name.scheme() == QLatin1String("qrc"))
        fileName = QLatin1String(":/") + name.path();
    else
        fileName = name.toLocalFile();

    if (isAbsoluteFileName(fileName))
        return fileName;

    QString slash("/");

    foreach (QString path, searchPaths) {
        if (!path.endsWith(slash))
            path.append(slash);
        path.append(fileName);
        if (QFileInfo(path).isReadable())
            return path;
    }

    if (stack.isEmpty())
        return fileName;

    QFileInfo path(QFileInfo(currentURL.toLocalFile()).absolutePath(), fileName);
    return path.absoluteFilePath();
}

void QTextBrowserPrivate::activateAnchor(const QString &href)
{
    if (href.isEmpty())
        return;
    Q_Q(QTextBrowser);

    textOrSourceChanged = false;

    const QUrl url = currentURL.resolved(href);
    emit q->anchorClicked(url);

    if (!textOrSourceChanged)
        q->setSource(url);
}

/*!
    \class QTextBrowser qtextbrowser.h
    \brief The QTextBrowser class provides a rich text browser with hypertext navigation.

    \ingroup text

    This class extends QTextEdit (in read-only mode), adding some
    navigation functionality so that users can follow links in
    hypertext documents. The contents of QTextEdit are set with
    setHtml() or setPlainText(), but QTextBrowser also implements the
    setSource() function, making it possible to set the text to a named
    document. The name is looked up in a list of search paths and in the
    directory of the current document factory. If a document name ends with
    an anchor (for example, "\c #anchor"), the text browser automatically
    scrolls to that position (using scrollToAnchor()). When the user clicks
    on a hyperlink, the browser will call setSource() itself with the link's
    \c href value as argument. You can track the current source by connecting
    to the sourceChanged() signal.

    QTextBrowser provides backward() and forward() slots which you can
    use to implement Back and Forward buttons. The home() slot sets
    the text to the very first document displayed. The anchorClicked()
    signal is emitted when the user clicks an anchor.

    If you want to provide your users with editable rich text use
    QTextEdit. If you want a text browser without hypertext navigation
    use QTextEdit, and use QTextEdit::setReadOnly() to disable
    editing. If you just need to display a small piece of rich text
    use QLabel.
*/

/*!
    \property QTextBrowser::modified
    \brief whether the contents of the text browser have been modified
*/

/*!
    \property QTextBrowser::readOnly
    \brief whether the text browser is read-only
*/

/*!
    \property QTextBrowser::undoRedoEnabled
    \brief whether the text browser supports undo/redo operations
*/

void QTextBrowserPrivate::init()
{
    Q_Q(QTextBrowser);
    q->setReadOnly(true);
    q->setUndoRedoEnabled(false);
    viewport->setMouseTracking(true);
    QObject::connect(q->document(), SIGNAL(contentsChanged()), q, SLOT(documentModified()));
}

/*!
    Constructs an empty QTextBrowser with parent \a parent.
*/
QTextBrowser::QTextBrowser(QWidget *parent)
    : QTextEdit(*new QTextBrowserPrivate, parent)
{
    Q_D(QTextBrowser);
    d->init();
}

#ifdef QT3_SUPPORT
/*!
    Use one of the constructors that doesn't take the \a name
    argument and then use setObjectName() instead.
*/
QTextBrowser::QTextBrowser(QWidget *parent, const char *name)
    : QTextEdit(*new QTextBrowserPrivate, parent)
{
    setObjectName(name);
    Q_D(QTextBrowser);
    d->init();
}
#endif

/*!
    \internal
*/
QTextBrowser::~QTextBrowser()
{
}

/*!
    \property QTextBrowser::source
    \brief the name of the displayed document.

    This is a an invalid url if no document is displayed or if the
    source is unknown.

    When setting this property QTextBrowser tries to find a document
    with the specified name in the paths of the searchPaths property
    and directory of the current source, unless the value is an absolute
    file path. It also checks for optional anchors and scrolls the document
    accordingly

    If the first tag in the document is \c{<qt type=detail>}, the
    document is displayed as a popup rather than as new document in
    the browser window itself. Otherwise, the document is displayed
    normally in the text browser with the text set to the contents of
    the named document with setHtml().
*/
QUrl QTextBrowser::source() const
{
    Q_D(const QTextBrowser);
    if (d->stack.isEmpty())
        return QUrl();
    else
        return d->stack.top();
}

/*!
    \property QTextBrowser::searchPaths
    \brief the search paths used by the text browser to find supporting
    content

    QTextBrowser uses this list to locate images and documents.
*/

QStringList QTextBrowser::searchPaths() const
{
    Q_D(const QTextBrowser);
    return d->searchPaths;
}

void QTextBrowser::setSearchPaths(const QStringList &paths)
{
    Q_D(QTextBrowser);
    d->searchPaths = paths;
}

/*!
    Reloads the current set source.
*/
void QTextBrowser::reload()
{
    Q_D(QTextBrowser);
    QUrl s = d->currentURL;
    d->currentURL = QUrl();
    setSource(s);
}

void QTextBrowser::setSource(const QUrl &url)
{
    Q_D(QTextBrowser);
    if (isVisible())
        qApp->setOverrideCursor(Qt::WaitCursor);

    d->textOrSourceChanged = true;

    QString txt;

    bool doSetText = false;

    QUrl currentUrlWithoutFragment = d->currentURL;
    currentUrlWithoutFragment.setFragment(QString());
    QUrl urlWithoutFragment = url;
    urlWithoutFragment.setFragment(QString());

    if (url.isValid()
        && (urlWithoutFragment != currentUrlWithoutFragment || d->forceLoadOnSourceChange)) {
        QVariant data = loadResource(QTextDocument::HtmlResource, url);
        if (data.type() == QVariant::String) {
            txt = data.toString();
        } else if (data.type() == QVariant::ByteArray) {
            QByteArray ba = data.toByteArray();
            QTextCodec *codec = Qt::codecForHtml(ba);
            txt = codec->toUnicode(ba);
        }
        if (txt.isEmpty())
            qWarning("QTextBrowser: no document for %s", url.toString().toLatin1().constData());

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

    if (!d->home.isValid())
        d->home = url;

    if (d->stack.isEmpty() || d->stack.top() != url)
        d->stack.push(url);

    int stackCount = d->stack.count();
    if (d->stack.top() == url)
        stackCount--;
    emit backwardAvailable(stackCount > 0);

    stackCount = d->forwardStack.count();
    if (d->forwardStack.isEmpty() || d->forwardStack.top() == url)
        stackCount--;
    emit forwardAvailable(stackCount > 0);

    if (doSetText)
        QTextEdit::setHtml(txt);

    d->forceLoadOnSourceChange = false;

    if (!url.fragment().isEmpty()) {
        scrollToAnchor(url.fragment());
    } else {
        d->hbar->setValue(0);
        d->vbar->setValue(0);
    }

    if (isVisible())
        qApp->restoreOverrideCursor();

    emit sourceChanged(url);
}

/*!
    \fn void QTextBrowser::backwardAvailable(bool available)

    This signal is emitted when the availability of backward()
    changes. \a available is false when the user is at home();
    otherwise it is true.
*/

/*!
    \fn void QTextBrowser::forwardAvailable(bool available)

    This signal is emitted when the availability of forward() changes.
    \a available is true after the user navigates backward() and false
    when the user navigates or goes forward().
*/

/*!
    \fn void QTextBrowser::sourceChanged(const QUrl &src)

    This signal is emitted when the source has changed, \a src
    being the new source.

    Source changes happen both programmatically when calling
    setSource(), forward(), backword() or home() or when the user
    clicks on links or presses the equivalent key sequences.
*/

/*!  \fn void QTextBrowser::highlighted(const QUrl &link)

    This signal is emitted when the user has selected but not
    activated an anchor in the document. The URL referred to by the
    anchor is passed in \a link.
*/

/*!  \fn void QTextBrowser::highlighted(const QString &link)
     \overload

     Convenience signal that allows connecting to a slot
     that takes just a QString, like for example QStatusBar's
     message().
*/


/*!
    \fn void QTextBrowser::anchorClicked(const QUrl &link)

    This signal is emitted when the user clicks an anchor. The
    URL referred to by the anchor is passed in \a link.
*/

/*!
    Changes the document displayed to the previous document in the
    list of documents built by navigating links. Does nothing if there
    is no previous document.

    \sa forward(), backwardAvailable()
*/
void QTextBrowser::backward()
{
    Q_D(QTextBrowser);
    if (d->stack.count() <= 1)
        return;
    d->forwardStack.push(d->stack.pop());
    setSource(d->stack.pop());
    emit forwardAvailable(true);
}

/*!
    Changes the document displayed to the next document in the list of
    documents built by navigating links. Does nothing if there is no
    next document.

    \sa backward(), forwardAvailable()
*/
void QTextBrowser::forward()
{
    Q_D(QTextBrowser);
    if (d->forwardStack.isEmpty())
        return;
    setSource(d->forwardStack.pop());
    emit forwardAvailable(!d->forwardStack.isEmpty());
}

/*!
    Changes the document displayed to be the first document the
    browser displayed.
*/
void QTextBrowser::home()
{
    Q_D(QTextBrowser);
    if (d->home.isValid())
        setSource(d->home);
}

/*!
    The event \a ev is used to provide the following keyboard shortcuts:
    \table
    \header \i Keypress            \i Action
    \row \i Alt+Left Arrow  \i \l backward()
    \row \i Alt+Right Arrow \i \l forward()
    \row \i Alt+Up Arrow    \i \l home()
    \endtable
*/
void QTextBrowser::keyPressEvent(QKeyEvent *ev)
{
    Q_D(QTextBrowser);

    if (ev->modifiers() & Qt::AltModifier) {
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
    } else if ((ev->key() == Qt::Key_Return
                || ev->key() == Qt::Key_Enter)
               && d->focusIndicator.hasSelection()) {

        QTextCursor cursor = d->focusIndicator;
        if (cursor.selectionStart() != cursor.position())
            cursor.setPosition(cursor.selectionStart());
        cursor.movePosition(QTextCursor::NextCharacter);

        ev->accept();

        const QString href = cursor.charFormat().anchorHref();
        d->activateAnchor(href);
        return;
    }
    QTextEdit::keyPressEvent(ev);
}

/*!
    \reimp
*/
void QTextBrowser::mouseMoveEvent(QMouseEvent *e)
{
    Q_D(QTextBrowser);
    QTextEdit::mouseMoveEvent(e);

    QString anchor = anchorAt(e->pos());
    if (anchor.isEmpty()) {
        d->viewport->setCursor(Qt::ArrowCursor);
        emit highlighted(QUrl());
        emit highlighted(QString());
    } else {
        d->viewport->setCursor(Qt::PointingHandCursor);

        QUrl url = QUrl(d->currentURL).resolved(anchor);
        emit highlighted(url);
        // convenience to ease connecting to QStatusBar::showMessage(const QString &)
        emit highlighted(url.toString());
    }

}

/*!
    \reimp
*/
void QTextBrowser::mouseReleaseEvent(QMouseEvent *e)
{
    Q_D(QTextBrowser);
    QTextEdit::mouseReleaseEvent(e);

    if (!(e->button() & Qt::LeftButton) || d->cursor.hasSelection())
        return;

    const QString anchor = anchorAt(e->pos());
    d->activateAnchor(anchor);
}

/*!
    \reimp
*/
void QTextBrowser::focusOutEvent(QFocusEvent *ev)
{
    Q_D(QTextBrowser);
    if (ev->reason() != Qt::ActiveWindowFocusReason
        && ev->reason() != Qt::PopupFocusReason) {
        d->focusIndicator.clearSelection();
        d->viewport->update();
    }
    QTextEdit::focusOutEvent(ev);
}

/*!
    \reimp
*/
bool QTextBrowser::focusNextPrevChild(bool next)
{
    Q_D(QTextBrowser);

    if (!d->readOnly)
        return false;

    if (!d->focusIndicator.hasSelection()) {
        d->focusIndicator = QTextCursor(d->doc);
        if (next)
            d->focusIndicator.movePosition(QTextCursor::Start);
        else
            d->focusIndicator.movePosition(QTextCursor::End);
    }

    Q_ASSERT(!d->focusIndicator.isNull());

    int anchorStart = -1;
    int anchorEnd = -1;

    if (next) {
        const int startPos = d->focusIndicator.selectionEnd();

        QTextBlock block = d->doc->findBlock(startPos);
        QTextBlock::Iterator it = block.begin();

        while (!it.atEnd() && it.fragment().position() < startPos)
            ++it;

        while (block.isValid()) {
            anchorStart = -1;

            // find next anchor
            for (; !it.atEnd(); ++it) {
                const QTextFragment fragment = it.fragment();
                const QTextCharFormat fmt = fragment.charFormat();

                if (fmt.isAnchor() && fmt.hasProperty(QTextFormat::AnchorHref)) {
                    anchorStart = fragment.position();
                    break;
                }
            }

            if (anchorStart != -1) {
                anchorEnd = -1;

                // find next non-anchor fragment
                for (; !it.atEnd(); ++it) {
                    const QTextFragment fragment = it.fragment();
                    const QTextCharFormat fmt = fragment.charFormat();

                    if (!fmt.isAnchor()) {
                        anchorEnd = fragment.position();
                        break;
                    }
                }

                if (anchorEnd == -1)
                    anchorEnd = block.position() + block.length() - 1;

                // make found selection
                break;
            }

            block = block.next();
            it = block.begin();
        }
    } else {
        int startPos = d->focusIndicator.selectionStart();
        if (startPos > 0)
            --startPos;

        QTextBlock block = d->doc->findBlock(startPos);
        QTextBlock::Iterator blockStart = block.begin();
        QTextBlock::Iterator it = block.end();

        if (startPos == block.position()) {
            it = block.begin();
        } else {
            do {
                if (it == blockStart) {
                    it = QTextBlock::Iterator();
                    block = QTextBlock();
                } else {
                    --it;
                }
            } while (!it.atEnd() && it.fragment().position() + it.fragment().length() - 1 > startPos);
        }

        while (block.isValid()) {
            anchorStart = -1;

            if (!it.atEnd()) {
                do {
                    const QTextFragment fragment = it.fragment();
                    const QTextCharFormat fmt = fragment.charFormat();

                    if (fmt.isAnchor() && fmt.hasProperty(QTextFormat::AnchorHref)) {
                        anchorStart = fragment.position() + fragment.length();
                        break;
                    }

                    if (it == blockStart)
                        it = QTextBlock::Iterator();
                    else
                        --it;
                } while (!it.atEnd());
            }

            if (anchorStart != -1 && !it.atEnd()) {
                anchorEnd = -1;

                do {
                    const QTextFragment fragment = it.fragment();
                    const QTextCharFormat fmt = fragment.charFormat();

                    if (!fmt.isAnchor()) {
                        anchorEnd = fragment.position() + fragment.length();
                        break;
                    }

                    if (it == blockStart)
                        it = QTextBlock::Iterator();
                    else
                        --it;
                } while (!it.atEnd());

                if (anchorEnd == -1)
                    anchorEnd = qMax(0, block.position());

                break;
            }

            block = block.previous();
            it = block.end();
            if (it != block.begin())
                --it;
            blockStart = block.begin();
        }

    }

    if (anchorStart != -1 && anchorEnd != -1) {
        d->focusIndicator.setPosition(anchorStart);
        d->focusIndicator.setPosition(anchorEnd, QTextCursor::KeepAnchor);
    } else {
        d->focusIndicator.clearSelection();
    }

    if (d->focusIndicator.hasSelection()) {
        qSwap(d->focusIndicator, d->cursor);
        ensureCursorVisible();
        qSwap(d->focusIndicator, d->cursor);
        d->viewport->update();
        return true;
    } else {
        d->viewport->update();
        return false;
    }
}

/*!
  \reimp
*/
void QTextBrowser::paintEvent(QPaintEvent *e)
{
    Q_D(QTextBrowser);
    QPainter p(d->viewport);
    d->paint(&p, e);
}

/*!
    This function is called when the document is loaded. The \a type
    indicates the type of resource to be loaded. For each image in
    the document, this function is called once.

    The default implementation ignores \a type and tries to locate
    the resources by interpreting \a name as a file name. If it is
    not an absolute path it tries to find the file in the paths of
    the \l searchPaths property and in the same directory as the
    current source. On success, the result is a QVariant that stores
    a QByteArray with the contents of the file.

    If you reimplement this function, you can return other QVariant
    types. The table below shows which variant types are supported
    depending on the resource type:

    \table
    \header \i ResourceType  \i QVariant::Type
    \row    \i QTextDocument::HtmlResource  \i QString or QByteArray
    \row    \i QTextDocument::ImageResource \i QImage, QPixmap or QByteArray
    \endtable
*/
QVariant QTextBrowser::loadResource(int /*type*/, const QUrl &name)
{
    Q_D(QTextBrowser);

    QByteArray data;
    QUrl resolved = name;
    if (!isAbsoluteFileName(name.toLocalFile()))
        resolved = source().resolved(name);    
    QString fileName = d->findFile(resolved);
    QFile f(fileName);
    if (f.open(QFile::ReadOnly)) {
        data = f.readAll();
        f.close();
    } else {
        qWarning("QTextBrowser: cannot open '%s' for reading", fileName.toLocal8Bit().data());
    }

    return data;
}

#include "moc_qtextbrowser.cpp"
