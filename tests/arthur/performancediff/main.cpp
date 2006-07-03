#include <QApplication>

#include "performancediff.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    PerformanceDiff generator;

    generator.run(argc, argv);

    return 0;
}
