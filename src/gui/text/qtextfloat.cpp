#include "qtextfloat.h"
#include "qtextformat_p.h"
#include "qtextblockiterator.h"

class QTextFloatPrivate : public QTextFramePrivate
{
};

QTextFloat::QTextFloat(QObject *parent)
    : QTextFrame(*new QTextFloatPrivate, parent)
{
}

QTextFloat::~QTextFloat()
{
}
