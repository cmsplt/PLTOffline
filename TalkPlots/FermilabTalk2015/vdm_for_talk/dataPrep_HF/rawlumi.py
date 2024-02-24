### Occupancy Lumi Ring 1 Threshold 1 ###

def RawLumiOcc1Th1BX(occ,bx):

	active = 0
	below = 0
	for hlx in range(36):

		below += occ[hlx].data[bx + 0*4096]
		active += occ[hlx].data[bx + 0*4096]
		active += occ[hlx].data[bx + 1*4096]
		active += occ[hlx].data[bx + 2*4096]

	import math
	try:
		return -math.log(below/float(active))
	except ZeroDivisionError:
		return 0

def RawLumiOcc1Th1BXErr(occ,bx):

	active = 0
	below = 0
	for hlx in range(36):

		below += occ[hlx].data[bx + 0*4096]
		active += occ[hlx].data[bx + 0*4096]
		active += occ[hlx].data[bx + 1*4096]
		active += occ[hlx].data[bx + 2*4096]

	import math
	try:
		return math.sqrt(1/float(below) - 1/float(active))
	except ZeroDivisionError:
		return 0

def RawLumiOcc1Th1(occ):
	return [RawLumiOcc1Th1BX(occ,bx) for bx in range(3564)]

def RawLumiOcc1Th1Err(occ):
	return [RawLumiOcc1Th1BXErr(occ,bx) for bx in range(3564)]

def RawLumiOcc1Th1BXHLX(occ,bx,hlx):

	active = 0
	below = 0
	
	below = occ[hlx].data[bx + 0*4096]
	active += occ[hlx].data[bx + 0*4096]
	active += occ[hlx].data[bx + 1*4096]
	active += occ[hlx].data[bx + 2*4096]

	import math
	try:
		return -math.log(below/float(active))
	except ZeroDivisionError:
		return 0

def RawLumiOcc1Th1HLX(occ,hlx):
	return [RawLumiOcc1Th1BXHLX(occ,bx,hlx) for bx in range(3564)] 

def RawLumiOcc1Th1BXHLXErr(occ,bx,hlx):

	active = 0
	below = 0
	
	below = occ[hlx].data[bx + 0*4096]
	active += occ[hlx].data[bx + 0*4096]
	active += occ[hlx].data[bx + 1*4096]
	active += occ[hlx].data[bx + 2*4096]

	import math
	try:
		return math.sqrt(1/float(below) - 1/float(active))
	except ZeroDivisionError:
		return 0

def RawLumiOcc1Th1HLXErr(occ,hlx):
	return [RawLumiOcc1Th1BXHLXErr(occ,bx,hlx) for bx in range(3564)] 

# + side
def RawLumiOcc1Th1BXPlus(occ,bx):

	hlxs = [6,7,8,9,10,11,18,19,20,21,22,23,30,31,32,33,34,35]

	active = 0
	below = 0
	for hlx in hlxs:

		below += occ[hlx].data[bx + 0*4096]
		active += occ[hlx].data[bx + 0*4096]
		active += occ[hlx].data[bx + 1*4096]
		active += occ[hlx].data[bx + 2*4096]

	import math
	try:
		return -math.log(below/float(active))
	except ZeroDivisionError:
		return 0


def RawLumiOcc1Th1Plus(occ):
	return [RawLumiOcc1Th1BXPlus(occ,bx) for bx in range(3564)]

def RawLumiOcc1Th1BXPlusErr(occ,bx):

	hlxs = [6,7,8,9,10,11,18,19,20,21,22,23,30,31,32,33,34,35]

	active = 0
	below = 0
	for hlx in hlxs:

		below += occ[hlx].data[bx + 0*4096]
		active += occ[hlx].data[bx + 0*4096]
		active += occ[hlx].data[bx + 1*4096]
		active += occ[hlx].data[bx + 2*4096]

	import math
	try:
		return math.sqrt(1/float(below) - 1/float(active))
	except ZeroDivisionError:
		return 0


