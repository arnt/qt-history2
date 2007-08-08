/*
  ccodeparser.h
*/

#ifndef CCODEPARSER_H
#define CCODEPARSER_H

#include "cppcodeparser.h"

class CCodeParser : public CppCodeParser
{
public:
    CCodeParser();
    ~CCodeParser();

    virtual QString language();
    virtual QString headerFileNameFilter();
    virtual QString sourceFileNameFilter();
};

#endif
