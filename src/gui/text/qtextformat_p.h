#ifndef QTEXTFORMAT_P_H
#define QTEXTFORMAT_P_H

#include "qtextformat.h"
#include <qvector.h>

class Q_GUI_EXPORT QTextFormatCollection
{
public:
    int createReferenceIndex(const QTextFormat &newFormat);
    QTextFormat updateReferenceIndex(int index, const QTextFormat &newFormat);

    int indexForFormat(const QTextFormat &f);
    bool hasFormatCached(const QTextFormat &format) const;

    QTextFormat format(int idx, int defaultFormatType = -1) const;

    inline QTextBlockFormat blockFormat(int index) const
    { return format(index, QTextFormat::BlockFormat).toBlockFormat(); }
    inline QTextCharFormat charFormat(int index) const
    { return format(index, QTextFormat::CharFormat).toCharFormat(); }
    inline QTextListFormat listFormat(int index) const
    { return format(index, QTextFormat::ListFormat).toListFormat(); }
    inline QTextTableFormat tableFormat(int index) const
    { return format(index, QTextFormat::TableFormat).toTableFormat(); }
    inline QTextImageFormat imageFormat(int index) const
    { return format(index, QTextFormat::ImageFormat).toImageFormat(); }

    inline int numFormats() const { return formats.count(); }

private:
    int indexToReference(int idx) const;
    int referenceToIndex(int ref) const;

    mutable QVector<QTextFormat> formats;
    QVector<int> formatReferences;
};

#endif // QTEXTFORMAT_P_H