def RawLumiOcc1Th1PlusErr(occ):
	return [RawLumiOcc1Th1BXPlusErr(occ,bx) for bx in range(3564)]

# - side
def RawLumiOcc1Th1BXMinus(occ,bx):

	hlxs = [0,1,2,3,4,5,12,13,14,15,16,17,24,25,26,27,28,29] 

	active = 0
	below = 0
	for hlx in hlxs:

		below += occ[hlx].data[bx + 0*4096]
		active += occ[hlx].data[bx + 0*4096]
		active += occ[hlx].data[bx + 1*4096]
		active += occ[hlx].data[bx + 2*4096]

	import math
	try: 
		return -math.log(below/float(active))
	except ZeroDivisionError:
		return 0


def RawLumiOcc1Th1Minus(occ):
	return [RawLumiOcc1Th1BXPlus(occ,bx) for bx in range(3564)]

def RawLumiOcc1Th1BXMinusErr(occ,bx):

	hlxs = [0,1,2,3,4,5,12,13,14,15,16,17,24,25,26,27,28,29] 

	active = 0
	below = 0
	for hlx in hlxs:

		below += occ[hlx].data[bx + 0*4096]
		active += occ[hlx].data[bx + 0*4096]
		active += occ[hlx].data[bx + 1*4096]
		active += occ[hlx].data[bx + 2*4096]

	import math
	try: 
		return math.sqrt(1/float(below) - 1/float(active))
	except ZeroDivisionError:
		return 0


def RawLumiOcc1Th1MinusErr(occ):
	return [RawLumiOcc1Th1BXPlusErr(occ,bx) for bx in range(3564)]

### Occupancy Lumi Ring 1 Threshold 2 ###

def RawLumiOcc1Th2BX(occ,bx):

	active = 0
	below = 0
	for hlx in range(36):

		below += occ[hlx].data[bx + 0*4096]
		below += occ[hlx].data[bx + 1*4096]
		active += occ[hlx].data[bx + 0*4096]
		active += occ[hlx].data[bx + 1*4096]
		active += occ[hlx].data[bx + 2*4096]

	import math
	try:
		return -math.log(below/float(active))
	except ZeroDivisionError:
		return 0

def RawLumiOcc1Th2(occ):
	return [RawLumiOcc1Th2BX(occ,bx) for bx in range(3564)]

def RawLumiOcc1Th2BXErr(occ,bx):

	active = 0
	below = 0
	for hlx in range(36):

		below += occ[hlx].data[bx + 0*4096]
		below += occ[hlx].data[bx + 1*4096]
		active += occ[hlx].data[bx + 0*4096]
		active += occ[hlx].data[bx + 1*4096]
		active += occ[hlx].data[bx + 2*4096]

	import math
	try:
		return math.sqrt(1/float(below) - 1/float(active))
	except ZeroDivisionError:
		return 0

def RawLumiOcc1Th2Err(occ):
	return [RawLumiOcc1Th2BXErr(occ,bx) for bx in range(3564)]

def RawLumiOcc1Th2BXHLX(occ,bx,hlx):

	active = 0
	below = 0
	
	below += occ[hlx].data[bx + 0*4096]
	below += occ[hlx].data[bx + 1*4096]
	active += occ[hlx].data[bx + 0*4096]
	active += occ[hlx].data[bx + 1*4096]
	active += occ[hlx].data[bx + 2*4096]

	import math
	try:
		return -math.log(below/float(active))
	except ZeroDivisionError:
		return 0

def RawLumiOcc1Th2HLX(occ,hlx):
	return [RawLumiOcc1Th2BXHLX(occ,bx,hlx) for bx in range(3564)] 

def RawLumiOcc1Th2BXHLXErr(occ,bx,hlx):

	active = 0
	below = 0
	
	below += occ[hlx].data[bx + 0*4096]
	below += occ[hlx].data[bx + 1*4096]
	active += occ[hlx].data[bx + 0*4096]
	active += occ[hlx].data[bx + 1*4096]
	active += occ[hlx].data[bx + 2*4096]

	import math
	try:
		return math.sqrt(1/float(below) - 1/float(active))
	except ZeroDivisionError:
		return 0

