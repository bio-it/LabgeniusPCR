import serial
import sys
import time

arg = sys.argv

if len(arg) != 2:
	print('python launcher.py <PORT>')
	sys.exit()

pcr = serial.Serial(port=arg[1], baudrate=115200)

PROTOCOL_SET = [
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

temper     = 0

setFlag    = True

time.sleep(3)

while True:
	if setFlag and index < 7:
		pcr.write(('T' + str(PROTOCOL_SET[index][1])).encode())
		setFlag = False

	pcr.write('C'.encode())
	temper = int(pcr.readline().decode()[2:])

	if (abs(temper - PROTOCOL_SET[index][1]) <= 1):
		remain += 1

	if (remain == PROTOCOL_SET[index][2]):
		setFlag = True
		remain = 0
		index += 1

	if (PROTOCOL_SET[index][0] == 250):
		if (PROTOCOL_SET[index][2] == remainGoto):
			remainGoto = 0
			index += 1
		else:
			remainGoto += 1
			index = PROTOCOL_SET[index][1]

	print('%d/%d' % (temper, PROTOCOL_SET[index][1]))
	print('index %d, remain %d/%d, goto %d' % (index, remain, PROTOCOL_SET[index][2], reaminGoto))

	time.sleep(1)