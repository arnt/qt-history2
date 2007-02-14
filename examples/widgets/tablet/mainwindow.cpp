#include <QtGui>

#include "mainwindow.h"
#include "tabletcanvas.h"

MainWindow::MainWindow(TabletCanvas *canvas) 
{
    myCanvas = canvas;
    createActions();	
    createMenus();

    myCanvas->setColor(Qt::red);
    myCanvas->setLineWidthType(TabletCanvas::LineWidthPressure);
    myCanvas->setAlphaChannelType(TabletCanvas::NoAlpha);
    myCanvas->setColorSaturationType(TabletCanvas::NoSaturation);

    setWindowTitle(tr("Tablet Example"));
    setCentralWidget(myCanvas);
}

void MainWindow::brushColorAct()
{
    QColor color = QColorDialog::getColor(myCanvas->color());
    
    if (color.isValid())
	myCanvas->setColor(color);
}

void MainWindow::alphaActionTriggered(QAction *action)
{
    if (action == alphaChannelPressureAction) {
	myCanvas->setAlphaChannelType(TabletCanvas::AlphaPressure);	
    } else if (action == alphaChannelTiltAction) {
	myCanvas->setAlphaChannelType(TabletCanvas::AlphaTilt);
    } else {
	myCanvas->setAlphaChannelType(TabletCanvas::NoAlpha);
    }
}

void MainWindow::lineWidthActionTriggered(QAction *action)
{
    if (action == lineWidthPressureAction) {
	myCanvas->setLineWidthType(TabletCanvas::LineWidthPressure);
    } else if (action == lineWidthTiltAction) {
	myCanvas->setLineWidthType(TabletCanvas::LineWidthTilt);
    } else {
	myCanvas->setLineWidthType(TabletCanvas::NoLineWidth);
    }
}

void MainWindow::saturationActionTriggered(QAction *action)
{
    if (action == colorSaturationVTiltAction) {
	myCanvas->setColorSaturationType(TabletCanvas::SaturationVTilt);
    } else if (action == colorSaturationHTiltAction) {
	myCanvas->setColorSaturationType(TabletCanvas::SaturationHTilt);
    } else if (action == colorSaturationPressureAction) {
	myCanvas->setColorSaturationType(TabletCanvas::SaturationPressure);
    } else {
	myCanvas->setColorSaturationType(TabletCanvas::NoSaturation);
    } 
}

void MainWindow::saveAct()
{
    QString path = QDir::currentPath() + "/untitled.png";
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Picture"), 
						     path);
    
    if (!myCanvas->saveImage(fileName))
	QMessageBox::information(this, "Error Saving Picture", 
				 "Could not save the image");  
}

void MainWindow::loadAct()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Picture"),
						    QDir::currentPath());

    if (!myCanvas->loadImage(fileName))
	QMessageBox::information(this, "Error Opening Picture",
				 "Could not open picture");
}

void MainWindow::aboutAct()
{
    QMessageBox::about(this, tr("About Tablet Example"),
		       tr("This example shows use of a Wacom tablet in Qt"));
}

