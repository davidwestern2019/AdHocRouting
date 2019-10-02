/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 The Boeing Company
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program; if not, write to the Free Software
  * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
  *
  */

// custom aodv implementation
#include "ns3/aodv_sim.h"

#define TRANS_POWER       10
#define RX_GAIN           10

#define minSpeed_mpers    2
#define maxSpeed_mpers    4
#define xSize_m           300
#define ySize_m           300

#define NUM_NODES         5

#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/packet.h"

#include <iostream>

using namespace ns3;

NodeContainer c;
AODVns3* aodvArray[NUM_NODES];

void testAodv()
{
  // Test sending from node 1 to node 3
  string msg = "Hello world";
  char* buffer = (char*)(malloc(msg.length() + 5));
  uint8_t temp = 0x00;

  Ptr<Ipv4> nodeIpv4 = c.Get(1)->GetObject<Ipv4>(); // Get Ipv4 instance of the node
  Ipv4Address dest = nodeIpv4->GetAddress (1, 0).GetLocal(); 
   // add aodv object 
  uint8_t* ipBuf = (uint8_t*)(malloc(4)); 
  dest.Serialize(ipBuf);

  // TODO: Do this the right way
  temp = *ipBuf;
  ipBuf[0] = ipBuf[3];
  ipBuf[3] = temp;
  temp = ipBuf[1];
  ipBuf[1] = ipBuf[2];
  ipBuf[2] = temp;

  IP_ADDR destIp;
  memcpy(&(destIp),(ipBuf),4);

  temp = 0x00;
  memcpy(buffer, &temp, 1);
  buffer++;
  memcpy(buffer, &destIp, 4);
  buffer--;

  aodvArray[1]->sendPacket(buffer, msg.length() + 5, aodvArray[3]->getIp());// aodvArray[1]->getIp(), 654, aodvArray[3]->getIp());
}

int SendPacket(char* buffer, int length, IP_ADDR dest, int port, IP_ADDR source)
{
  cout << "Sending a packet from " << getStringFromIp(source) << " to " << getStringFromIp(dest) << endl;

  // TODO: do this right.
  string ipString = getStringFromIp(source);
  int index = int(ipString.at(ipString.length()-1)-'0');
  Ptr<Node> sourceNode = c.Get(index);
  ipString = getStringFromIp(dest);

  if (getStringFromIp(dest).substr(ipString.length()-3) == "255")
  {
    Ptr<Ipv4> destIpv4 = c.Get(index)->GetObject<Ipv4>(); // Get Ipv4 instance of the node
    Ipv4Address destAddr = destIpv4->GetAddress (1, 0).GetLocal();  
    Ptr<Socket> socket = Socket::CreateSocket(sourceNode, UdpSocketFactory::GetTypeId());
    InetSocketAddress remote = InetSocketAddress(destAddr, 654);
    socket->SetAllowBroadcast(true);
    socket->Connect(remote);  // Test sending from node 1 to node 3
    Ptr<Packet> packet = Create<Packet>(reinterpret_cast<const uint8_t*> (buffer), length); 
    UdpHeader header;
    // fill header
    packet->AddHeader(header);

    socket->Send(packet);
  }
  else 
  {
    ipString = getStringFromIp(dest);
    index = int(ipString.at(ipString.length()-1)-'0');

    Ptr<Ipv4> destIpv4 = c.Get(index)->GetObject<Ipv4>(); // Get Ipv4 instance of the node
    Ipv4Address destAddr = destIpv4->GetAddress (1, 0).GetLocal();  
    Ptr<Socket> socket = Socket::CreateSocket(sourceNode, UdpSocketFactory::GetTypeId());
    InetSocketAddress remote = InetSocketAddress(destAddr, 654);
    socket->SetAllowBroadcast(false);
    socket->Connect(remote);  // Test sending from node 1 to node 3
    Ptr<Packet> packet = Create<Packet>(reinterpret_cast<const uint8_t*> (buffer), length); 
    UdpHeader header;
    // fill header
    packet->AddHeader(header);

    socket->Send(packet);
  }
  
   return 0;
}

