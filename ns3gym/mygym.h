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


#ifndef MY_GYM_ENTITY_H
#define MY_GYM_ENTITY_H

#include "ns3/stats-module.h"
#include "ns3/opengym-module.h"
#include "ns3/spectrum-module.h"

namespace ns3 {


class MyGymEnv : public OpenGymEnv
{
public:
  
  MyGymEnv ();
  MyGymEnv (int64_t highestRepIndex, int64_t lastSegmentIndex);
  
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
  void PrintState();
  void UpdateState(int64_t segmentCounter,
  int64_t bufferNow,
  int64_t lastchunkfinishtime, 
  int64_t lastchunkstarttime, 
  int64_t m_lastchunksize,
  int64_t rebuffertime);


private:

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
}

#endif // MY_GYM_ENTITY_H
