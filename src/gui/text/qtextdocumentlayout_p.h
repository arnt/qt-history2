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

class QTextListFormat;

class QTextDocumentLayoutPrivate;

// ### remove the export again, for the non-public class
class Q_GUI_EXPORT QTextDocumentLayout : public QAbstractTextDocumentLayout
{
    Q_DECLARE_PRIVATE(QTextDocumentLayout)
    Q_OBJECT
public:
    QTextDocumentLayout(QTextDocument *doc);

    // from the abstract layout
    void draw(QPainter *painter, const PaintContext &context);
    void documentChange(int from, int oldLength, int length);
    int hitTest(const QPoint &point, Qt::HitTestAccuracy accuracy) const;

    void setSize(QTextInlineObject item, const QTextFormat &format);
    void layoutObject(QTextInlineObject item, const QTextFormat &format);
    void drawObject(QPainter *p, const QRectF &rect, QTextInlineObject item,
                    const QTextFormat &format);

    int numPages() const;

    virtual QSize sizeUsed() const;

    void adjustSize();

    virtual void setPageSize(const QSize &size);
    virtual QSize pageSize() const;

    // flags passed to QTextLayout objects of blocks
    void setBlockTextFlags(int flags);
    int blockTextFlags() const;

    // internal, to support the ugly FixedColumnWidth wordwrap mode in QTextEdit
    void setFixedColumnWidth(int width);

    virtual QRect frameBoundingRect(QTextFrame *frame) const;

signals:
    void usedSizeChanged();
};

#endif // QTEXTDOCUMENTLAYOUT_P_H
