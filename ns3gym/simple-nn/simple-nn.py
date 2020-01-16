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
import time
import os

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
BITRATE_REWARD_MAP = {0: 0, 300: 1, 750: 2, 1200: 3, 1850: 12, 2850: 15, 4300: 20, 8000: 25, 15000: 30}

M_IN_K = 1000000.0



DEFAULT_QUALITY = 0  # default video quality without agent
REBUF_PENALTY = 4.3  # 1 sec rebuffering -> this number of Mbps
SMOOTH_PENALTY = 1



def get_reward(args, obs):
    reward =0
    if args.reward == "quality":
        
        ## option 1. reward for just quality
        reward = post_data['lastquality']
        
    elif args.reward == "rebuff" : 
        
        ## option 2. combine reward for quality and rebuffer time
        #           tune up the knob on rebuf to prevent it more
        reward = obs['lastquality'] - 0.1 * (obs['RebufferTime'] )
        
    elif args.reward == "rebuff_lowvar" :
        # option 3. give a fixed penalty if video is stalled
        #           this can reduce the variance in reward signal
        reward = obs['lastquality'] - 10 * ((obs['RebufferTime'] ) > 0)
        
    elif args.reward == "default" : 
        # option 4. use the metric in SIGCOMM MPC paper

        # --linear reward--
        reward = VIDEO_BIT_RATE[obs['lastquality']] / 1000.0 \
                - REBUF_PENALTY * obs['RebufferTime'] / M_IN_K \
                - SMOOTH_PENALTY * np.abs(VIDEO_BIT_RATE[obs['lastquality']] -
                                      float(obs['lastChunkSize']) ) / M_IN_K

    elif args.reward == "log" :
        # --log reward--
        log_bit_rate = np.log(VIDEO_BIT_RATE[obs['lastquality']] / float(VIDEO_BIT_RATE[0]))   
        log_last_bit_rate = np.log(obs['last_bit_rate'] / float(VIDEO_BIT_RATE[0]))

        reward = log_bit_rate \
                  - 4.3 * rebuffer_time / M_IN_K \
                  - SMOOTH_PENALTY * np.abs(log_bit_rate - log_last_bit_rate)
                  
    elif args.reward == "hd" :
    # --hd reward--
        reward = BITRATE_REWARD[obs['lastquality']] \
             - 8 * rebuffer_time / M_IN_K - np.abs(BITRATE_REWARD[post_data['lastquality']] - BITRATE_REWARD_MAP[obs['last_bit_rate']])

    return reward

def load_model(args):

    model = keras.Sequential()
    
    if (args.useModel is not None):
    
        # load json and create model
        json_file = open('saved-models/' +args.useModel + '.json', 'r')
        loaded_model_json = json_file.read()
        json_file.close()
        model = keras.models.model_from_json(loaded_model_json)
        # load weights into new model
        model.load_weights('saved-models/' + args.useModel + ".h5")
        print("Loaded model from disk")

    else:
        model.add(keras.layers.Dense(S_INFO, activation='relu'))   
        model.add(keras.layers.Dense(A_DIM, activation='softmax'))         


    model.compile(optimizer=tf.train.AdamOptimizer(0.001),
                  loss='categorical_crossentropy',
                  metrics=['accuracy'])
    return model

   

def save_model(args, model):
        # serialize model to JSON
    if (args.saveModel  is not None):

        model_json = model.to_json()
        with open('saved-models/' +args.saveModel + ".json", "w") as json_file:
            json_file.write(model_json)
        # serialize weights to HDF5
        model.save_weights('saved-models/' +args.saveModel+".h5")
        print("Saved model to disk")
         

def set_video_dimensions(args):
    reader = csv.reader(open(args.segmentSizeFile, "rb"), delimiter=" ")
    global VIDEO_SIZES, A_DIM, TOTAL_VIDEO_CHUNKS 
    VIDEO_SIZES = np.asarray(list(reader))
    A_DIM = VIDEO_SIZES.shape[0]
    TOTAL_VIDEO_CHUNKS = VIDEO_SIZES.shape[1]


