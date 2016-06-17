import subprocess

def ahrs():
	subprocess.call(["./vn100_linux_basic"])

def count():
	count = 1
	while(count < 11):
		print str(count)
		count += 1


ahrs()
count()