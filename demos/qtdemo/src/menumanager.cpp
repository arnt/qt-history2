/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "menumanager.h"
#include "colors.h"
#include "menucontent.h"
#include "examplecontent.h"

MenuManager *MenuManager::pInstance = 0;

MenuManager * MenuManager::instance()
{
    if (!MenuManager::pInstance)
        MenuManager::pInstance = new MenuManager();
    return MenuManager::pInstance;
}

MenuManager::MenuManager()
{
    this->ticker = 0;
    this->assistant = new QAssistantClient(QLibraryInfo::location(QLibraryInfo::BinariesPath), this);
    this->score = new Score();
    this->currentMenu = "-no menu";
    this->currentExample = "-no example";
    this->readXmlDocument();
}

MenuManager::~MenuManager()
{
    delete this->score;
    delete this->contentsDoc;
}

void MenuManager::readXmlDocument()
{
    this->contentsDoc = new QDomDocument();
    QString errorStr;
    int errorLine;
    int errorColumn;
    
    QFile file(":/xml/examples.xml");
    bool statusOK = this->contentsDoc->setContent(&file, true, &errorStr, &errorLine, &errorColumn);
    if (!statusOK){
        QMessageBox::critical(0,
                              QObject::tr("DOM Parser"),
                              QObject::tr("Could not read or find the contents document. Error at line %1, column %2:\n%3")
                              .arg(errorLine).arg(errorColumn).arg(errorStr)
                              );
        exit(-1);
    }
}

void MenuManager::itemSelected(int userCode, const QString &menuName)
{
    if (userCode == LAUNCH)
        this->launchExample(this->currentExample);
    else if (userCode == DOCUMENTATION)
            this->assistant->showPage(info[this->currentExample]["docfile"]);
    else if (userCode == QUIT){
            this->window->loop = false;
            QCoreApplication::quit();
    } else if (userCode == FULLSCREEN)
            this->window->toggleFullscreen();
    else {
        if (userCode == ROOT){
            this->score->queueMovie(this->currentMenu + " -out", Score::FROM_START, Score::LOCK_ITEMS);
            this->score->queueMovie(this->currentExample + " -out");
            this->score->queueMovie(this->currentExample + " -buttons -out");
            this->score->queueMovie("Qt Examples and Demos", Score::FROM_START, Score::UNLOCK_ITEMS);
            this->score->queueMovie("Qt Examples and Demos -info");
            if (!Colors::noTicker)
                this->ticker->useGuideQt();
            this->score->queueMovie("ticker", Score::NEW_ANIMATION_ONLY);
            this->window->switchTimerOnOff(true);
        } else if (userCode == MENU1){
            this->currentMenu = menuName;
            this->currentExample = menuName + " -info";
            this->score->queueMovie("Qt Examples and Demos -out", Score::FROM_START, Score::LOCK_ITEMS);
            this->score->queueMovie("Qt Examples and Demos -info -out");
            this->score->queueMovie(this->currentMenu, Score::FROM_START, Score::UNLOCK_ITEMS);
            this->score->queueMovie(this->currentExample);
            if (!Colors::noTicker)
                this->ticker->useGuideTt();
        } else if (userCode == MENU2){
            this->score->queueMovie(this->currentExample + " -out", Score::NEW_ANIMATION_ONLY);
            this->score->queueMovie(this->currentExample + " -buttons -out", Score::NEW_ANIMATION_ONLY);
            this->score->queueMovie("ticker -out", Score::NEW_ANIMATION_ONLY);        
            this->score->queueMovie(this->currentMenu + " -shake");
            this->score->queueMovie(menuName, Score::NEW_ANIMATION_ONLY);
            this->score->queueMovie(menuName + " -buttons", Score::NEW_ANIMATION_ONLY);
            this->currentExample = menuName;
            this->window->switchTimerOnOff(false);
        }
        this->score->playQue();
    }

    // Playing new movies might include
    // loading etc. So ignore the FPS
    // at this point
    this->window->fpsHistory.clear();
}

