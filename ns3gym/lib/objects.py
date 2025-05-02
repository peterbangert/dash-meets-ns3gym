#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import yaml
import csv
import numpy as np

class Observations(dict):

    seconds_in_micro = 0.000001
    mega_in_kilo = 0.001 
    mega_in_bytes = 0.000001
    rebuffer_time = 0
    total_rebuffer_time = 0

    def __init__(self, args):
        self.dict =0 
        with open('conf.d/objects.yml','r') as stream:
            try:
                self.dict = yaml.load(stream, Loader=yaml.Loader)
            except yaml.YAMLError as exc:
                print(exc)
        self.__dict__ = self.dict
        self.set_video_dimensions(args)


    def reset(self, args, obs):
        self.__init__(args)
        self.update_observations(obs)


    def set_video_dimensions(self, args):
        reader = csv.reader(open("../../../" + args.segmentSizeFile, "r"), delimiter=" ")
        self.video_sizes = np.asarray(list(reader))
        self.A_DIM = self.video_sizes.shape[0]
        self.total_video_chunks = self.video_sizes.shape[1]
        self.segmentDuration = args.segmentDuration


    def update_observations(self, obs):

        # Calculate time spent Rebuffing during last segment download.
        if (obs['bufferLevelOld'] == 0 and obs['segmentCounter'] > 1) :
            self.rebuffer_time = obs['timeNow'] - self.__dict__['playbackStart'][-1] + self.segmentDuration
            self.total_rebuffer_time += self.rebuffer_time 
        else: 
            self.rebuffer_time =0
       
        # Throughput Data
        self.__dict__["transmissionRequested"].append(obs['transmissionRequested'])
        self.__dict__["transmissionStart"].append(obs['transmissionStart'])
        self.__dict__["transmissionEnd"].append(obs['transmissionEnd'])
        self.__dict__["bytesReceived"].append(obs['bytesReceived'])

        # Buffer Data
        self.__dict__["timeNow"].append(obs['timeNow'])
        self.__dict__["bufferLevelOld"].append(obs['bufferLevelOld'])
        self.__dict__["bufferLevelNew"].append(obs['bufferLevelNew'])

        # Playback Data
        self.__dict__["playbackIndex"].append(obs['playbackIndex'])
        self.__dict__["playbackStart"].append(obs['playbackStart'])

        # Simulation Variables
        self.__dict__["segmentCounter"] =  obs['segmentCounter']
        self.__dict__["left_chunks"] = self.__dict__["total_video_chunks"] - (obs['segmentCounter'] +1)

        print(" OBS -----------------------------")
        print(obs)
        print(" Action History -----------------------------")
        print(self.__dict__["playbackIndex"])
        

class Action(object):

    seconds_in_micro = 0.000001

    def __init__(self, action_space):
        self.index_limit = action_space["nextRepIndex"].n
        self.delay_limit = action_space["nextDownloadDelay"].n
        self.nextDownloadDelay = 0
        self.nextRepIndex = 0

    def __get_nextRepIndex(self):
        return self.__nextRepIndex

    def __set_nextRepIndex(self, value):
        if (value < 0  or value > self.index_limit):
            raise ValueError("Next Representation index must be with boundes [%d:%d]" % (0,self.index_limit) )
        self.__nextRepIndex = value

    def __get_nextDownloadDelay(self):
        return self.__nextDownloadDelay
    
    def __set_nextDownloadDelay(self, value):
        if (value < 0  or value > self.delay_limit):
            raise ValueError("Download Delay time must be with boundes [%d:%d]" % (0,self.delay_limit) )
        self.__nextDownloadDelay = int(value)

    nextRepIndex = property(__get_nextRepIndex,__set_nextRepIndex)
    nextDownloadDelay = property(__get_nextDownloadDelay,__set_nextDownloadDelay)

    def getAction(self):
        return {
            "nextRepIndex":self.nextRepIndex,
            "nextDownloadDelay": self.nextDownloadDelay
        }


class Args(object):
    def __init__(self, args):
        self.__dict__ = args