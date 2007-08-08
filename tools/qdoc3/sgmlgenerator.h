/*
  sgmlgenerator.h
*/

#ifndef SGMLGENERATOR_H
#define SGMLGENERATOR_H

#include "bookgenerator.h"

class SgmlGenerator : public BookGenerator
{
public:
    SgmlGenerator();
    ~SgmlGenerator();

    virtual QString format();

protected:
    // ###
};

#endif