void MenuManager::launchExample(const QString &name)
{
    QString executable = this->info[name]["executable"];
    
    QProcess *process = new QProcess(this);
    connect(process, SIGNAL(finished(int)), this, SLOT(exampleFinished()));
    connect(process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(exampleError(QProcess::ProcessError)));
    
#ifdef Q_OS_WIN
    //make sure it finds the dlls on windows
    QString curpath = QString::fromLocal8Bit(qgetenv("PATH").constData());
    QString newpath = QString("PATH=%1;%2").arg(QLibraryInfo::location(QLibraryInfo::BinariesPath), curpath);
    process->setEnvironment(QStringList(newpath));
#endif
    
    //    process->setWorkingDirectory(info[name]["executable"]);
    process->start(executable);
}

void MenuManager::exampleFinished()
{
}

void MenuManager::exampleError(QProcess::ProcessError error)
{
    Q_UNUSED(error);
    QMessageBox::critical(0, tr("Failed to launch the example"),
                          tr("Could not launch the example. Ensure that it have been build."),
                          QMessageBox::Cancel);
}

void MenuManager::init(MainWindow *window)
{
    this->window = window;
    
    // Create ticker:
    this->createTicker();
    
    // Create first level menu:
    QDomElement rootElement = this->contentsDoc->documentElement();
    this->createLeftMenu1(rootElement);
    
    // Create second level menus:
    QDomNode level2MenuNode = rootElement.firstChild();
    while (!level2MenuNode.isNull()){
        QDomElement level2MenuElement = level2MenuNode.toElement();
        this->createRightMenu1(level2MenuElement);
        
        // create leaf menu and example info:
        QDomNode exampleNode = level2MenuElement.firstChild();
        while (!exampleNode.isNull()){
            QDomElement exampleElement = exampleNode.toElement();
            this->readInfoAboutExample(exampleElement);
            this->createLeafMenu(exampleElement);
            exampleNode = exampleNode.nextSibling();
        }
        
        level2MenuNode = level2MenuNode.nextSibling();
    }
}

void MenuManager::readInfoAboutExample(const QDomElement &example)
{
    QString name = example.attribute("name");
    this->info[name]["filename"] = example.attribute("filename");
    this->info[name]["executable"] = this->resolveExecutable(example);
    this->info[name]["docfile"] = this->resolveDocFile(example);
    this->info[name]["imgfile"] = this->resolveImgFile(example);
}

QString MenuManager::resolveExecutable(const QDomElement &example)
{
    QDomElement parent = example.parentNode().toElement();
    
    QDir dir;
    if (parent.tagName() == "demos")
        dir = QDir(QLibraryInfo::location(QLibraryInfo::DemosPath));
    else
        dir = QDir(QLibraryInfo::location(QLibraryInfo::ExamplesPath));
    
    QString fileName = example.attribute("filename");
    dir.cd(parent.attribute("dirname"));
    dir.cd(fileName);
    
    QFile unixFile(dir.path() + "/" + fileName);
    if (unixFile.exists()) return unixFile.fileName();
    QFile winR(dir.path() + "\\release\\" + fileName + ".exe");
    if (winR.exists()) return winR.fileName();
    QFile winD(dir.path() + "\\debug\\" + fileName + ".exe");
    if (winD.exists()) return winD.fileName();
    QFile mac(dir.path() + "/" + fileName + ".app");
    if (mac.exists()) return mac.fileName();
    
    return "QtDemo: Executable not found!";
}

QString MenuManager::resolveDocFile(const QDomElement &example)
{
    QDomElement parent = example.parentNode().toElement();
    QString docRootPath = QLibraryInfo::location(QLibraryInfo::DocumentationPath);
    if (parent.tagName() == "demos")
        return docRootPath + "/html/demos-" + example.attribute("filename") + ".html";
    else
        return docRootPath + "/html/" + parent.attribute("dirname") + "-" + example.attribute("filename") + ".html";
}

