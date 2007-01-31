#include "undocommand.h"
#include <QDebug>


static QString roleToString(QPalette::ColorRole role)
{
    switch (role) {
    case QPalette::WindowText: return QLatin1String("WindowText");
    case QPalette::Button: return QLatin1String("Button");
    case QPalette::Light: return QLatin1String("Light");
    case QPalette::Midlight: return QLatin1String("Midlight");
    case QPalette::Dark: return QLatin1String("Dark");
    case QPalette::Mid: return QLatin1String("Mid");
    case QPalette::Text: return QLatin1String("Text");
    case QPalette::BrightText: return QLatin1String("BrightText");
    case QPalette::ButtonText: return QLatin1String("ButtonText");
    case QPalette::Base: return QLatin1String("Base");
    case QPalette::Window: return QLatin1String("Window");
    case QPalette::Shadow: return QLatin1String("Shadow");
    case QPalette::Highlight: return QLatin1String("Highlight");
    case QPalette::HighlightedText: return QLatin1String("HighlightedText");
    case QPalette::Link: return QLatin1String("Link");
    case QPalette::LinkVisited: return QLatin1String("LinkVisited");
    case QPalette::AlternateBase: return QLatin1String("AlternateBase");
    default: break;
    }
    Q_ASSERT(0);
    return QString();
}


static QString groupToString(QPalette::ColorGroup group)
{
    switch (group) {
    case QPalette::Active: return QLatin1String("Active");
    case QPalette::Disabled: return QLatin1String("Disabled");
    case QPalette::Inactive: return QLatin1String("Inactive");
    case QPalette::All: return QLatin1String("All");
    default: break;
    }
    Q_ASSERT(0);
    return QString();
}

static QModelIndexList indexes(PaletteModel *model, QPalette::ColorRole role, QPalette::ColorGroup group)
{
    QModelIndexList ret;
    if (group == QPalette::All) {
        ret << model->index(role, model->groupToColumn(QPalette::Active))
            << model->index(role, model->groupToColumn(QPalette::Inactive))
            << model->index(role, model->groupToColumn(QPalette::Disabled));
    } else {
        ret << model->index(role, model->groupToColumn(group));
    }
    return ret;
}




ColorChange::ColorChange(PaletteModel *mod, QPalette::ColorRole r, const QColor &col,
                         QPalette::ColorGroup g, QUndoCommand *parent)
    : QUndoCommand(parent), model(mod), role(r), group(g), color(col)
{
    setText(QObject::tr("Change %1(%2) to %3").arg(roleToString(role)).arg(groupToString(group)).arg(col.name()));
}

void ColorChange::undo()
{
    foreach (QModelIndex index, indexes(model, role, group)) {
        model->setData(index, old[model->columnToGroup(index.column())], Qt::BackgroundColorRole);
    }
}

void ColorChange::redo()
{
    foreach (QModelIndex index, indexes(model, role, group)) {
        const QModelIndex index = model->index(role, model->groupToColumn(group));
        old[model->columnToGroup(index.column())] = qVariantValue<QColor>(index.data(Qt::BackgroundColorRole));
        model->setData(index, color, Qt::BackgroundColorRole);
    }
}

