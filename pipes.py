import subprocess
p = subprocess.Popen(["./VectorNav/examples/vn100_linux_basic/vn100_linux_basic"])
input = p.communicate(input=None)
for val in input:
	print str(val) + 'python'
