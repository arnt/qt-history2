#ifndef QTEXTFORMAT_P_H
#define QTEXTFORMAT_P_H

#include "qtextformat.h"
#include <qvector.h>

class QTextFormatCollection
{
public:
    int createReferenceIndex(const QTextFormat &newFormat);
    void updateReferenceIndex(int index, const QTextFormat &newFormat);

    int indexForFormat(const QTextFormat &f) const;

    QTextFormat format(int idx, int defaultFormatType = -1) const;

    QTextBlockFormat blockFormat(int index) const
    { return format(index, QTextFormat::BlockFormat).toBlockFormat(); }
    QTextCharFormat charFormat(int index) const
    { return format(index, QTextFormat::CharFormat).toCharFormat(); }
    QTextListFormat listFormat(int index) const
    { return format(index, QTextFormat::ListFormat).toListFormat(); }
    QTextTableFormat tableFormat(int index) const
    { return format(index, QTextFormat::TableFormat).toTableFormat(); }
    QTextImageFormat imageFormat(int index) const
    { return format(index, QTextFormat::ImageFormat).toImageFormat(); }

    int numFormats() const { return formats.count(); }

private:
    int indexToReference(int idx) const;
    int referenceToIndex(int ref) const;

    mutable QVector<QTextFormat> formats;
    QVector<int> formatReferences;
};

#endif // QTEXTFORMAT_P_H