QString MenuManager::resolveImgFile(const QDomElement &example)
{
    QDomElement parent = example.parentNode().toElement();
    QString docRootPath = QLibraryInfo::location(QLibraryInfo::DocumentationPath);
    if (parent.tagName() == "demos")
        return docRootPath + "/html/images/" + example.attribute("filename") + "-demo.png";
    else
        return docRootPath + "/html/images/" + example.attribute("filename") + "-example.png";
}

void MenuManager::createLeftMenu1(const QDomElement &el)
{
    Movie *movie_in = new Movie();
    Movie *movie_out = new Movie();
    this->score->insertMovie(el.attribute("name"), movie_in);
    this->score->insertMovie(el.attribute("name")+ " -out", movie_out);
    
    qreal sw = this->window->scene->sceneRect().width();
    int xOffset = 15;
    int yOffset = 10;
    
    QDomNode n = el.firstChild();
    int i=0;
    while (!n.isNull()){
        QDomElement e = n.toElement();
        
        TextButton *item = new TextButton(e.attribute("name"), TextButton::LEFT, MENU1, this->window->scene, 0);
        item->setRecursiveVisible(false);
        item->setZValue(10);
        qreal ih = item->sceneBoundingRect().height();
        qreal iw = item->sceneBoundingRect().width();
        qreal ihp = ih + 3;
        
        // create in-animation:
        DemoItemAnimation *anim = new DemoItemAnimation(item, DemoItemAnimation::ANIM_IN);
        anim->setDuration(float(1000 + (i * 20)) * Colors::animSpeedButtons);
        anim->setStartPos(QPointF(xOffset, -ih));
        anim->setPosAt(0.20, QPointF(xOffset, -ih));
        anim->setPosAt(0.50, QPointF(xOffset, (i * ihp) + yOffset + Colors::contentStartY + (10 * float(i / 4.0f))));
        anim->setPosAt(0.60, QPointF(xOffset, (i * ihp) + yOffset + Colors::contentStartY));
        anim->setPosAt(0.70, QPointF(xOffset, (i * ihp) + yOffset + Colors::contentStartY + (5 * float(i / 4.0f))));
        anim->setPosAt(0.80, QPointF(xOffset, (i * ihp) + yOffset + Colors::contentStartY));
        anim->setPosAt(0.90, QPointF(xOffset, (i * ihp) + yOffset + Colors::contentStartY + (2 * float(i / 4.0f))));
        anim->setPosAt(1.00, QPointF(xOffset, (i * ihp) + yOffset + Colors::contentStartY));
        movie_in->append(anim);
        
        // create out-animation:
        anim = new DemoItemAnimation(item, DemoItemAnimation::ANIM_OUT);
        anim->hideOnFinished = true;
        anim->setDuration((700 + (30 * i)) * Colors::animSpeedButtons);
        anim->setStartPos(QPointF(xOffset, (i * ihp) + yOffset + Colors::contentStartY));
        anim->setPosAt(0.60, QPointF(xOffset, 600 - ih - ih));
        anim->setPosAt(0.65, QPointF(xOffset + 20, 600 - ih));
        anim->setPosAt(1.00, QPointF(sw + iw, 600 - ih));
        movie_out->append(anim);
        
        i++;
        n = n.nextSibling();
    }

    // create quit button:
    TextButton *backButton = new TextButton("Quit", TextButton::RIGHT, QUIT, this->window->scene, 0, TextButton::GREEN);
    backButton->setRecursiveVisible(false);
    backButton->setZValue(10);
    qreal iw = backButton->sceneBoundingRect().width();
    
    // create in-animation:
    DemoItemAnimation *BackButtonIn = new DemoItemAnimation(backButton, DemoItemAnimation::ANIM_IN);
    BackButtonIn->setDuration(1800 * Colors::animSpeedButtons);
    BackButtonIn->setStartPos(QPointF(-iw, Colors::contentStartY + Colors::contentHeight - 35));
    BackButtonIn->setPosAt(0.5, QPointF(-iw, Colors::contentStartY + Colors::contentHeight - 35));
    BackButtonIn->setPosAt(0.7, QPointF(xOffset, Colors::contentStartY + Colors::contentHeight - 35));
    BackButtonIn->setPosAt(1.0, QPointF(xOffset, Colors::contentStartY + Colors::contentHeight - 26));
    movie_in->append(BackButtonIn);
    
    // create out-animation:
    DemoItemAnimation *BackButtonOut = new DemoItemAnimation(backButton, DemoItemAnimation::ANIM_OUT);
    BackButtonOut->setDuration(400 * Colors::animSpeedButtons);
    BackButtonOut->setStartPos(QPointF(xOffset, Colors::contentStartY + Colors::contentHeight - 26));
    BackButtonOut->setPosAt(1.0, QPointF(-iw, Colors::contentStartY + Colors::contentHeight - 26));
    movie_out->append(BackButtonOut);

    // Create fullscreen button:
    xOffset = 70;
    TextButton *item = new TextButton("Toggle fullscreen", TextButton::RIGHT, FULLSCREEN, this->window->scene, 0, TextButton::GREEN);
    item->setRecursiveVisible(false);
    item->setZValue(10);
    
    // create in-animation:
    DemoItemAnimation *anim = new DemoItemAnimation(item, DemoItemAnimation::ANIM_IN);
    anim->setDuration(1800 * Colors::animSpeedButtons);
    anim->setStartPos(QPointF(sw, Colors::contentStartY + Colors::contentHeight - 35));
    anim->setPosAt(0.5, QPointF(sw, Colors::contentStartY + Colors::contentHeight - 35));
    anim->setPosAt(0.7, QPointF(xOffset + 535, Colors::contentStartY + Colors::contentHeight - 35));
    anim->setPosAt(1.0, QPointF(xOffset + 535, Colors::contentStartY + Colors::contentHeight - 26));
    movie_in->append(anim);
    
    // create out-animation:
    anim = new DemoItemAnimation(item, DemoItemAnimation::ANIM_OUT);
    anim->hideOnFinished = true;
    anim->setDuration(400 * Colors::animSpeedButtons);
    anim->setStartPos(QPointF(xOffset + 535, Colors::contentStartY + Colors::contentHeight - 26));
    anim->setPosAt(1.0, QPointF(sw, Colors::contentStartY + Colors::contentHeight - 26));
    movie_out->append(anim);
    
    // Create menu content:
    this->createInfo(new MenuContentItem(el, this->window->scene, 0), el.attribute("name") + " -info");
}

