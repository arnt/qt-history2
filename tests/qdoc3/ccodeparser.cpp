/*
  ccodeparser.cpp
*/

#include "ccodeparser.h"

CCodeParser::CCodeParser()
{
}

CCodeParser::~CCodeParser()
{
}

QString CCodeParser::language()
{
    return "C";
}

QString CCodeParser::headerFileNameFilter()
{
    return "*.ch *.h";
}

QString CCodeParser::sourceFileNameFilter()
{
    return "*.c";
}
