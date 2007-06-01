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

#ifndef MENU_MANAGER_H
#define MENU_MANAGER_H

#include <QtGui>
#include <QtXml>
#include <QtAssistant/QAssistantClient>
#include "score.h"
#include "textbutton.h"
#include "mainwindow.h"
#include "itemcircleanimation.h"

typedef QHash<QString, QString> StringHash;
typedef QHash<QString, StringHash> HashHash;

class MenuManager : public QObject
{
    Q_OBJECT
    
public:
    enum BUTTON_TYPE {ROOT, MENU1, MENU2, LAUNCH, DOCUMENTATION, QUIT, FULLSCREEN};
    
    // singleton pattern:
    static MenuManager *instance();
    virtual ~MenuManager();
    
    void init(MainWindow *window);
    void itemSelected(int userCode, const QString &menuName = "");
    
    HashHash info;
    ItemCircleAnimation *ticker;
    MainWindow *window;
    Score *score;
    int currentMenuCode;
    
private slots:
    void exampleFinished();
    void exampleError(QProcess::ProcessError error);
    
private:
    // singleton pattern:
    MenuManager();
    static MenuManager *pInstance;
    
    void readXmlDocument();
    void getDocumentationDir();
    void readInfoAboutExample(const QDomElement &example);
    void launchExample(const QString &uniqueName);
    
    QDomElement createMenu(const QDomElement &firstExample, BUTTON_TYPE type, Movie *movieIn, Movie *movieOut, Movie *movieShake);
    void createLowLeftButton(const QString label, BUTTON_TYPE type, Movie *movieIn, Movie *movieOut, Movie *movieShake);
    void createLowRightButton(const QString label, BUTTON_TYPE type, Movie *movieIn, Movie *movieOut, Movie *movieShake);
    void createLowRightLeafButton(const QString label, int pos, BUTTON_TYPE type, Movie *movieIn, Movie *movieOut, Movie */*movieShake*/);
    void createRootMenu(const QDomElement &el);
    void createSubMenu(const QDomElement &el);
    void createLeafMenu(const QDomElement &el);
    void createInfo(DemoItem *item, const QString &name);
    void createTicker();
    
    QString resolveExecutable(const QDomElement &example);
    QString resolveDocFile(const QDomElement &example);
    QString resolveImgFile(const QDomElement &example);
    
    QDomDocument *contentsDoc;
    QAssistantClient *assistant;
    QString currentMenu;
    QString currentExample;
    QDir docDir;
    QDir imgDir;
};

#endif // MENU_MANAGER_H

