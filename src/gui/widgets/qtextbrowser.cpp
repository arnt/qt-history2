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

#include "qtextbrowser.h"
#include "qtextedit_p.h"

#ifndef QT_NO_TEXTBROWSER

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
    QTextBrowserPrivate()
        : textOrSourceChanged(false), forceLoadOnSourceChange(false),
          hadSelectionOnMousePress(false) {}

    void init();

    struct HistoryEntry {
        QUrl url;
        int hpos;
        int vpos;
    };

    QStack<HistoryEntry> stack;
    QStack<HistoryEntry> forwardStack;
    QUrl home;
    QUrl currentURL;

    QStringList searchPaths;

    /*flag necessary to give the linkClicked() signal some meaningful
      semantics when somebody connected to it calls setText() or
      setSource() */
    bool textOrSourceChanged;
    QString highlightedAnchor; // Anchor below cursor
    bool forceLoadOnSourceChange;

#ifndef QT_NO_CURSOR
    QCursor oldCursor;
#endif

    QString findFile(const QUrl &name) const;

    inline void _q_documentModified()
    {
        textOrSourceChanged = true;
        forceLoadOnSourceChange = true;
    }

    void _q_activateAnchor(const QString &href);

    void setSource(const QUrl &url);

    QString anchorOnMousePress;
    bool hadSelectionOnMousePress;

#ifdef QT_KEYPAD_NAVIGATION
    void keypadMove(bool next);
#endif
};

