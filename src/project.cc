#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include <cstdlib>

#include "../build/ns3/core-module.h"
#include "../build/ns3/network-module.h"
#include "../build/ns3/csma-module.h"
#include "../build/ns3/internet-module.h"
#include "../build/ns3/point-to-point-module.h"
#include "../build/ns3/applications-module.h"
#include "../build/ns3/ipv4-global-routing-helper.h"
#include "../build/ns3/olsr-helper.h"
#include "../build/ns3/global-route-manager.h"
#include "../build/ns3/mobility-module.h"
#include "../build/ns3/netanim-module.h"
#include "../build/ns3/assert.h"
#include "../build/ns3/flow-monitor-helper.h"
#include "../build/ns3/flow-monitor.h"
#include "../build/ns3/flow-classifier.h"
#include "../build/ns3/flow-monitor-module.h"

#define NODES 50
#define AUTONOUMOUS_SYSTEMS 5
#define LINKS 5
#define NODES_PER_AS NODES/AUTONOUMOUS_SYSTEMS
#define ANIMATION_TIME 10

using namespace ns3;


bool allTraversed(bool nodes[], int reference)
{
    for(int i=0; i<NODES_PER_AS; i++)
    {
        if(nodes[i]==false && i!=reference)
            return false;
    }
    return true;
}

void createRandomConnections(NodeContainer AS, NetDeviceContainer connections[][NODES_PER_AS], int* nConnections, CsmaHelper& csma)
{
    srand(4);

    for(int i=0; i<NODES_PER_AS; i++)
    {
        bool flagTraversed[NODES_PER_AS] = {false};
        bool flagConnMade[NODES_PER_AS] = {false};
        while(nConnections[i]<LINKS)
        {
            int r = rand()%NODES_PER_AS;
            flagTraversed[r]= true;
            if(allTraversed(flagTraversed, i))
                break;
            if(nConnections[r]<LINKS && r!=i && !flagConnMade[r])
            {
                connections[i][r] = csma.Install(NodeContainer(AS.Get(i), AS.Get(r)));
                nConnections[r]++;
                nConnections[i]++;
                flagConnMade[r] = true;
                std::cout<<"Connection between "<<i<<" and "<<r<<std::endl;
            }
        }
    }
}

void assignIPAddressesRandomly(NetDeviceContainer connections[NODES_PER_AS][NODES_PER_AS], std::string base, std::string mask)
{
    Ipv4AddressHelper ipv4;
    for(int i=0, subnet=1;i<NODES_PER_AS; i++)
        for(int j=0;j<NODES_PER_AS;j++)
        {
            if(connections[i][j].GetN()>0)
            {
                std::string address = base + "."+std::to_string(subnet)+".0";
                ipv4.SetBase(Ipv4Address(address.c_str()), Ipv4Mask(mask.c_str()));
                ipv4.Assign(connections[i][j]);
                subnet++;
            }
        }
}

void CalculateMetrics (FlowMonitorHelper &flowHelper)
{
    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowHelper.GetClassifier ());
    FlowMonitor::FlowStatsContainer stats = flowHelper.GetMonitor ()->GetFlowStats ();

    uint32_t totalPacketsSent = 0;
    uint32_t totalPacketsReceived = 0;
    uint32_t lostPackets = 0;
    int j = 0;
    float avgThroughput = 0;
    Time Jitter;
    Time Delay;

    for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator iter = stats.begin ();iter != stats.end (); ++iter)
    {
        Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (iter->first);

        NS_LOG_UNCOND ("Flow ID: " << iter->first);
        NS_LOG_UNCOND ("Source Address: " << t.sourceAddress
                                          << ", Destination Address: " << t.destinationAddress);
        NS_LOG_UNCOND ("Packets Sent: " << iter->second.txPackets);
        NS_LOG_UNCOND ("Packets Received: " << iter->second.rxPackets);

        totalPacketsSent += iter->second.txPackets;
        totalPacketsReceived += iter->second.rxPackets;
        lostPackets = lostPackets + (iter->second.txPackets - iter->second.rxPackets);
        avgThroughput = avgThroughput + (iter->second.rxBytes * 8.0 /
                                         (iter->second.timeLastRxPacket.GetSeconds () -
                                          iter->second.timeFirstTxPacket.GetSeconds ()) /
                                         1024);
        Delay = Delay + (iter->second.delaySum);
        Jitter = Jitter + (iter->second.jitterSum);
        j++;
    }

    avgThroughput = avgThroughput / j;
    NS_LOG_UNCOND ("--------Total results of the simulation----------" << std::endl);
    NS_LOG_UNCOND ("Total Sent Packets  = " << totalPacketsSent);
    NS_LOG_UNCOND ("Total Received Packets = " << totalPacketsReceived);
    NS_LOG_UNCOND ("Total Lost Packets = " << lostPackets);
    NS_LOG_UNCOND ("Packet Loss Ratio = " << ((lostPackets * 100) / totalPacketsSent) << "%");
    NS_LOG_UNCOND ("Packet Delivery Ratio = " << ((totalPacketsReceived * 100) / totalPacketsSent)
                                              << "%");
    NS_LOG_UNCOND ("Average Throughput = " << avgThroughput << " Kbps");
    NS_LOG_UNCOND ("End to End Delay = " << Delay);
    NS_LOG_UNCOND ("End to End Jitter Delay =" << Jitter);
    NS_LOG_UNCOND ("Total Flow IDs: " << j);
}


