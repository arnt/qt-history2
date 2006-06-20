#include <QAbstractTextDocumentLayout>

class QTestDocumentLayout : public QAbstractTextDocumentLayout
{
public:
    QTestDocumentLayout(QTextDocument *doc) : QAbstractTextDocumentLayout(doc), f(-1), called(false) {}
    virtual void draw(QPainter *, const PaintContext &)  {}
    virtual int hitTest(const QPointF &, Qt::HitTestAccuracy ) const { return 0; }

    virtual void documentChanged(int from, int oldLength, int length);

    virtual int pageCount() const { return 1; }
    virtual QSizeF documentSize() const { return QSizeF(); }

    virtual QRectF frameBoundingRect(QTextFrame *) const { return QRectF(); }
    virtual QRectF blockBoundingRect(const QTextBlock &) const { return QRectF(); }

    int f;
    int o;
    int l;

    void expect(int from, int oldLength, int length) {
	f = from;
	o = oldLength;
	l = length;
	error = false;
	called = false;
    }
    bool error;
    bool called;
};

void QTestDocumentLayout::documentChanged(int from, int oldLength, int length)
{
    called = true;

    if (f < 0)
	return;

    if(from != f ||
       o != oldLength ||
       l != length) {
	qDebug("checkDocumentChanged: got %d %d %d, expected %d %d %d", from, oldLength, length, f, o, l);
	error = true;
    }
}