static bool isAbsoluteFileName(const QString &name)
{
    return !name.isEmpty()
           && (name[0] == QLatin1Char('/')
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

    foreach (QString path, searchPaths) {
        if (!path.endsWith(QLatin1Char('/')))
            path.append(QLatin1Char('/'));
        path.append(fileName);
        if (QFileInfo(path).isReadable())
            return path;
    }

    if (stack.isEmpty())
        return fileName;

    QFileInfo path(QFileInfo(currentURL.toLocalFile()).absolutePath(), fileName);
    return path.absoluteFilePath();
}

void QTextBrowserPrivate::_q_activateAnchor(const QString &href)
{
    if (href.isEmpty())
        return;
    Q_Q(QTextBrowser);

    textOrSourceChanged = false;

    const QUrl url = isAbsoluteFileName(currentURL.toLocalFile())
                     ? currentURL.resolved(href) : QUrl(href);
    emit q->anchorClicked(url);

    if (textOrSourceChanged)
        return;

    q->setSource(url);
}

void QTextBrowserPrivate::setSource(const QUrl &url)
{
    Q_Q(QTextBrowser);
#ifndef QT_NO_CURSOR
    if (q->isVisible())
        qApp->setOverrideCursor(Qt::WaitCursor);
#endif
    textOrSourceChanged = true;

    QString txt;

    bool doSetText = false;

    QUrl currentUrlWithoutFragment = currentURL;
    currentUrlWithoutFragment.setFragment(QString());
    QUrl newUrlWithoutFragment = currentURL.resolved(url);
    newUrlWithoutFragment.setFragment(QString());

    if (url.isValid()
        && (newUrlWithoutFragment != currentUrlWithoutFragment || forceLoadOnSourceChange)) {
        QVariant data = q->loadResource(QTextDocument::HtmlResource, url);
        if (data.type() == QVariant::String) {
            txt = data.toString();
        } else if (data.type() == QVariant::ByteArray) {
#ifndef QT_NO_TEXTCODEC
            QByteArray ba = data.toByteArray();
            QTextCodec *codec = Qt::codecForHtml(ba);
            txt = codec->toUnicode(ba);
#else
	    txt = data.toString();
#endif
        }
        if (txt.isEmpty())
            qWarning("QTextBrowser: No document for %s", url.toString().toLatin1().constData());

        if (q->isVisible()) {
            QString firstTag = txt.left(txt.indexOf(QLatin1Char('>')) + 1);
            if (firstTag.left(3) == QLatin1String("<qt") && firstTag.contains(QLatin1String("type")) && firstTag.contains(QLatin1String("detail"))) {
#ifndef QT_NO_CURSOR
                qApp->restoreOverrideCursor();
#endif
#ifndef QT_NO_WHATSTHIS
                QWhatsThis::showText(QCursor::pos(), txt, q);
#endif
                return;
            }
        }

        currentURL = currentURL.resolved(url);
        doSetText = true;
    }

    if (!home.isValid())
        home = url;

    if (doSetText)
        q->QTextEdit::setHtml(txt);

    forceLoadOnSourceChange = false;

    if (!url.fragment().isEmpty()) {
        q->scrollToAnchor(url.fragment());
    } else {
        hbar->setValue(0);
        vbar->setValue(0);
    }

#ifndef QT_NO_CURSOR
    if (q->isVisible())
        qApp->restoreOverrideCursor();
#endif
    emit q->sourceChanged(url);
}

#ifdef QT_KEYPAD_NAVIGATION
void QTextBrowserPrivate::keypadMove(bool next)
{
    Q_Q(QTextBrowser);

    const int height = viewport->height();
    const int yOffset = vbar->value();
    if (control->setFocusToNextOrPreviousAnchor(next)) {
        const int cursYOffset = qRound(control->cursorRect().top());
        const int overlap = 20;
        if (next) {
            if (cursYOffset > yOffset + height) {
                vbar->setValue(yOffset + height - overlap);
                if (cursYOffset > vbar->value() + height) {
                    emit q->highlighted(QUrl());
                    emit q->highlighted(QString());
                    return;
                }
            } else if (cursYOffset < yOffset) {
                if (yOffset < vbar->maximum())
                    vbar->setValue(yOffset + height - overlap);
                else
                    vbar->setValue(0);
                emit q->highlighted(QUrl());
                emit q->highlighted(QString());
                return;
            }
        } else {
            if (cursYOffset < yOffset) {
                vbar->setValue(yOffset - height + overlap);
                if (cursYOffset < vbar->value()) {
                    emit q->highlighted(QUrl());
                    emit q->highlighted(QString());
                    return;
                }
            } else if (cursYOffset > yOffset + height) {
                if (yOffset > 0)
                    vbar->setValue(yOffset - height + overlap);
                else
                    vbar->setValue(vbar->maximum());
                emit q->highlighted(QUrl());
                emit q->highlighted(QString());
                return;
            }
        }

        QTextCursor cursor = control->textCursor();
        if (cursor.selectionStart() != cursor.position())
            cursor.setPosition(cursor.selectionStart());
        cursor.movePosition(QTextCursor::NextCharacter);
        QTextCharFormat charFmt = cursor.charFormat();
        emit q->highlighted(QUrl(charFmt.anchorHref()));
        emit q->highlighted(charFmt.anchorHref());
    } else {
        const int yOffset = vbar->value();
        const int overlap = 20;
        if (next) {
            if (yOffset == vbar->maximum())
                vbar->setValue(0);
            else
                vbar->setValue(yOffset + height - overlap);
        } else {
            if (yOffset == 0)
                vbar->setValue(vbar->maximum());
            else
                vbar->setValue(yOffset - height + overlap);
        }
        QTextCursor cursor = control->textCursor();
        cursor.clearSelection();
        control->setTextCursor(cursor);

        emit q->highlighted(QUrl());
        emit q->highlighted(QString());
    }
}
#endif

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
    signal is emitted when the user clicks an anchor. To override the
    default navigation behavior of the browser, call the setSource()
    function to supply new document text in a slot connected to this
    signal.

    If you want to provide your users with editable rich text use
    QTextEdit. If you want a text browser without hypertext navigation
    use QTextEdit, and use QTextEdit::setReadOnly() to disable
    editing. If you just need to display a small piece of rich text
    use QLabel.

    If you want to load documents stored in the Qt resource system use
    qrc as the scheme in the URL to load. For example, for the document
    resource path \c{:/docs/index.html} use \c{qrc:/docs/index.html} as
    the URL with setSource().

    \sa QTextEdit, QTextDocument
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
    control->setTextInteractionFlags(Qt::TextBrowserInteraction);
#ifndef QT_NO_CURSOR
    viewport->setCursor(oldCursor);
#endif
    q->setUndoRedoEnabled(false);
    viewport->setMouseTracking(true);
    QObject::connect(q->document(), SIGNAL(contentsChanged()), q, SLOT(_q_documentModified()));
    QObject::connect(control, SIGNAL(activateLinkRequest(const QString &)),
                     q, SLOT(_q_activateAnchor(const QString &)));
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
    setObjectName(QString::fromAscii(name));
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
        return d->stack.top().url;
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

    int hpos = d->hbar->value();
    int vpos = d->vbar->value();

    d->setSource(url);

    if (!url.isValid())
        return;

    if (!d->stack.isEmpty() && d->stack.top().url == url) {
        // the same url you are already watching
    } else {
        if (!d->stack.isEmpty()) {
            d->stack.top().hpos = hpos;
            d->stack.top().vpos = vpos;
        }
        QTextBrowserPrivate::HistoryEntry entry;
        entry.url = url;
        entry.hpos = 0;
        entry.vpos = 0;
        d->stack.push(entry);

        emit backwardAvailable(d->stack.count() > 1);

        if (!d->forwardStack.isEmpty() && d->forwardStack.top().url == url) {
            d->forwardStack.pop();
            emit forwardAvailable(d->forwardStack.count() > 0);
        } else {
            d->forwardStack.clear();
            emit forwardAvailable(false);
        }
    }
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

    Note that the browser will automatically handle navigation to the
    location specified by \a link unless you call setSource() in a slot
    connected. This mechanism is used to override the default navigation
    features of the browser.
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
    d->forwardStack.top().hpos = d->hbar->value();
    d->forwardStack.top().vpos = d->vbar->value();
    d->setSource(d->stack.top().url);
    d->hbar->setValue(d->stack.top().hpos);
    d->vbar->setValue(d->stack.top().vpos);
    emit backwardAvailable(d->stack.count() > 1);
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
    if (!d->stack.isEmpty()) {
        d->stack.top().hpos = d->hbar->value();
        d->stack.top().vpos = d->vbar->value();
    }
    d->stack.push(d->forwardStack.pop());
    setSource(d->stack.top().url);
    d->hbar->setValue(d->stack.top().hpos);
    d->vbar->setValue(d->stack.top().vpos);
    emit backwardAvailable(true);
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
#ifdef QT_KEYPAD_NAVIGATION
    switch (ev->key()) {
    case Qt::Key_Select:
        if (QApplication::keypadNavigationEnabled() && !hasEditFocus()) {
            setEditFocus(true);
            return;
        }
        break;
    case Qt::Key_Back:
        if (QApplication::keypadNavigationEnabled()) {
            if (hasEditFocus()) {
                setEditFocus(false);
                ev->accept();
                return;
            }
        }
        QTextEdit::keyPressEvent(ev);
        return;
    default:
        if (QApplication::keypadNavigationEnabled() && !hasEditFocus()) {
            ev->ignore();
            return;
        }
    }
#endif

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
    }
