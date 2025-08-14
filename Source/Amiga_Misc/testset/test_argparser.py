import Dos

TestError = 'FAILED --- ArgParser'

def test(a,b):
        if a!=b: raise TestError


print 'TESTING ARGPARSER SETUP...'

ap=Dos.ArgParser('ONE')
test(ap.defaults,{'ONE': None})
test(ap.types,(('ONE', 'X'),))
ap.new('ONE,TWO')
test(ap.defaults,{'TWO': None, 'ONE': None})
test(ap.types,(('ONE', 'X'), ('TWO', 'X')))
ap.new('STR,INT/N')
test(ap.defaults,{'STR': None, 'INT': None})
test(ap.types,(('STR', 'X'), ('INT', 'N')))
ap.new('STR/K,INT/N/K')
test(ap.defaults,{'STR': None, 'INT': None})
test(ap.types,(('STR', 'X'), ('INT', 'N')))
ap.new('REQ/A,INTLIST/M/N')
test(ap.defaults,{'INTLIST': []})
test(ap.types,(('REQ', 'X'), ('INTLIST', 'I')))
ap.new('REQ/A/K,STRLIST/M')
test(ap.defaults,{'STRLIST': []})
test(ap.types,(('REQ', 'X'), ('STRLIST', 'A')))
ap.new('REQ/A/K,REST/F,FLAG/S')
test(ap.defaults,{'FLAG': 0, 'REST': None})
test(ap.types,(('REQ', 'X'), ('REST', 'X'), ('FLAG', 'S')))
ap.new('BLA/M,REST/F,FLAG/S/A')
test(ap.defaults,{'BLA':[], 'REST': None})
test(ap.types,(('BLA', 'A'), ('REST', 'X'), ('FLAG', 'S')))
#ap.new('TOGGLE/T')
#test(ap.defaults,{'TOGGLE': 0})
#test(ap.types,(('TOGGLE', 'T'),))

print 'TESTING ARGPARSER PARSE...'
print '1) STRING'
ap.new('STR')
test(ap.parse('string'),{'STR':'string'})
test(ap.parse('STR string'),{'STR':'string'})
test(ap.parse(''),{'STR':None})
print '2) NUMBER'
ap.new('NUM/N')
test(ap.parse('1234'),{'NUM':1234})
test(ap.parse('NUM=1234'),{'NUM':1234})
test(ap.parse(''),{'NUM':None})
print '3) TOGGLE'
print '(not yet supported)'
#ap.new('TOG/T')
#test(ap.parse('TOG'),{'TOG':-1})
#ap.defaults['TOG']=999
#test(ap.parse('TOG'),{'TOG':0})
#test(ap.parse(''),{'TOG':999})
print '4) SWITCH'
ap.new('SW/S')
test(ap.parse('SW'),{'SW':-1})
ap.defaults['SW']=999
test(ap.parse('SW'),{'SW':-1})
test(ap.parse(''),{'SW':999})
print '5) STRING LIST'
ap.new('STRL/M')
test(ap.parse('foo bar foobar'),{'STRL':['foo','bar','foobar']})
test(ap.parse('foo'),{'STRL':['foo']})
test(ap.parse('STRL foo'),{'STRL':['foo']})
ap.defaults['STRL']=['dflt']
test(ap.parse('blabla'),{'STRL':['blabla']})
test(ap.parse(''),{'STRL':['dflt']})
print '6) NUMBER LIST'
ap.new('NL/M/N')
test(ap.parse('1 2 3'),{'NL':[1,2,3]})
test(ap.parse('123'),{'NL':[123]})
test(ap.parse('NL 123'),{'NL':[123]})
ap.defaults['NL']=[999]
test(ap.parse('42 42 42'),{'NL':[42,42,42]})
test(ap.parse(''),{'NL':[999]})
print '7) /A'
ap.new('STR/A')
test(ap.defaults,{})
test(ap.parse('string'),{'STR':'string'})
test(ap.parse('STR string'),{'STR':'string'})
print '8) /F'
ap.new('STR/F,SW/S')
test(ap.parse('foo bar   foobar 42'),{'STR':'foo bar   foobar 42', 'SW':0})
test(ap.parse('foo bar SW  foobar 42'),{'STR':'foo bar SW  foobar 42', 'SW':0})
test(ap.parse('SW foo bar SW  foobar 42'),{'STR':'foo bar SW  foobar 42', 'SW':-1})
print '9) /K'
ap.new('STR/K')
test(ap.parse('STR string'),{'STR':'string'})
ap.new('STR/K/F')
test(ap.parse('STR string foo bar'),{'STR':'string foo bar'})

