#
#	Test miscellaneous Amiga specific stuff...
#

import os
import amiga
import time
import amiga

TestError = 'FAILED --- Amiga misc '

def testeq(res,check,info):
	if res!=check:
		raise TestError+info

def testcrc32(str,check):
	testeq(amiga.crc32(str),check,'amiga.crc32')


########################################## amiga.crc32

print 'TEST amiga.crc32'
testcrc32('',0xFFFFFFFF)
testcrc32('a',0x174841BC)
testcrc32('abc',0xCADBBE3D)
testcrc32('abcdefghijklmnopqrstuvwxyz',0xB3D8AF42)
testcrc32('ABCDEFGHIJKLMNOPQRSTUVWXYZ',0x540887DD)
testcrc32('12345678901234567890123456789012345678901234567890123456789012345678901234567890  The quick brown Fox jumps over the lazy Dog',0x0383D5D6)
print 'amiga.crc32 TEST OK'

print 'TEST amiga.touch, filedates, Amiga dates/times'
try:
	os.remove('t:touchtest')
except os.error:
	pass

try:
	amiga.touch('t:touchtest')
except:
	raise TestError+'amiga.touch'

try:
	# use amiga.touch to set the filedate to a certain date
	datestr = ('18-Jul-98','13:33:33')
	datestamp = amiga.StrToDate(datestr[0],datestr[1])
	unixtime = amiga.DS2time(datestamp)
	unixtimestr = time.ctime(unixtime)
	testeq(unixtime,900765213.0,'amiga.DS2time')
	testeq(unixtimestr,'Sat Jul 18 13:33:33 1998','amiga.DS2time')
	amiga.touch('t:touchtest',unixtime)
	ds=amiga.Examine('t:touchtest')[amiga.fib_Date]
	testeq(ds, datestamp, 'amiga.touch')
	
	# use amiga.SetFileDate to set the filedate to the current date/time
	datestamp=amiga.DateStamp()
	datestr=amiga.DateToStr(datestamp)
	unixtime = amiga.DS2time(datestamp)
	unixtimestr = time.ctime(unixtime)
	amiga.SetFileDate('t:touchtest',datestamp)
	ds=amiga.Examine('t:touchtest')[amiga.fib_Date]
	testeq(ds, datestamp, 'amiga.SetFileDate')

	print 'amiga.touch/Amigadates TEST OK'

except:
	raise TestError+'Dos filedates/Amiga dates'

