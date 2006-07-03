#include <QApplication>

#include "datagenerator.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    DataGenerator tester;

    tester.run(argc, argv);

    return 0;
}
