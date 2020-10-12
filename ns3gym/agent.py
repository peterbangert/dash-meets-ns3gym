#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#import gym
import tensorflow as tf

from ns3gym import ns3env
import argparse
from lib.objects import Observations, Args, Action
from networks.ns3gym_network import Ns3gymNetwork
from lib.utils import animate, ns3run, load_yaml

def main(args):

    print(args.ns3gymClients)
    obslist = [Observations(args)  for i in range(args.ns3gymClients)]
    observations = obslist[0]
    
    ns3run(args)
    env = ns3env.Ns3Env(port=observations.port,
        stepTime=observations.stepTime,
        startSim=observations.startSim, 
        simSeed=observations.seed, 
        simArgs=observations.simArgs, 
        debug=observations.debug)
    
    obs = env.reset()
    #observations.update_observations(obs)
    for ob in obslist:
        ob.update_observations(obs)

    ob_space = env.observation_space
    ac_space = env.action_space

    action = Action(ac_space)
    print(action)
    print(observations)

    print("action space", ac_space)
    print("obs space : ", ob_space)

    NN_list = [Ns3gymNetwork(args, obslist[i], action, i) for i in range(args.ns3gymClients)] 


    for episode in range(observations.episodes):
        
        while True: 

            NN = NN_list[obs['clientId']]
            observations = obslist[obs['clientId']]


            NN.forward_pass()
            print(action.getAction())
            obs, reward, done, info = env.step(action.getAction())
            
            NN = NN_list[obs['clientId']]
            observations = obslist[obs['clientId']]


            observations.update_observations(obs)

            NN.train()

        if (episode < (observations.episodes -1)):
            ## Reset Environment
            ns3run(args)
            obs = env.reset()
            observations.reset(args, obs)

    if observations.animate:
        animate(observations)    



def get_args():
    parser = argparse.ArgumentParser(description='Arguments for Neural Network')
    parser.add_argument('--simulationId', type=str, help='Simulation Id')
    parser.add_argument('--segmentSizeFilePath', type=str, help='Location of segment size file, default is in repo')
    #parser.add_argument('--bitRate', type=str, help='Bitrate between server and AP')
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


'''


def main(args):

    observations = [Observations(args) for i in args['ns3gymClients']]
    
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


    for episode in range(observations.episodes):
        
        while (observations.segmentCounter < observations.total_video_chunks ): 

            NN.forward_pass()
            print(action.getAction())
            obs, reward, done, info = env.step(action.getAction())

            observations.update_observations(obs)

            NN.train()

        if (episode < (observations.episodes -1)):
            ## Reset Environment
            ns3run(args)
            obs = env.reset()
            observations.reset(args, obs)

    if observations.animate:
        animate(observations)    


'''