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

#ifndef QTEXTCONTROL_P_H
#define QTEXTCONTROL_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtGui/qtextdocument.h>
#include <QtGui/qtextoption.h>
#include <QtGui/qtextcursor.h>
#include <QtGui/qtextformat.h>
#include <QtGui/qtextedit.h>
#include <QtCore/qrect.h>

#ifdef QT3_SUPPORT
#include <QtGui/qtextobject.h>
#include <QtGui/qtextlayout.h>
#endif

QT_BEGIN_HEADER

QT_MODULE(Gui)

class QStyleSheet;
class QTextDocument;
class QMenu;
class QTextControlPrivate;
class QMimeData;
class QAbstractScrollArea;
class QEvent;
class QTimerEvent;

class Q_GUI_EXPORT QTextControl : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QTextControl)
    Q_PROPERTY(QString html READ toHtml WRITE setHtml NOTIFY textChanged USER true)
    Q_PROPERTY(bool overwriteMode READ overwriteMode WRITE setOverwriteMode)
    Q_PROPERTY(bool acceptRichText READ acceptRichText WRITE setAcceptRichText)
    Q_PROPERTY(int cursorWidth READ cursorWidth WRITE setCursorWidth)
    Q_PROPERTY(Qt::TextInteractionFlags textInteractionFlags READ textInteractionFlags WRITE setTextInteractionFlags)
    Q_PROPERTY(bool openExternalLinks READ openExternalLinks WRITE setOpenExternalLinks)
public:
    explicit QTextControl(QObject *parent = 0);
    explicit QTextControl(const QString &text, QObject *parent = 0);
    explicit QTextControl(QTextDocument *doc, QObject *parent = 0);
    virtual ~QTextControl();

    void setDocument(QTextDocument *document);
    QTextDocument *document() const;

    void setTextCursor(const QTextCursor &cursor);
    QTextCursor textCursor() const;

    void setTextInteractionFlags(Qt::TextInteractionFlags flags);
    Qt::TextInteractionFlags textInteractionFlags() const;

    void mergeCurrentCharFormat(const QTextCharFormat &modifier);

    void setCurrentCharFormat(const QTextCharFormat &format);
    QTextCharFormat currentCharFormat() const;

    bool find(const QString &exp, QTextDocument::FindFlags options = 0);

    inline QString toPlainText() const
    { return document()->toPlainText(); }
    inline QString toHtml() const
    { return document()->toHtml(); }

    void ensureCursorVisible();

    virtual QVariant loadResource(int type, const QUrl &name);
#ifndef QT_NO_CONTEXTMENU
    QMenu *createStandardContextMenu(const QPointF &pos, QWidget *parent);
#endif

    QTextCursor cursorForPosition(const QPointF &pos) const;
    QRectF cursorRect(const QTextCursor &cursor) const;
    QRectF cursorRect() const;
    QRectF selectionRect(const QTextCursor &cursor) const;
    QRectF selectionRect() const;

    QString anchorAt(const QPointF &pos) const;
    QPointF anchorPosition(const QString &name) const;

    QString anchorAtCursor() const;

    bool overwriteMode() const;
    void setOverwriteMode(bool overwrite);

    int cursorWidth() const;
    void setCursorWidth(int width);

    bool acceptRichText() const;
    void setAcceptRichText(bool accept);

#ifndef QT_NO_TEXTEDIT
    void setExtraSelections(const QList<QTextEdit::ExtraSelection> &selections);
    QList<QTextEdit::ExtraSelection> extraSelections() const;
#endif

    void setTextWidth(qreal width);
    qreal textWidth() const;
    QSizeF size() const;

    void setOpenExternalLinks(bool open);
    bool openExternalLinks() const;

    void moveCursor(QTextCursor::MoveOperation op, QTextCursor::MoveMode mode = QTextCursor::MoveAnchor);

    bool canPaste() const;

    void setCursorIsFocusIndicator(bool b);
    bool cursorIsFocusIndicator() const;

#ifndef QT_NO_PRINTER
    void print(QPrinter *printer) const;
#endif

public Q_SLOTS:
    void setPlainText(const QString &text);
    void setHtml(const QString &text);

#ifndef QT_NO_CLIPBOARD
    void cut();
    void copy();
    void paste();
#endif

    void undo();
    void redo();

    void clear();
    void selectAll();

    void insertPlainText(const QString &text);
    void insertHtml(const QString &text);

    void append(const QString &text);

    void adjustSize();

Q_SIGNALS:
    void textChanged();
    void undoAvailable(bool b);
    void redoAvailable(bool b);
    void currentCharFormatChanged(const QTextCharFormat &format);
    void copyAvailable(bool b);
    void selectionChanged();
    void cursorPositionChanged();

    // control signals
    void updateRequest(const QRectF &rect = QRectF());
    void documentSizeChanged(const QSizeF &);
    void visibilityRequest(const QRectF &rect);
    void microFocusChanged();
    void linkActivated(const QString &link);
    void linkHovered(const QString &);

public:
    // control properties
    QPalette palette() const;
    void setPalette(const QPalette &pal);

    virtual void processEvent(QEvent *e, const QMatrix &matrix, QWidget *contextWidget = 0);
    void processEvent(QEvent *e, const QPointF &coordinateOffset = QPointF(), QWidget *contextWidget = 0);

    // control methods
    void drawContents(QPainter *painter, const QRectF &rect = QRectF(), QWidget *widget = 0);

    void setFocus(bool focus, Qt::FocusReason = Qt::OtherFocusReason);

    virtual QVariant inputMethodQuery(Qt::InputMethodQuery property) const;

    virtual QMimeData *createMimeDataFromSelection() const;
    virtual bool canInsertFromMimeData(const QMimeData *source) const;
    virtual void insertFromMimeData(const QMimeData *source);

    bool setFocusToAnchor(const QTextCursor &newCursor);
    bool setFocusToNextOrPreviousAnchor(bool next);
    bool findNextPrevAnchor(const QTextCursor& from, bool next, QTextCursor& newAnchor);


protected:
    virtual void timerEvent(QTimerEvent *e);

    virtual bool event(QEvent *e);

private:
    Q_DISABLE_COPY(QTextControl)
    Q_PRIVATE_SLOT(d_func(), void _q_updateCurrentCharFormatAndSelection())
    Q_PRIVATE_SLOT(d_func(), void _q_emitCursorPosChanged(const QTextCursor &))
    Q_PRIVATE_SLOT(d_func(), void _q_deleteSelected())
    Q_PRIVATE_SLOT(d_func(), void _q_copyLink())
};

QT_END_HEADER

#endif // QTEXTCONTROL_H
