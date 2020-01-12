#ifndef RL_ALGORITHM_H
#define RL_ALGORITHM_H

#include "tcp-stream-adaptation-algorithm.h"
#include "ns3/opengym-module.h"
#include "ns3/core-module.h"
#include "ns3gym/mygym.h"


namespace ns3 {
/**
 * \ingroup tcpStream
 * \brief Implementation of a new adaptation algorithm
 */
class RLAlgorithm : public AdaptationAlgorithm
{
public:

	RLAlgorithm ( const videoData &videoData,
             const playbackData & playbackData,
			 const bufferData & bufferData,
			 const throughputData & throughput,
              int  numberOfClients,
              int simulationId);

	algorithmReply GetNextRep ( const int64_t segmentCounter, int64_t clientId);

	uint32_t openGymPort = 5555;

	Ptr<OpenGymInterface> openGymInterface;
	Ptr<MyGymEnv> myGymEnv;
	
private:
	const int64_t m_highestRepIndex;
    const int64_t m_lastSegmentIndex;
    int64_t m_repindex;

};


} // namespace ns3
#endif /* RL_ALGORITHM_H */