/*********************************
 * aodv.h
 *
 * Custom c++ implementation of the aodv routing protocol. 
 * 
 * Author: Josh Rands
 * Date: 9/4/2019
 ********************************/

/* aodv includes */
#include "aodv_routing_table.h"
#include "aodv_rreq.h"
#include "aodv_rrep.h"
#include "aodv_rerr.h"

#include "send_packet.h"

#include <vector>
#include <functional>
#include <queue>

class AODV : public RoutingProtocol
{
public:
	// default constructor 
	AODV();
	AODV(const char* ip);
	AODV(IP_ADDR ip);
	~AODV();

	// handle received data. if not in routing table, attempt local fix and then RERR 
	void receivePacket(char* packet, int length, IP_ADDR source);
	// try to send data to a destination - the next hop is determined from the routing table  
	void sendPacket(char* packet, int length, IP_ADDR finalDestination, IP_ADDR origIP = -1);

	static const int AODV_PORT = 8060;
	static const int DATA_PORT = 8080;

	// decode a received packet buffer from UPD port 654
	void decodeReceivedPacketBuffer(char* packet, int length, IP_ADDR source);

	// RREQ - Route Request 
	RREQHelper rreqHelper;
	// broadcast rreq to all neighbors
	void broadcastRREQBuffer(rreqPacket rreq);
	// make a decision on a received rreq packet using the rreq helper 
	void handleRREQ(char* buffer, int length, IP_ADDR source);

	// RREP - Route Reply
	RREPHelper rrepHelper;
	// handle a received rrep message 
	void handleRREP(char* buffer, int length, IP_ADDR source);

	// RERR - Route Error
	RERRHelper rerrHelper;
	// handle a received rerr message 
	void handleRERR(char* buffer, int length, IP_ADDR source);

	// Network Monitoring
	virtual void repairLink(IP_ADDR brokenLink, IP_ADDR finalDest, char* buffer, int length, IP_ADDR origIP, int port);
	virtual bool linkExists(IP_ADDR dest);
	virtual bool attemptLocalRepair(IP_ADDR brokenLink, IP_ADDR finalDest);
	virtual void getOneHopNeighbors();

	// output the current contents of the routing table 
	void logRoutingTable();

	// get the routing table 
	AODVRoutingTable* getTable() { return m_aodvTable; } 

	// abstract function to be overwritten by child class 
	virtual int socketSendPacket(char *buffer, int length, IP_ADDR dest, int port) = 0;

protected:
	// node sequence number. MUST increment on a route discovery
	uint32_t sequenceNum;
	// node rreq id. Incremented by one during route discovery
	uint32_t rreqID;
	// vector of one hop neighbors to this node. Can be from network monitoring, HELLO messages, etc
	vector<IP_ADDR> m_neighbors;
	// aodv routing table
	AODVRoutingTable* m_aodvTable;
	// map of rreq ids and their corresponding packet to be sent once the route is established
	map<IP_ADDR, queue<pair<char*, int>>> rreqPacketBuffer;
};

/* AODVTest class
 * For testing AODV with unit tests. */
class AODVTest : public AODV
{
public: 	
	// debugging values 
	static int globalPacketCount;
	static IP_ADDR lastNode; 
	static IP_ADDR lastReceive;

	AODVTest(IP_ADDR ip) : AODV(ip) {}
	AODVTest(const char* ip) : AODV(ip) {}
	~AODVTest() { m_physicalNeighbors.clear(); }

	int socketSendPacket(char *buffer, int length, IP_ADDR dest, int port);

	// Network Monitoring 

	// add/remove node to neighbor list
	void addNeighbor(AODVTest* node);
	void removeNeighbor(AODVTest* node);

	// return true if node is neighbor
	bool isNeighbor(AODVTest node);

	// see if packet was put in queue
	bool packetInRreqBuffer(IP_ADDR dest);

private:
	vector<AODVTest*> m_physicalNeighbors;

};
