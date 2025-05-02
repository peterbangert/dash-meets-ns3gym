#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import numpy as np

class Reward():

    def __init__(self, args):
        self.opt = args.reward

    def reward(self, obs, action):
        reward = 0
        if self.opt == "quality":
            reward = self.quality(obs)
        elif self.opt == "rebuff" : 
            reward = self.rebuff(obs)
        elif self.opt == "rebuff_lowvar" :
            reward = self.rebuff_lowvar(obs)
        elif self.opt == "log" :
            reward = self.log(obs)
        elif self.opt == "hd" :
            reward = self.hd(obs)
        elif self.opt == "default" : 
            reward = self.default(obs, action)
        return reward

    # option 4. use the metric in SIGCOMM MPC paper
       # --linear reward--
    def default(self, obs, action):
        reward = obs.video_bitrate[obs.playbackIndex[-1]] * obs.mega_in_kilo \
                - obs.rebuffer_penalty * obs.rebuffer_time * obs.seconds_in_micro \
                - obs.smooth_penalty * np.abs(obs.video_bitrate[action] -
                                      obs.video_bitrate[obs.playbackIndex[-1]] )  * obs.mega_in_kilo 

        print(" REWARD : ", reward)

        return reward

    ## option 1. reward for just quality
    def quality(self, obs):
        return obs.last_quality
    
    ## option 2. combine reward for quality and rebuffer time
        #           tune up the knob on rebuf to prevent it more
    def rebuff(self, obs):
        
        reward = obs.last_quality - 0.1 * (obs.rebuffer_time )
        return reward
    
    # option 3. give a fixed penalty if video is stalled
        #           this can reduce the variance in reward signal
    def rebuff_lowvar(self, obs):
        
        reward = obs.last_quality - 10 * ((obs.rebuffer_time ) > 0)
        return reward
        
    # --log reward--    
    def log(self, obs):
          
        log_bit_rate = np.log(obs.video_bitrate[obs.last_quality] / float(obs.video_bitrate[0]))   
        log_last_bit_rate = np.log(obs.last_bit_rate / float(obs.video_bitrate[0]))

        reward = log_bit_rate \
                  - 4.3 * obs.rebuffer_time * obs.mega_in_kilo\
                  - obs.smooth_penalty * np.abs(log_bit_rate - log_last_bit_rate)
        return reward
    
    # --hd reward--
    def hd(self, obs):
        reward = obs.bitrate_reward_map[obs.last_quality] \
             - 8 * obs.rebuffer_time * obs.mega_in_kilo- np.abs(obs.bitrate_reward_map[obs.last_quality] - obs.bitrate_reward_map_MAP[obs['last_bit_rate']])
        return reward