def RawLumiOcc1Th2HLXErr(occ,hlx):
	return [RawLumiOcc1Th2BXHLXErr(occ,bx,hlx) for bx in range(3564)] 

# + side
def RawLumiOcc1Th2BXPlus(occ,bx):

	hlxs = [6,7,8,9,10,11,18,19,20,21,22,23,30,31,32,33,34,35]

	active = 0
	below = 0
	for hlx in hlxs:

		below += occ[hlx].data[bx + 0*4096]
		below += occ[hlx].data[bx + 1*4096]
		active += occ[hlx].data[bx + 0*4096]
		active += occ[hlx].data[bx + 1*4096]
		active += occ[hlx].data[bx + 2*4096]

	import math
	try:
		return -math.log(below/float(active))
	except ZeroDivisionError:
		return 0

def RawLumiOcc1Th2Plus(occ):
	return [RawLumiOcc1Th2BXPlus(occ,bx) for bx in range(3564)]

def RawLumiOcc1Th2BXPlusErr(occ,bx):

	hlxs = [6,7,8,9,10,11,18,19,20,21,22,23,30,31,32,33,34,35]

	active = 0
	below = 0
	for hlx in hlxs:

		below += occ[hlx].data[bx + 0*4096]
		below += occ[hlx].data[bx + 1*4096]
		active += occ[hlx].data[bx + 0*4096]
		active += occ[hlx].data[bx + 1*4096]
		active += occ[hlx].data[bx + 2*4096]

	import math
	try:
		return math.sqrt(1/float(below) - 1/float(active))
	except ZeroDivisionError:
		return 0

def RawLumiOcc1Th2PlusErr(occ):
	return [RawLumiOcc1Th2BXPlusErr(occ,bx) for bx in range(3564)]

# - side
def RawLumiOcc1Th2BXMinus(occ,bx):

	hlxs = [0,1,2,3,4,5,12,13,14,15,16,17,24,25,26,27,28,29] 

	active = 0
	below = 0
	for hlx in hlxs:

		below += occ[hlx].data[bx + 0*4096]
		below += occ[hlx].data[bx + 1*4096]
		active += occ[hlx].data[bx + 0*4096]
		active += occ[hlx].data[bx + 1*4096]
		active += occ[hlx].data[bx + 2*4096]

	import math
	try: 
		return -math.log(below/float(active))
	except ZeroDivisionError:
		return 0

def RawLumiOcc1Th2Minus(occ):
	return [RawLumiOcc1Th2BXPlus(occ,bx) for bx in range(3564)]

def RawLumiOcc1Th2BXMinusErr(occ,bx):

	hlxs = [0,1,2,3,4,5,12,13,14,15,16,17,24,25,26,27,28,29] 

	active = 0
	below = 0
	for hlx in hlxs:

		below += occ[hlx].data[bx + 0*4096]
		below += occ[hlx].data[bx + 1*4096]
		active += occ[hlx].data[bx + 0*4096]
		active += occ[hlx].data[bx + 1*4096]
		active += occ[hlx].data[bx + 2*4096]

	import math
	try: 
		return math.sqrt(1/float(below) - 1/float(active))
	except ZeroDivisionError:
		return 0

def RawLumiOcc1Th2MinusErr(occ):
	return [RawLumiOcc1Th2BXPlusErr(occ,bx) for bx in range(3564)]

### Occupancy Lumi Ring 2 Threshold 1 ###

def RawLumiOcc2Th1BX(occ,bx):

	active = 0
	below = 0
	for hlx in range(36):

		below += occ[hlx].data[bx + 3*4096]
		active += occ[hlx].data[bx + 3*4096]
		active += occ[hlx].data[bx + 4*4096]
		active += occ[hlx].data[bx + 5*4096]

	import math
	try:
		return -math.log(below/float(active))
	except ZeroDivisionError:
		return 0

