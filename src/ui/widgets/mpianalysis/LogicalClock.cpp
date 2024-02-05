#include <QGraphicsEllipseItem>

#include "LogicalClock.hpp"
#include "src/ui/Constants.hpp"
#include "src/ui/views/CommunicationIndicator.hpp"
#include "src/ui/views/NodeIndicator.hpp"

constexpr int X_OFFSET = 50;
constexpr int Y_OFFSET = 80;
constexpr int RADIUS = 15;

const QColor SEND_COLOR("#4A69A6");
const QColor RECV_COLOR("#508B52");
const QColor COLLECTIVES_COLOR("#C32727");
const QColor OTHERS_COLOR("#A9A7A7");

LogicalClock::LogicalClock(TraceDataProxy *data, std::map<uint64_t, std::vector<Node*>> nodes, QWidget *parent): QGraphicsView(parent),
    data(data), nodes(nodes){

    auto scene = new QGraphicsScene();
    this->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    this->setAutoFillBackground(false);
    this->setStyleSheet("background: transparent");
    this->setScene(scene);
    this->populateScene(scene);

}


// Index aliases for accessing elements in pendingCollectives tuples:
// IDX_MAX_NODE_COUNT: Represents the largest node count among members, used for X-axis offset.
// IDX_START_Y: Index for the starting Y-coordinate.
// IDX_END_Y: Index for the ending Y-coordinate.
// IDX_INITIAL_LOCATION: Represents the location of the first occurrence of a specific collective communication.
// IDX_FINISHED_MEMBERS: Used to avoid redundant iterations.

constexpr size_t IDX_MAX_NODE_COUNT = 0;
constexpr size_t IDX_START_Y = 1;
constexpr size_t IDX_END_Y = 2;
constexpr size_t IDX_INITIAL_LOCATION = 3;
constexpr size_t IDX_FINISHED_MEMBERS = 4;

std::map<uint64_t, std::pair<int, int>> DEBUG_COUNTER;

int DEBUG_CALL_COUNTER = 0;

void LogicalClock::populateScene(QGraphicsScene* scene) {
    
    // Build nodes_ map with finishing state for each node.
    std::map<uint64_t, std::vector<std::pair<Node*, bool>>> nodes_;
    std::pair<Node*, Node*> matchingNodes;

    for (auto &item : nodes) {
        for(auto node: item.second){
            auto location = node->getSlot()->location->ref();
            if(nodes_.find(location) == nodes_.end()) nodes_[location] = std::vector<std::pair<Node*, bool>>();
            nodes_[location].push_back(std::make_pair(node, false));
        }
    }

    // Map contains x-axis offset(first) and vector index(second) by location
    std::map<uint64_t, std::pair<int,int>> nodeCountMap;        
       
    std::map<const Communication*, std::tuple<int, int>> pendingEdges; 
    std::map<const CollectiveCommunicationEvent*, std::tuple<int, int, int, uint64_t, std::set<uint16_t>>> pendingCollectives;
    
    drawNodes(nodes_, nodeCountMap, pendingEdges, pendingCollectives, scene);

    for(size_t i = 0; i < nodeCountMap.size(); ++i){
        QGraphicsLineItem* line = scene->addLine(0, i * Y_OFFSET, 1080, i * Y_OFFSET);
        line->setZValue(-1); 

        QString label = QString("Rank: %1").arg(i);
        QGraphicsTextItem* textItem = new QGraphicsTextItem(label);
        textItem->setPos(-textItem->boundingRect().width() - 10, i * Y_OFFSET - textItem->boundingRect().height() / 2);
        textItem->setZValue(-1);
        scene->addItem(textItem);
    }

    for(const auto& collectivCommunication: pendingCollectives){
        int x = (std::get<IDX_MAX_NODE_COUNT>(collectivCommunication.second) * X_OFFSET);
        int yStart = std::get<IDX_START_Y>(collectivCommunication.second);
        int yEnd = std::get<IDX_END_Y>(collectivCommunication.second);

        QGraphicsLineItem* line = scene->addLine(x, yStart, x, yEnd);
        line->setPen(QPen(Qt::red));
    }
    
    // for(const auto& item: DEBUG_COUNTER){
    //     std::cout << "Location: " << item.first 
    //             << " P2P: " << item.second.first 
    //             << " Collectives: " << item.second.second << std::endl;
    // }
}

