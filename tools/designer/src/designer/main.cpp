#include <qdebug.h>
#include "mainwindow.h"
#include "designerapp.h"

extern int qInitResources_formeditor();
extern int qInitResources_widgetbox();

int main(int argc, char *argv[])
{
    bool bSendOpenRequest = false;
    int port = 0;
    QStringList filenames;
    DesignerApplication app(argc, argv);

    if (argc != 1) {
        int currentArg = 1;
        QString opt = QString::fromLocal8Bit(argv[currentArg]).toLower();
        if (opt == QString("-server"))
        {
            DesignerServer *server = new DesignerServer(&app);
            if (server->serverPort() == 0) {
                fprintf(stderr, "Could not start server");
                fflush(stderr);
                return 1;
            }
            printf("%d\n", server->serverPort());
            qDebug("%d\n", server->serverPort());
            fflush(stdout);
            ++currentArg;
        }
        else if (opt == QString("-port"))
        {
            ++currentArg;
            if (currentArg >= argc) {
                fprintf(stderr, "Wrong number of arguments");
                fflush(stderr);
                return 1;
            }
            opt = QString::fromLocal8Bit(argv[currentArg]).toLower();
            bool cOk;
            port = opt.toUInt(&cOk);
            if (!cOk) {
                fprintf(stderr, "Not a valid port number");
                fflush(stderr);
                return 1;
            }
            bSendOpenRequest = true;
            ++currentArg;
        }

        // Assume that everything else there is now a file.
        for (; currentArg < argc; ++currentArg)
            filenames.append(QString::fromLocal8Bit(argv[currentArg]));
    }

    if (bSendOpenRequest) {
        DesignerServer::sendOpenRequest(port, filenames);
        return 0; //done
    }

    qInitResources_formeditor();
    qInitResources_widgetbox();

    MainWindow mw;
    app.setMainWindow(&mw);
    QObject::connect(&app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()));
    mw.show();
    if (filenames.count() > 0) {
        foreach(QString filename, filenames) {
            mw.readInForm(filename);
        }
    }/* else {
        mw.newForm();
    }*/

    return app.exec();
}