int main(int argc, char *argv[])
{
    // The below value configures the default behavior of global routing.
    // By default, it is disabled.  To respond to interface events, set to true
    Config::SetDefault ("ns3::Ipv4GlobalRouting::RespondToInterfaceEvents", BooleanValue (true));

    // Allow the user to override any of the defaults and the above
    // Bind ()s at run-time, via command-line arguments
    CommandLine cmd;
    cmd.Parse (argc, argv);

    // create Autonomous Systems
    NodeContainer AS1, AS2, AS3, AS4, AS5;
    AS1.Create(NODES_PER_AS);
    AS2.Create(NODES_PER_AS);
    AS3.Create(NODES_PER_AS);
    AS4.Create(NODES_PER_AS);
    AS5.Create(NODES_PER_AS);


    // Set up positions
    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.SetPositionAllocator ("ns3::UniformDiscPositionAllocator",
                                   "X", StringValue ("250.0"),
                                   "Y", StringValue ("250.0"),
                                   "rho", DoubleValue(50));
    mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                               "Mode", StringValue ("Time"),
                               "Time", StringValue ("0.003s"),
                               "Speed", StringValue ("ns3::ConstantRandomVariable[Constant=1000.0]"),
                               "Bounds", StringValue ("200|300|200|300"));
    mobility.Install (AS1);

    MobilityHelper mobility1;
    mobility1.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility1.SetPositionAllocator ("ns3::UniformDiscPositionAllocator",
                                   "X", StringValue ("250.0"),
                                   "Y", StringValue ("50.0"),
                                   "rho", DoubleValue(50));
    mobility1.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                                "Mode", StringValue ("Time"),
                                "Time", StringValue ("0.003s"),
                                "Speed", StringValue ("ns3::ConstantRandomVariable[Constant=1000.0]"),
                                "Bounds", StringValue ("200|300|0|100"));
    mobility1.Install (AS2);

    MobilityHelper mobility2;
    mobility2.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility2.SetPositionAllocator ("ns3::UniformDiscPositionAllocator",
                                   "X", StringValue ("250.0"),
                                   "Y", StringValue ("450.0"),
                                   "rho", DoubleValue(50));
    mobility2.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                               "Mode", StringValue ("Time"),
                               "Time", StringValue ("0.003s"),
                               "Speed", StringValue ("ns3::ConstantRandomVariable[Constant=1000.0]"),
                               "Bounds", StringValue ("200|300|400|500"));
    mobility2.Install (AS3);

    MobilityHelper mobility3;
    mobility3.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility3.SetPositionAllocator ("ns3::UniformDiscPositionAllocator",
                                   "X", StringValue ("50.0"),
                                   "Y", StringValue ("250.0"),
                                   "rho", DoubleValue(50));
    mobility3.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                               "Mode", StringValue ("Time"),
                               "Time", StringValue ("0.003s"),
                               "Speed", StringValue ("ns3::ConstantRandomVariable[Constant=1000.0]"),
                               "Bounds", StringValue ("0|100|200|300"));
    mobility3.Install (AS4);

    MobilityHelper mobility4;
    mobility4.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility4.SetPositionAllocator ("ns3::UniformDiscPositionAllocator",
                                   "X", StringValue ("450.0"),
                                   "Y", StringValue ("250.0"),
                                   "rho", DoubleValue(50));
    mobility4.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                               "Mode", StringValue ("Time"),
                               "Time", StringValue ("0.003s"),
                               "Speed", StringValue ("ns3::ConstantRandomVariable[Constant=1000.0]"),
                               "Bounds", StringValue ("400|500|200|300"));
    mobility4.Install (AS5);



    // configure CSMA
    CsmaHelper csma;
    csma.SetChannelAttribute ("DataRate", StringValue("5Mbps"));
    csma.SetChannelAttribute ("Delay", StringValue("6ms"));


    PointToPointHelper p2p;
    p2p.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
    p2p.SetChannelAttribute ("Delay", StringValue ("5ms"));

    // making connections among nodes in Autonomous Systems randomly

    NetDeviceContainer connectionsAS1[NODES_PER_AS][NODES_PER_AS];
    NetDeviceContainer connectionsAS2[NODES_PER_AS][NODES_PER_AS];
    NetDeviceContainer connectionsAS3[NODES_PER_AS][NODES_PER_AS];
    NetDeviceContainer connectionsAS4[NODES_PER_AS][NODES_PER_AS];
    NetDeviceContainer connectionsAS5[NODES_PER_AS][NODES_PER_AS];
    int nConnectionsAS1[NODES_PER_AS] = {0};
    int nConnectionsAS2[NODES_PER_AS] = {0};
    int nConnectionsAS3[NODES_PER_AS] = {0};
    int nConnectionsAS4[NODES_PER_AS] = {0};
    int nConnectionsAS5[NODES_PER_AS] = {0};

    createRandomConnections(AS1, connectionsAS1, nConnectionsAS1, csma);
    createRandomConnections(AS2, connectionsAS2, nConnectionsAS2, csma);
    createRandomConnections(AS3, connectionsAS3, nConnectionsAS3, csma);
    createRandomConnections(AS4, connectionsAS4, nConnectionsAS4, csma);
    createRandomConnections(AS5, connectionsAS5, nConnectionsAS5, csma);

    NetDeviceContainer link1 = p2p.Install(NodeContainer(AS1.Get(9), AS2.Get(1)));
    NetDeviceContainer link2 = p2p.Install(NodeContainer(AS1.Get(0), AS5.Get(0)));
    NetDeviceContainer link3 = p2p.Install(NodeContainer(AS5.Get(9), AS3.Get(8)));
    NetDeviceContainer link4 = p2p.Install(NodeContainer(AS3.Get(9), AS4.Get(2)));

    // Using Link State using OLSR
    OlsrHelper olsr;
    Ipv4StaticRoutingHelper staticRouting;

    Ipv4ListRoutingHelper routingProtocolsList;
    routingProtocolsList.Add(staticRouting, 0);
    routingProtocolsList.Add(olsr, 10);

    // install Internet
    InternetStackHelper internet;
    internet.Install (AS1);
    internet.Install (AS2);
    internet.Install (AS3);
    internet.Install (AS4);
    internet.Install (AS5);



    // Assign IP Addresses
    assignIPAddressesRandomly(connectionsAS1, "10.1", "255.255.255.0");
    assignIPAddressesRandomly(connectionsAS2, "10.2", "255.255.255.0");
    assignIPAddressesRandomly(connectionsAS3, "10.3", "255.255.255.0");
    assignIPAddressesRandomly(connectionsAS4, "10.4", "255.255.255.0");
    assignIPAddressesRandomly(connectionsAS5, "10.5", "255.255.255.0");

    Ipv4AddressHelper ipv4;
    ipv4.SetBase ("10.6.0.0", "255.255.255.0");
    ipv4.Assign(link1);
    ipv4.SetBase ("10.7.0.0", "255.255.255.0");
    ipv4.Assign(link2);
    ipv4.SetBase ("10.8.0.0", "255.255.255.0");
    ipv4.Assign(link3);
    ipv4.SetBase ("10.9.0.0", "255.255.255.0");
    ipv4.Assign(link4);

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();



    // RFC 863 discard port ("9") indicates packet should be thrown away
    // by the system.  We allow this silent discard to be overridden
    // by the PacketSink application.
    uint16_t port = 9;

    // Create the OnOff application to send UDP datagrams of size
    // 512 bytes (default) at a rate of 500 Kb/s (default) from n0
    OnOffHelper onoff ("ns3::UdpSocketFactory",
                       Address (InetSocketAddress (Ipv4Address ("10.4.8.2"), port)));
    onoff.SetConstantRate (DataRate ("500kb/s"));

    ApplicationContainer app = onoff.Install (AS2.Get (6));
    // Start the application
    app.Start (Seconds (1.0));
    app.Stop (Seconds (10.0));

    // Create a packet sink to receive these packets
    PacketSinkHelper sink ("ns3::UdpSocketFactory",
                           Address (InetSocketAddress (Ipv4Address::GetAny(), port)));
    app = sink.Install (AS4.Get (7));
    app.Start (Seconds (1.0));
    app.Stop (Seconds (10.0));

    AsciiTraceHelper ascii;
    Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream ("Trace/dynamic-global-routing.tr");
    p2p.EnableAsciiAll (stream);
    csma.EnableAsciiAll (stream);
    internet.EnableAsciiIpv4All (stream);


    AnimationInterface anim ("XML/project.xml");
    anim.SetMobilityPollInterval(Seconds(0.001));
    anim.EnablePacketMetadata(false);
    anim.SetStartTime(Seconds(1.0));
    anim.SetStopTime(Seconds(10.0));

    Simulator::Stop(Seconds(ANIMATION_TIME));

    Ptr<FlowMonitor> flowMonitor;
    FlowMonitorHelper flowHelper;
    flowMonitor = flowHelper.InstallAll ();

    Simulator::Run ();
    CalculateMetrics(flowHelper);
    Simulator::Destroy ();

}