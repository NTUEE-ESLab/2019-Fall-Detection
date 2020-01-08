#!/usr/bin/env python3
import RPi.GPIO as GPIO;
from time import sleep;
GPIO.setmode(GPIO.BCM)

import socket

import numpy as np
import json
import time
import random
import pickle
import math
import os
import csv
import sys

HOST = '192.168.1.237'  # Standard loopback interface address (localhost)
PORT = 65431            # Port to listen on (non-privileged ports are > 1023)

sample = 0

POUT1 = 18 #red
POUT2 = 23 #green

GPIO.setup(POUT1, GPIO.OUT);
GPIO.setup(POUT2, GPIO.OUT);
GPIO.output(POUT1, False);
GPIO.output(POUT2, False);

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
        #accel_arr = np.zeros((1000,3))
        #abs_acc = np.zeros(1000)
        try:
            while True:
                data = conn.recv(1024)
                dec = data.decode()
                movement_list = ["predict","fall","run","walk","still"]
                print(movement_list[int(dec)])
                if int(dec)==0:
                    GPIO.output(POUT2, True);
                    GPIO.output(POUT1, True);
                elif int(dec)==1:
                    GPIO.output(POUT1, True);
                    GPIO.output(POUT2, False);
                else:
                    GPIO.output(POUT1, False);
                    GPIO.output(POUT2, True);

        except KeyboardInterrupt:
            GPIO.cleanup() # clean up GPIO on CTRL+C exit
        GPIO.cleanup() # clean up GPIO on normal exit
                        """
            if (dec!=""):
                x = dec.split(",")[:-1]
                #print(x)
                n_data = int(len(x)/3)
                for b in range(n_data):
                    accel_arr[a] = np.array(x[3*b:3*(b+1)])
                    sq_sum = np.sum(accel_arr[a]**2)
                    print(math.sqrt(sq_sum))
                    abs_acc[a] = math.sqrt(sq_sum)
                    a = a+1
                    #print(x[3*(b-1):3*b])
                #print(dec)
            """
        """
        with open('accel_arr_'+sys.argv[1], 'wb') as f:
            pickle.dump(accel_arr,f)
            #print(type(data))
            #conn.sendall(get_graph_data().encode('utf-8'))
        with open('abs_acc_'+sys.argv[1]+'.csv', 'w', newline='', encoding='utf-8') as wf:
            writer = csv.writer(wf)
            for index in range (1000):
                writer.writerow([abs_acc[index]])
        """
