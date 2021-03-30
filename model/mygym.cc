
#include "mygym.h"
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>


//#include "ns3/core-module.h"
//#include "ns3/opengym-module.h"


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("MyGymEnv");

NS_OBJECT_ENSURE_REGISTERED (MyGymEnv);
  
MyGymEnv::MyGymEnv(const videoData & videoData,
                         const playbackData & playbackData,
                         const bufferData & bufferData,
                         const throughputData & throughput,
                         Ptr<OpenGymInterface> openGymInterface) :

    AdaptationAlgorithm(videoData, playbackData, bufferData, throughput),
    
    m_highestRepIndex (videoData.averageBitrate.size () - 1),
    m_lastSegmentIndex( (int64_t) videoData.segmentSize.at (0).size () - 1)
{
    NS_LOG_INFO(this);
    //NS_LOG_INFO("Connecting to AI Proxy");
    //openGymInterface = CreateObject<OpenGymInterface> (openGymPort);
    SetOpenGymInterface(openGymInterface);
    m_repindex =0;
    m_downloadDelay =0;
    
}

algorithmReply
MyGymEnv::GetNextRep(const int64_t segmentCounter, int64_t clientId) {
    const int64_t timeNow = Simulator::Now().GetMicroSeconds();

    m_segmentCounter = segmentCounter;
    m_clientId = clientId;
    // Default Response
    algorithmReply answer;
    answer.nextRepIndex = 0;
    answer.nextDownloadDelay = 0;
    answer.decisionTime = timeNow;
    answer.decisionCase = 0;
    answer.delayDecisionCase = 0;
    

    if (segmentCounter > 0)
    {
      Notify();
      answer.nextDownloadDelay = m_downloadDelay;
      answer.nextRepIndex = m_repindex;      
    }
    
    return answer;
}


MyGymEnv::~MyGymEnv ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
MyGymEnv::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MyGymEnv")
    .SetParent<OpenGymEnv> ()
    .SetGroupName ("OpenGym")
  ;
  return tid;
}

void
MyGymEnv::DoDispose ()
{
  NS_LOG_FUNCTION (this);
}


/*
Define observation space
*/
Ptr<OpenGymSpace> 
MyGymEnv::GetObservationSpace()
{
  NS_LOG_FUNCTION (this);

  Ptr<OpenGymDictSpace> space = CreateObject<OpenGymDictSpace> ();
  return space;
}

/*
Define action space 
*/
Ptr<OpenGymSpace> 
MyGymEnv::GetActionSpace()
{
  NS_LOG_FUNCTION (this);
  Ptr<OpenGymDiscreteSpace> nextRepIndex = CreateObject<OpenGymDiscreteSpace> (m_highestRepIndex );
  Ptr<OpenGymDiscreteSpace> nextDownloadDelay = CreateObject<OpenGymDiscreteSpace> (m_lastSegmentIndex * m_videoData.segmentDuration );
 
  Ptr<OpenGymDictSpace> space = CreateObject<OpenGymDictSpace> ();
  space->Add("nextRepIndex", nextRepIndex);
  space->Add("nextDownloadDelay", nextDownloadDelay);

  return space;

}

/*
Define game over condition
*/
bool 
MyGymEnv::GetGameOver()
{
  NS_LOG_FUNCTION (this);
  
  bool isGameOver = false;
  //if (m_segmentCounter == m_lastSegmentIndex ){ //|| m_bufferNow ==0) {
  //    NS_LOG_UNCOND ("seg counter: " << m_segmentCounter);
  //    NS_LOG_UNCOND ("last seg: " << m_lastSegmentIndex);
  //    isGameOver = true;
  //    NS_LOG_UNCOND ("GetGameOver: " << isGameOver);
  //}
  
  return isGameOver;
}

