/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright 2016 Technische Universitaet Berlin
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
 */

// - TCP Stream server and user-defined number of clients connected with an AP
// - WiFi connection
// - Tracing of throughput, packet information is done in the client

#include "ns3/point-to-point-helper.h"
#include <fstream>
#include "ns3/core-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include <ns3/buildings-module.h>
#include "ns3/building-position-allocator.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include "ns3/flow-monitor-module.h"
#include "ns3/tcp-stream-helper.h"
#include "ns3/tcp-stream-interface.h"
#include "ns3/traffic-control-module.h"
#include "ns3/opengym-module.h"
#include "ns3/csma-module.h"

template <typename T>
std::string ToString(T val)
{
    std::stringstream stream;
    stream << val;
    return stream.str();
}

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("TcpStreamExample");

void
CourseChange (std::string context, Ptr<const MobilityModel> model)
{
  Vector position = model->GetPosition ();
  NS_LOG_UNCOND (context << " time : " << Now().GetSeconds()  <<
    " x = " << position.x << ", y = " << position.y);
}




int
main (int argc, char *argv[])
{
//
// Users may find it convenient to turn on explicit debugging
// for selected modules; the below lines suggest how to do this
//
// #if 1
//   LogComponentEnable ("TcpStreamExample", LOG_LEVEL_INFO);
//   LogComponentEnable ("TcpStreamClientApplication", LOG_LEVEL_INFO);
//   LogComponentEnable ("TcpStreamServerApplication", LOG_LEVEL_INFO);
// #endif

  uint64_t segmentDuration = 2000000;
  // The simulation id is used to distinguish log file results from potentially multiple consequent simulation runs.
  uint32_t simulationId = 1;
  
  uint32_t pandaClients = 0;
  uint32_t festiveClients = 0;
  uint32_t tobascoClients = 0;
  uint32_t ns3gymClients = 0;

  float firstThroughputChange = 0.0;
  float firstThroughputChangeTime = 40.0;
  float secondThroughputChange = 0.0;
  float secondThroughputChangeTime = 80.0;

  uint32_t openGymPort = 5555;
  Ptr<OpenGymInterface> openGymInterface;
 
  bool moving = false;

  std::string adaptationAlgo = "ns3gym";
  std::string segmentSizeFilePath = "contrib/dash-meets-ns3gym/segmentSizes.txt";
  uint32_t linkRate = 100000;
  uint32_t wifiRate = 0;
  bool shortGuardInterval = true;

  CommandLine cmd;
  cmd.Usage ("Simulation of streaming with DASH.\n");
  cmd.AddValue ("simulationId", "The simulation's index (for logging purposes)", simulationId);

  cmd.AddValue ("pandaClients", "The number of Panda clients", pandaClients);
  cmd.AddValue ("festiveClients", "The number of Festive clients", festiveClients);
  cmd.AddValue ("tobascoClients", "The number of Tobasco clients", tobascoClients);
  cmd.AddValue ("ns3gymClients", "The number of Ns3gym clients", ns3gymClients);

  cmd.AddValue ("moving", "Moving or Stationary", moving);

  cmd.AddValue ("firstThroughputChange", "Percentage throughput change", firstThroughputChange);
  cmd.AddValue ("firstThroughputChangeTime", "Time (s) of first throughput change", firstThroughputChangeTime);
  cmd.AddValue ("secondThroughputChange", "Percentage throughput change", secondThroughputChange);
  cmd.AddValue ("secondThroughputChangeTime", "Time (s) of first throughput change", secondThroughputChangeTime);

  cmd.AddValue ("segmentDuration", "The duration of a video segment in microseconds", segmentDuration);
  cmd.AddValue ("segmentSizeFile", "The relative path (from ns-3.x directory) to the file containing the segment sizes in bytes", segmentSizeFilePath);
  cmd.AddValue ("linkRate", "Traffic speed from server", linkRate);
  cmd.AddValue ("wifiRate", "Traffic speed from server", wifiRate);
  
  cmd.Parse (argc, argv);

  std::string temp;  
  //vector<string> adaptationAlgos;
  std::vector<std::string> adaptationAlgos; ///< list of attributes
  
  
  for (uint i =0; i < pandaClients; i++) {
      adaptationAlgos.push_back("panda");
  }
  for (uint i =0; i < festiveClients; i++) {
      adaptationAlgos.push_back("festive");
  }
  for (uint i =0; i < tobascoClients; i++) {
      adaptationAlgos.push_back("tobasco");
  }
  for (uint i =0; i < ns3gymClients; i++) {
      adaptationAlgos.push_back("ns3gym");
  }
  
  if (ns3gymClients > 0) {
      openGymInterface = CreateObject<OpenGymInterface> (openGymPort);
  }
  

  uint32_t numberOfClients = ns3gymClients + tobascoClients + festiveClients + pandaClients;

  
  Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue (1446));
  //Config::SetDefault("ns3::TcpSocket::SndBufSize", UintegerValue (524288));
  //Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue (524288));
  Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpNewReno"));


  WifiHelper wifiHelper;
  wifiHelper.SetStandard (WIFI_PHY_STANDARD_80211n_5GHZ);
  //wifiHelper.SetStandard (WIFI_PHY_STANDARD_80211b);
  if (wifiRate == 0 ) {
    wifiHelper.SetRemoteStationManager ("ns3::MinstrelHtWifiManager");//
  } else {
    wifiHelper.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue ("HtMcs24"));
  }


  /* Set up Legacy Channel */
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
  
  // We do not set an explicit propagation loss model here, we just use the default ones that get applied with the building model.

  /* Setup Physical Layer */
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO);
  wifiPhy.Set ("TxPowerStart", DoubleValue (20.0));//
  wifiPhy.Set ("TxPowerEnd", DoubleValue (20.0));//
  wifiPhy.Set ("TxPowerLevels", UintegerValue (1));//
  wifiPhy.Set ("TxGain", DoubleValue (0));//
  wifiPhy.Set ("RxGain", DoubleValue (0));//
  wifiPhy.SetErrorRateModel ("ns3::YansErrorRateModel");//
  wifiPhy.SetChannel (wifiChannel.Create ());
  wifiPhy.Set("ShortGuardEnabled", BooleanValue(shortGuardInterval));
  wifiPhy.Set("ChannelWidth",UintegerValue (20) );
  wifiPhy.Set ("Antennas", UintegerValue (4));
  wifiPhy.Set ("MaxSupportedTxSpatialStreams", UintegerValue (4));
  wifiPhy.Set ("MaxSupportedRxSpatialStreams", UintegerValue (4));
  // wifiPhy.Set ("RxAntennas", UintegerValue (4));

  /* Create Nodes */
  NodeContainer clientNodes;
  clientNodes.Create (numberOfClients );

  /* Determin access point and server node */
  NodeContainer p2pNodes;
  p2pNodes.Create (2);
  Ptr<Node> apNode = p2pNodes.Get (0);

  NodeContainer csmaNodes;
  csmaNodes.Add (p2pNodes.Get (1));
  csmaNodes.Create (numberOfClients);

  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

  NetDeviceContainer csmaDevices;
  csmaDevices = csma.Install (csmaNodes);


  /* Configure clients as STAs in the WLAN */
  NodeContainer staContainer;
  /* Begin at +2, because position 0 is the access point and position 1 is the server */
  for (NodeContainer::Iterator i = clientNodes.Begin () ; i != clientNodes.End (); ++i)
    {
      staContainer.Add (*i);
    }

  /* Determin client nodes for object creation with client helper class */
  std::vector <std::pair <Ptr<Node>, std::string> > clients;
  int algoIndex = 0;
  for (NodeContainer::Iterator i = clientNodes.Begin () ; i != clientNodes.End (); ++i)
    {
      
      std::pair <Ptr<Node>, std::string> client (*i, adaptationAlgos.at(algoIndex) );
      clients.push_back (client);
      algoIndex ++;
    }

  /* Set up WAN link between server node and access point*/
  PointToPointHelper p2p;
  //p2p.SetDeviceAttribute ("DataRate", StringValue ("100000kb/s")); // This must not be more than the maximum throughput in 802.11n
  p2p.SetDeviceAttribute ("DataRate", StringValue (ToString(linkRate) +"kb/s")); // This must not be more than the maximum throughput in 802.11n
  p2p.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2p.SetChannelAttribute ("Delay", StringValue ("45ms"));
  NetDeviceContainer wanIpDevices;
  wanIpDevices = p2p.Install (p2pNodes);



  /* create MAC layers */
  WifiMacHelper wifiMac;
  /* WLAN configuration */
  Ssid ssid = Ssid ("network");
  /* Configure STAs for WLAN*/

  wifiMac.SetType ("ns3::StaWifiMac",
                    "Ssid", SsidValue (ssid));
  NetDeviceContainer staDevices;
  staDevices = wifiHelper.Install (wifiPhy, wifiMac, staContainer);

  /* Configure AP for WLAN*/
  wifiMac.SetType ("ns3::ApWifiMac",
                    "Ssid", SsidValue (ssid));
  NetDeviceContainer apDevice;
  apDevice = wifiHelper.Install (wifiPhy, wifiMac, apNode);


  Config::Set ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/ChannelWidth", UintegerValue (40));

  /* Determin WLAN devices (AP and STAs) */
  NetDeviceContainer wlanDevices;
  wlanDevices.Add (staDevices);
  wlanDevices.Add (apDevice);

  /* Internet stack */
  InternetStackHelper stack;
  stack.Install (clientNodes);
  stack.Install (apNode);
  stack.Install (csmaNodes);


  /* Assign IP addresses */
  Ipv4AddressHelper address;

  /* IPs for WAN */
  address.SetBase ("76.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer wanInterface = address.Assign (wanIpDevices);
  

  address.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer csmaInterfaces;
  csmaInterfaces = address.Assign (csmaDevices);

  /* IPs for WLAN (STAs and AP) */
  address.SetBase ("192.168.1.0", "255.255.255.0");
  address.Assign (wlanDevices);

  /* Populate routing table */
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  uint16_t port = 9;


//////////////////////////////////////////////////////////////////////////////////////////////////
//// Set up Building
//////////////////////////////////////////////////////////////////////////////////////////////////
  double roomHeight = 3;
  double roomLength = 6;
  double roomWidth = 5;
  uint32_t xRooms = 8;
  uint32_t yRooms = 3;
  uint32_t nFloors = 6;

  Ptr<Building> b = CreateObject <Building> ();
  b->SetBoundaries (Box ( 0.0, xRooms * roomWidth,
                          0.0, yRooms * roomLength,
                          0.0, nFloors * roomHeight));
  b->SetBuildingType (Building::Office);
  b->SetExtWallsType (Building::ConcreteWithWindows);
  b->SetNFloors (6);
  b->SetNRoomsX (8);
  b->SetNRoomsY (3);

  Vector posAp = Vector ( 1.0, 1.0, 1.0);
  // give the server node any position, it does not have influence on the simulation, it has to be set though,
  // because when we do: mobility.Install (networkNodes);, there has to be a position as place holder for the server
  // because otherwise the first client would not get assigned the desired position.
  Vector posServer = Vector (1.5, 1.5, 1.5);

  /* Set up positions of nodes (AP and server) */
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (posAp);
  positionAlloc->Add (posServer);


  Ptr<RandomRoomPositionAllocator> randPosAlloc = CreateObject<RandomRoomPositionAllocator> ();
  randPosAlloc->AssignStreams (simulationId);

  // create folder so we can log the positions of the clients
  const char * mylogsDir = dashLogDirectory.c_str();
  mkdir (mylogsDir, 0775);
  std::string algodirstr (dashLogDirectory +  adaptationAlgo );  
  const char * algodir = algodirstr.c_str();
  mkdir (algodir, 0775);
  std::string dirstr (dashLogDirectory + adaptationAlgo + "/" + ToString (numberOfClients) + "/");
  const char * dir = dirstr.c_str();
  mkdir(dir, 0775);

  std::ofstream clientPosLog;
  std::string clientPos = dashLogDirectory +  adaptationAlgo + "/" + ToString (numberOfClients) + "/" + "sim" + ToString (simulationId) + "_"  + "clientPos.txt";
  //std::string dLog = dashLogDirectory + m_algoName + "/" +  numberOfClients  + "/sim" + simulationId + "_" + "cl" + clientId + "_"  + "downloadLog.txt";

  clientPosLog.open (clientPos.c_str());
  NS_ASSERT_MSG (clientPosLog.is_open(), "Couldn't open clientPosLog file");

  NS_LOG_UNCOND("here");
  
  // allocate clients to positions
  for (uint i = 0; i < numberOfClients; i++)
    {
      Vector pos = Vector (randPosAlloc->GetNext());
      positionAlloc->Add (pos);

      // log client positions
      clientPosLog << ToString(pos.x) << ", " << ToString(pos.y) << ", " << ToString(pos.z) << "\n";
      clientPosLog.flush ();
    }


  MobilityHelper mobility;
  mobility.SetPositionAllocator (positionAlloc);

  if (moving) {
    //mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel", "Bounds", RectangleValue (Rectangle ( 0.0, xRooms * roomWidth, 0.0, yRooms * roomLength)));
    mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel", "Distance", StringValue("5"), "Bounds", StringValue("0|30|0|18"));
  } else {;
    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  }
  
  // install all of the nodes that have been added to positionAlloc to the mobility model
  mobility.Install (staContainer);

  NS_LOG_UNCOND("here");

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (apNode);
  //BuildingsHelper::Install (clientNodes); // networkNodes contains all nodes, stations and ap
  //BuildingsHelper::MakeMobilityModelConsistent ();

  // if logging of the packets between AP---Server or AP and the STAs is wanted, these two lines can be activated

  p2p.EnablePcapAll ("p2p-", true);
  wifiPhy.EnablePcapAll ("wifi-", true);

  NS_LOG_UNCOND("here");

  TcpStreamServerHelper serverHelper (port);
  ApplicationContainer serverApps;
  for (uint i = 0; i < numberOfClients; i++)
    {
      serverApps.Add(serverHelper.Install (csmaNodes.Get(i+1)));
      serverApps.Get(i)->SetStartTime (Seconds (1.0));
    }
  NS_LOG_UNCOND("here");

  /* Install TCP/UDP Transmitter on the station */
  TcpStreamClientHelper clientHelper (Address(csmaInterfaces.GetAddress (1)), port);
  clientHelper.SetAttribute ("SegmentDuration", UintegerValue (segmentDuration));
  clientHelper.SetAttribute ("SegmentSizeFilePath", StringValue (segmentSizeFilePath));
  clientHelper.SetAttribute ("NumberOfClients", UintegerValue(numberOfClients));
  clientHelper.SetAttribute ("SimulationId", UintegerValue (simulationId));
  ApplicationContainer clientApps = clientHelper.Install (clients, openGymInterface);
  for (uint i = 0; i < clientApps.GetN (); i++)
    {      
      double startTime = 2.0 + ((i * 3) / 15.0);
      clientApps.Get (i)->SetStartTime (Seconds (startTime));
      clientApps.Get (i)->SetAttribute("RemoteAddress", AddressValue (Address(csmaInterfaces.GetAddress (i+1))));
    }

  Simulator::Schedule (Seconds(firstThroughputChangeTime), 
    Config::Set, "/NodeList/1/DeviceList/0/$ns3::PointToPointNetDevice/DataRate",
    StringValue (ToString(linkRate * (1 - firstThroughputChange)) +"kb/s"));
  Simulator::Schedule (Seconds(secondThroughputChangeTime), 
    Config::Set, "/NodeList/1/DeviceList/0/$ns3::PointToPointNetDevice/DataRate",
    StringValue (ToString(linkRate * (1 - secondThroughputChange)) +"kb/s"));


  NS_LOG_INFO ("Run Simulation.");
  NS_LOG_INFO ("Sim: " << simulationId << "Clients: " << numberOfClients);
  Simulator::Run ();

  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");

}