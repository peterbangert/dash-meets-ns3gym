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
			 const throughputData & throughput);

	algorithmReply GetNextRep ( const int64_t segmentCounter, int64_t clientId);

	//MyGymEnv ();
	//MyGymEnv (int64_t highestRepIndex, int64_t lastSegmentIndex);

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
	void ClearObs();
	uint32_t GetRepIndex();

	


	uint32_t openGymPort = 5555;

	Ptr<OpenGymInterface> openGymInterface;
	//Ptr<MyGymEnv> myGymEnv;
	
private:
	
    int64_t m_repindex;
  int64_t m_highestRepIndex;
  int64_t m_lastSegmentIndex;
  uint32_t m_old_rep_index;
  uint32_t m_new_rep_index;
  int64_t m_bufferNow;
  int64_t m_bufferLast;
  int64_t m_segmentCounter;
  int64_t m_reward;  
  int64_t m_lastChunkFinishTime;
  int64_t m_lastChunkStartTime;
  int64_t m_lastChunkSize;
  int64_t m_rebufferTime;
};


} // namespace ns3
#endif /* RL_ALGORITHM_H */