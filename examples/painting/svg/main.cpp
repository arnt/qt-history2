#include "qsvgview.h"

#include <QScrollArea>
#include <QApplication>
#include <QKeyEvent>
#include <QtDebug>

class SvgWindow : public QScrollArea
{
public:
    SvgWindow(const QString &file, const QString &type)
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

static void usage(const char *prog)
{
    qWarning()<<"Usage:"<<prog<<" <renderer> file";
    qWarning()<<"Please specify a SVG file to load.";
    qWarning()<<"Available rendereres: '-gl', '-native' and '-image'";
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    if (argc < 2) {
        usage(argv[0]);
        return 1;
    }

    QString file, type;
    if (argc == 3) {
        QString dummy = argv[1];
        if (dummy.startsWith('-')) {
            type = argv[1];
            file = argv[2];
        } else {
            file = argv[1];
            type = argv[2];
        }
    } else {
        file = argv[1];
        if (file.startsWith('-')) {
            usage(argv[0]);
            return 1;
        }
    }

    SvgWindow *area = new SvgWindow(file, type);
    area->show();
    return app.exec();
}
