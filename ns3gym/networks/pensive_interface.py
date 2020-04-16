import requests
import http.client
import json
import libtmux
import time
import random

class Pensive_Interface():

    headers = {"Content-type": "application/json",
            "Accept": "text/plain"}


    def __init__(self, args, observations, action):
        self.action = action
        self.observations = observations
        self.pensive_run()
        time.sleep(5)
        self.conn = http.client.HTTPConnection("localhost", 8333, timeout=10)


    def pensive_run(self):  
        server = libtmux.Server()
        if (server.has_session('pensive')):
            session = server.find_where({'session_name':'pensive'})
            pane = session.attached_pane
            pane.send_keys("cowsay 'pensive'")
            pane.send_keys("python rl_server_no_training.py")
        else :
            session = server.new_session(session_name="pensive")
            window = session.new_window(attach=True, window_name='penvise_server')
            pane =   window.attached_pane
            pane.send_keys("cd networks")
            pane.send_keys("cowsay 'new pensive'")
            pane.send_keys("python rl_server_no_training.py")

    def payload(self):
        return {"RebufferTime":self.observations.rebuffer_time* self.observations.seconds_in_micro,
            "lastRequest": self.observations.segmentCounter,
            "lastquality": self.observations.playbackIndex[-1],
            "lastChunkFinishTime":float(self.observations.transmissionEnd[-1]) * self.observations.seconds_in_micro,
            "lastChunkStartTime":float(self.observations.transmissionStart[-1]) * self.observations.seconds_in_micro,
            "lastChunkSize": float(self.observations.bytesReceived[-1]) * self.observations.mega_in_bytes,
            "buffer": self.observations.bufferLevelOld[-1] * self.observations.seconds_in_micro
            }

    def forward_pass(self):
        self.params = json.dumps(self.payload())

        self.conn.request("POST", "", self.params, self.headers)
        response = self.conn.getresponse()
        self.action.nextRepIndex =  int(response.read())

        #decide delay time
        randbuf = random.uniform(28, 32)
        if (self.observations.bufferLevelNew[-1]  > randbuf / self.observations.seconds_in_micro):
            self.action.nextDownloadDelay = self.observations.bufferLevelNew[-1] - randbuf / self.observations.seconds_in_micro
        else:
            self.action.nextDownloadDelay = 0

    def train(self):
        return