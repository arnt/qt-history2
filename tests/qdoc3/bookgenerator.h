/*
  bookgenerator.h
*/

#ifndef BOOKGENERATOR_H
#define BOOKGENERATOR_H

#include "generator.h"

class BookGenerator : public Generator
{
public:
    BookGenerator() { }
    ~BookGenerator() { }

    virtual void generateTree( const Tree *tree, const CodeMarker *marker );

protected:
    // ###
};

#endif
