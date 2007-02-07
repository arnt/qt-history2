#include <QtGui>

int main(int argv, char **args)
{
	QImageWriter writer;
	writer.setFormat("png");
	if (writer.supportsOption(QImageIOHandler::Description))
	    qDebug() << "Png supports embedded text";
    return 0;
}
