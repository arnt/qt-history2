#ifndef QTEXTLAYOUTER_H
#define QTEXTLAYOUTER_H

#ifndef QT_H
#include <qabstracttextdocumentlayout.h>
#endif // QT_H

class QTextListFormat;

class QTextDocumentLayoutPrivate;

class QTextDocumentLayout : public QAbstractTextDocumentLayout
{
    Q_DECLARE_PRIVATE(QTextDocumentLayout);
public:
    QTextDocumentLayout();

    // from the abstract layout
    void draw(QPainter *painter, const PaintContext &context);
    void documentChange(int from, int oldLength, int length);
    int hitTest(const QPoint &point, QText::HitTestAccuracy accuracy) const;

    int numPages() const;

    // ### remove me
    int totalHeight() const;

    virtual void setPageSize(const QSize &size);
    virtual QSize pageSize() const;
};

#endif // QTEXTLAYOUTER_H