void MenuManager::createRightMenu1(const QDomElement &el)
{
    Movie *movie_in = new Movie();
    Movie *movie_out = new Movie();
    Movie *movie_shake = new Movie();
    this->score->insertMovie(el.attribute("name"), movie_in);
    this->score->insertMovie(el.attribute("name")+ " -out", movie_out);
    this->score->insertMovie(el.attribute("name")+ " -shake", movie_shake);
    
    qreal sw = this->window->scene->sceneRect().width();
    int xOffset = 15;
    int yOffset = 10;
    
    QDomNode n = el.firstChild();
    int i=0;
    while (!n.isNull()){
        QDomElement e = n.toElement();
        
        TextButton *item = new TextButton(e.attribute("name"), TextButton::LEFT, MENU2, this->window->scene, 0);
        item->setRecursiveVisible(false);
        item->setZValue(10);
        qreal ih = item->sceneBoundingRect().height();
        qreal iw = item->sceneBoundingRect().width();
        qreal ihp = ih + 3;
        
        // create in-animation:
        DemoItemAnimation *anim = new DemoItemAnimation(item, DemoItemAnimation::ANIM_IN);
        anim->setDuration(float(1000 + (i * 20)) * Colors::animSpeedButtons);
        anim->setStartPos(QPointF(xOffset, -ih));
        anim->setPosAt(0.20, QPointF(xOffset, -ih));
        anim->setPosAt(0.50, QPointF(xOffset, (i * ihp) + yOffset + Colors::contentStartY + (10 * float(i / 4.0f))));
        anim->setPosAt(0.60, QPointF(xOffset, (i * ihp) + yOffset + Colors::contentStartY));
        anim->setPosAt(0.70, QPointF(xOffset, (i * ihp) + yOffset + Colors::contentStartY + (5 * float(i / 4.0f))));
        anim->setPosAt(0.80, QPointF(xOffset, (i * ihp) + yOffset + Colors::contentStartY));
        anim->setPosAt(0.90, QPointF(xOffset, (i * ihp) + yOffset + Colors::contentStartY + (1 * float(i / 4.0f))));
        anim->setPosAt(1.00, QPointF(xOffset, (i * ihp) + yOffset + Colors::contentStartY));
        movie_in->append(anim);
        
        // create out-animation:
        anim = new DemoItemAnimation(item, DemoItemAnimation::ANIM_OUT);
        anim->hideOnFinished = true;
        anim->setDuration((700 + (30 * i)) * Colors::animSpeedButtons);
        anim->setStartPos(QPointF(xOffset, (i * ihp) + yOffset + Colors::contentStartY));
        anim->setPosAt(0.60, QPointF(xOffset, 600 - ih - ih));
        anim->setPosAt(0.65, QPointF(xOffset + 20, 600 - ih));
        anim->setPosAt(1.00, QPointF(sw + iw, 600 - ih));
        movie_out->append(anim);
        
        // create shake-animation:
        anim = new DemoItemAnimation(item);
        anim->setDuration(700 * Colors::animSpeedButtons);
        anim->setStartPos(QPointF(xOffset, (i * ihp) + yOffset + Colors::contentStartY));
        anim->setPosAt(0.55, QPointF(xOffset, (i * ihp) + yOffset + Colors::contentStartY - i*2.0));
        anim->setPosAt(0.70, QPointF(xOffset - 20, (i * ihp) + yOffset + Colors::contentStartY - i*1.5));
        anim->setPosAt(0.80, QPointF(xOffset, (i * ihp) + yOffset + Colors::contentStartY - i*1.0));
        anim->setPosAt(0.90, QPointF(xOffset - 5, (i * ihp) + yOffset + Colors::contentStartY - i*0.5));
        anim->setPosAt(1.00, QPointF(xOffset, (i * ihp) + yOffset + Colors::contentStartY));
        movie_shake->append(anim);
        
        i++;
        n = n.nextSibling();
    }
    
    // create backbutton:
    TextButton *backButton = new TextButton("Main menu", TextButton::RIGHT, ROOT, this->window->scene, 0, TextButton::GREEN);
    backButton->setRecursiveVisible(false);
    backButton->setZValue(10);
    qreal iw = backButton->sceneBoundingRect().width();
    
    DemoItemAnimation *BackButtonIn = new DemoItemAnimation(backButton, DemoItemAnimation::ANIM_IN);
    BackButtonIn->setDuration(1800 * Colors::animSpeedButtons);
    BackButtonIn->setStartPos(QPointF(-iw, Colors::contentStartY + Colors::contentHeight - 35));
    BackButtonIn->setPosAt(0.5, QPointF(-iw, Colors::contentStartY + Colors::contentHeight - 35));
    BackButtonIn->setPosAt(0.7, QPointF(xOffset, Colors::contentStartY + Colors::contentHeight - 35));
    BackButtonIn->setPosAt(1.0, QPointF(xOffset, Colors::contentStartY + Colors::contentHeight - 26));
    movie_in->append(BackButtonIn);
    
    DemoItemAnimation *BackButtonOut = new DemoItemAnimation(backButton, DemoItemAnimation::ANIM_OUT);
    BackButtonOut->setDuration(400 * Colors::animSpeedButtons);
    BackButtonOut->setStartPos(QPointF(xOffset, Colors::contentStartY + Colors::contentHeight - 26));
    BackButtonOut->setPosAt(1.0, QPointF(-iw, Colors::contentStartY + Colors::contentHeight - 26));
    movie_out->append(BackButtonOut);
    
    // create menu content text
    this->createInfo(new MenuContentItem(el, this->window->scene, 0), el.attribute("name") + " -info");
}

