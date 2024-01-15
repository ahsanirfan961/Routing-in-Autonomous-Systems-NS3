# Routing-in-Autonomous-Systems-NS3

  This is a network simulation made in Network Simulator 3 that shows how the packets move between different routers. 

  <h2>Build Instructions (Ubuntu):</h2>
  1. Install NS-3 and all its dependencies<br/>
  2. Paste the project.cc in the scratch folder of ns-3.29 folder<br/>
  3. Comment out the line number 316 in the file ns-3.29/src/internet/model/global-route-manager-impl.cc<br/>
  4. Create directory named XML in ns-3.29 folder<br/>
  5. Run shell ./waf scratch/project.cc <br/>
  6. Open the XML/project.xml file in Network Animator
