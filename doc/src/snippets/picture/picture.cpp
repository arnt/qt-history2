#include <QtGui/QtGui>

void myProcessing(const QString &)
{
}

int main()
{
    QWidget myWidget;
    {
        // RECORD
        QPicture picture;
        QPainter painter;
        painter.begin(&picture);           // paint in picture
        painter.drawEllipse(10,20, 80,70); // draw an ellipse
        painter.end();                     // painting done
        picture.save("drawing.pic");       // save picture
    }

    {
        // REPLAY
        QPicture picture;
        picture.load("drawing.pic");           // load picture
        QPainter painter;
        painter.begin(&myWidget);              // paint in myWidget
        painter.drawPicture(0, 0, picture);    // draw the picture at (0,0)
        painter.end();                         // painting done
    }

    QPicture myPicture;
    {
        // FORMATS
        QStringList list = myPicture.inputFormatList();
        foreach (QString string, list)
            myProcessing(string);
    }

    {
        // OUTPUT
        QStringList list = myPicture.outputFormatList();
        foreach (QString string, list)
            myProcessing(string);
    }

    {
        // PIC READ
        QPictureIO iio;
        QPixmap  pixmap;
        iio.setFileName("vegeburger.bmp");
        if (iio.read()) {        // OK
            QPicture picture = iio.picture();
            QPainter painter(&pixmap);
            painter.drawPicture(0, 0, picture);
        }
    }

    {
        QPixmap pixmap;
        // PIC WRITE
        QPictureIO iio;
        QPicture   picture;
        QPainter painter(&picture);
        painter.drawPixmap(0, 0, pixmap);
        iio.setPicture(picture);
        iio.setFileName("vegeburger.bmp");
        iio.setFormat("BMP");
        if (iio.write())
            return true; // returned true if written successfully
    }

}

// SVG READ
void readSVG(QPictureIO *picture)
{
    // read the picture using the picture->ioDevice()
}

// SVG WRITE
void writeSVG(QPictureIO *picture)
{
    // write the picture using the picture->ioDevice()
}

// USE SVG
void foo() {

    // add the SVG picture handler
    // ...
    QPictureIO::defineIOHandler("SVG", 0, 0, readSVG, writeSVG);
    // ...

}
