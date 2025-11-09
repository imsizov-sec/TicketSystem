#include "src/header/prioritydelegate.h"
#include <QPainter>
#include <QApplication>

PriorityDelegate::PriorityDelegate(QObject* parent)
    : QStyledItemDelegate(parent) {}

void PriorityDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                             const QModelIndex& index) const {
    QStyleOptionViewItem opt(option);
    initStyleOption(&opt, index);

    QString priority = index.sibling(index.row(), 2).data().toString(); // Колонка "Приоритет"

    QColor background;
    if (priority == "Немедленный")
        background = QColor("#ff4c4c");
    else if (priority == "Высокий")
        background = QColor("#ffc4c4");
    else if (priority == "Средний")
        background = QColor("#fff2b0");

    if (background.isValid()) {
        painter->fillRect(option.rect, background);
    }

    QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &opt, painter);
}
