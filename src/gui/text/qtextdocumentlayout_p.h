#ifndef QTEXTDOCUMENTLAYOUT_P_H
#define QTEXTDOCUMENTLAYOUT_P_H

#ifndef QT_H
#include <qabstracttextdocumentlayout.h>
#endif // QT_H

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
    int hitTest(const QPoint &point, QText::HitTestAccuracy accuracy) const;

    void setSize(QTextInlineObject item, const QTextFormat &format);
    void layoutObject(QTextInlineObject item, const QTextFormat &format);
    void drawObject(QPainter *p, const QRect &rect, QTextInlineObject item,
                    const QTextFormat &format, QTextLayout::SelectionType selType);

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

signals:
    void usedSizeChanged();
};

#endif // QTEXTDOCUMENTLAYOUT_P_H