void ReceivePacket (Ptr<Socket> socket)
{
  Ptr<Packet> packet;
  while (packet = socket->Recv ())
    {
      NS_LOG_UNCOND ("Received one packet! On Node " + to_string(socket->GetNode()->GetId()));
      UdpHeader header;
      packet->RemoveHeader(header);

      uint32_t length = packet->GetSize();
      uint8_t* packetBuffer = (uint8_t*)(malloc(length));

      packet->CopyData(packetBuffer, length);
//      for (uint32_t i = 0; i < length; i++)
//        cout << packetBuffer[i];

      Ptr<Ipv4> ipv4 = socket->GetNode()->GetObject<Ipv4> (); // Get Ipv4 instance of the node
      Ipv4Address addr = ipv4->GetAddress (1, 0).GetLocal ();  
      // add aodv object 
      uint8_t* ipBuf = (uint8_t*)(malloc(4)); 
      addr.Serialize(ipBuf);

      // TODO: Do this the right way
      uint8_t temp;
      temp = *ipBuf;
      ipBuf[0] = ipBuf[3];
      ipBuf[3] = temp;
      temp = ipBuf[1];
      ipBuf[1] = ipBuf[2];
      ipBuf[2] = temp;

      IP_ADDR source;
      memcpy(&(source),(ipBuf),4);

      aodvArray[socket->GetNode()->GetId()]->decodeReceivedPacketBuffer((char*)(packetBuffer), length, source);
    }
}

void SendHello(Ptr<Node> source, Ipv4Address dest)
{
  Ptr<Socket> socket = Socket::CreateSocket(source, UdpSocketFactory::GetTypeId());
  InetSocketAddress remote = InetSocketAddress(dest, 654);
  socket->SetAllowBroadcast(false);
  socket->Connect(remote);

  string msg = "Hello world";
  char* data = (char*)(malloc(msg.length() + 5));
  uint8_t temp = 0x00;

   // add aodv object 
  uint8_t* ipBuf = (uint8_t*)(malloc(4)); 
  dest.Serialize(ipBuf);

  // TODO: Do this the right way
  temp = *ipBuf;
  ipBuf[0] = ipBuf[3];
  ipBuf[3] = temp;
  temp = ipBuf[1];
  ipBuf[1] = ipBuf[2];
  ipBuf[2] = temp;

  IP_ADDR destIp;
  memcpy(&(destIp),(ipBuf),4);

  temp = 0x00;
  memcpy(data, &temp, 1);
  data++;
  memcpy(data, &destIp, 4);
  data--;

  for (uint32_t i = 0; i < msg.length(); i++)
    data[i+5] = msg.at(i);

  Ptr<Packet> packet = Create<Packet>(reinterpret_cast<const uint8_t*> (data), msg.length()+5);
  UdpHeader header;
  // fill header
  packet->AddHeader(header);

  socket->Send(packet);
}

void GenerateTraffic (NodeContainer c, uint32_t pktSize,
                             uint32_t pktCount, Time pktInterval )
{
  for (uint32_t i = 0; i < c.GetN(); i++)
  {
    Ptr<Ipv4> ipv4 = c.Get(i)->GetObject<Ipv4>(); // Get Ipv4 instance of the node
    Ipv4Address addr = ipv4->GetAddress (1, 0).GetLocal();  
    Simulator::Schedule(Seconds(i), &SendHello, c.Get(0), addr);
    NS_LOG_UNCOND("Node 0 to Node " + to_string(i));
  }
}

int main (int argc, char *argv[])
{
  std::string phyMode ("DsssRate1Mbps");
  double rss = -80;  // -dBm
  uint16_t numNodes = NUM_NODES;

  double duration = 60.00;

  CommandLine cmd;

  cmd.AddValue ("phyMode", "Wifi Phy mode", phyMode);
  cmd.AddValue ("rss", "received signal strength", rss);
//  cmd.AddValue ("packetSize", "size of application packet sent", packetSize);
//  cmd.AddValue ("numPackets", "number of packets generated", numPackets);
//  cmd.AddValue ("interval", "interval (seconds) between packets", interval);
 
  cmd.Parse (argc, argv);
  // Convert to time object
  //Time interPacketInterval = Seconds (interval);

  // disable fragmentation for frames below 2200 bytes
  Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("2200"));
  // turn off RTS/CTS for frames below 2200 bytes
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("2200"));
  // Fix non-unicast data rate to be the same as that of unicast
  Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode",
                      StringValue (phyMode));

  c.Create (numNodes);

  // The below set of helpers will help us to put together the wifi NICs we want
  WifiHelper wifi;