void MenuManager::createLeafMenu(const QDomElement &el)
{
    QString name = el.attribute("name");
    Movie *movie_in = new Movie();
    Movie *movie_out = new Movie();
    this->score->insertMovie(name + " -buttons", movie_in);
    this->score->insertMovie(name + " -buttons -out", movie_out);
    
    qreal sw = this->window->scene->sceneRect().width();
    qreal sh = this->window->scene->sceneRect().height();
    int xOffset = 75;
    
    // Create launch button
    if (el.attribute("executable") != "false"){
        TextButton *item = new TextButton("Launch", TextButton::RIGHT, LAUNCH, this->window->scene, 0, TextButton::GREEN);
        item->setRecursiveVisible(false);
        item->setZValue(10);
        
        // create in-animation:
        DemoItemAnimation *anim = new DemoItemAnimation(item, DemoItemAnimation::ANIM_IN);
        anim->setDuration(1050 * Colors::animSpeedButtons);
        anim->setStartPos(QPointF(sw, Colors::contentStartY + Colors::contentHeight - 35));
        anim->setPosAt(0.10, QPointF(sw, Colors::contentStartY + Colors::contentHeight - 35));
        anim->setPosAt(0.30, QPointF(xOffset + 330, Colors::contentStartY + Colors::contentHeight - 35));
        anim->setPosAt(0.35, QPointF(xOffset + 350, Colors::contentStartY + Colors::contentHeight - 35));
        anim->setPosAt(0.40, QPointF(xOffset + 330, Colors::contentStartY + Colors::contentHeight - 35));
        anim->setPosAt(0.45, QPointF(xOffset + 335, Colors::contentStartY + Colors::contentHeight - 35));
        anim->setPosAt(0.50, QPointF(xOffset + 330, Colors::contentStartY + Colors::contentHeight - 35));
        anim->setPosAt(1.00, QPointF(xOffset + 330, Colors::contentStartY + Colors::contentHeight - 26));
        movie_in->append(anim);
        
        // create out-animation:
        anim = new DemoItemAnimation(item, DemoItemAnimation::ANIM_OUT);
        anim->hideOnFinished = true;
        anim->setDuration(300 * Colors::animSpeedButtons);
        anim->setStartPos(QPointF(xOffset + 330, Colors::contentStartY + Colors::contentHeight - 26));
        anim->setPosAt(1.0, QPointF(xOffset + 330, sh));
        movie_out->append(anim);
    }
    
    // Create documentation button:
    TextButton *item = new TextButton("Documentation", TextButton::RIGHT, DOCUMENTATION, this->window->scene, 0, TextButton::GREEN);
    item->setRecursiveVisible(false);
    item->setZValue(10);
    
    // create in-animation:
    DemoItemAnimation *anim = new DemoItemAnimation(item, DemoItemAnimation::ANIM_IN);
    anim->setDuration(1050 * Colors::animSpeedButtons);
    anim->setStartPos(QPointF(sw, Colors::contentStartY + Colors::contentHeight - 35));
    anim->setPosAt(0.10, QPointF(sw, Colors::contentStartY + Colors::contentHeight - 35));
    anim->setPosAt(0.30, QPointF(xOffset + 530, Colors::contentStartY + Colors::contentHeight - 35));
    anim->setPosAt(0.35, QPointF(xOffset + 560, Colors::contentStartY + Colors::contentHeight - 35));
    anim->setPosAt(0.40, QPointF(xOffset + 530, Colors::contentStartY + Colors::contentHeight - 35));
    anim->setPosAt(0.45, QPointF(xOffset + 535, Colors::contentStartY + Colors::contentHeight - 35));
    anim->setPosAt(0.50, QPointF(xOffset + 530, Colors::contentStartY + Colors::contentHeight - 35));
    anim->setPosAt(1.00, QPointF(xOffset + 530, Colors::contentStartY + Colors::contentHeight - 26));
    movie_in->append(anim);
    
    // create out-animation:
    anim = new DemoItemAnimation(item, DemoItemAnimation::ANIM_OUT);
    anim->hideOnFinished = true;
    anim->setDuration(300 * Colors::animSpeedButtons);
    anim->setStartPos(QPointF(xOffset + 530, Colors::contentStartY + Colors::contentHeight - 26));
    anim->setPosAt(1.0, QPointF(xOffset + 530, sh));
    movie_out->append(anim);
    
    // Create info item:
    this->createInfo(new ExampleContent(name, this->window->scene, 0), el.attribute("name"));
}

