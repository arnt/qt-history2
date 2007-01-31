#ifndef UNDOCOMMAND_H
#define UNDOCOMMAND_H

#include <QPalette>
#include <QColor>
#include <QUndoCommand>
#include "appearancepage.h"
class ColorChange : public QUndoCommand
{
public:
    ColorChange(PaletteModel *model, QPalette::ColorRole role, const QColor &color,
                QPalette::ColorGroup group = QPalette::All, QUndoCommand *parent = 0);
    virtual void undo();
    virtual void redo();
private:
    PaletteModel *model;
    const QPalette::ColorRole role;
    const QPalette::ColorGroup group;
    const QColor color;
    QColor old[QPalette::NColorGroups];
};

#endif
