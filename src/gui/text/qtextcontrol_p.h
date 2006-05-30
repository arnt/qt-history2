/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
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

#include <QtGui/qevent.h>
#include <QtGui/qtextdocument.h>
#include <QtGui/qtextoption.h>
#include <QtGui/qtextcursor.h>
#include <QtGui/qtextformat.h>

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

class Q_AUTOTEST_EXPORT QTextControl : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QTextControl)
    Q_ENUMS(LineWrapMode)
    Q_PROPERTY(LineWrapMode lineWrapMode READ lineWrapMode WRITE setLineWrapMode)
    QDOC_PROPERTY(QTextOption::WrapMode wordWrapMode READ wordWrapMode WRITE setWordWrapMode)
    Q_PROPERTY(int lineWrapColumnOrWidth READ lineWrapColumnOrWidth WRITE setLineWrapColumnOrWidth)
    Q_PROPERTY(bool readOnly READ isReadOnly WRITE setReadOnly)
    Q_PROPERTY(QString html READ toHtml WRITE setHtml NOTIFY textChanged USER true)
    Q_PROPERTY(bool overwriteMode READ overwriteMode WRITE setOverwriteMode)
    Q_PROPERTY(int tabStopWidth READ tabStopWidth WRITE setTabStopWidth)
    Q_PROPERTY(bool acceptRichText READ acceptRichText WRITE setAcceptRichText)
    Q_PROPERTY(int cursorWidth READ cursorWidth WRITE setCursorWidth)
public:
    enum LineWrapMode {
        NoWrap,
        WidgetWidth,
        FixedPixelWidth,
        FixedColumnWidth
    };

    explicit QTextControl(QObject *parent = 0);
    explicit QTextControl(const QString &text, QObject *parent = 0);
    virtual ~QTextControl();

    void setDocument(QTextDocument *document);
    QTextDocument *document() const;

    void setTextCursor(const QTextCursor &cursor);
    QTextCursor textCursor() const;

    bool isReadOnly() const;
    void setReadOnly(bool ro);

    void mergeCurrentCharFormat(const QTextCharFormat &modifier);

    void setCurrentCharFormat(const QTextCharFormat &format);
    QTextCharFormat currentCharFormat() const;

    LineWrapMode lineWrapMode() const;
    void setLineWrapMode(LineWrapMode mode);

    int lineWrapColumnOrWidth() const;
    void setLineWrapColumnOrWidth(int w);

    QTextOption::WrapMode wordWrapMode() const;
    void setWordWrapMode(QTextOption::WrapMode policy);

    bool find(const QString &exp, QTextDocument::FindFlags options = 0);

    inline QString toPlainText() const
    { return document()->toPlainText(); }
    inline QString toHtml() const
    { return document()->toHtml(); }

    void ensureCursorVisible();

    virtual QVariant loadResource(int type, const QUrl &name);
#ifndef QT_NO_CONTEXTMENU
    QMenu *createStandardContextMenu();
#endif

    QTextCursor cursorForPosition(const QPointF &pos) const;
    QRectF cursorRect(const QTextCursor &cursor) const;
    QRectF cursorRect() const;

    QString anchorAt(const QPointF &pos) const;

    bool overwriteMode() const;
    void setOverwriteMode(bool overwrite);

    int tabStopWidth() const;
    void setTabStopWidth(int width);

    int cursorWidth() const;
    void setCursorWidth(int width);

    bool acceptRichText() const;
    void setAcceptRichText(bool accept);

    struct ExtraSelection
    {
        QTextCursor cursor;
        QTextCharFormat format;
    };
    void setExtraSelections(const QList<ExtraSelection> &selections);
    QList<ExtraSelection> extraSelections() const;

public Q_SLOTS:
    void setPlainText(const QString &text);
    void setHtml(const QString &text);

#ifndef QT_NO_CLIPBOARD
    void cut();
    void copy();
    void paste();
#endif

    void clear();
    void selectAll();

    void insertPlainText(const QString &text);
    void insertHtml(const QString &text);

    void append(const QString &text);

    void ensureAnchorIsVisible(const QString &name);

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

public:
    // control properties
    QPalette palette() const;
    void setPalette(const QPalette &pal);

    // control methods
    void drawContents(QPainter *painter, const QRectF &rect = QRectF());

    void setFocus(bool focus, Qt::FocusReason = Qt::OtherFocusReason);

    virtual QVariant inputMethodQuery(Qt::InputMethodQuery property) const;

    virtual bool event(QEvent *e);

protected:
    virtual void timerEvent(QTimerEvent *e);

    virtual QMimeData *createMimeDataFromSelection() const;
    virtual bool canInsertFromMimeData(const QMimeData *source) const;
    virtual void insertFromMimeData(const QMimeData *source);

private:
    Q_DISABLE_COPY(QTextControl)
    Q_PRIVATE_SLOT(d_func(), void updateCurrentCharFormatAndSelection())
    Q_PRIVATE_SLOT(d_func(), void emitCursorPosChanged(const QTextCursor &))
    Q_PRIVATE_SLOT(d_func(), void deleteSelected())
    Q_PRIVATE_SLOT(d_func(), void undo())
    Q_PRIVATE_SLOT(d_func(), void redo())
    Q_PRIVATE_SLOT(d_func(), void setCursorAfterUndoRedo(int, int, int))
};

QT_END_HEADER

#endif // QTEXTCONTROL_H
