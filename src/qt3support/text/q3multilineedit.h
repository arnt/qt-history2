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

#ifndef Q3MULTILINEEDIT_H
#define Q3MULTILINEEDIT_H

#include <Qt3Support/q3textedit.h>

QT_MODULE(Qt3SupportLight)

#ifndef QT_NO_MULTILINEEDIT

class Q3MultiLineEditCommand;
class QValidator;
class Q3MultiLineEditData;

class Q_COMPAT_EXPORT Q3MultiLineEdit : public Q3TextEdit
{
    Q_OBJECT
    Q_PROPERTY(int numLines READ numLines)
    Q_PROPERTY(bool atBeginning READ atBeginning)
    Q_PROPERTY(bool atEnd READ atEnd)
    Q_PROPERTY(Qt::Alignment alignment READ alignment WRITE setAlignment)
    Q_PROPERTY(bool edited READ edited WRITE setEdited DESIGNABLE false)

public:
    Q3MultiLineEdit(QWidget* parent=0, const char* name=0);
    ~Q3MultiLineEdit();

    QString textLine(int line) const;
    int numLines() const;

    virtual void insertLine(const QString &s, int line = -1);
    virtual void insertAt(const QString &s, int line, int col) {
        insertAt(s, line, col, false);
    }
    virtual void insertAt(const QString &s, int line, int col, bool mark);
    virtual void removeLine(int line);
    virtual void setCursorPosition(int line, int col) {
        setCursorPosition(line, col, false);
    }
    virtual void setCursorPosition(int line, int col, bool mark);
    bool atBeginning() const;
    bool atEnd() const;

    void setAlignment(Qt::Alignment flags);
    Qt::Alignment alignment() const;

    void setEdited(bool);
    bool edited() const;

    bool hasMarkedText() const;
    QString markedText() const;

    void cursorWordForward(bool mark);
    void cursorWordBackward(bool mark);

    // noops
    bool autoUpdate() const { return true; }
    virtual void setAutoUpdate(bool) {}

    int totalWidth() const { return contentsWidth(); }
    int totalHeight() const { return contentsHeight(); }

    int maxLines() const { return QWIDGETSIZE_MAX; }
    void setMaxLines(int) {}

public Q_SLOTS:
    void deselect() { selectAll(false); }

protected:
    QPoint cursorPoint() const;
    virtual void insertAndMark(const QString&, bool mark);
    virtual void newLine();
    virtual void killLine();
    virtual void pageUp(bool mark=false);
    virtual void pageDown(bool mark=false);
    virtual void cursorLeft(bool mark=false, bool wrap = true);
    virtual void cursorRight(bool mark=false, bool wrap = true);
    virtual void cursorUp(bool mark=false);
    virtual void cursorDown(bool mark=false);
    virtual void backspace();
    virtual void home(bool mark=false);
    virtual void end(bool mark=false);

    bool getMarkedRegion(int *line1, int *col1, int *line2, int *col2) const;
    int lineLength(int row) const;

private:
    Q_DISABLE_COPY(Q3MultiLineEdit)

    Q3MultiLineEditData *d;
};

#endif // QT_NO_MULTILINEEDIT

#endif // Q3MULTILINEEDIT_H
