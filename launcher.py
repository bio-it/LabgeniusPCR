import serial
import sys
import time

arg = sys.argv

if len(arg) != 2:
	print('python launcher.py <PORT>')
	sys.exit()

pcr = serial.Serial(port=arg[1], baudrate=115200)

protocol = [
  [0  , 95, 180],
  [1  , 95, 10 ],
  [2  , 60, 30 ],
  [3  , 72, 30 ],
  [250, 1 , 34 ],
  [4  , 95, 10 ],
  [5  , 50, 30 ]
]

index      = 0
remain     = 0
reaminGoto = 0

setFlag    = True
time.sleep(1)

print(pcr.writable())
print(pcr.readable())

while True:
	result = pcr.write('C'.encode('utf-8'))
	print(result)
	result = pcr.readline().decode()
	print(result)

	"""
pcr.write('T40'.encode())

#pcr.close()

time.sleep(1)

while True:
	if setFlag and index < 7:
		pcr.write('T ', PROTOCOL_SET[index][1])
		setFlag = False

	pcr.write('C ', PROTOCOL_SET[index][1])

	if (abs(Protocol.Message - PROTOCOL_SET[index][1]) <= 1):
		remain++

	if (remain == PROTOCOL_SET[index][2]):
		setFlag = True
		remain = 0
		index++

	if (PROTOCOL_SET[index][0] == 250):
		if (PROTOCOL_SET[index][2] == remainGoto):
			remainGoto = 0
			index++
		else:
			remainGoto++
			index = PROTOCOL_SET[index][1]

time.sleep(1)
"""