def get_state_array(obs):

    download_time = (float(obs['lastChunkFinishTime']) - float(obs['lastChunkStartTime'])) / M_IN_K
    size = float(obs['lastChunkSize']) / M_IN_K
    throughput = size / download_time
    buff = obs['buffer'] / M_IN_K
    left_chunks =  TOTAL_VIDEO_CHUNKS - obs['lastRequest']
    #buffer_delta = 
    rebuffer_time = obs['RebufferTime'] / M_IN_K

    next_state = np.asarray([download_time, size, throughput, buff, left_chunks, rebuffer_time])
    next_state = np.reshape(next_state, [1, 6])
    
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

    ## Simulation Parameters
    stepIdx = 0
    cur_episode = 0

    ## Training Parameters
    epsilon = 1.0               # exploration rate
    epsilon_min = 0.01
    epsilon_decay = 0.999
    totalRebuf = 0
    actionHistory = []
    throughputHistory = []

    env = env_init()
    ob_space = env.observation_space
    ac_space = env.action_space
    model = load_model(args)


    while cur_episode < args.episodes:

        ep_cumulative_reward = 0
        ep_start_time = time.time()
        ep_cumulative_rebuffer = 0 
        print("Episode : ", cur_episode)
        state = get_state_array(env.reset())

        while True:
            stepIdx += 1

            if np.random.rand(1) < epsilon:
                action = np.random.randint(A_DIM)
            else:
                action = np.argmax(model.predict(state)[0])
            obs, reward, done, info = env.step(action)
        
            if done:
                stepIdx = 0
                if cur_episode + 1 < args.episodes:
                    env.reset()
                break

            reward = get_reward(args,obs)
            next_state = get_state_array(obs)
   
            # Performance Statistics
            actionHistory.append(action)
            ep_cumulative_reward += reward
            ep_cumulative_rebuffer += obs['RebufferTime'] / M_IN_K
            

            # Train
            target = reward
            if not done:
                p = model.predict(next_state)
                target = (reward + 0.95 * np.amax(p))

            target_f = model.predict(state)
            target_f[0][action] = target
            model.fit(state, target_f, epochs=1, verbose=0)
            state = next_state
            if epsilon > epsilon_min: epsilon *= epsilon_decay


            if args.animate:
                
                throughputHistory.append(next_state[0][2] * M_IN_K)
                plt.clf()
                fig, ax = plt.subplots(2)
                
                ax[0].plot(actionHistory)
                ax[0].set(ylabel="Request Quality (0-8)")
                ax[0].set_xlim([0,TOTAL_VIDEO_CHUNKS])
                ax[0].set_ylim([0,A_DIM])
                
                ax[1].plot(throughputHistory)
                ax[1].set(ylabel="Est. Throughput")
                ax[1].set_xlim([0,TOTAL_VIDEO_CHUNKS])
                #ax[1].set_ylim([0,M_IN_K*2])
                
                plt.xlabel("Segment Index")
                fig.suptitle("Rep index over Time")
                fn = str(stepIdx)
                if len(fn) < len(str(TOTAL_VIDEO_CHUNKS)):
                    fn = "0" * ( len(str(TOTAL_VIDEO_CHUNKS)) - len(fn)) + fn

                fig.savefig('./saved-models/animation/' + fn +  '.png')
                #plt.show()
                plt.close(fig)


        cur_episode += 1
        print "Cumulative reward : " , ep_cumulative_reward , " Time : " , time.time() - ep_start_time , " Cumulative rebuffer : " , ep_cumulative_rebuffer
        


    save_model(args, model)
    
    plt.plot(actionHistory)
    plt.ylabel('Rep Index')
    plt.show()

def create_animation(args):
    os.chdir("saved-models/animation")
    os.system("ffmpeg -framerate 8 -pattern_type glob -i '*.png' -c:v libx264 -r 30 -pix_fmt yuv420p " + args.animate +".mp4")
    os.system('find . -name "*.png" -type f -delete')


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
    parser.add_argument('--reward', type=str, default="default",
                       help='Reward function, options : quality, rebuff, rebuff_lowvar, default, log, hd')
    parser.add_argument('--animate', type=str, default=None,
                       help='Create mp4 animation of result, give filename, default none')
    return parser.parse_args()



if __name__ == "__main__":
    try:
        args = get_args()
        set_video_dimensions(args)
        main(args)
        if args.animate:
            create_animation(args)
    except (KeyboardInterrupt, SystemExit):
        print("Ctrl-C -> Exit")
    finally:
        
        print("Done")


