import subprocess
import threading
import time

def ahrs():
	subprocess.call(["./vn100_linux_basic"])

def count():
	count = 1
	while(count < 11):
		print str(count)
		count += 1
		time.sleep(1)


class AhrsThread(threading.Thread):
	def __init__(self, threadID):
		threading.Thread.__init__(self)
        	self.threadID = threadID
	
	def run(self):
		ahrs()

class MainThread(threading.Thread):
	def __init__(self, threadID):
		threading.Thread.__init__(self)
        	self.threadID = threadID
	
	def run(self):
		count()


thread1 = AhrsThread(1)
thread2 = MainThread(2)

thread1.start()
thread2.start()