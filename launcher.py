import serial
import sys
import time

arg = sys.argv

if len(arg) != 2:
	print('python launcher.py <PORT>')
	sys.exit()

pcr = serial.Serial(port=arg[1], baudrate=115200)

PROTOCOL_SET = [
  [1  , 95, 180],
  [2  , 95, 10 ],
  [3  , 60, 30 ],
  [4  , 72, 30 ],
  [250, 1 , 34 ],
  [5  , 95, 10 ],
  [6  , 50, 30 ]
]

index      = 0
remain     = 0
remainGoto = 0
count      = 0

temper     = 0

setFlag    = True
cntFlag    = False

time.sleep(3)

file = open('./log%d.txt' % time.time(), 'w')

while True:
	startTime = time.time()

	if index < len(PROTOCOL_SET):
		if setFlag:
			pcr.write(('T' + str(PROTOCOL_SET[index][1])).encode())
			setFlag = False
	else:
		break

	pcr.write('C'.encode())
	raw = pcr.readline().decode()[2:].split(', ')
	temper = int(raw[0]) + float(raw[1]) * 0.01

	if abs(temper - PROTOCOL_SET[index][1]) <= 1:
		cntFlag = True

	if cntFlag:
		remain += 1

	if remain == PROTOCOL_SET[index][2]:
		setFlag = True
		cntFlag = False
		remain = 0
		index += 1

	if PROTOCOL_SET[index][0] == 250:
		if PROTOCOL_SET[index][2] == remainGoto:
			remainGoto = 0
			index += 1
		else:
			remainGoto += 1
			index = PROTOCOL_SET[index][1]

	count += 1
	file.write('%d %.2f\n' % (count, temper))

	print('%.2f/%d' % (temper, PROTOCOL_SET[index][1]))
	print('index %d/%d, remain %d/%d, goto %d' % (index + 1, len(PROTOCOL_SET), remain, PROTOCOL_SET[index][2], remainGoto))
	
	while time.time() - startTime < 1:
		pass

file.close()

print('end.')