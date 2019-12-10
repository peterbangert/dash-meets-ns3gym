#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import argparse
from ns3gym import ns3env
import matplotlib.pyplot as plt
import numpy as np
import http.client
import json

startSim = 0
iterationNum = 5

port = 5555;
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
actionHistory = []


## Setup Pensive Gateway

headers = {"Content-type": "application/json",
            "Accept": "text/plain"}
body = {"RebufferTime":0,
        #"pastThroughput":"0",
        "lastRequest":0,
        "lastquality":0,
        "lastChunkFinishTime":0,
        "lastChunkStartTime":0,
        "lastChunkSize":0,
        "buffer":0
        }
conn = http.client.HTTPConnection("localhost", 8333)


try:
    while True:
        print("Start iteration: ", currIt)
        obs = env.reset()
        print("Step: ", stepIdx)
        print("---obs:", obs)

        while True:
            stepIdx += 1
            
            if stepIdx == 1:
                params = json.dumps(body)
            else:
                params = json.dumps(obs)

            conn.request("POST", "", params, headers)
            response = conn.getresponse()
            print(response.status, response.reason)
            
            action =  int(response.read())
            print( action)
            actionHistory.append(action)

            
            print("---action: ", action)
            print("Step: ", stepIdx)
            obs, reward, done, info = env.step(action)

            

            print("---obs, reward, done, info: ", obs, reward, done, info)

            # Convert nano seconds to seconds
            obs["lastChunkFinishTime"] = float(obs["lastChunkFinishTime"]) / 1000000
            obs["lastChunkStartTime"] = float(obs["lastChunkStartTime"]) / 1000000
            obs["buffer"] = float(obs["buffer"]) / 1000000
            obs["lastChunkSize"] = float(obs["lastChunkSize"]) / 1000000


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
