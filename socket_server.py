#!/usr/bin/env python3

import socket

import numpy as np
import json
import time
import random
import pickle
import os

HOST = '192.168.5.111'  # Standard loopback interface address (localhost)
PORT = 65431            # Port to listen on (non-privileged ports are > 1023)

sample = 0

def get_graph_data():

    global sample

    sample += 1

    graph_to_send = json.dumps({
        'x':random.uniform(-1,1),
        'y':random.uniform(-1,1),
        'z':random.uniform(-1,1),
        's':sample
    })
    return graph_to_send

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.bind((HOST, PORT))
    s.listen()
    conn, addr = s.accept()
    with conn:
        print('Connected by', addr)
        a = 0
        accel_arr = np.zeros((1000,3))
        while a < 1000:
            data = conn.recv(1024)
            dec = data.decode()
            if (dec!=""):
                x = dec.split(",")[:-1]
                print("x=",x)
                n_data = int(len(x)/3)
                for b in range(n_data):
                    accel_arr[a] = np.array(x[3*b:3*(b+1)])
                    a = a+1
                    print(x[3*(b-1):3*b])
                #print(dec)
        with open('accel_arr', 'wb') as f:
            pickle.dump(accel_arr,f)
            #print(type(data))
            #conn.sendall(get_graph_data().encode('utf-8'))

