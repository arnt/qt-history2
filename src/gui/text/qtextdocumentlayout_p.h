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

#ifndef QTEXTDOCUMENTLAYOUT_P_H
#define QTEXTDOCUMENTLAYOUT_P_H

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

#include <qabstracttextdocumentlayout.h>
#include <QtGui/qtextoption.h>

class QTextListFormat;

class QTextDocumentLayoutPrivate;

// ### remove the export again, for the non-public class
class Q_GUI_EXPORT QTextDocumentLayout : public QAbstractTextDocumentLayout
{
    Q_DECLARE_PRIVATE(QTextDocumentLayout)
    Q_OBJECT
public:
    explicit QTextDocumentLayout(QTextDocument *doc);

    // from the abstract layout
    void draw(QPainter *painter, const PaintContext &context);
    int hitTest(const QPointF &point, Qt::HitTestAccuracy accuracy) const;


    int pageCount() const;
    QSizeF documentSize() const;

    void adjustSize();

    enum { LTR = 0x40000000, RTL = 0x80000000 };
    // flags passed to QTextLayout objects of blocks
    void setBlockTextFlags(int flags);
    int blockTextFlags() const;
    void setWordWrapMode(QTextOption::WrapMode mode);
    QTextOption::WrapMode wordWrapMode() const;

    // internal, to support the ugly FixedColumnWidth wordwrap mode in QTextEdit
    void setFixedColumnWidth(int width);

    virtual QRectF frameBoundingRect(QTextFrame *frame) const;
    virtual QRectF blockBoundingRect(const QTextBlock &block) const;

protected:
    void documentChanged(int from, int oldLength, int length);
    void resizeInlineObject(QTextInlineObject item, int posInDocument, const QTextFormat &format);
    void positionInlineObject(QTextInlineObject item, int posInDocument, const QTextFormat &format);
    void drawInlineObject(QPainter *p, const QRectF &rect, QTextInlineObject item,
                          int posInDocument, const QTextFormat &format);
};

#endif // QTEXTDOCUMENTLAYOUT_P_H
