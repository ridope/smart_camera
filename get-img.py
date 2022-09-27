#!/usr/bin/python3
import serial
import os, fnmatch
import sys
import traceback
import subprocess
import numpy as np
from scipy.io import savemat
from PIL import Image
import threading, queue
import time
from struct import *
from enum import Enum
import matplotlib.pyplot as plt
import schedule

cmd = Enum('CMD_TYPE', 'REBOOT TRANS_PHOTO TRANS_FFT TRANS_IFFT PHOTO_SIZE START_TRANS STOP_TRANS OP_TIME HELP CAMERA_RST CAMERA_TRIG CAMERA_EXPO CAMERA_AVG CAMERA_SIZE CAMERA_FOV CAMERA_IMG VGA_SIZE NULL_CMD', start=48)

tx_buffer = queue.Queue()
rx_buffer = queue.Queue()
uart = serial.Serial("/dev/ttyUSB0", 115200)

next_img = True
image_name = ""

tx_buffer.put("\n".encode())

def tx():
    while True:
        item = tx_buffer.get()
        uart.write(item)
        uart.write('\n'.encode())
        tx_buffer.task_done()

def rx():
    while True:
        if uart.in_waiting != 0:
            item = uart.read_until()
            if(len(item) > 2):
                if(item[1] >= cmd.REBOOT.value and item[1] <= cmd.NULL_CMD.value):
                    format = "<cIffc"
                        
                    while(len(item) < calcsize(format)):
                        item += uart.read_until()

                    try:
                        item_temp = unpack(format, item)
                    except error as err:
                        print(err)
                        print("Got item len: ", len(item))


                    rx_buffer.put(item_temp)

                else:
                    item = item.decode()
                    #print(item, end="")
            
def get_img():
    global image_name, next_img
    N = 0
    M = 0
    time = 0
    cont = 0
    img_array = []

    while True:
        item = rx_buffer.get()
        rx_buffer.task_done() 

        if(item[1] == cmd.CAMERA_EXPO.value):
            print("Cam expo: ", int(item[2]))
        
        if(item[1] == cmd.CAMERA_AVG.value):
            print("Cam avg: ", int(item[2])) 

        if(item[1] == cmd.PHOTO_SIZE.value):
            N = int(item[2])
            M = int(item[3])
            #print('\033[1;35;48m'+ "pyGot size!" + '\033[1;37;0m')
            img_array = np.zeros(N*M, dtype=np.uint8)
        
        elif(item[1] == cmd.OP_TIME.value):
            time = item[2]
            #print("FFT time: ", time)
 
        elif(item[1] == cmd.START_TRANS.value):
            #print("pyGot start flag!")
            flag = item[1]

            while flag != cmd.STOP_TRANS.value:
                item = rx_buffer.get()
                rx_buffer.task_done() 

                flag = item[1]

                if(flag == cmd.TRANS_PHOTO.value):
                    pixel = item[2]

                    img_array[cont] = pixel
                    cont += 1

                    if(cont+1 == N*M):
                        #print("pyGot in the end!")
                        break

            #print("pyGot stop flag!")
            im_array_reshaped = np.reshape(img_array, (N,M))

            im_array_normalized = im_array_reshaped / im_array_reshaped.max() # normalize the data to 0 - 1
            im_array_normalized = 255 * im_array_normalized # Now scale by 255
            im_array_normalized = im_array_normalized.astype(np.uint8)

            im = Image.fromarray(im_array_reshaped, mode="L")
            im.convert("L"). save("result.png")
            plt.figure(1); plt.clf()
            plt.imshow(im, cmap='gray',vmin=0, vmax=255)
            #plt.show()
            plt.pause(1)

            N=0
            M=0
            cont = 0
            img_array = []
            next_img = True

def run_timer_func():
    while True:
        schedule.run_pending()
        time.sleep(1)

# turn-on the tx thread
threading.Thread(target=tx, daemon=True).start()
threading.Thread(target=rx, daemon=True).start()
threading.Thread(target=get_img, daemon=True).start()
threading.Thread(target=run_timer_func, daemon=True).start()

def send_get_cmd():
    data_send = pack("<iff", cmd.CAMERA_TRIG.value, 0,0)
    tx_buffer.put(data_send)


schedule.every(1).seconds.do(send_get_cmd)

try:
    print("Available commands: ")
    print("get             - Gets image from FPGA")
    print("reboot           - Reboots the RISCV")
    while True:        
        value = input()

        if(value=="expo"):
            expo_value = input("Exposition value: ")

            data_send = pack("<iff", cmd.CAMERA_EXPO.value, int(expo_value),0)
            tx_buffer.put(data_send)
        
        if(value=="re"):
            data_send = pack("<iff", cmd.REBOOT.value, 0,0)
            tx_buffer.put(data_send)


except KeyboardInterrupt:
        traceback.print_exc(file=sys.stdout)
        tx_buffer.join()    
        rx_buffer.join()      
        uart.close()        
        print("\nGoodbye!\n")
        sys.exit()
except:
    traceback.print_exc(file=sys.stdout)
    tx_buffer.join()    
    rx_buffer.join()     
    uart.close()     
    sys.exit()