def RawLumiOcc2Th1(occ):
	return [RawLumiOcc2Th1BX(occ,bx) for bx in range(3564)]

def RawLumiOcc2Th1BXErr(occ,bx):

	active = 0
	below = 0
	for hlx in range(36):

		below += occ[hlx].data[bx + 3*4096]
		active += occ[hlx].data[bx + 3*4096]
		active += occ[hlx].data[bx + 4*4096]
		active += occ[hlx].data[bx + 5*4096]

	import math
	try:
		return math.sqrt(1/float(below) - 1/float(active))
	except ZeroDivisionError:
		return 0

def RawLumiOcc2Th1Err(occ):
	return [RawLumiOcc2Th1BXErr(occ,bx) for bx in range(3564)]

def RawLumiOcc2Th1BXHLX(occ,bx,hlx):

	active = 0
	below = 0
	
	below += occ[hlx].data[bx + 3*4096]
	active += occ[hlx].data[bx + 3*4096]
	active += occ[hlx].data[bx + 4*4096]
	active += occ[hlx].data[bx + 5*4096]

	import math
	try:
		return -math.log(below/float(active))
	except ZeroDivisionError:
		return 0

def RawLumiOcc2Th1HLX(occ,hlx):
	return [RawLumiOcc2Th1BXHLX(occ,bx,hlx) for bx in range(3564)] 

def RawLumiOcc2Th1BXHLXErr(occ,bx,hlx):

	active = 0
	below = 0
	
	below += occ[hlx].data[bx + 3*4096]
	active += occ[hlx].data[bx + 3*4096]
	active += occ[hlx].data[bx + 4*4096]
	active += occ[hlx].data[bx + 5*4096]

	import math
	try:
		return math.sqrt(1/float(below) - 1/float(active))
	except ZeroDivisionError:
		return 0

def RawLumiOcc2Th1HLXErr(occ,hlx):
	return [RawLumiOcc2Th1BXHLXErr(occ,bx,hlx) for bx in range(3564)] 

# + side
def RawLumiOcc2Th1BXPlus(occ,bx):

	hlxs = [6,7,8,9,10,11,18,19,20,21,22,23,30,31,32,33,34,35]

	active = 0
	below = 0
	for hlx in hlxs:

		below += occ[hlx].data[bx + 3*4096]
		active += occ[hlx].data[bx + 3*4096]
		active += occ[hlx].data[bx + 4*4096]
		active += occ[hlx].data[bx + 5*4096]

	import math
	try:
		return -math.log(below/float(active))
	except ZeroDivisionError:
		return 0


def RawLumiOcc2Th1Plus(occ):
	return [RawLumiOcc2Th1BXPlus(occ,bx) for bx in range(3564)]

def RawLumiOcc2Th1BXPlusErr(occ,bx):

	hlxs = [6,7,8,9,10,11,18,19,20,21,22,23,30,31,32,33,34,35]

	active = 0
	below = 0
	for hlx in hlxs:

		below += occ[hlx].data[bx + 3*4096]
		active += occ[hlx].data[bx + 3*4096]
		active += occ[hlx].data[bx + 4*4096]
		active += occ[hlx].data[bx + 5*4096]

	import math
	try:
		return math.sqrt(1/float(below) - 1/float(active))
	except ZeroDivisionError:
		return 0


def RawLumiOcc2Th1PlusErr(occ):
	return [RawLumiOcc2Th1BXPlusErr(occ,bx) for bx in range(3564)]

# - side
def RawLumiOcc2Th1BXMinus(occ,bx):

	hlxs = [0,1,2,3,4,5,12,13,14,15,16,17,24,25,26,27,28,29] 

	active = 0
	below = 0
	for hlx in hlxs:

		below += occ[hlx].data[bx + 3*4096]
		active += occ[hlx].data[bx + 3*4096]
		active += occ[hlx].data[bx + 4*4096]
		active += occ[hlx].data[bx + 5*4096]

	import math
	try: 
		return -math.log(below/float(active))
	except ZeroDivisionError:
		return 0


