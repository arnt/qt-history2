#ifndef GLWORKSPACE_H
#define GLWORKSPACE_H

#include <q3mainwindow.h>

class QWorkspace;
class QPrinter;

class GLWorkspace : public Q3MainWindow
{
    Q_OBJECT
public:
    GLWorkspace( QWidget *parent = 0, const char *name = 0, Qt::WFlags f = 0 );
    ~GLWorkspace();

protected:
    void setupSceneActions();

protected slots:
    void newWirebox();
    void newGear();
    void newTexture();
    void newNurbs();
    void filePrint( int x, int y );
    void filePrintWindowRes();
    void filePrintLowRes();
    void filePrintMedRes();
    void filePrintHighRes();
    bool filePrintSetup();
    void fileClose();

private:
    QWorkspace *workspace;
    QPrinter *printer;
};

#endif //GLWORKSPACE_H
