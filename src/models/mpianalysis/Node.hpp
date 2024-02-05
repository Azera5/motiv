#ifndef MOTIV_NODE_HPP
#define MOTIV_NODE_HPP

#include <QColor>

#include "src/models/communication/CollectiveCommunicationEvent.hpp"
#include "src/models/communication/Communication.hpp"
#include "src/models/Slot.hpp"

class Node {
public:
    Node(Slot* slot);

public: //methods

// Slot related functions
Slot* getSlot();
uint64_t getLocation();
otf2::definition::string getRegionName();


// Communication related functions
bool hasCommunication();
bool hasCollectiveCommunication();

void setCommunication(const Communication *communication);
void setCollectiveCommunication(const CollectiveCommunicationEvent *collectiveCommunication);
void setCollectiveCommunicationMemberRef(const CollectiveCommunicationEvent::Member *member);


const Communication* getCommunication();
const CollectiveCommunicationEvent* getCollectiveCommunication();
const CollectiveCommunicationEvent::Member* getMemberRef();
CommunicationKind getCommunicationKind();

// only for Nodes with P2P communication
uint64_t getConnectedCommunicationRank();
const CommunicationEvent* getOwnEvent();
const CommunicationEvent* getConnectedEvent();


// Node related functions
void addConnectedNodes(Node* node);

void setColor();
QColor getColor();

std::map<uint64_t, Node*>& getConnectedNodes();

// Custom comparison operator to sort Node objects in ascending order of endTime
bool operator<(const Node& other) const;

private: //data
    Slot *slot;
    // QColor color;

    std::map<uint64_t, Node*> connectedNodes;

    const Communication *communication = nullptr;

    // Exists only for collective communication Nodes
    const CollectiveCommunicationEvent *collectiveCommunication = nullptr;
    const CollectiveCommunicationEvent::Member *member = nullptr;
};

#endif //MOTIV_NODE_HPP