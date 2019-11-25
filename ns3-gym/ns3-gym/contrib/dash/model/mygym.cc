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
  uint32_t numdata = 1;
  int64_t low = -2000000;
  int64_t high = 2000000;  // hardcoded buffer size from tcp-stream.cc

  // Observation will be buffer load and num segments in buffer

  std::vector<uint32_t> shape = {numdata,};
  std::string dtype = TypeNameGet<uint32_t> ();
  Ptr<OpenGymBoxSpace> space = CreateObject<OpenGymBoxSpace> (low, high, shape, dtype);
  NS_LOG_UNCOND ("GetObservationSpace: " << space);
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
  NS_LOG_UNCOND ("GetActionSpace: " << space);
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
  if (m_segmentCounter == m_lastSegmentIndex -1 || m_bufferNow ==0) {
      //m_reward = 0;
      NS_LOG_UNCOND ("seg counter: " << m_segmentCounter);
      NS_LOG_UNCOND ("last seg: " << m_lastSegmentIndex);
      NS_LOG_UNCOND ("buffer underrun: " << m_bufferNow);
      isGameOver = true;
  }
  NS_LOG_UNCOND ("GetGameOver: " << isGameOver);
  return isGameOver;
}

/*
Collect observations
*/
Ptr<OpenGymDataContainer> 
MyGymEnv::GetObservation()
{
  NS_LOG_FUNCTION (this);
  uint32_t numdata = 1;
  std::vector<uint32_t> shape = {numdata,};
  Ptr<OpenGymBoxContainer<uint32_t> > box = CreateObject<OpenGymBoxContainer<uint32_t> >(shape);

  box->AddValue(m_bufferNow - m_bufferLast);
  
  NS_LOG_UNCOND ("GetObservation: " << box);
  return box;
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

  NS_LOG_UNCOND ("-------------------------reward: " << m_reward);
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
  
  NS_LOG_UNCOND("GetExtraInfo: " << myInfo);
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
  
  PrintState();

  return true;
}

void
MyGymEnv::PrintState()
{
  NS_LOG_UNCOND ("Rep old: " << m_old_rep_index << " --  Rep new : " << m_new_rep_index);
  //NS_LOG_UNCOND ("State new : " << m_new_rep_index);
}
  
void
MyGymEnv::ClearObs()
{
  NS_LOG_FUNCTION (this);
  
}

void
MyGymEnv::UpdateState(int64_t segmentCounter,int64_t bufferNow)
{
  m_segmentCounter =segmentCounter;
  m_bufferLast = m_bufferNow;
  m_bufferNow = bufferNow;
}

uint32_t
MyGymEnv::GetRepIndex()
{
  Notify();
  return m_new_rep_index;
}

} // ns3 namespace