#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import gym
import tensorflow as tf

from ns3gym import ns3env
import argparse
from lib.objects import Observations, Args, Action
from networks.ns3gym_network import Ns3gymNetwork
from lib.utils import animate, ns3run, load_yaml

def main(args):

    observations = Observations(args)
    
    ns3run(args)
    env = ns3env.Ns3Env(port=observations.port,
        stepTime=observations.stepTime,
        startSim=observations.startSim, 
        simSeed=observations.seed, 
        simArgs=observations.simArgs, 
        debug=observations.debug)
    
    obs = env.reset()
    observations.update_observations(obs)

    ob_space = env.observation_space
    ac_space = env.action_space

    action = Action(ac_space)
    print(action)
    print(observations)

    print("action space", ac_space)
    print("obs space : ", ob_space)

    NN = Ns3gymNetwork(args, observations, action)


    for episode in range(args.episodes):
        
        while (observations.segmentCounter < observations.total_video_chunks -1): 

            NN.forward_pass()
            print(action.getAction())
            obs, reward, done, info = env.step(action.getAction())

            observations.update_observations(obs)

            NN.train()

        if (episode < (args.episodes -1)):
            ## Reset Environment
            ns3run(args)
            obs = env.reset()
            observations.reset(args, obs)

    if args.animate:
        animate(observations)    



def get_args():
    parser = argparse.ArgumentParser(description='Arguments for Neural Network')
    parser.add_argument('--segment_size_file', type=str, help='Location of segment size file, default is in repo')
    parser.add_argument('--episodes', type=int, help='No of training episodes, default 1')
    parser.add_argument('--save_model',type=str, help='Save the trained model, filename')
    parser.add_argument('--load_model', type=str, help='Name of saved model, searches saved-models/')
    parser.add_argument('--reward', type=str, help='Reward function, options : quality, rebuff, rebuff_lowvar, default, log, hd')
    parser.add_argument('--animate', type=str, help='Create mp4 animation of result, give filename, default none')
    parser.add_argument('--medium', type=str, help='wifi or ethernet')
    parser.add_argument('--competitors', type=str, help='Number of devices competing on LAN')
    parser.add_argument('--network', type=str, help='Choose which network the agent should use to decide')
    args = Args({**load_yaml('conf.d/simulation.yml'), **{ k:v for k,v in vars(parser.parse_args()).items() if v is not None}})
    return  args



if __name__ == "__main__":
    try:
        args = get_args()
        main(args)
    except (KeyboardInterrupt, SystemExit):
        print("Ctrl-C -> Exit")
    finally:
        print("Done")


