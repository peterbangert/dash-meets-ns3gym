#!/usr/bin/env python

import socket
import logging



TCP_IP = '127.0.0.1'
TCP_PORT = 5005
BUFFER_SIZE = 1024

INIT = "INIT"
REP_INDEX = "REPINDEX"
END = "END"
NEXT_REP = "NEXTREP="
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind((TCP_IP, TCP_PORT))


def initialize_rl(numclients):
	# import gym
	return None

def rl_handler(data, client):
	# import gym
	repindex =1
	return str(repindex)

def rl_terminate(client):
	#import gym
	return None

"""
	AI proxy decouples ns3 simulation with RL Algorithm 
	Accepts TCP conn at port 5005
	Returns Next representative index based off parameters
"""
def ai_proxy():
	s.listen(1)
	conn, addr = s.accept()


	while 1:
		data = conn.recv(BUFFER_SIZE).decode('utf-8')

		print(data)
		if INIT in data:
			numclients, simid = data[5:].split(",")

			## Initialize gym with num clients
			initialize_rl(numclients)

			print("initializing")

			logging.basicConfig(filename='logs/clients_'+numclients+ "_simid_"+ simid +'.log',
								filemode='w', format='%(name)s - %(levelname)s - %(message)s',
								level=logging.DEBUG)
			logging.info('Connection address:' + addr[0])
			conn.sendall(bytes("Start\n", "utf-8"))

		elif REP_INDEX in data:
			logging.info('Request Rep Index')
			client = data.split("=")[1]
			nextrep = rl_handler(data, client)
			conn.sendall(bytes(NEXT_REP+nextrep + "\n", "utf-8"))

		elif END in data :
			logging.info('Simulation Ended')
			client = data.split("=")[1]
			# Inform AI sim has ended
			rl_terminate(client)
			break

		else:
			print(data)
			logging.error('Recieved Unexpected Message')
	conn.close()



if __name__ == "__main__": ai_proxy()