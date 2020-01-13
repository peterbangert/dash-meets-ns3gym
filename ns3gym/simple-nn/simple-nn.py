#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import gym
import tensorflow as tf
import tensorflow.contrib.slim as slim
import numpy as np
import matplotlib as mpl
import matplotlib.pyplot as plt
from tensorflow import keras

from ns3gym import ns3env
import argparse
from datetime import date
import csv


# Obervation Data
#   bit_rate, buffer_size, rebuffering_time, bandwidth_measurement, chunk_til_video_end
S_INFO = 6 

# Number of Video qualities
A_DIM = 0

# Number of total chunks of video                   
TOTAL_VIDEO_CHUNKS = 221

# 2d array of segments sizes, by quality
#   VIDEO_SIZES[n][m], n quaity, m segment
VIDEO_SIZES = []

# Bitrates of Qualities
# Low 240p, Low 360p, Medium 480p, HD 720p, HD1080 1080p, Quad HD 1440p, 4k 2160p, 8k 4320p
VIDEO_BIT_RATE = [300,750,1200,1850,2850,4300,8000,15000]  # Kbps

M_IN_K = 1000000.0



DEFAULT_QUALITY = 0  # default video quality without agent
REBUF_PENALTY = 4.3  # 1 sec rebuffering -> this number of Mbps
SMOOTH_PENALTY = 1




def set_video_dimensions(args):
    reader = csv.reader(open(args.segmentSizeFile, "rb"), delimiter=" ")
    global VIDEO_SIZES, A_DIM, TOTAL_VIDEO_CHUNKS 
    VIDEO_SIZES = np.asarray(list(reader))
    A_DIM = VIDEO_SIZES.shape[0]
    TOTAL_VIDEO_CHUNKS = VIDEO_SIZES.shape[1]


def get_state_array(obs):

    download_time = float(obs['lastChunkFinishTime']) - float(obs['lastChunkStartTime']) / M_IN_K
    size = float(obs['lastChunkSize']) / M_IN_K
    throughput = size / download_time
    buff = obs['buffer'] / M_IN_K
    left_chunks =  TOTAL_VIDEO_CHUNKS - obs['lastRequest']
    rebuffer_time = obs['RebufferTime'] / M_IN_K

    next_state = np.asarray([download_time, size, throughput, buff, left_chunks, rebuffer_time])
    next_state = np.reshape(next_state, [1, 6])
    print next_state
    return next_state

def env_init():
    startSim = 0
    port = 5555;
    simTime = 20 # seconds
    stepTime = 1.0  # seconds
    seed = 0
    simArgs = {"--simTime": simTime,
               "--testArg": 123}
    debug = False
    env = ns3env.Ns3Env(port=port, stepTime=stepTime, startSim=startSim, simSeed=seed, simArgs=simArgs, debug=debug)
    env.reset()
    return env


def main(args):


    print A_DIM

    ## Simulation Parameters
    stepIdx = 0
    cur_episode = 0

    ## Training Parameters
    epsilon = 1.0               # exploration rate
    epsilon_min = 0.01
    epsilon_decay = 0.999
    totalRebuf = 0
    actionHistory = []

    env = env_init()
    
    ob_space = env.observation_space
    ac_space = env.action_space
    print("Observation space: ", ob_space,  ob_space.dtype)
    print("Action space: ", ac_space, ac_space.dtype)




    model = keras.Sequential()
    
    model.add(keras.layers.Dense(S_INFO, activation='relu'))   
    model.add(keras.layers.Dense(A_DIM, activation='softmax'))         
    if (args.useModel is not None):
        model = keras.models.load_model('saved-models/' + args.useModel)
    model.compile(optimizer=tf.train.AdamOptimizer(0.001),
                  loss='categorical_crossentropy',
                  metrics=['accuracy'])



    while cur_episode < args.episodes:

        print("Start iteration: ", cur_episode)
        state = get_state_array(env.reset())

        while True:
            stepIdx += 1

            if np.random.rand(1) < epsilon:
                action = np.random.randint(A_DIM)
            else:
                action = np.argmax(model.predict(state)[0])

            actionHistory.append(action)

            
            print("---action: ", action)

            print("Step: ", stepIdx)
            obs, reward, done, info = env.step(action)
            print("---obs, reward, done, info: ", obs, reward, done, info)

            if done:
                stepIdx = 0
                if cur_episode + 1 < args.episodes:
                    env.reset()
                break

            reward = VIDEO_BIT_RATE[obs['lastquality']] / M_IN_K \
                - REBUF_PENALTY * obs['RebufferTime'] / M_IN_K \
                - SMOOTH_PENALTY * np.abs(VIDEO_BIT_RATE[obs['lastquality']] -
                                      float(obs['lastChunkSize']) ) / M_IN_K

            next_state = get_state_array(obs)
            print(next_state)

            # Train
            target = reward
            if not done:

                p = model.predict(next_state)
                print(p)

                target = (reward + 0.95 * np.amax(p))

            target_f = model.predict(state)
            target_f[0][action] = target
            model.fit(state, target_f, epochs=1, verbose=0)

            state = next_state
            
            if epsilon > epsilon_min: epsilon *= epsilon_decay

        cur_episode += 1
        


    if (args.saveModel is not None):
        model.save('saved-models/' + args.saveModel)


    print(actionHistory)
    plt.plot(actionHistory)
    plt.ylabel('Rep Index')
    plt.show()


def get_args():
    parser = argparse.ArgumentParser(description='Arguments for Neural Network')
    parser.add_argument('--segmentSizeFile', type=str, default="../../segmentSizes.txt",
                       help='Location of segment size file, default is in repo')
    parser.add_argument('--episodes', type=int, default=1,
                       help='No of training episodes, default 1')
    parser.add_argument('--saveModel',type=str, default=None,
                       help='Save the trained model, filename')
    parser.add_argument('--useModel', type=str, default=None,
                       help='Name of saved model, searches saved-models/')
    return parser.parse_args()



if __name__ == "__main__":
    try:
        args = get_args()
        set_video_dimensions(args)
        main(args)
    except (KeyboardInterrupt, SystemExit):
        print("Ctrl-C -> Exit")
    finally:
        
        print("Done")


