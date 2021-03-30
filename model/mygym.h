#ifndef MY_GYM_ENV_H
#define MY_GYM_ENV_H

#include "tcp-stream-adaptation-algorithm.h"
#include "ns3/opengym-module.h"
#include "ns3/core-module.h"



namespace ns3 {
/**
 * \ingroup tcpStream
 * \brief Implementation of a new adaptation algorithm
 */
class MyGymEnv : public AdaptationAlgorithm, public OpenGymEnv
{
public:
	MyGymEnv ( const videoData &videoData,
             const playbackData & playbackData,
			 const bufferData & bufferData,
			 const throughputData & throughput,
			 Ptr<OpenGymInterface> openGymInterface);

	algorithmReply GetNextRep ( const int64_t segmentCounter, int64_t clientId);

	virtual ~MyGymEnv ();
	static TypeId GetTypeId (void);
	virtual void DoDispose ();

	Ptr<OpenGymSpace> GetActionSpace();
	Ptr<OpenGymSpace> GetObservationSpace();
	bool GetGameOver();
	Ptr<OpenGymDataContainer> GetObservation();
	float GetReward();
	std::string GetExtraInfo();
	bool ExecuteActions(Ptr<OpenGymDataContainer> action);
	
	

	//uint32_t openGymPort = 5555;
	//Ptr<OpenGymInterface> openGymInterface;
	
private:
	
	int64_t m_clientId;
	int64_t m_repindex;
	int64_t m_downloadDelay;
	int64_t m_reward;  
	int64_t m_highestRepIndex;
	int64_t m_lastSegmentIndex;
	int64_t m_segmentCounter;

};


} // namespace ns3
#endif /* RL_ALGORITHM_H */