print '10) COMBINED'
ap.new('FROM/A/M,TO/A,ALL/S,QUIET/S,BUF=BUFFER/K/N')
test(ap.defaults,{'BUF=BUFFER': None, 'ALL': 0, 'QUIET': 0})
test(ap.types,(('FROM', 'A'), ('TO', 'X'), ('ALL', 'S'), ('QUIET', 'S'), ('BUF=BUFFER', 'N')))

# Set our custom defaults:
ap.defaults['BUF=BUFFER']=999
ap.defaults['QUIET']=888
test(ap.parse('f1 f2 f3 dest'),{'BUF=BUFFER': 999, 'TO': 'dest', 'ALL': 0, 'QUIET': 888, 'FROM': ['f1', 'f2', 'f3']})
test(ap.parse('1 2 3 dest'),{'BUF=BUFFER': 999, 'TO': 'dest', 'ALL': 0, 'QUIET': 888, 'FROM': ['1', '2', '3']})
test(ap.parse('1 2 3 4'),{'BUF=BUFFER': 999, 'TO': '4', 'ALL': 0, 'QUIET': 888, 'FROM': ['1', '2', '3']})
test(ap.parse('src dest ALL'),{'BUF=BUFFER': 999, 'TO': 'dest', 'ALL': -1, 'QUIET': 888, 'FROM': ['src']})
test(ap.parse('src dest QUIET'),{'BUF=BUFFER': 999, 'TO': 'dest', 'ALL': 0, 'QUIET': -1, 'FROM': ['src']})
test(ap.parse('src dest QUIET ALL'),{'BUF=BUFFER': 999, 'TO': 'dest', 'ALL': -1, 'QUIET': -1, 'FROM': ['src']})
test(ap.parse('src dest BUF=10'),{'BUF=BUFFER': 10, 'TO': 'dest', 'ALL': 0, 'QUIET': 888, 'FROM': ['src']})

def terr(f,a):
 fault=0
 try:
	try:
		f(a)
	except (Dos.error,ValueError,SystemError,TypeError):
		fault=1
 finally:
	if not fault:
		raise TestError,'should have given an error'


print 'TESTING ARGPARSER ERRORS...'

terr(ap.new,'/')
terr(ap.new,'A/')
terr(ap.new,'A/A/A')
terr(ap.new,'A/M/F')
terr(ap.new,'A/F/M')
terr(ap.new,'A/M,B/M/N')
terr(ap.new,'A/M,B³³/±')
terr(ap.new,'A/M,B/Z')
terr(ap.new,'A,A')
terr(ap.new,'A,B,A')


ap.new('FROM/A/M,TO/A,ALL/S,QUIET/S,BUF=BUFFER/K/N')
ap.template='KAPUT'
terr(ap.parse,'foo')

ap.new('A,B')
ap.types=(1,2)
terr(ap.parse,'A B')
ap.reset()
test(ap.types,(('A', 'X'), ('B', 'X')))

ap.new('A,B')
ap.types=(('A',42),('B',42))
terr(ap.parse,'A B')

ap.new('A,B')
ap.types=(('A','Z'),('B','Z'))
terr(ap.parse,'A B')

ap.new('')
terr(ap.parse,'foo bar')
terr(ap.parse,'foo')
test(ap.parse(''),{})

del ap
print 'ARGPARSER OK!'
