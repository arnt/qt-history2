#include "qtextfloat.h"
#include "qtextformat_p.h"
#include "qtextblockiterator.h"

class QTextFloatPrivate : public QTextFormatGroupPrivate
{
};

QTextFloat::QTextFloat(QObject *parent)
    : QTextFormatGroup(*new QTextFloatPrivate, parent)
{
}

QTextFloat::~QTextFloat()
{
}