#ifdef QT_KEYPAD_NAVIGATION
    else if (QApplication::keypadNavigationEnabled()) {
        Q_D(QTextBrowser);
        if (ev->key() == Qt::Key_Up) {
            d->keypadMove(false);
            return;
        } else if (ev->key() == Qt::Key_Down) {
            d->keypadMove(true);
            return;
        }
    }
#endif
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
    if (anchor == d->highlightedAnchor)
        return;

    if (anchor.isEmpty()) {
#ifndef QT_NO_CURSOR
        if (d->viewport->cursor().shape() != Qt::PointingHandCursor)
            d->oldCursor = d->viewport->cursor();
        d->viewport->setCursor(d->oldCursor);
#endif
        emit highlighted(QUrl());
        emit highlighted(QString());
    } else {
#ifndef QT_NO_CURSOR
        d->viewport->setCursor(Qt::PointingHandCursor);
#endif

        const QUrl url = isAbsoluteFileName(d->currentURL.toLocalFile())
                         ? d->currentURL.resolved(anchor) : QUrl(anchor);
        emit highlighted(url);
        // convenience to ease connecting to QStatusBar::showMessage(const QString &)
        emit highlighted(url.toString());
    }
    d->highlightedAnchor = anchor;
}

/*!
    \reimp
*/
void QTextBrowser::mousePressEvent(QMouseEvent *e)
{
    Q_D(QTextBrowser);
    d->anchorOnMousePress = anchorAt(e->pos());
    if (!d->control->textCursor().hasSelection() && !d->anchorOnMousePress.isEmpty()) {
        QTextCursor cursor = cursorForPosition(e->pos());
        setTextCursor(cursor);
    }

    QTextEdit::mousePressEvent(e);

    d->hadSelectionOnMousePress = d->control->textCursor().hasSelection();
}

