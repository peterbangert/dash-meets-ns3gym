#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import argparse
from ns3gym import ns3env
import matplotlib.pyplot as plt
import numpy as np


startSim = 0
iterationNum = 5

port = 5555
simTime = 20 # seconds
stepTime = 1.0  # seconds
seed = 0
simArgs = {"--simTime": simTime,
           "--testArg": 123}
debug = False

env = ns3env.Ns3Env(port=port, stepTime=stepTime, startSim=startSim, simSeed=seed, simArgs=simArgs, debug=debug)
# simpler:
#env = ns3env.Ns3Env()
env.reset()

ob_space = env.observation_space
ac_space = env.action_space
print("Observation space: ", ob_space,  ob_space.dtype)
print("Action space: ", ac_space, ac_space.dtype)

stepIdx = 0
currIt = 0

probabilities = np.ones(ac_space.n)

actionHistory = []

try:
    while True:
        print("Start iteration: ", currIt)
        obs = env.reset()
        print("Step: ", stepIdx)
        print("---obs:", obs)

        while True:
            stepIdx += 1
            #action = env.action_space.sample()
            norm_probs = probabilities / np.sum(probabilities)
            action = np.random.choice(ac_space.n, p=norm_probs)
            actionHistory.append(action)

            
            print("---action: ", action)

            print("Step: ", stepIdx)
            obs, reward, done, info = env.step(action)

            probabilities[action] += reward
            print("---obs, reward, done, info: ", obs, reward, done, info)

            if done:
                stepIdx = 0
                if currIt + 1 < iterationNum:
                    env.reset()
                break

        currIt += 1
        if currIt == iterationNum:
            break

except (KeyboardInterrupt, SystemExit):
    print("Ctrl-C -> Exit")
finally:
    env.close()
    print("Done")


print(actionHistory)
plt.plot(actionHistory)
plt.ylabel('Rep Index')
plt.show()
