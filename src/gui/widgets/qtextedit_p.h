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

#ifndef QTEXTEDIT_P_H
#define QTEXTEDIT_P_H

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

#include "private/qabstractscrollarea_p.h"
#include "QtGui/qtextdocumentfragment.h"
#include "QtGui/qscrollbar.h"
#include "QtGui/qtextcursor.h"
#include "QtGui/qtextformat.h"
#include "QtGui/qmenu.h"
#include "QtGui/qabstracttextdocumentlayout.h"
#include "QtCore/qbasictimer.h"
#include "private/qtextcontrol_p.h"
#include "qtextedit.h"

#ifndef QT_NO_TEXTEDIT

class QMimeData;

class QTextEditPrivate : public QAbstractScrollAreaPrivate
{
    Q_DECLARE_PUBLIC(QTextEdit)
public:
    QTextEditPrivate();

    void init(const QString &html = QString());
    void paint(QPainter *p, QPaintEvent *e);
    void _q_repaintContents(const QRectF &contentsRect);

    inline QPoint mapToContents(const QPoint &point) const
    { return QPoint(point.x() + horizontalOffset(), point.y() + verticalOffset()); }

    void _q_adjustScrollbars();
    void _q_ensureVisible(const QRectF &rect);
    void ensureViewportLayouted();
    void relayoutDocument();

    void createAutoBulletList();
    void pageUpDown(QTextCursor::MoveOperation op, QTextCursor::MoveMode moveMode);

    inline int horizontalOffset() const
    { return q_func()->isRightToLeft() ? (hbar->maximum() - hbar->value()) : hbar->value(); }
    inline int verticalOffset() const
    { return vbar->value(); }

    inline void sendControlEvent(QEvent *e)
    { control->processEvent(e, QPointF(horizontalOffset(), verticalOffset()), viewport); }

    void _q_currentCharFormatChanged(const QTextCharFormat &format);

    QTextControl *control;

    QTextEdit::AutoFormatting autoFormatting;
    bool tabChangesFocus;

    QBasicTimer autoScrollTimer;

    QTextEdit::LineWrapMode lineWrap;
    int lineWrapColumnOrWidth;

    bool ignoreAutomaticScrollbarAdjustement;

    // Qt3 COMPAT only
    // ### non-compat'ed append needs it, too
    Qt::TextFormat textFormat;
    bool preferRichText;

    QString anchorToScrollToWhenVisible;

#ifdef QT_KEYPAD_NAVIGATION
    QBasicTimer deleteAllTimer;
#endif
};
#endif // QT_NO_TEXTEDIT

#ifndef QT_NO_CONTEXTMENU
class QUnicodeControlCharacterMenu : public QMenu
{
    Q_OBJECT
public:
    QUnicodeControlCharacterMenu(QObject *editWidget, QWidget *parent);

private Q_SLOTS:
    void menuActionTriggered();

private:
    QObject *editWidget;
};
#endif // QT_NO_CONTEXTMENU

// also used by QLabel
class QTextEditMimeData : public QMimeData
{
public:
    inline QTextEditMimeData(const QTextDocumentFragment &aFragment) : fragment(aFragment) {}

    virtual QStringList formats() const;
protected:
    virtual QVariant retrieveData(const QString &mimeType, QVariant::Type type) const;
private:
    void setup() const;

    mutable QTextDocumentFragment fragment;
};

#endif // QTEXTEDIT_P_H