void MenuManager::createInfo(DemoItem *item, const QString &name)
{
    Movie *movie_in = new Movie();
    Movie *movie_out = new Movie();
    this->score->insertMovie(name, movie_in);
    this->score->insertMovie(name + " -out", movie_out);
    
    item->setZValue(8);
    item->setRecursiveVisible(false);
    
    float xOffset = 220.0f;
    DemoItemAnimation *infoIn = new DemoItemAnimation(item, DemoItemAnimation::ANIM_IN);
    infoIn->timeline->setCurveShape(QTimeLine::LinearCurve);
    infoIn->setDuration(650);
    infoIn->setStartPos(QPointF(this->window->scene->sceneRect().width(), Colors::contentStartY));
    infoIn->setPosAt(0.60, QPointF(xOffset, Colors::contentStartY));
    infoIn->setPosAt(0.70, QPointF(xOffset + 40, Colors::contentStartY));
    infoIn->setPosAt(0.80, QPointF(xOffset, Colors::contentStartY));
    infoIn->setPosAt(0.90, QPointF(xOffset + 15, Colors::contentStartY));
    infoIn->setPosAt(1.00, QPointF(xOffset, Colors::contentStartY));
    movie_in->append(infoIn);
    
    DemoItemAnimation *infoOut = new DemoItemAnimation(item, DemoItemAnimation::ANIM_OUT);
    infoOut->timeline->setCurveShape(QTimeLine::EaseInCurve);
    infoOut->setDuration(300);
    infoOut->hideOnFinished = true;
    infoOut->setStartPos(QPointF(xOffset, Colors::contentStartY));
    infoOut->setPosAt(1.0, QPointF(-600, Colors::contentStartY));
    movie_out->append(infoOut);
}

