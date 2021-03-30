#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import yaml
import matplotlib as mpl
import matplotlib.pyplot as plt
import os
import subprocess
from os import system
import libtmux

def ns3run(args):

    
    
    command = ['./waf',  '--run="', 'tcp-stream-compete']

    print(vars(args))
    for k,v in vars(args).items():
        command.append("--" + k + "=" + str(v))
    command.append('"')

    server = libtmux.Server()
    if (server.has_session('ns3')):
        session = server.find_where({'session_name':'ns3'})
        pane = session.attached_pane
        pane.send_keys("cowsay 'here again'")
        pane.send_keys(" ".join(command))
    else :
        session = server.new_session(session_name="ns3")
        window = session.new_window(attach=True, window_name='ns3run')
        pane =   window.attached_pane
        pane.send_keys("cd ../../../")
        pane.send_keys("ls")
        pane.send_keys("cowsay 'new session'")
        pane.send_keys(" ".join(command))
        

def load_yaml(file):
    dict = None
    with open(file,'r') as stream:
            try:
                dict = yaml.load(stream, Loader=yaml.Loader)
            except yaml.YAMLError as exc:
                print(exc)
    return dict

def animate(obs):
        
    plt.clf()
    fig, ax = plt.subplots(2)
    
    ax[0].plot(obs.actionHistory)
    ax[0].set(ylabel="Request Quality (0-8)")
    ax[0].set_xlim([0,obs.total_video_chunks])
    ax[0].set_ylim([0,obs.adim])
    
    ax[1].plot(obs.throughputHistory)
    ax[1].set(ylabel="Est. Throughput")
    ax[1].set_xlim([0,obs.total_video_chunks])
    #ax[1].set_ylim([0,M_IN_K*2])
    
    plt.xlabel("Segment Index")
    fig.suptitle("Rep index over Time")
    fn = "file"
    if len(fn) < len(str(obs.total_video_chunks)):
        fn = "0" * ( len(str(obs.total_video_chunks)) - len(fn)) + fn

    fig.savefig('./saved-models/animation/' + fn +  '.png')
    #plt.show()
    plt.close(fig)

def plot_actions(obs):

    plt.plot(obs.actionHistory)
    plt.ylabel('Rep Index')
    plt.show()

def create_animation(args):
    os.chdir("saved-models/animation")
    os.system("ffmpeg -framerate 8 -pattern_type glob -i '*.png' -c:v libx264 -r 30 -pix_fmt yuv420p " + args.animate +".mp4")
    os.system('find . -name "*.png" -type f -delete')
