#include "qtextfloat.h"
#include "qtextformat_p.h"
#include "qtextblockiterator.h"

class QTextFloatPrivate : public QTextGroupPrivate
{
};

QTextFloat::QTextFloat(QObject *parent)
    : QTextGroup(*new QTextFloatPrivate, parent)
{
}

QTextFloat::~QTextFloat()
{
}
