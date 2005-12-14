#include "QtGui/qmatrix.h"
#include "QtCore/qstring.h"
#include "QtCore/qvector.h"
#include "private/qstroker_p.h"
#include "private/qfontengine_p.h"
#include "QtGui/qprinter.h"

const char *qt_real_to_string(qreal val, char *buf);
const char *qt_int_to_string(int val, char *buf);

namespace QPdf {

    class ByteStream
    {
    public:
        ByteStream(QByteArray *b) :ba(b) {}
        ByteStream &operator <<(char chr) { *ba += chr; return *this; }
        ByteStream &operator <<(const char *str) { *ba += str; return *this; }
        ByteStream &operator <<(const QByteArray &str) { *ba += str; return *this; }
        ByteStream &operator <<(qreal val) { char buf[256]; *ba += qt_real_to_string(val, buf); return *this; }
        ByteStream &operator <<(int val) { char buf[256]; *ba += qt_int_to_string(val, buf); return *this; }
        ByteStream &operator <<(const QPointF &p) { char buf[256]; *ba += qt_real_to_string(p.x(), buf);
            *ba += qt_real_to_string(p.y(), buf); return *this; }
    private:
        QByteArray *ba;
    };

    enum PathFlags {
        ClipPath,
        FillPath,
        StrokePath,
        FillAndStrokePath
    };
    QByteArray generatePath(const QPainterPath &path, const QMatrix &matrix, PathFlags flags);
    QByteArray generateMatrix(const QMatrix &matrix);
    QByteArray generateDashes(const QPen &pen);
    QByteArray patternForBrush(const QBrush &b);
#ifdef USE_NATIVE_GRADIENTS
    QByteArray generateLinearGradientShader(const QLinearGradient *lg, const QPointF *page_rect, bool alpha = false);
#endif

    struct Stroker {
        Stroker();
        void setPen(const QPen &pen);
        void strokePath(const QPainterPath &path);
        ByteStream *stream;
        bool first;
        QMatrix matrix;
        bool zeroWidth;
    private:
        QStroker basicStroker;
        QDashStroker dashStroker;
        QStrokerOps *stroker;
    };

    class Font
    {
    public:
        Font(QFontEngine *fe, int obj_id = 0)
            : object_id(obj_id), fontEngine(fe)
            { fontEngine->ref.ref(); addGlyph(0); }
        ~Font() {
            if (!fontEngine->ref.deref())
                delete fontEngine;
        }

        QByteArray toTruetype() const;
        QByteArray toType1() const;
        QByteArray widthArray() const;
        QByteArray createToUnicodeMap() const;
        QVector<int> getReverseMap() const;
        QByteArray glyphName(unsigned int glyph, const QVector<int> reverseMap) const;

        static QByteArray glyphName(unsigned short unicode, bool symbol);

        int addGlyph(int index) {
            int idx = glyph_indices.indexOf(index);
            if (idx < 0) {
                idx = glyph_indices.size();
                glyph_indices.append(index);
            }
            return idx;
        }
        const int object_id;
        QFontEngine *fontEngine;
        QList<int> glyph_indices;
        int nGlyphs() const { return glyph_indices.size(); }
        mutable QFixed emSquare;
        mutable QVector<QFixed> widths;
    };

    QByteArray ascii85Encode(const QByteArray &input);

    const char *toHex(ushort u, char *buffer);
    const char *toHex(uchar u, char *buffer);    


    struct PaperSize {
        int width, height;
    };
    PaperSize paperSize(QPrinter::PageSize pageSize);
    const char *paperSizeToString(QPrinter::PageSize pageSize);
};
