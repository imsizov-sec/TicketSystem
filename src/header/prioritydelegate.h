#pragma once

#include <QStyledItemDelegate>

class PriorityDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    explicit PriorityDelegate(QObject* parent = nullptr);
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;
};
