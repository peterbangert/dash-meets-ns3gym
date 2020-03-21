
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
                         const throughputData & throughput) :

    AdaptationAlgorithm(videoData, playbackData, bufferData, throughput),
    
    m_highestRepIndex (videoData.averageBitrate.size () - 1),
    m_lastSegmentIndex( (int64_t) videoData.segmentSize.at (0).size () - 1)
{
    NS_LOG_INFO(this);
    NS_LOG_INFO("Connecting to AI Proxy");
    uint32_t openGymPort = 5555;

    openGymInterface = CreateObject<OpenGymInterface> (openGymPort);
    SetOpenGymInterface(openGymInterface);
    
}

algorithmReply
MyGymEnv::GetNextRep(const int64_t segmentCounter, int64_t clientId) {
    const int64_t timeNow = Simulator::Now().GetMicroSeconds();

    // Default Response
    algorithmReply answer;
    answer.nextRepIndex = 0;
    answer.nextDownloadDelay = 0;
    answer.decisionTime = timeNow;
    answer.decisionCase = 0;
    answer.delayDecisionCase = 0;

    m_repindex =  GetRepIndex();
    answer.nextRepIndex = m_repindex;
        
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
  uint32_t highestRepIndex = m_highestRepIndex;

  Ptr<OpenGymDiscreteSpace> space = CreateObject<OpenGymDiscreteSpace> (highestRepIndex);
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
  if (m_segmentCounter == m_lastSegmentIndex -1 ){ //|| m_bufferNow ==0) {
      NS_LOG_UNCOND ("seg counter: " << m_segmentCounter);
      NS_LOG_UNCOND ("last seg: " << m_lastSegmentIndex);
      isGameOver = true;
      NS_LOG_UNCOND ("GetGameOver: " << isGameOver);
  }
  
  return isGameOver;
}

/*
Collect observations
*/
Ptr<OpenGymDataContainer> 
MyGymEnv::GetObservation()
{
  NS_LOG_FUNCTION (this);
  
  Ptr<OpenGymDictContainer> space = CreateObject<OpenGymDictContainer> ();
  
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
  m_old_rep_index = m_new_rep_index;
  Ptr<OpenGymDiscreteContainer> discrete = DynamicCast<OpenGymDiscreteContainer>(action);
  m_new_rep_index = discrete->GetValue();
  
  return true;
}

  
void
MyGymEnv::ClearObs()
{
  NS_LOG_FUNCTION (this);
}



uint32_t
MyGymEnv::GetRepIndex()
{
  Notify();
  return m_new_rep_index;
}

} // ns3 namespace


