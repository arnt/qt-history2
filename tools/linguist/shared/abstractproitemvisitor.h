#ifndef ABSTRACTPROITEMVISITOR
#define ABSTRACTPROITEMVISITOR

#include "proitems.h"

struct AbstractProItemVisitor {
    virtual ~AbstractProItemVisitor() {}
    virtual bool visitBeginProBlock(ProBlock *block) = 0;
    virtual bool visitEndProBlock(ProBlock *block) = 0;

    virtual bool visitBeginProVariable(ProVariable *variable) = 0;
    virtual bool visitEndProVariable(ProVariable *variable) = 0;

    virtual bool visitBeginProFile(ProFile *value) = 0;
    virtual bool visitEndProFile(ProFile *value) = 0;

    virtual bool visitProValue(ProValue *value) = 0;
    virtual bool visitProFunction(ProFunction *function) = 0;
    virtual bool visitProOperator(ProOperator *function) = 0;
    virtual bool visitProCondition(ProCondition *function) = 0;

};

#endif // ABSTRACTPROITEMVISITOR