/*!
    \reimp
*/
void QTextBrowser::mouseReleaseEvent(QMouseEvent *e)
{
    Q_D(QTextBrowser);
    QTextEdit::mouseReleaseEvent(e);

    if (!(e->button() & Qt::LeftButton))
        return;

    const QString anchor = anchorAt(e->pos());

    if (anchor.isEmpty())
        return;

    if (!d->control->textCursor().hasSelection()
        || (anchor == d->anchorOnMousePress && d->hadSelectionOnMousePress))
        d->_q_activateAnchor(anchor);
}

/*!
    \reimp
*/
void QTextBrowser::focusOutEvent(QFocusEvent *ev)
{
    Q_D(QTextBrowser);
#ifndef QT_NO_CURSOR
    d->viewport->setCursor((!(d->control->textInteractionFlags() & Qt::TextEditable)) ? d->oldCursor : Qt::IBeamCursor);
#endif
    QTextEdit::focusOutEvent(ev);
}

/*!
    \reimp
*/
bool QTextBrowser::focusNextPrevChild(bool next)
{
    Q_D(QTextBrowser);
    if (d->control->setFocusToNextOrPreviousAnchor(next))
        return true;
    return QTextEdit::focusNextPrevChild(next);
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
    \row    \i QTextDocument::StyleSheetResource \i QString or QByteArray
    \endtable
*/
QVariant QTextBrowser::loadResource(int /*type*/, const QUrl &name)
{
    Q_D(QTextBrowser);

    QByteArray data;
    QUrl resolved = name;
    if (!isAbsoluteFileName(name.toLocalFile()) && isAbsoluteFileName(source().toLocalFile()))
        resolved = source().resolved(name);
    QString fileName = d->findFile(resolved);
    QFile f(fileName);
    if (f.open(QFile::ReadOnly)) {
        data = f.readAll();
        f.close();
    } else {
        qWarning("QTextBrowser: Cannot open '%s' for reading", fileName.toLocal8Bit().data());
    }

    return data;
}

/*!
      Returns true if the text browser can go backward in the document history
      using backward().

      \sa backwardAvailable(), backward()
*/
bool QTextBrowser::isBackwardAvailable() const
{
    Q_D(const QTextBrowser);
    return d->stack.count() > 1;
}

/*!
      Returns true if the text browser can go forward in the document history
      using forward().

      \sa forwardAvailable(), forward()
*/
bool QTextBrowser::isForwardAvailable() const
{
    Q_D(const QTextBrowser);
    return !d->forwardStack.isEmpty();
}

/*!
    Clears the history of visited documents and disables the forward and
    backward navigation.

    \sa backward(), forward()
*/
void QTextBrowser::clearHistory()
{
    Q_D(QTextBrowser);
    d->forwardStack.clear();
    if (!d->stack.isEmpty())
        d->stack.resize(1);
    emit forwardAvailable(false);
    emit backwardAvailable(false);
}

/*! \reimp */
bool QTextBrowser::event(QEvent *e)
{
    return QTextEdit::event(e);
}

#include "moc_qtextbrowser.cpp"
#endif // QT_NO_TEXTBROWSER
