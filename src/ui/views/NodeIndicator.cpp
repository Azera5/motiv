#include "NodeIndicator.hpp"

#include <QPen>
#include <QGraphicsSceneMouseEvent>


#include "src/ui/Constants.hpp"

NodeIndicator::NodeIndicator(Slot *representedSlot, qreal x, qreal y, qreal w, qreal h, QGraphicsItem *parent)
    : GenericIndicator<Slot, QGraphicsEllipseItem>(representedSlot, parent) {    
    this->setRect(x, y, w, h);
    }
