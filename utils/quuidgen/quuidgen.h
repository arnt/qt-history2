#ifndef QUUIDGEN_H
#define QUUIDGEN_H

#include "quuidbase.h"

class QUuidGen : public QUuidBase
{
    Q_OBJECT

public:
    QUuidGen();

protected:
    void newUuid();
    void copyUuid();
    void formatChanged();

private:
    QString result;
};

#endif //QUUIDGEN_H
