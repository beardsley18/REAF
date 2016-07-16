import serial
import time
import random
import threading
# ls /dev/tty*
ser = serial.Serial('/dev/ttyACM0',57600)
# false if off, true if on
t1_status = False
t2_status = False
t3_status = False
t4_status = False

t1_stat_changed = False
t2_stat_changed = False
t3_stat_changed = False
t4_stat_changed = False

def send(lett):
    global t1_stat_changed
    global t2_stat_changed
    global t3_stat_changed
    global t4_stat_changed
    global t1_status
    global t2_status
    global t3_status
    global t4_status
    while True:
        ser.write(lett)
        time.sleep(.5)
        if lett == 'a':
            if t1_stat_changed:
                t1_stat_changed = False
                if t1_status:
                    t1_status = False
                else:
                    t1_status = True
                break
        elif lett == 'b':
            if t2_stat_changed:
                t2_stat_changed = False
                if t2_status:
                    t2_status = False
                else:
                    t2_status = True
                break
        elif lett == 'c':
            if t3_stat_changed:
                t3_stat_changed = False
                if t3_status:
                    t3_status = False
                else:
                    t3_status = True
                break
        elif lett == 'd':
            if t4_stat_changed:
                t4_stat_changed = False
                if t4_status:
                    t4_status = False
                else:
                    t4_status = True
                break

class readThread(threading.Thread):
	def __init__(self, threadID):
	    threading.Thread.__init__(self)
            self.threadID = threadID
	
	def run(self):
            global t1_stat_changed
            global t2_stat_changed
            global t3_stat_changed
            global t4_stat_changed
            while True:
                outFile = open('output_pressure_thruster_test.txt', 'a')
                line = ser.readline()
                if 'Receiving' in line:
                    # print line[len(line)-3]
                    # print '\'' + line + '\''
                    if line[len(line)-3] == 'a':
                        t1_stat_changed = True
                    elif line[len(line)-3] == 'b':
                        t2_stat_changed = True
                    elif line[len(line)-3] == 'c':
                        t3_stat_changed = True
                    elif line[len(line)-3] == 'd':
                        t4_stat_changed = True
                # else:
                outFile.write(line)
                outFile.close()

class writeThread(threading.Thread):
	def __init__(self, threadID):
	    threading.Thread.__init__(self)
            self.threadID = threadID
	
	def run(self):
            send('b')
            send('a')
            for x in range(0,50000):
                print 'Sending a'
                print x

            send('a')
            for x in range(0, 50000):
                # ser.write('b')
                print 'Sending b'
                print x

            send('b')
            for x in range(0, 50000):
                print 'Sending both'
                print x

            send('a')
            send('b')

            for x in range(0, 50000):
                print x

            send('b')
    
   
thread1 = writeThread(1)
thread2 = readThread(2)
thread1.start()
thread2.start()
