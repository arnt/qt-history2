#include <QApplication>
#include <QCopChannel>
#include <QDirectPainter>
#include <QImage>
#include <QScreen>
#include <QPainter>

int main(int argc, char** argv)
{
#ifdef Q_WS_QWS
    QApplication app(argc, argv);
    QDirectPainter dp;
    QImage screenImage(dp.frameBuffer(), dp.screenWidth(), dp.screenHeight(),
                       dp.linestep(), QScreen::instance()->pixelFormat());
    QPainter p;

    QStringList args = app.arguments();

    dp.setRegion(QRect(args.at(1).toInt(),
                       args.at(2).toInt(),
                       args.at(3).toInt(),
                       args.at(4).toInt()));

    for (;;) {
        dp.startPainting();
        p.begin(&screenImage);
        p.setPen(Qt::NoPen);
        p.setBrush(Qt::blue);
        p.drawRects(dp.allocatedRegion().rects());
        p.end();
        dp.endPainting(dp.allocatedRegion());
        app.processEvents();
    }
#else
    Q_UNUSED(argc);
    Q_UNUSED(argv);
#endif

    return 0;
}

