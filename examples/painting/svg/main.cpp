#include "qsvgview.h"

#include <QScrollArea>
#include <QApplication>
#include <QKeyEvent>
#include <QtDebug>

class SvgWindow : public QScrollArea
{
public:
    SvgWindow(const char *file, const char *type)
    {
        QWidget *view = 0;

        if (type == QLatin1String("-gl")) {
            view = new QSvgGLView(file, this);
        } else if (type == QLatin1String("-native")) {
            view = new QSvgNativeView(file, this);
        } else
            view = new QSvgRasterView(file, this);
        setWidget(view);
    }

    void keyPressEvent(QKeyEvent *e)
    {
        if (e->key() == Qt::Key_Escape) {
            showNormal();
        } else if (e->key() == Qt::Key_F) {
            showFullScreen();
        }
    }
};

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    if (argc < 2) {
        qWarning()<<argv[0]<<" file <renderer>";
        qWarning()<<"Please specify a SVG file to load.\n"
                  <<"Available rendereres '-gl', '-native' and '-image'";
        return 1;
    }

    SvgWindow *area = new SvgWindow(argv[1], argv[2]);
    area->show();
    return app.exec();
}
