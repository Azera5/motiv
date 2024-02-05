#ifndef MOTIV_NODEINDICATOR_HPP
#define MOTIV_NODEINDICATOR_HPP

#include <QGraphicsEllipseItem>

#include "src/ui/TraceDataProxy.hpp"
#include "src/models/Slot.hpp"
#include "src/types.hpp"
#include "GenericIndicator.hpp"


/**
 * @brief Indicator for collective communications
 *
 * A slot is indicated by a rectangle.
 */
class NodeIndicator : public GenericIndicator<Slot, QGraphicsEllipseItem> {
public: // constructors
    /**
     * @brief Creates a new instance of the SlotIndicator class
     * @param rect The rect the slot should be rendered
     * @param representedSlot The Slot object the indicator is representing
     * @param parent The parent QGraphicsItem
     */
    NodeIndicator(Slot *representedSlot, qreal x, qreal y, qreal w, qreal h, QGraphicsItem *parent = nullptr);
};


#endif //MOTIV_NODEINDICATOR_HPP