void MainWindow::createActions()
{
    brushColorAction = new QAction(tr("&Brush Color..."), this);
    brushColorAction->setShortcut(tr("Ctrl+C"));
    connect(brushColorAction, SIGNAL(triggered()),
	    this, SLOT(brushColorAct()));

    alphaChannelPressureAction = new QAction(tr("&Pressure"), this);
    alphaChannelPressureAction->setCheckable(true);

    alphaChannelTiltAction = new QAction(tr("&Tilt"), this);
    alphaChannelTiltAction->setCheckable(true);

    noAlphaChannelAction = new QAction(tr("No Alpha Channel"), this);
    noAlphaChannelAction->setCheckable(true);
    noAlphaChannelAction->setChecked(true);

    alphaChannelGroup = new QActionGroup(this);
    alphaChannelGroup->addAction(alphaChannelPressureAction);
    alphaChannelGroup->addAction(alphaChannelTiltAction);
    alphaChannelGroup->addAction(noAlphaChannelAction);
    connect(alphaChannelGroup, SIGNAL(triggered(QAction *)),
	    this, SLOT(alphaActionTriggered(QAction *)));    

    colorSaturationVTiltAction = new QAction(tr("&Vertical Tilt"), this);
    colorSaturationVTiltAction->setCheckable(true);

    colorSaturationHTiltAction = new QAction(tr("&Horizontal Tilt"), this);
    colorSaturationHTiltAction->setCheckable(true);

    colorSaturationPressureAction = new QAction(tr("&Pressure"), this);
    colorSaturationPressureAction->setCheckable(true);

    noColorSaturationAction = new QAction(tr("&No Color Saturation"), this);
    noColorSaturationAction->setCheckable(true);
    noColorSaturationAction->setChecked(true);

    colorSaturationGroup = new QActionGroup(this);
    colorSaturationGroup->addAction(colorSaturationVTiltAction);
    colorSaturationGroup->addAction(colorSaturationHTiltAction);
    colorSaturationGroup->addAction(colorSaturationPressureAction);
    colorSaturationGroup->addAction(noColorSaturationAction);
    connect(colorSaturationGroup, SIGNAL(triggered(QAction *)),
	    this, SLOT(saturationActionTriggered(QAction *)));

    lineWidthPressureAction = new QAction(tr("&Pressure"), this);
    lineWidthPressureAction->setCheckable(true);
    lineWidthPressureAction->setChecked(true);

    lineWidthTiltAction = new QAction(tr("&Tilt"), this);
    lineWidthTiltAction->setCheckable(true);

    lineWidthFixedAction = new QAction(tr("&Fixed"), this);
    lineWidthFixedAction->setCheckable(true);

    lineWidthGroup = new QActionGroup(this);
    lineWidthGroup->addAction(lineWidthPressureAction);
    lineWidthGroup->addAction(lineWidthTiltAction);
    lineWidthGroup->addAction(lineWidthFixedAction);
    connect(lineWidthGroup, SIGNAL(triggered(QAction *)),
	    this, SLOT(lineWidthActionTriggered(QAction *)));    

    exitAction = new QAction(tr("E&xit"), this);
    exitAction->setShortcut(tr("Ctrl+X"));
    connect(exitAction, SIGNAL(triggered()),
	    this, SLOT(close()));

    loadAction = new QAction(tr("&Open..."), this);
    loadAction->setShortcut(tr("Ctrl+O"));
    connect(loadAction, SIGNAL(triggered()),
	    this, SLOT(loadAct()));

    saveAction = new QAction(tr("&Save As..."), this);
    saveAction->setShortcut(tr("Ctrl+S"));
    connect(saveAction, SIGNAL(triggered()),
	    this, SLOT(saveAct()));

    aboutAction = new QAction(tr("A&bout"), this);  
    aboutAction->setShortcut(tr("Ctrl+B"));
    connect(aboutAction, SIGNAL(triggered()),
	    this, SLOT(aboutAct()));
    
    aboutQtAction = new QAction(tr("About &Qt"), this);
    aboutQtAction->setShortcut(tr("Ctrl+Q"));
    connect(aboutQtAction, SIGNAL(triggered()),
	    qApp, SLOT(aboutQt()));
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(loadAction);
    fileMenu->addAction(saveAction);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAction);
    
    brushMenu = menuBar()->addMenu(tr("&Brush"));
    brushMenu->addAction(brushColorAction);

    tabletMenu = menuBar()->addMenu(tr("&Tablet"));

    lineWidthMenu = tabletMenu->addMenu(tr("&Line Width"));
    lineWidthMenu->addAction(lineWidthPressureAction);
    lineWidthMenu->addAction(lineWidthTiltAction);
    lineWidthMenu->addAction(lineWidthFixedAction);    

    alphaChannelMenu = tabletMenu->addMenu(tr("&Alpha Channel"));
    alphaChannelMenu->addAction(alphaChannelPressureAction);
    alphaChannelMenu->addAction(alphaChannelTiltAction);
    alphaChannelMenu->addAction(noAlphaChannelAction);

    colorSaturationMenu = tabletMenu->addMenu(tr("&Color Saturation"));
    colorSaturationMenu->addAction(colorSaturationVTiltAction);
    colorSaturationMenu->addAction(colorSaturationHTiltAction);
    colorSaturationMenu->addAction(noColorSaturationAction);

    helpMenu = menuBar()->addMenu("&Help"); 
    helpMenu->addAction(aboutAction);    
    helpMenu->addAction(aboutQtAction);
}