void LogicalClock::drawNodes(std::map<uint64_t, std::vector<std::pair<Node*, bool>>>& nodes_,
    std::map<uint64_t, std::pair<int,int>>& nodeCountMap,
    std::map<const Communication*, std::tuple<int, int>>& pendingEdges,
    std::map<const CollectiveCommunicationEvent*, std::tuple<int, int, int, uint64_t, std::set<uint16_t>>>& pendingCollectives,
    QGraphicsScene* scene,
    uint64_t location_,
    std::pair<Node*,Node*>* matchingNodes)
    {
    DEBUG_CALL_COUNTER++;
    auto onTimedElementSelected = [this](TimedElement *element) { this->data->setTimeElementSelection(element); };
    auto item = nodes_.lower_bound(location_);
    
    

    for(; item!= nodes_.end(); ++item){
        auto location = item->first;
        if(nodeCountMap.find(location)==nodeCountMap.end()){ 
            nodeCountMap[location].first = 1;
            nodeCountMap[location].second = 0;
        }
        int& count = nodeCountMap[location].first;     

        // Continue node vector iteration at last position 
        for(size_t j = nodeCountMap[location].second; j < item->second.size(); ++j){
            auto& node = item->second[j];
            if(node.second) continue;
            QColor color = OTHERS_COLOR;
            

            auto region = QString::fromStdString(node.first->getRegionName().str());            
            auto DEBUG_REGION = region.toStdString();
            if(region.contains("Init") || region.contains("Comm") || region.contains("Group") || region.contains("Finalize")) {
                node.second = true;
                nodeCountMap[location].second++;
                continue; //workaround
                }

            int y = (location * Y_OFFSET) - RADIUS;
            int x = (count * X_OFFSET) - RADIUS;

            
            // Node is P2P communication
            if(node.first->hasCommunication()){
                
                // Set node color
                if(region.contains("send", Qt::CaseInsensitive)) color = SEND_COLOR;
                else color = RECV_COLOR; 

                // Preparing offset for node
                uint64_t connectedNodeRank = node.first->getConnectedCommunicationRank();
                
                if(nodeCountMap.find(connectedNodeRank)==nodeCountMap.end()){
                    nodeCountMap[connectedNodeRank].first = 1;
                    nodeCountMap[connectedNodeRank].second = 0;
                }
                
                if(pendingEdges.find(node.first->getCommunication()) == pendingEdges.end()){
                    // Blocking communication needs a specific x-axis-offset
                    if(node.first->getCommunicationKind() & CommunicationKind::BlockingPointToPoint) {                        
                        // node define offset because is start event
                        if(node.first->getOwnEvent() == node.first->getCommunication()->getStartEvent()) { 
                            nodeCountMap[connectedNodeRank].first = std::max(nodeCountMap[connectedNodeRank].first, (count+1));
                        }                        
                        // Node has to wait for offset from connected node
                        else{
                            if(!node.first->getConnectedNodes().empty()){
                                auto matchingNodes = std::make_pair(node.first, node.first->getConnectedNodes()[connectedNodeRank]);
                                drawNodes(nodes_, nodeCountMap, pendingEdges, pendingCollectives, scene, connectedNodeRank, &matchingNodes); 
                            }
                            //TODO WARNUNG PRINTEN
                            else drawNodes(nodes_, nodeCountMap, pendingEdges, pendingCollectives, scene, connectedNodeRank);                            
                        }
                        if(node.second) continue;

                        // overwrite x, maybe count has changed!                       
                        x = (count * X_OFFSET) - RADIUS;
                    }
                    
                    if(pendingEdges.find(node.first->getCommunication()) == pendingEdges.end()) pendingEdges[node.first->getCommunication()] = std::make_tuple(x, y);
                    // if(region.contains("send",Qt::CaseInsensitive)) nodeCountMap[connectedNodeRank].first = std::max(nodeCountMap[connectedNodeRank].first, (count+1));
                }
                // Preparing coordinates for edges                
                // if(node.first->getOwnEvent() == node.first->getCommunication()->getEndEvent() && !(node.second)){
                uint64_t pendingY = std::get<1>(pendingEdges[node.first->getCommunication()]);
                uint64_t pendingLocation = (pendingY + RADIUS)/Y_OFFSET;
                if(pendingLocation != location || connectedNodeRank == location){
                    x = (nodeCountMap[location].first * X_OFFSET) - RADIUS;
                    
                    auto nonConstCommunication = const_cast<Communication*>(node.first->getCommunication());                
                    qreal fromX = x + RADIUS;
                    qreal fromY = y + RADIUS;

                    qreal toX = std::get<0>(pendingEdges[node.first->getCommunication()]) + RADIUS;
                    qreal toY = std::get<1>(pendingEdges[node.first->getCommunication()]) + RADIUS;

                    if(!region.contains("send", Qt::CaseInsensitive)){
                        fromX = toX;
                        fromY = toY;

                        toX = x + RADIUS;
                        toY = y + RADIUS;
                    }

                    auto arrow = new CommunicationIndicator(nonConstCommunication, fromX, fromY, toX, toY);
                    arrow->setZValue(layers::Z_LAYER_P2P_COMMUNICATIONS);
                    arrow->setPen(QPen(Qt::black));
                    arrow->setOnSelected(onTimedElementSelected);
                    scene->addItem(arrow);                 
                }
            }
            
            
            // Node is collective communication 
            if(node.first->hasCollectiveCommunication()){
                color = COLLECTIVES_COLOR;          
                const auto collectiveCommunication = node.first->getCollectiveCommunication();
                int lineLength = 2*RADIUS;
                int yStart = y;
                int yEnd = yStart + lineLength;

                if(pendingCollectives.find(collectiveCommunication) == pendingCollectives.end()) pendingCollectives[collectiveCommunication] = std::make_tuple(count, yStart, yEnd, location, std::set<uint16_t>());
                std::set<uint16_t>& membersSet = std::get<IDX_FINISHED_MEMBERS>(pendingCollectives[collectiveCommunication]);

                std::get<IDX_MAX_NODE_COUNT>(pendingCollectives[collectiveCommunication]) = std::max(count, std::get<IDX_MAX_NODE_COUNT>(pendingCollectives[collectiveCommunication]));          
                std::get<IDX_START_Y>(pendingCollectives[collectiveCommunication]) = std::min(yStart, std::get<IDX_START_Y>(pendingCollectives[collectiveCommunication]));            
                std::get<IDX_END_Y>(pendingCollectives[collectiveCommunication]) = std::max(yEnd, std::get<IDX_END_Y>(pendingCollectives[collectiveCommunication]));
                
                membersSet.insert(location);

                // auto collectiveType = collectiveCommunication->getOperation();
                // if(collectiveType == otf2::collective_type::reduce){
                //     std::cout << "DEBUG HERE" << std::endl;
                // }
                
                if(node.first->getCommunicationKind()==CommunicationKind::Synchronizing){
                    if(location_ == std::get<IDX_INITIAL_LOCATION>(pendingCollectives[node.first->getCollectiveCommunication()])){ 
                                       
                        for(auto &member: collectiveCommunication->getMembers()){
                            auto memberLocation = member->getLocation()->ref();
                            if(membersSet.find(memberLocation) == membersSet.end()){
                                if(!node.first->getConnectedNodes().empty()){                                
                                    auto matchingNodes = std::make_pair(node.first, node.first->getConnectedNodes()[memberLocation]);
                                    drawNodes(nodes_, nodeCountMap, pendingEdges, pendingCollectives, scene, memberLocation, &matchingNodes);
                                } else drawNodes(nodes_, nodeCountMap, pendingEdges, pendingCollectives, scene, memberLocation);
                            }                                 
                        }
                    }
                }
                
                std::get<IDX_MAX_NODE_COUNT>(pendingCollectives[collectiveCommunication]) = std::get<IDX_MAX_NODE_COUNT>(pendingCollectives[collectiveCommunication]);                 
                std::get<IDX_START_Y>(pendingCollectives[collectiveCommunication]) = std::min(yStart, std::get<IDX_START_Y>(pendingCollectives[collectiveCommunication]));                
                std::get<IDX_END_Y>(pendingCollectives[collectiveCommunication]) = std::max(yEnd, std::get<IDX_END_Y>(pendingCollectives[collectiveCommunication]));             
                                
                count = std::get<IDX_MAX_NODE_COUNT>(pendingCollectives[node.first->getCollectiveCommunication()]);
                x = (count * X_OFFSET) - RADIUS;
                count++;

            }
           
            
            
            if(node.second) continue;
                        
            auto ellipseItem = new NodeIndicator(node.first->getSlot(), x, y, 2*RADIUS, 2*RADIUS);
            //ellipseItem->setOnDoubleClick(onTimedElementDoubleClicked);
            ellipseItem->setOnSelected(onTimedElementSelected);

            // QBrush brush(node.first->getColor());
            QBrush brush(color);
            ellipseItem->setBrush(brush);

            QString hoverText = region;
            ellipseItem->setToolTip(hoverText);
            ellipseItem->setAcceptHoverEvents(true);

            if(node.first->hasCommunication() && (node.first->getCommunicationKind() & CommunicationKind::BlockingPointToPoint)){
                QPen pen(COLLECTIVES_COLOR);
                pen.setWidth(2);
                ellipseItem->setPen(pen);
            }

            // if(node.first->hasCommunication() && (node.first->getCommunicationKind() & CommunicationKind::NonBlockingPointToPoint)){
            //     QGraphicsEllipseItem* coverPattern = new QGraphicsEllipseItem(x, y, 2*RADIUS, 2*RADIUS);
            //     QPixmap patternPixmap(16, 16); 
            //     patternPixmap.fill(color); 

            //     QBrush patternBrush(patternPixmap);
            //     patternBrush.setStyle(Qt::DiagCrossPattern);
            //     coverPattern->setBrush(patternBrush);
            //     scene->addItem(coverPattern);
            //     coverPattern->setZValue(5);
            // }           

            node.second = true;
            nodeCountMap[location].second++;
            if(node.first->hasCollectiveCommunication()){
                if(!(node.first->getCommunicationKind()==CommunicationKind::Synchronizing)) scene->addItem(ellipseItem);

                if(matchingNodes !=nullptr){
                    auto initialLocation = std::get<IDX_INITIAL_LOCATION>(pendingCollectives[node.first->getCollectiveCommunication()]);                
                    if(node.first->getCollectiveCommunication() == matchingNodes->first->getCollectiveCommunication() && initialLocation != location_) return;                    
                }

            }else{
                if(matchingNodes !=nullptr){
                    scene->addItem(ellipseItem);
                   
                    if(node.first == matchingNodes->second && location_ != 0){
                        // count++;
                        return;
                    }
                }else scene->addItem(ellipseItem);
            count++;
            }    
        }
    }

}