#ifndef RECTITEM_H
#define RECTITEM_H

#include <QObject>
#include <QGraphicsItem>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

class RectItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_PROPERTY(QRect geometry READ geometry WRITE setGeometry)
    Q_INTERFACES(QGraphicsItem)

public:
    explicit RectItem(QObject *parent = nullptr);

    QRect geometry() const;
    void setGeometry(const QRect &value);

    QRectF boundingRect() const;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget);

    QString text() const;
    void setText(const QString &text, const QColor &color = Qt::black, const int &alignment = Qt::AlignHCenter | Qt::AlignTop);

    QColor color() const;
    void setColor(const QColor &color);

private:
    QRect rect;
    QString rectText;
    QColor rectColor;
    QColor penColor;
    int rectTextAlignment;
};

#endif // RECTITEM_H
