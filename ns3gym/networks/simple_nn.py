#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import gym
import tensorflow as tf


import numpy as np
import matplotlib as mpl
import matplotlib.pyplot as plt
from tensorflow import keras

from ns3gym import ns3env
import argparse
from datetime import date
import csv
import time
import os

from .rewards import Reward



class Simple_NN():

    S_INFO = 6
    epsilon = 1.0               # exploration rate
    epsilon_min = 0.01
    epsilon_decay = 0.999
    

    def __init__(self, args, observations, action):    
        self.reward = Reward(args)
        self.action = action
        self.observations = observations
        self.model = self.load_model(args)
    
    def forward_pass(self):

        print("FORWARD PASS")
        print("-------------------------------")
        state = self.get_state_array()
        if (np.random.rand(1) < self.epsilon):
            self.action.nextRepIndex = np.random.randint(self.observations.A_DIM)
        else:
            self.action.nextRepIndex = np.argmax(self.model.predict(state)[0])
        if self.epsilon > self.epsilon_min: self.epsilon *= self.epsilon_decay



    def train(self):
        #Train              
        state = self.get_state_array()
        p = self.model.predict(state)
        target = (self.reward.reward(self.observations, self.action.nextRepIndex) + 0.95 * np.amax(p))

        target_f = self.model.predict(state)
        target_f[0][self.action.nextRepIndex] = target
        self.model.fit(state, target_f, epochs=1, verbose=0)


    def get_state_array(self):

        download_time = (float(self.observations.transmissionEnd[-1]) - float(self.observations.transmissionStart[-1])) * self.observations.seconds_in_micro
        size = float(self.observations.bytesReceived[-1]) * self.observations.mega_in_bytes
        throughput = size / download_time
        buff = self.observations.bufferLevelOld[-1] * self.observations.seconds_in_micro
        left_chunks =  self.observations.left_chunks
        rebuffer_time = self.observations.total_rebuffer_time * self.observations.seconds_in_micro

        next_state = np.asarray([download_time, size, throughput, buff, left_chunks, rebuffer_time])
        next_state = np.reshape(next_state, [1, 6])

        print("---- STATE ARRAY : \n "
                "download time (s) : ", download_time, "\n",
                "size (mb) : " , size, "\n",
                "throughput mb/s : ", throughput,"\n",
                "buffer (s) : ", buff,"\n",
                "segments left : ", left_chunks,"\n",
                "rebuffer time (s) : ", rebuffer_time,"\n")

        return next_state    
    

    def load_model(self, args):

        if (args.load_model is not False):
            model = keras.models.load_model('saved-models/' + args.useModel +'.h5')
            print("Loaded model from disk")

        else:
            model = keras.Sequential()
            model.add(keras.layers.Dense(self.S_INFO, activation='relu'))   
            model.add(keras.layers.Dense(self.observations.A_DIM, activation='softmax'))         

        model.compile(optimizer=tf.compat.v1.train.AdamOptimizer(0.001),
                    loss='categorical_crossentropy',
                    metrics=['accuracy'])
        return model

    

    def save_model(self, args, model):
            # serialize model to JSON
        if (args.saveModel  is not None):

            model.save('saved-models/' +args.saveModel+".h5")
            print("Saved model to disk")

