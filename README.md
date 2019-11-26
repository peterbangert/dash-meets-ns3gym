# DASH meets ns3-gym

> Combining DASH the discrete video streaming framework with ns3gym, the ns3 api toolkit for developing ML based algorithms.


## Installation 

1. Follow guide to setting up ns3

   - [tutorial](https://www.nsnam.org/docs/release/3.30/tutorial/singlehtml/index.html)

2. Follow section below for setting up ns3-gym 
  
   - [ns3-gym setup](#ns3-gym)

3. Follow section below for setting up DASH in ns3

   - [dash setup](#dash-ns3)

4. Program Execution

   - [command](#program-execution)

## ns3-gym


> [OpenAI Gym](https://gym.openai.com/) is a toolkit for reinforcement learning (RL) widely used in research. The network simulator [ns–3](https://www.nsnam.org/) is the de-facto standard for academic and industry studies in the areas of networking protocols and communication technologies. ns3-gym is a framework that integrates both OpenAI Gym and ns-3 in order to encourage usage of RL in networking research.

 - Reference : https://github.com/tkn-tub/ns3-gym

### Installation


1. Install all required dependencies required by ns-3.
```
# minimal requirements for C++:
apt-get install gcc g++ python

see https://www.nsnam.org/wiki/Installation
```
2. Install ZMQ and Protocol Buffers libs:
```
# to install protobuf-3.6 on ubuntu 16.04:
sudo add-apt-repository ppa:maarten-fonville/protobuf
sudo apt-get update

apt-get install libzmq5 libzmq5-dev
apt-get install libprotobuf-dev
apt-get install protobuf-compiler
```
3. Configure and build ns-3 project (if you are going to use Python virtual environment, please execute these commands inside it):
```
# Opengym Protocol Buffer messages (C++ and Python) are build during configure
./waf configure
./waf build
```

4. Install ns3gym located in src/opengym/model/ns3gym (Python3 required)
```
pip3 install ./src/opengym/model/ns3gym
```



## DASH-ns3

> A simulation model for HTTP-based adaptive streaming applications. 

 - Reference : https://github.com/haraldott/dash

DASH ns3 is already setup to run out of the box in this repository, for information on how DASH is setup and how to develop with DASH, please see the referenced repository above.



## Program Execution

### Parameters 
- simulationId 
- numberOfClients
- segmentDuration
  - The duration of a segment in microseconds.
- adaptationAlgo: 
  - The name of the adaptation algorithm the client uses for the simulation. The 'pre-installed' algorithms are tobasco, festive and panda.
- segmentSizeFile
  - The relative path (from the ns-3.x/ folder) of the file containing the sizes of the segments of the video. The segment sizes have to be provided as a (n, m) matrix, with n being the number of representation levels and m being the total number of segments. A two-segment long, three representations containing segment size file would look like the following:

 1564 22394  
 1627 46529  
 1987 121606  

#### Example

```bash
./waf --run="tcp-stream --simulationId=1 --numberOfClients=1 --adaptationAlgo=rl-algorithm --segmentDuration=2000000 --segmentSizeFile=contrib/dash/segmentSizes.txt"
```





## Contact

* Peter Bangert, TU-Berlin, petbangert@gmail.com