void MenuManager::createTicker()
{
    if (!Colors::noTicker){
        Movie *movie_in = new Movie();
        Movie *movie_out = new Movie();
        this->score->insertMovie("ticker", movie_in);
        this->score->insertMovie("ticker -out", movie_out);
        
        this->ticker = new ItemCircleAnimation(this->window->scene, 0);
        this->ticker->setZValue(50);
        this->ticker->hide();
        
        // Move ticker in:
        int qtendpos = 485;
        int qtPosY = 120;
        DemoItemAnimation *qtIn = new DemoItemAnimation(this->ticker, DemoItemAnimation::ANIM_IN);
        qtIn->setDuration(500);
        qtIn->startDelay = 1500;
        qtIn->setStartPos(QPointF(this->window->scene->sceneRect().width(), Colors::contentStartY + qtPosY));
        qtIn->setPosAt(0.60, QPointF(qtendpos, Colors::contentStartY + qtPosY));
        qtIn->setPosAt(0.70, QPointF(qtendpos + 30, Colors::contentStartY + qtPosY));
        qtIn->setPosAt(0.80, QPointF(qtendpos, Colors::contentStartY + qtPosY));
        qtIn->setPosAt(0.90, QPointF(qtendpos + 5, Colors::contentStartY + qtPosY));
        qtIn->setPosAt(1.00, QPointF(qtendpos, Colors::contentStartY + qtPosY));
        movie_in->append(qtIn);
        
        // Move ticker out:
        DemoItemAnimation *qtOut = new DemoItemAnimation(this->ticker, DemoItemAnimation::ANIM_OUT);
        qtOut->hideOnFinished = true;
        qtOut->setDuration(500);
        qtOut->setStartPos(QPointF(qtendpos, Colors::contentStartY + qtPosY));
        qtOut->setPosAt(1.00, QPointF(this->window->scene->sceneRect().width() + 700, Colors::contentStartY + qtPosY));
        movie_out->append(qtOut);
    }
}