/*  if (verbose)
    {
      wifi.EnableLogComponents ();  // Turn on all Wifi logging
    }
*/
  wifi.SetStandard (WIFI_PHY_STANDARD_80211b);

  YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
  // This is one parameter that matters when using FixedRssLossModel
  // set it to zero; otherwise, gain will be added
  wifiPhy.Set ("RxGain", DoubleValue (RX_GAIN) );
  // ns-3 supports RadioTap and Prism tracing extensions for 802.11b
  wifiPhy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11_RADIO);

  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  // The below FixedRssLossModel will cause the rss to be fixed regardless
  // of the distance between the two stations, and the transmit power
  wifiChannel.AddPropagationLoss("ns3::LogDistancePropagationLossModel");
  wifiPhy.SetChannel (wifiChannel.Create ());

  wifiPhy.Set("TxPowerStart", DoubleValue(TRANS_POWER));
  wifiPhy.Set("TxPowerEnd", DoubleValue(TRANS_POWER));

  // Add a mac and disable rate control
  WifiMacHelper wifiMac;
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode",StringValue (phyMode),
                                "ControlMode",StringValue (phyMode));
  // Set it to adhoc mode
  wifiMac.SetType ("ns3::AdhocWifiMac");
  NetDeviceContainer devices = wifi.Install (wifiPhy, wifiMac, c);

  // Note that with FixedRssLossModel, the positions below are not
  // used for received signal strength.
  MobilityHelper mobility;

  std::string xBounds = "ns3::UniformRandomVariable[Min=" + std::to_string(-1 * xSize_m/2)
    + "|Max=" + std::to_string(xSize_m/2) + "]";
  std::string yBounds = "ns3::UniformRandomVariable[Min=" + std::to_string(-1 * ySize_m/2)
    + "|Max=" + std::to_string(ySize_m/2) + "]";

  ObjectFactory positionModel;
  positionModel.SetTypeId("ns3::RandomRectanglePositionAllocator");
  positionModel.Set("X", StringValue(xBounds));
  positionModel.Set("Y", StringValue(yBounds));
  Ptr<PositionAllocator> positionAllocPtr = positionModel.Create()->GetObject<PositionAllocator>();
  mobility.SetPositionAllocator(positionAllocPtr);

/* Keep nodes immobile for initial tests 
  std::string speedInput = "ns3::UniformRandomVariable[Min=" + std::to_string(minSpeed_mpers) + "|Max=" + std::to_string(maxSpeed_mpers) + "]";
  // set mobility model for adhoc nodes
  mobility.SetMobilityModel("ns3::RandomWaypointMobilityModel",
    "Speed", StringValue(speedInput),
    "Pause", StringValue("ns3::ConstantRandomVariable[Constant=0.5]"),
    "PositionAllocator", PointerValue(positionAllocPtr));
*/

  mobility.Install (c);

  InternetStackHelper internet;
  internet.Install (c);

  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("192.168.1.0", "255.255.255.0");
  ipv4.Assign (devices);

  for(int i = 0; i < numNodes; i++){
    Ptr<Socket> recvSink = Socket::CreateSocket (c.Get (i), UdpSocketFactory::GetTypeId ());
    InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), 654);
    recvSink->Bind (local);
    recvSink->SetRecvCallback (MakeCallback (&ReceivePacket));
//    c.Get(i)->SetAttribute("id", IntegerValue(i));

    Ptr<Ipv4> ipv4 = c.Get(i)->GetObject<Ipv4> (); // Get Ipv4 instance of the node
    Ipv4Address addr = ipv4->GetAddress (1, 0).GetLocal ();  
    // add aodv object 
    uint8_t* ipBuf = (uint8_t*)(malloc(4)); 
    addr.Serialize(ipBuf);

    // TODO: Do this the right way
    uint8_t temp;
    temp = *ipBuf;
    ipBuf[0] = ipBuf[3];
    ipBuf[3] = temp;
    temp = ipBuf[1];
    ipBuf[1] = ipBuf[2];
    ipBuf[2] = temp;

    IP_ADDR ip;
    memcpy(&(ip),(ipBuf),4);

    aodvArray[i] = new AODVns3(ip);
    aodvArray[i]->ns3SocketSendPacket = &SendPacket;
  }

  // Tracing
//  wifiPhy.EnablePcap ("wifi-simple-adhoc", devices);

//  Simulator::Schedule (Seconds (1.0), &GenerateTraffic,
//                       c, packetSize, numPackets, Seconds(1));

  Simulator::Schedule(Seconds(1.0), &testAodv);// &(aodvArray[1]->socketSendPacket), buffer, msg.length() + 5, aodvArray[1]->getIp(), 654, aodvArray[3]->getIp());

  Simulator::Stop (Seconds (duration + 10.0));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
