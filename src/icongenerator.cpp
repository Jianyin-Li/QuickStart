#include "icongenerator.h"
#include <QPainter>
#include <QFont>
#include <QFontMetrics>
#include <QRandomGenerator>
#include <QCryptographicHash>
#include <QByteArray>
#include <QRadialGradient>
#include <QLinearGradient>

IconGenerator::IconGenerator(QObject *parent)
    : QObject(parent)
{
}

QIcon IconGenerator::generateDefaultIcon(const QString &name, int size)
{
    if (name.isEmpty()) {
        return generateIcon("?", Qt::gray, Qt::white, size);
    }

    QString firstChar = name.left(1).toUpper();

    QColor backgroundColor = getColorForName(name);

    return generateIcon(firstChar, backgroundColor, Qt::white, size);
}

QIcon IconGenerator::generateIcon(const QString &text, const QColor &backgroundColor,
                                 const QColor &textColor, int size)
{
    QPixmap pixmap = drawCircularIcon(text, backgroundColor, textColor, size);
    return QIcon(pixmap);
}

QIcon IconGenerator::generateBackArrow(const QColor &color, int size)
{
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(QPen(color, qMax(1.6, size * 0.11), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    // Chevron pointing left, inset so the stroke is never clipped.
    QPen pen = painter.pen();
    qreal inset = pen.widthF() / 2.0 + 1.0;
    qreal midY = size / 2.0;
    qreal leftX = inset;
    qreal rightX = size - inset;
    painter.drawLine(QPointF(rightX, inset), QPointF(leftX, midY));
    painter.drawLine(QPointF(leftX, midY), QPointF(rightX, size - inset));
    painter.end();

    return QIcon(pixmap);
}

const QList<QColor>& IconGenerator::getDefaultColors()
{
    // Muted, mid-tone colours. A grid of fully saturated primaries reads as
    // noise; these stay distinguishable but sit quietly next to each other.
    static const QList<QColor> colors = {
        QColor(90, 132, 178),   // Soft blue
        QColor(186, 96, 88),    // Soft red
        QColor(198, 152, 66),   // Soft amber
        QColor(76, 143, 108),   // Soft green
        QColor(142, 108, 162),  // Soft violet
        QColor(84, 148, 160),   // Soft teal
        QColor(190, 120, 84),   // Soft orange
        QColor(140, 116, 100),  // Soft taupe
        QColor(138, 143, 148),  // Soft grey
        QColor(112, 130, 140)   // Soft slate
    };
    return colors;
}

static QColor darkenColor(const QColor &c, int factor)
{
    return QColor(qMax(0, c.red() - factor),
                  qMax(0, c.green() - factor),
                  qMax(0, c.blue() - factor));
}

QColor IconGenerator::getColorForName(const QString &name)
{
    if (name.isEmpty()) {
        return Qt::gray;
    }

    QByteArray hash = QCryptographicHash::hash(name.toUtf8(), QCryptographicHash::Md5);

    int colorIndex = static_cast<unsigned char>(hash[0]) % getDefaultColors().size();

    return getDefaultColors().at(colorIndex);
}

QPixmap IconGenerator::drawCircularIcon(const QString &text, const QColor &backgroundColor,
                                       const QColor &textColor, int size)
{
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QRadialGradient gradient(size / 2.0, size / 2.0, size / 2.0);
    QColor lighter = backgroundColor.lighter(130);
    QColor darkerColor = darkenColor(backgroundColor, 40);
    gradient.setColorAt(0.0, lighter);
    gradient.setColorAt(0.6, backgroundColor);
    gradient.setColorAt(1.0, darkerColor);

    painter.setBrush(gradient);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(2, 2, size - 4, size - 4);

    painter.setPen(textColor);

    int fontSize = size * 0.48;
    if (text.length() > 1) {
        fontSize = size * 0.32;
    }

    QFont font;
    font.setPixelSize(fontSize);
    font.setBold(true);
    painter.setFont(font);

    QFontMetrics metrics(font);
    QRect textRect = metrics.boundingRect(text);

    int x = (size - textRect.width()) / 2 - textRect.left();
    int y = (size - textRect.height()) / 2 - textRect.top() + 1;

    painter.drawText(x, y, text);

    return pixmap;
}

QPixmap IconGenerator::drawSquareIcon(const QString &text, const QColor &backgroundColor,
                                     const QColor &textColor, int size)
{
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QLinearGradient gradient(0, 0, size, size);
    QColor lighter = backgroundColor.lighter(130);
    QColor darkerColor = darkenColor(backgroundColor, 40);
    gradient.setColorAt(0.0, lighter);
    gradient.setColorAt(1.0, darkerColor);

    painter.setBrush(gradient);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(2, 2, size - 4, size - 4, 10, 10);

    painter.setPen(textColor);

    int fontSize = size * 0.48;
    if (text.length() > 1) {
        fontSize = size * 0.32;
    }

    QFont font;
    font.setPixelSize(fontSize);
    font.setBold(true);
    painter.setFont(font);

    QFontMetrics metrics(font);
    QRect textRect = metrics.boundingRect(text);

    int x = (size - textRect.width()) / 2 - textRect.left();
    int y = (size - textRect.height()) / 2 - textRect.top() + 1;

    painter.drawText(x, y, text);

    return pixmap;
}