/*
Collect observations
*/
Ptr<OpenGymDataContainer> 
MyGymEnv::GetObservation()
{
  NS_LOG_FUNCTION (this);

  Ptr<OpenGymDiscreteContainer> clientId = CreateObject<OpenGymDiscreteContainer> ( ); 
  Ptr<OpenGymDiscreteContainer> segmentCounter = CreateObject<OpenGymDiscreteContainer> ( ); 
  Ptr<OpenGymDiscreteContainer> timeNow = CreateObject<OpenGymDiscreteContainer> ( );
  Ptr<OpenGymDiscreteContainer> bufferLevelOld = CreateObject<OpenGymDiscreteContainer> ( );
  Ptr<OpenGymDiscreteContainer> bufferLevelNew = CreateObject<OpenGymDiscreteContainer> ( );
  Ptr<OpenGymDiscreteContainer> transmissionRequested = CreateObject<OpenGymDiscreteContainer> ( );
  Ptr<OpenGymDiscreteContainer> transmissionStart = CreateObject<OpenGymDiscreteContainer> ( );
  Ptr<OpenGymDiscreteContainer> transmissionEnd = CreateObject<OpenGymDiscreteContainer> ( );
  Ptr<OpenGymDiscreteContainer> bytesReceived = CreateObject<OpenGymDiscreteContainer> ( );
  Ptr<OpenGymDiscreteContainer> playbackIndex = CreateObject<OpenGymDiscreteContainer> ( );
  Ptr<OpenGymDiscreteContainer> playbackStart = CreateObject<OpenGymDiscreteContainer> ( );
  
  clientId->SetValue( m_clientId );
  segmentCounter->SetValue(m_segmentCounter );
  timeNow->SetValue(m_bufferData.timeNow.back() );
  bufferLevelOld->SetValue(m_bufferData.bufferLevelOld.back() ); 
  bufferLevelNew ->SetValue(m_bufferData.bufferLevelNew.back() );
  transmissionRequested ->SetValue(m_throughput.transmissionRequested.back());
  transmissionStart ->SetValue(m_throughput.transmissionStart.back());
  transmissionEnd ->SetValue(m_throughput.transmissionEnd.back());
  bytesReceived ->SetValue(m_throughput.bytesReceived.back());
  playbackIndex ->SetValue(m_playbackData.playbackIndex.back());
  playbackStart->SetValue(m_playbackData.playbackStart.back());


  Ptr<OpenGymDictContainer> space = CreateObject<OpenGymDictContainer> ();
  space->Add("clientId", clientId);
  space->Add("segmentCounter", segmentCounter);
  space->Add("timeNow", timeNow);
  space->Add("bufferLevelOld",bufferLevelOld ); 
  space->Add("bufferLevelNew",bufferLevelNew); 
  space->Add("transmissionRequested",transmissionRequested); 
  space->Add("transmissionStart",transmissionStart); 
  space->Add("transmissionEnd",transmissionEnd); 
  space->Add("bytesReceived",bytesReceived); 
  space->Add("playbackIndex",playbackIndex); 
  space->Add("playbackStart",playbackStart); 
  

  return space;
}

/*
Define reward function
*/
float 
MyGymEnv::GetReward()
{
  NS_LOG_FUNCTION (this);
  
  return m_reward;
}

/*
Define extra info. Optional
*/
std::string 
MyGymEnv::GetExtraInfo()
{
  NS_LOG_FUNCTION (this);
  std::string myInfo = "no extra info";
  return myInfo;
}

/*
Execute received actions
*/
bool 
MyGymEnv::ExecuteActions(Ptr<OpenGymDataContainer> action)
{

  NS_LOG_FUNCTION (this);
  Ptr<OpenGymDictContainer> dict = DynamicCast<OpenGymDictContainer>(action);
  Ptr<OpenGymDiscreteContainer> repIndex = DynamicCast<OpenGymDiscreteContainer>(dict->Get("nextRepIndex"));
  Ptr<OpenGymDiscreteContainer> downloadDelay = DynamicCast<OpenGymDiscreteContainer>(dict->Get("nextDownloadDelay"));
  m_repindex = repIndex->GetValue();
  m_downloadDelay = downloadDelay->GetValue();
  return true;
}

} // ns3 namespace


