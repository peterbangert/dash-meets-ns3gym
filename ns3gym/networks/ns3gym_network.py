
from .pensive_interface import Pensive_Interface
from .simple_nn import Simple_NN

class Ns3gymNetwork():

    def __init__(self, args, observations, action):
        if observations.network == "pensive":
            self.network = Pensive_Interface(args, observations, action)
        if observations.network == "simple-nn":
            self.network = Simple_NN(args, observations, action)

    def forward_pass(self):
        self.network.forward_pass()

    def train(self):
        self.network.train()