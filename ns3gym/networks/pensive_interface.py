import requests
import http.client
import json
import libtmux
import time
import random

class Pensive_Interface():

    headers = {"Content-type": "application/json",
            "Accept": "text/plain"}


    def __init__(self, args, observations, action, clientId):
        self.action = action
        self.observations = observations
        self.clientId = clientId
        self.pensive_run()
        time.sleep(5)
        print(8333+clientId)
        
        self.conn = http.client.HTTPConnection("localhost", 8333 + clientId, timeout=10)


    def pensive_run(self):  
        server = libtmux.Server()
        if (server.has_session('pensive' + str(self.clientId))):
            session = server.find_where({'session_name':'pensive' + str(self.clientId)})
            pane = session.attached_pane
            pane.send_keys("cowsay 'pensive'")
            pane.send_keys("python rl_server_no_training.py "+ str(self.clientId))
        else :
            session = server.new_session(session_name="pensive" + str(self.clientId))
            window = session.new_window(attach=True, window_name='penvise_server')
            pane =   window.attached_pane
            pane.send_keys("cd networks")
            pane.send_keys("cowsay 'new pensive'")
            pane.send_keys("python rl_server_no_training.py " + str(self.clientId))

    def payload(self):
        return {"RebufferTime":self.observations.rebuffer_time* self.observations.seconds_in_micro,
            "lastRequest": self.observations.segmentCounter,
            "lastquality": self.observations.playbackIndex[-1],
            "lastChunkFinishTime":float(self.observations.transmissionEnd[-1]) * self.observations.seconds_in_micro,
            "lastChunkStartTime":float(self.observations.transmissionRequested[-1]) * self.observations.seconds_in_micro,
            "lastChunkSize": float(self.observations.bytesReceived[-1]) * self.observations.mega_in_bytes,
            "buffer": self.observations.bufferLevelOld[-1] * self.observations.seconds_in_micro
            }

    def forward_pass(self):
        self.params = json.dumps(self.payload())

        self.conn.request("POST", "", self.params, self.headers)
        response = self.conn.getresponse()
        self.action.nextRepIndex =  int(response.read())


        download_time = (float(self.observations.transmissionEnd[-1]) - float(self.observations.transmissionRequested[-1])) * self.observations.seconds_in_micro
        size = float(self.observations.bytesReceived[-1]) * self.observations.mega_in_bytes
        throughput = size / download_time * 8  #bytes to bits
        print("throughput est (Mb/s) : ", throughput )
        #decide delay time
        #randbuf = random.uniform(28, 32)
        #if (self.observations.bufferLevelNew[-1]  > randbuf / self.observations.seconds_in_micro):
        #    self.action.nextDownloadDelay = self.observations.bufferLevelNew[-1] - randbuf / self.observations.seconds_in_micro
        #else:
        #    self.action.nextDownloadDelay = 0
        self.action.nextDownloadDelay = 0
    def train(self):
        return
