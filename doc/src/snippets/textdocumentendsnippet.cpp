#include <QtGui>
#include <iostream.h>

int main(int argv, char **args)
{
    QString contentString("One\nTwp\nThree");

    QTextDocument *doc = new QTextDocument(contentString);

    for (QTextBlock it = doc->begin(); it != doc->end(); it = it.next())
        cout << it.text().toStdString() << endl;

    return 0;
}
