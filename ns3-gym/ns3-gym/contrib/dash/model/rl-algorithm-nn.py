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


startSim = 0
iterationNum = 4

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

model = keras.Sequential()
model.add(keras.layers.Dense(ob_space, input_shape=(ob_space,), activation='relu'))
model.add(keras.layers.Dense(ac_space, activation='softmax'))
model.compile(optimizer=tf.train.AdamOptimizer(0.001),
              loss='categorical_crossentropy',
              metrics=['accuracy'])

stepIdx = 0
currIt = 0
epsilon_min = 1.0               # exploration rate
epsilon_min = 0.01
epsilon_decay = 0.999

actionHistory = []

try:
    while True:
        print("Start iteration: ", currIt)
        obs = env.reset()
        print("Step: ", stepIdx)
        print("---obs:", obs)

        while True:
            stepIdx += 1

            if np.random.rand(1) < epsilon:
                action = np.random.randint(a_size)
            else:
                action = np.argmax(model.predict(state)[0])

            action = env.action_space.sample()
            actionHistory.append(action)

            
            print("---action: ", action)

            print("Step: ", stepIdx)
            obs, reward, done, info = env.step(action)
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
