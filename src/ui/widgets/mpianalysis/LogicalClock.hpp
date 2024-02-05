#ifndef MOTIV_LOGICALCLOCK_HPP
#define MOTIV_LOGICALCLOCK_HPP

#include <QWidget>
#include <QGraphicsView>
#include <QLabel>
#include <set>



#include "src/ui/TraceDataProxy.hpp"
#include "src/models/mpianalysis/Node.hpp"


class LogicalClock : public QGraphicsView {
    Q_OBJECT

public:
    /**
     * @brief Creates a new instance of the LogicalClock class
     *
     * @param data The data proxy to obtain the current selection of the trace and to connect to change events
     * @param nodes A map of node vectors indexed by location
     * @param parent The parent QWidget
     */
    explicit LogicalClock(TraceDataProxy *data , std::map<uint64_t, std::vector<Node*>> nodes, QWidget *parent = nullptr);


private: //methods
    void populateScene(QGraphicsScene *element);
    void drawNodes( std::map<uint64_t, std::vector<std::pair<Node*, bool>>>& nodes_, 
        std::map<uint64_t, std::pair<int,int>>& nodeCountMap,
        std::map<const Communication*, std::tuple<int, int>>& pendingEdges,
        std::map<const CollectiveCommunicationEvent*, std::tuple<int, int, int, uint64_t, std::set<uint16_t>>>& pendingCollectives,
        QGraphicsScene* scene,        
        uint64_t location_ = 0,
        std::pair<Node*,Node*>* matchingNodes = nullptr
        );
    
private: // data
    TraceDataProxy *data = nullptr;
    std::map<uint64_t, std::vector<Node*>> nodes;
};

#endif //MOTIV_LOGICALCLOCK_HPP