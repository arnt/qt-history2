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

#ifndef QLINEEDIT_P_H
#define QLINEEDIT_P_H

#include <private/qwidget_p.h>

#include <qtextlayout.h>
#include <qbasictimer.h>
#include <qstyleoption.h>

class QLineEditPrivate : public QWidgetPrivate
{
    Q_DECLARE_PUBLIC(QLineEdit)
public:

    QLineEditPrivate()
        : cursor(0), cursorTimer(0), frame(1),
          cursorVisible(0), separator(0), readOnly(0),
          direction(QChar::DirON), dragEnabled(1), alignment(0),
          echoMode(0), textDirty(0), selDirty(0), validInput(1),
          ascent(0), maxLength(32767), hscroll(0), lastCursorPos(-1), maskData(0),
          modifiedState(0), undoState(0), selstart(0), selend(0),
          imstart(0), imend(0), imselstart(0), imselend(0)
        {}
    ~QLineEditPrivate()
    {
        delete [] maskData;
    }
    void init(const QString&);
    QStyleOptionFrame getStyleOption() const;

    QString text;
    int cursor;
    int cursorTimer; // -1 for non blinking cursor.
    QPoint tripleClick;
    QBasicTimer tripleClickTimer;
    uint frame : 1;
    uint cursorVisible : 1;
    uint separator : 1;
    uint readOnly : 1;
    uint direction : 5;
    uint dragEnabled : 1;
    uint alignment : 3;
    uint echoMode : 2;
    uint textDirty : 1;
    uint selDirty : 1;
    uint validInput : 1;
    int ascent;
    int maxLength;
    int hscroll;
    int lastCursorPos;

    enum { UndoAct, RedoAct, CutAct, CopyAct, PasteAct, ClearAct, SelectAllAct, NCountActs };
    QAction *actions[NCountActs];

    inline void emitCursorPositionChanged();
    bool sendMouseEventToInputContext(QMouseEvent *e);

    void finishChange(int validateFromState = -1, bool update = false);

    QPointer<QValidator> validator;
    struct MaskInputData {
        enum Casemode { NoCaseMode, Upper, Lower };
        QChar maskChar; // either the separator char or the inputmask
        bool separator;
        Casemode caseMode;
    };
    QString inputMask;
    QChar blank;
    MaskInputData *maskData;
    inline int nextMaskBlank(int pos) {
        int c = findInMask(pos, true, false);
        separator |= (c != pos);
        return (c != -1 ?  c : maxLength);
    }
    inline int prevMaskBlank(int pos) {
        int c = findInMask(pos, false, false);
        separator |= (c != pos);
        return (c != -1 ? c : 0);
    }

    void setCursorVisible(bool visible);


    // undo/redo handling
    enum CommandType { Separator, Insert, Remove, Delete, RemoveSelection, DeleteSelection };
    struct Command {
        inline Command(){}
        inline Command(CommandType type, int pos, QChar c)
            :type(type),c(c),pos(pos){}
        uint type : 4;
        QChar c;
        int pos;
    };
    int modifiedState;
    int undoState;
    QVector<Command> history;
    void addCommand(const Command& cmd);
    void insert(const QString& s);
    void del(bool wasBackspace = false);
    void remove(int pos);

    inline void separate() { separator = true; }
    void undo(int until = -1);
    void redo();
    inline bool isUndoAvailable() const { return !readOnly && undoState; }
    inline bool isRedoAvailable() const { return !readOnly && undoState < (int)history.size(); }

    // bidi
    inline bool isRightToLeft() const { return direction==QChar::DirON?text.isRightToLeft():(direction==QChar::DirR); }

    // selection
    int selstart, selend;
    inline bool allSelected() const { return !text.isEmpty() && selstart == 0 && selend == (int)text.length(); }
    inline bool hasSelectedText() const { return !text.isEmpty() && selend > selstart; }
    inline void deselect() { selDirty |= (selend > selstart); selstart = selend = 0; }
    void removeSelectedText();
#ifndef QT_NO_CLIPBOARD
    void copy(bool clipboard = true) const;
#endif
    inline bool inSelection(int x) const
    { if (selstart >= selend) return false;
    int pos = xToPos(x, QTextLine::CursorOnCharacter);  return pos >= selstart && pos < selend; }

    // masking
    void parseInputMask(const QString &maskFields);
    bool isValidInput(QChar key, QChar mask) const;
    bool hasAcceptableInput(const QString &text) const;
    QString maskString(uint pos, const QString &str, bool clear = false) const;
    QString clearString(uint pos, uint len) const;
    QString stripString(const QString &str) const;
    int findInMask(int pos, bool forward, bool findSeparator, QChar searchChar = QChar()) const;

    // input methods
    int imstart, imend, imselstart, imselend;
    bool composeMode() const { return preeditLength(); }
    bool hasIMSelection() const { return imSelectionLength(); }
    int preeditLength() const { return ( imend - imstart ); }
    int imSelectionLength() const { return ( imselend - imselstart ); }

    // complex text layout
    QTextLayout textLayout;
    void updateTextLayout();
    void moveCursor(int pos, bool mark = false);
    void setText(const QString& txt, int pos = -1);
    int xToPosInternal( int x, QTextLine::CursorPosition ) const;
    int xToPos(int x, QTextLine::CursorPosition = QTextLine::CursorBetweenCharacters) const;
    inline int visualAlignment() const { return alignment ? alignment : int(isRightToLeft() ? Qt::AlignRight : Qt::AlignLeft); }
    QRect cursorRect() const;
    void updateMicroFocusHint();

#ifndef QT_NO_DRAGANDDROP
    // drag and drop
    QPoint dndPos;
    QBasicTimer dndTimer;
    void drag();
#endif

    void clipboardChanged();
};



#endif
