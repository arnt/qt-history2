/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

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
