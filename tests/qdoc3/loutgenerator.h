/*
  loutgenerator.h
*/

#ifndef LOUTGENERATOR_H
#define LOUTGENERATOR_H

#include "bookgenerator.h"

class LoutGenerator : public BookGenerator
{
public:
    LoutGenerator();
    ~LoutGenerator();

    virtual QString format();

protected:
    // ###
};

#endif