def RawLumiOcc2Th1Minus(occ):
	return [RawLumiOcc2Th1BXPlus(occ,bx) for bx in range(3564)]

def RawLumiOcc2Th1BXMinusErr(occ,bx):

	hlxs = [0,1,2,3,4,5,12,13,14,15,16,17,24,25,26,27,28,29] 

	active = 0
	below = 0
	for hlx in hlxs:

		below += occ[hlx].data[bx + 3*4096]
		active += occ[hlx].data[bx + 3*4096]
		active += occ[hlx].data[bx + 4*4096]
		active += occ[hlx].data[bx + 5*4096]

	import math
	try: 
		return math.sqrt(1/float(below) - 1/float(active))
	except ZeroDivisionError:
		return 0


def RawLumiOcc2Th1MinusErr(occ):
	return [RawLumiOcc2Th1BXPlusErr(occ,bx) for bx in range(3564)]

### Occupancy Lumi Ring 2 Threshold 2 ###

def RawLumiOcc2Th2BX(occ,bx):

	active = 0
	below = 0
	for hlx in range(36):

		below += occ[hlx].data[bx + 3*4096]
		below += occ[hlx].data[bx + 4*4096]
		active += occ[hlx].data[bx + 3*4096]
		active += occ[hlx].data[bx + 4*4096]
		active += occ[hlx].data[bx + 5*4096]

	import math
	return -math.log(below/float(active))

def RawLumiOcc2Th2(occ):
	return [RawLumiOcc2Th2BX(occ,bx) for bx in range(3564)]

def RawLumiOcc2Th2BXErr(occ,bx):

	active = 0
	below = 0
	for hlx in range(36):

		below += occ[hlx].data[bx + 3*4096]
		below += occ[hlx].data[bx + 4*4096]
		active += occ[hlx].data[bx + 3*4096]
		active += occ[hlx].data[bx + 4*4096]
		active += occ[hlx].data[bx + 5*4096]

	import math
	return math.log(1/float(below) - 1/float(active))

def RawLumiOcc2Th2Err(occ):
	return [RawLumiOcc2Th2BXErr(occ,bx) for bx in range(3564)]

def RawLumiOcc2Th2BXHLX(occ,bx,hlx):

	active = 0
	below = 0
	
	below += occ[hlx].data[bx + 3*4096]
	below += occ[hlx].data[bx + 4*4096]
	active += occ[hlx].data[bx + 3*4096]
	active += occ[hlx].data[bx + 4*4096]
	active += occ[hlx].data[bx + 5*4096]

	import math
	try:
		return -math.log(below/float(active))
	except ZeroDivisionError:
		return 0

def RawLumiOcc2Th2HLX(occ,hlx):
	return [RawLumiOcc2Th2BXHLX(occ,bx,hlx) for bx in range(3564)] 

def RawLumiOcc2Th2BXHLXErr(occ,bx,hlx):

	active = 0
	below = 0
	
	below += occ[hlx].data[bx + 3*4096]
	below += occ[hlx].data[bx + 4*4096]
	active += occ[hlx].data[bx + 3*4096]
	active += occ[hlx].data[bx + 4*4096]
	active += occ[hlx].data[bx + 5*4096]

	import math
	try:
		return math.log(1/float(below) - 1/float(active))
	except ZeroDivisionError:
		return 0

def RawLumiOcc2Th2HLXErr(occ,hlx):
	return [RawLumiOcc2Th2BXHLXErr(occ,bx,hlx) for bx in range(3564)] 

# + side
def RawLumiOcc2Th2BXPlus(occ,bx):

	hlxs = [6,7,8,9,10,11,18,19,20,21,22,23,30,31,32,33,34,35]

	active = 0
	below = 0
	for hlx in hlxs:

		below += occ[hlx].data[bx + 3*4096]
		below += occ[hlx].data[bx + 4*4096]
		active += occ[hlx].data[bx + 3*4096]
		active += occ[hlx].data[bx + 4*4096]
		active += occ[hlx].data[bx + 5*4096]

	import math
	try:
		return -math.log(below/float(active))
	except ZeroDivisionError:
		return 0

