# Testing md5 module

import string
from md5 import md5

def hexstr(s):
	h = string.hexdigits
	r = ''
	for c in s:
		i = ord(c)
		r = r + h[(i >> 4) & 0xF] + h[i & 0xF]
	return r

def md5test(s):
	return 'MD5 ("' + s + '") = ' + hexstr(md5(s).digest())

testset=['',
'a',
'abc',
'message digest',
'abcdefghijklmnopqrstuvwxyz',
'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789',
'12345678901234567890123456789012345678901234567890123456789012345678901234567890']

good=['d41d8cd98f00b204e9800998ecf8427e',
'0cc175b9c0f1b6a831c399e269772661',
'900150983cd24fb0d6963f7d28e17f72',
'f96b697d7cb7938d525a2f31aaf161d0',
'c3fcd3d76192e4007dfb496cca67e13b',
'd174ab98d277d9f5a5611c2c9f419d9f',
'57edf4a22be3c955ac49da2e2107b67a']

print 'MD5 test suite:'
for i in range(len(testset)):
#	print md5test(testset[i]),
	if hexstr(md5(testset[i]).digest())==good[i]:
#		print "OK"
		pass
	else:
		raise ValueError, "MD5 DIGEST WRONG"

print "OK"
