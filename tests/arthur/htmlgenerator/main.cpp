#include <QApplication>

#include "htmlgenerator.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    HTMLGenerator generator;

    generator.run(argc, argv);

    return 0;
}