def RawLumiOcc2Th2Plus(occ):
	return [RawLumiOcc2Th2BXPlus(occ,bx) for bx in range(3564)]

def RawLumiOcc2Th2BXPlusErr(occ,bx):

	hlxs = [6,7,8,9,10,11,18,19,20,21,22,23,30,31,32,33,34,35]

	active = 0
	below = 0
	for hlx in hlxs:

		below += occ[hlx].data[bx + 3*4096]
		below += occ[hlx].data[bx + 4*4096]
		active += occ[hlx].data[bx + 3*4096]
		active += occ[hlx].data[bx + 4*4096]
		active += occ[hlx].data[bx + 5*4096]

	import math
	try:
		return math.log(1/float(below) - 1/float(active))
	except ZeroDivisionError:
		return 0

def RawLumiOcc2Th2PlusErr(occ):
	return [RawLumiOcc2Th2BXPlusErr(occ,bx) for bx in range(3564)]

# - side
def RawLumiOcc2Th2BXMinus(occ,bx):

	hlxs = [0,1,2,3,4,5,12,13,14,15,16,17,24,25,26,27,28,29] 

	active = 0
	below = 0
	for hlx in hlxs:

		below += occ[hlx].data[bx + 3*4096]
		below += occ[hlx].data[bx + 4*4096]
		active += occ[hlx].data[bx + 3*4096]
		active += occ[hlx].data[bx + 4*4096]
		active += occ[hlx].data[bx + 5*4096]

	import math
	try: 
		return -math.log(below/float(active))
	except ZeroDivisionError:
		return 0

def RawLumiOcc2Th2Minus(occ):
	return [RawLumiOcc2Th2BXPlus(occ,bx) for bx in range(3564)]

def RawLumiOcc2Th2BXMinusErr(occ,bx):

	hlxs = [0,1,2,3,4,5,12,13,14,15,16,17,24,25,26,27,28,29] 

	active = 0
	below = 0
	for hlx in hlxs:

		below += occ[hlx].data[bx + 3*4096]
		below += occ[hlx].data[bx + 4*4096]
		active += occ[hlx].data[bx + 3*4096]
		active += occ[hlx].data[bx + 4*4096]
		active += occ[hlx].data[bx + 5*4096]

	import math
	try: 
		return math.sqrt(1/float(below) - 1/float(active))
	except ZeroDivisionError:
		return 0

def RawLumiOcc2Th2MinusErr(occ):
	return [RawLumiOcc2Th2BXPlusErr(occ,bx) for bx in range(3564)]

### ET Lumi

def RawLumiETBX(et,bx):

	etsum = 0

	for hlx in range(36):
		etsum += et[hlx].data[bx]

	return etsum/float(36*8*262144)

def RawLumiET(et):
	return [RawLumiETBX(et,bx) for bx in range(3564)]

def RawLumiETBXHLX(et,bx,hlx):
	return et[hlx].data[bx]/float(8*262144)

def RawLumiETHLX(et,hlx):
	return [RawLumiETBXHLX(et,bx,hlx) for bx in range(3564)]

# + side
def RawLumiETBXPlus(et,bx):

	hlxs = [6,7,8,9,10,11,18,19,20,21,22,23,30,31,32,33,34,35]
	etsum = 0

	for hlx in hlxs:
		etsum += et[hlx].data[bx]

	return etsum/float(18*8*262144)

def RawLumiETPlus(et):
	return [RawLumiETBXPlus(et,bx) for bx in range(3564)]

# - side
def RawLumiETBXMinus(et,bx):

	hlxs = [0,1,2,3,4,5,12,13,14,15,16,17,24,25,26,27,28,29]
	etsum = 0

	for hlx in hlxs:
		etsum += et[hlx].data[bx]

	return etsum/float(18*8*262144)

def RawLumiETMinus(et):
	return [RawLumiETBXMinus(et,bx) for bx in range(3564)]

