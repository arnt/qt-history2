#ifndef QTEXTLAYOUTER_H
#define QTEXTLAYOUTER_H

#ifndef QT_H
#include <qabstracttextdocumentlayout.h>
#include "private/qtextpiecetable_p.h"
#endif // QT_H

class QTextListFormat;

class QTextDocumentLayout : public QAbstractTextDocumentLayout
{
public:
    QTextDocumentLayout(QTextPieceTable *parent);

    // from the abstract layout
    void draw(QPainter *painter, const PaintContext &context);

    void documentChange(int from, int oldLength, int length);

    int hitTest(const QPoint &point, QText::HitTestAccuracy accuracy) const;


    inline QRect pageRect(int /*page*/) const
	{ QSize ps = pageSize(); return QRect(0, 0, ps.width(), totalHeight()); } // ####
    int numPages() const { return 1; } // #####

    // ### remove me
    int totalHeight() const;

    void setPageSize(const QSize &size) { QAbstractTextDocumentLayout::setPageSize(size); recreateAllBlocks(); }

private:
    void recreateAllBlocks();

    void drawBlock(QPainter *, const PaintContext &context, const QTextPieceTable::BlockIterator block);
    void drawListItem(QPainter *, const PaintContext &context,
		      const QTextPieceTable::BlockIterator block, const QTextLayout::Selection &selection);

    int indent(const QTextPieceTable::BlockIterator block) const;
    QTextListFormat listFormat(const QTextPieceTable::BlockIterator block) const;

    void layoutBlock(const QTextPieceTable::BlockIterator block, const QPoint &p, int width);
    QTextPieceTable::BlockIterator layoutCell(QTextPieceTable::BlockIterator block, QPoint *pos, int width);
    QTextPieceTable::BlockIterator layoutTable(QTextPieceTable::BlockIterator block, QPoint *pos, int width);

    int hitTest(QTextPieceTable::BlockIterator bl, const QPoint &point, QText::HitTestAccuracy accuracy) const;
    inline QTextPieceTable *pieceTable() const { return static_cast<QTextPieceTable *>(parent()); }
};

#endif // QTEXTLAYOUTER_H
