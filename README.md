# Routing-in-Autonomous-Systems-NS3

This ns-3 simulation project demonstrates the dynamic global routing behavior in a network consisting of multiple autonomous systems (AS). The simulation involves the creation of five autonomous systems, each with a specified number of nodes. The nodes within each AS are connected randomly using CSMA channels, creating a dynamic network topology.

## Features

- **Autonomous Systems Configuration:** The project sets up five autonomous systems with a fixed number of nodes in each AS.
- **Random Network Topology:** The connections between nodes within each AS are established randomly, simulating dynamic link changes.
- **Mobility Models:** Nodes are assigned constant position and random walk 2D mobility models to introduce movement in the network.
- **Point-to-Point Links:** Additional point-to-point links between specific nodes in different ASs are established to create inter-AS connections.
- **Global Routing:** The simulation employs the Optimized Link State Routing (OLSR) protocol along with static routing to achieve global routing within the entire network.
- **Traffic Generation:** On-off applications are used to generate UDP traffic between nodes in different ASs.
- **Metrics Calculation:** The project calculates various metrics such as packet delivery ratio, packet loss ratio, average throughput, end-to-end delay, and end-to-end jitter.
- **Visualization:** The simulation results can be visualized using the NetAnim tool, and trace files are generated for further analysis.

<h2>Build Instructions (Ubuntu):</h2>
  1. Install NS-3 and all its dependencies<br/>
  2. Paste the project.cc in the scratch folder of ns-3.29 folder<br/>
  3. Comment out the line number 316 in the file ns-3.29/src/internet/model/global-route-manager-impl.cc<br/>
  4. Create directory named XML in ns-3.29 folder<br/>
  5. Run shell ./waf scratch/project.cc <br/>
  6. Open the XML/project.xml file in Network Animator

## Visualization

The network topology and node movements can be visualized using NetAnim. The animation is saved as an XML file (`project.xml`) during the simulation.

Feel free to customize the simulation parameters and network configuration based on your specific requirements.


