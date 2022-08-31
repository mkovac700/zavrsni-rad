#include "rectitem.h"

RectItem::RectItem(QObject *parent) : QObject(parent)
{
    rect = QRect(0,0,30,100);
    rectText = "0";
    rectColor = Qt::red;
    penColor = Qt::white;
    rectTextAlignment = Qt::AlignHCenter | Qt::AlignTop;
}

QRect RectItem::geometry() const { return rect; }

void RectItem::setGeometry(const QRect &value) {
    if (rect != value) {
        prepareGeometryChange(); //fix for QPen glitch while animating
        rect = value;
        update();
    }
}

//QRectF RectItem::boundingRect() const { return QRectF(rect); }

QRectF RectItem::boundingRect() const
{
    int atop = rect.top();
    int aleft = rect.left();
    int awidth = rect.width();
    int aheight = rect.height();

    QRect modifiedRect = QRect(aleft,atop-20,awidth,aheight+20);

    return QRectF(modifiedRect);
}

void RectItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                     QWidget *widget) {
    Q_UNUSED(option)
    Q_UNUSED(widget)

    painter->setRenderHint(QPainter::Antialiasing);

    QFont font("Helvetica [Cronyx]", 14, QFont::Weight::Medium);
    painter->setFont(font);

    QPainterPath path;
    path.addRect(rect);

    painter->setPen(QPen(penColor,0.1,Qt::PenStyle::SolidLine,Qt::PenCapStyle::SquareCap,Qt::PenJoinStyle::BevelJoin));
    painter->fillPath(path, rectColor);
    painter->drawPath(path);
//    painter->drawText(rect,Qt::AlignHCenter | Qt::AlignBottom,rectText);
    painter->drawText(boundingRect(),rectTextAlignment,rectText);
}

QString RectItem::text() const { return rectText; }

void RectItem::setText(const QString &text, const QColor &color, const int &alignment){
    rectText = text;
    penColor = color;
    rectTextAlignment = alignment;
    update();
}

QColor RectItem::color() const { return rectColor; }

void RectItem::setColor(const QColor &color){
    rectColor = color;
    update();
}
