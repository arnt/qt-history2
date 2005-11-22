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

#ifndef QVFBOPTIONS_H

#include <QString>

class QVFbOptions
{
public:
    QVFbOptions(int &argc, char **argv);
    ~QVFbOptions();

    int width() const;
    int height() const;
    int depth() const;

    bool cursor() const;
    int protocol() const;
    QString skin() const;

    double zoom() const;
    int display() const;
    int rotation() const;

    void setWidth(int);
    void setHeight(int);
    void setDepth(int);
    void setCursor(bool);
    void setProtocol(int);
    void setZoom(double);
    void setDisplay(int);
    void setRotation(int);
    void setSkin(const QString &spec);

private:
    bool mCursor;
    int mProtocol;
};

extern QVFbOptions *qvfbOptions;

#define QVFBOPTIONS_H
#endif // QVFBOPTIONS_H
