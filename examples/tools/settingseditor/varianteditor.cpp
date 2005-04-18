#include "varianteditor.h"

/*
        Invalid = 0,
        Map = 1,
        List = 2,
        String = 3,
        StringList = 4,
        Font = 5,
        Pixmap = 6,
        Brush = 7,
        Rect = 8,
        Size = 9,
        Color = 10,
        Palette = 11,
        Icon = 13,
        Point = 14,
        Image = 15,
        Int = 16,
        UInt = 17,
        Bool = 18,
        Double = 19,
        Polygon = 21,
        Region = 22,
        Bitmap = 23,
        Cursor = 24,
        SizePolicy = 25,
        Date = 26,
        Time = 27,
        DateTime = 28,
        ByteArray = 29,
        BitArray = 30,
        KeySequence = 31,
        Pen = 32,
        LongLong = 33,
        ULongLong = 34,
        Char = 35,
        Url = 36,
        TextLength = 37,
        TextFormat = 38,
        Locale = 39,
        LineF = 40,
        RectF = 41,
        PointF = 42,
        Line = 43,
*/

VariantEditor::VariantEditor(QWidget *parent)
    : QDialog(parent)
{
}

void VariantEditor::setVariant(const QVariant &value)
{
    this->value = value;
    
}
