#ifndef APIGENERATOR_H
#define APIGENERATOR_H

#include <QTextStream>

#include "generator.h"

class ApiGenerator : public Generator
{
public:
    QString format();
    void generateTree(const Tree *tree, CodeMarker *marker);

private:
    void generateNode(const Node *node, CodeMarker *marker, int indent = 0);

    QTextStream out;
};

#endif
