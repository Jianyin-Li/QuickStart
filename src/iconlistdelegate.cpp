#include "iconlistdelegate.h"
#include <QPainter>
#include <QPainterPath>
#include <QApplication>

// Layout of a single grid card. Kept in one place so the painted geometry
// and the size hint can never drift apart.
static const int CARD_WIDTH = 112;
static const int CARD_HEIGHT = 128;
static const int CARD_MARGIN = 4;
static const int CARD_RADIUS = 8;
static const int ICON_SIZE = 48;
static const int ICON_Y_OFFSET = 14;
static const int TEXT_Y_OFFSET = 72;
static const int TEXT_HEIGHT = 40;

IconListDelegate::IconListDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void IconListDelegate::setDarkMode(bool dark)
{
    m_darkMode = dark;
}

QSize IconListDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    return QSize(CARD_WIDTH, CARD_HEIGHT);
}

void IconListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);

    bool hovered = option.state & QStyle::State_MouseOver;
    bool selected = option.state & QStyle::State_Selected;

    QRect cardRect = option.rect.adjusted(CARD_MARGIN, CARD_MARGIN, -CARD_MARGIN, -CARD_MARGIN);

    drawCardBackground(painter, cardRect, hovered, selected);

    QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
    if (!icon.isNull()) {
        int iconX = cardRect.center().x() - ICON_SIZE / 2;
        int iconY = cardRect.top() + ICON_Y_OFFSET;
        QPixmap pixmap = icon.pixmap(ICON_SIZE, ICON_SIZE);

        // A soft circular plate keeps arbitrary app icons visually calm.
        QColor plateColor = m_darkMode ? QColor(0xFF, 0xFF, 0xFF, 10)
                                      : QColor(0xF4, 0xF5, 0xF6);
        painter->setPen(Qt::NoPen);
        painter->setBrush(plateColor);
        painter->drawEllipse(iconX - 3, iconY - 3, ICON_SIZE + 6, ICON_SIZE + 6);

        QPainterPath clipPath;
        clipPath.addEllipse(iconX + 2, iconY + 2, ICON_SIZE - 4, ICON_SIZE - 4);
        painter->save();
        painter->setClipPath(clipPath);
        painter->drawPixmap(iconX + 2, iconY + 2, ICON_SIZE - 4, ICON_SIZE - 4, pixmap);
        painter->restore();
    }

    QString text = index.data(Qt::DisplayRole).toString();
    if (!text.isEmpty()) {
        QRect textRect(
            cardRect.left() + 5,
            cardRect.top() + TEXT_Y_OFFSET,
            cardRect.width() - 10,
            TEXT_HEIGHT
        );

        QFont font = painter->font();
        font.setPixelSize(11);
        font.setBold(false);
        painter->setFont(font);

        QColor textColor;
        if (m_darkMode) {
            textColor = selected ? QColor("#9cc4f0") : QColor("#cfd4d9");
        } else {
            textColor = selected ? QColor("#14538a") : QColor("#3c4043");
        }
        painter->setPen(textColor);

        painter->drawText(textRect, Qt::AlignCenter | Qt::TextWordWrap, text);
    }
}

void IconListDelegate::drawCardBackground(QPainter *painter, const QRect &rect, bool hovered, bool selected) const
{
    // Restrained depth: a hairline border and a flat fill. No drop shadows and
    // no stacked blur passes - they read as grime on a light background.
    QColor bgColor;
    QColor borderColor;
    qreal borderWidth = 1.0;

    if (m_darkMode) {
        if (selected) {
            bgColor = QColor("#22303f");
            borderColor = QColor("#3d6da5");
        } else if (hovered) {
            bgColor = QColor("#242830");
            borderColor = QColor("#343a42");
        } else {
            bgColor = QColor("#1e2126");
            borderColor = QColor("#2a2e34");
        }
    } else {
        if (selected) {
            bgColor = QColor("#eaf1fb");
            borderColor = QColor("#8ab4e8");
        } else if (hovered) {
            bgColor = QColor("#ffffff");
            borderColor = QColor("#c3c8cd");
        } else {
            bgColor = QColor("#ffffff");
            borderColor = QColor("#e6e8ea");
        }
    }

    QPainterPath cardPath;
    cardPath.addRoundedRect(rect, CARD_RADIUS, CARD_RADIUS);
    painter->setPen(QPen(borderColor, borderWidth));
    painter->setBrush(bgColor);
    painter->drawPath(cardPath);
}
