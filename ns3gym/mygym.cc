/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018 Technische Universität Berlin
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
 *
 * Author: Piotr Gawlowicz <gawlowicz@tkn.tu-berlin.de>
 */

#include "mygym.h"
#include "ns3/object.h"
#include "ns3/core-module.h"
#include "ns3/wifi-module.h"
#include "ns3/node-list.h"
#include "ns3/log.h"
#include <sstream>
#include <iostream>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("MyGymEnv");

NS_OBJECT_ENSURE_REGISTERED (MyGymEnv);


MyGymEnv::MyGymEnv ()
{
  NS_LOG_FUNCTION (this);

  m_new_rep_index =0;
  m_old_rep_index =0;
  m_highestRepIndex = 8;
  m_lastSegmentIndex = 10;
  m_reward =1;
  m_bufferLast =0;
  m_lastChunkFinishTime = 0;
  m_lastChunkStartTime = 0;
  m_lastChunkSize =0;
  m_rebufferTime = 0;

}


MyGymEnv::MyGymEnv (int64_t highestRepIndex, int64_t lastSegmentIndex)
{
  NS_LOG_FUNCTION (this);
  
  m_new_rep_index =0;
  m_old_rep_index =0;
  m_highestRepIndex = highestRepIndex;
  m_lastSegmentIndex = lastSegmentIndex;
  m_reward =1;
  m_bufferLast =0;
  m_lastChunkFinishTime = 0;
  m_lastChunkStartTime = 0;
  m_rebufferTime = 0;
  
}

MyGymEnv::~MyGymEnv ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
MyGymEnv::GetTypeId (void)
{
  static TypeId tid = TypeId ("MyGymEnv")
    .SetParent<OpenGymEnv> ()
    .SetGroupName ("OpenGym")
    .AddConstructor<MyGymEnv> ()
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
  
  uint32_t buffernum = 2000000;
  uint32_t lastReq = m_lastSegmentIndex;
  uint32_t lastQual = 8;
  uint32_t lastchunkfinishtime = 2000000;
  uint32_t lastchunkstarttime = 2000000;
  uint32_t rebuffertime = 2000000;
  uint32_t lastchunksize = 0;
  
  Ptr<OpenGymDiscreteSpace> buffer = CreateObject<OpenGymDiscreteSpace> (buffernum);
  Ptr<OpenGymDiscreteSpace> lastRequest = CreateObject<OpenGymDiscreteSpace> (lastReq);
  Ptr<OpenGymDiscreteSpace> lastQuality = CreateObject<OpenGymDiscreteSpace> (lastQual);
  Ptr<OpenGymDiscreteSpace> lastChunkFinishTime = CreateObject<OpenGymDiscreteSpace> (lastchunkfinishtime);
  Ptr<OpenGymDiscreteSpace> lastChunkStartTime = CreateObject<OpenGymDiscreteSpace> (lastchunkstarttime);
  Ptr<OpenGymDiscreteSpace> RebufferTime = CreateObject<OpenGymDiscreteSpace> (rebuffertime);
  Ptr<OpenGymDiscreteSpace> lastChunkSize = CreateObject<OpenGymDiscreteSpace> (lastchunksize);


  Ptr<OpenGymDictSpace> space = CreateObject<OpenGymDictSpace> ();
  space->Add("buffer", buffer);
  space->Add("lastRequest", lastRequest);
  space->Add("lastQuality", lastQuality);
  space->Add("lastChunkFinishTime", lastChunkFinishTime);
  space->Add("lastChunkStartTime", lastChunkStartTime);
  space->Add("RebufferTime", RebufferTime);
  space->Add("lastChunkSize", lastChunkSize);
  
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
  
  Ptr<OpenGymDiscreteContainer> buffer = CreateObject<OpenGymDiscreteContainer> ( );
  Ptr<OpenGymDiscreteContainer> lastRequest = CreateObject<OpenGymDiscreteContainer> ();
  Ptr<OpenGymDiscreteContainer> lastQuality = CreateObject<OpenGymDiscreteContainer> ();
  Ptr<OpenGymDiscreteContainer> lastChunkFinishTime = CreateObject<OpenGymDiscreteContainer> ();
  Ptr<OpenGymDiscreteContainer> lastChunkStartTime = CreateObject<OpenGymDiscreteContainer> ();
  Ptr<OpenGymDiscreteContainer> RebufferTime = CreateObject<OpenGymDiscreteContainer> ();
  Ptr<OpenGymDiscreteContainer> lastChunkSize = CreateObject<OpenGymDiscreteContainer> ();
  buffer->SetValue(m_bufferNow );
  lastRequest->SetValue(m_segmentCounter);
  lastQuality->SetValue(m_new_rep_index);
  lastChunkFinishTime->SetValue(m_lastChunkFinishTime );
  lastChunkStartTime->SetValue(m_lastChunkStartTime );
  RebufferTime->SetValue(m_rebufferTime);
  lastChunkSize->SetValue(m_lastChunkSize);

  Ptr<OpenGymDictContainer> space = CreateObject<OpenGymDictContainer> ();
  space->Add("buffer", buffer);
  space->Add("lastRequest", lastRequest);
  space->Add("lastquality", lastQuality);
  space->Add("lastChunkFinishTime", lastChunkFinishTime);
  space->Add("lastChunkStartTime", lastChunkStartTime);
  space->Add("RebufferTime", RebufferTime);
  space->Add("lastChunkSize", lastChunkSize);

  return space;
}



/*
Define reward function
*/
float 
MyGymEnv::GetReward()
{
  NS_LOG_FUNCTION (this);
  if (m_bufferNow > m_bufferLast) {
    m_reward = m_reward +  1;
  } else {
    m_reward = 0;
  }
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
MyGymEnv::PrintState()
{
  NS_LOG_UNCOND ("Rep old: " << m_old_rep_index << " --  Rep new : " << m_new_rep_index);
}
  
void
MyGymEnv::ClearObs()
{
  NS_LOG_FUNCTION (this);
}

void
MyGymEnv::UpdateState(int64_t segmentCounter,
  int64_t bufferNow, 
  int64_t lastchunkfinishtime, 
  int64_t lastchunkstarttime, 
  int64_t lastchunksize,
  int64_t rebuffertime)
{
  m_lastChunkFinishTime = lastchunkfinishtime;
  m_lastChunkStartTime = lastchunkstarttime;
  m_segmentCounter =segmentCounter;
  m_bufferLast = m_bufferNow;
  m_bufferNow = bufferNow;
  m_lastChunkSize = lastchunksize;
  m_rebufferTime = rebuffertime;
}

uint32_t
MyGymEnv::GetRepIndex()
{
  Notify();
  return m_new_rep_index;
}

} // ns3 namespace