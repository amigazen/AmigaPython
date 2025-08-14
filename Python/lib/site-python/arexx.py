"""
High level ARexx interface.
©Irmen de Jong

$VER: ARexx.py 1.5 (3.1.99)
"""

import ARexxll
import string
import Dos
import sys

error = ARexxll.error

errorstring = ARexxll.errorstring

# ARexx result codes:
RC_OK    =  0  # success
RC_WARN  =  5  # warning only
RC_ERROR = 10  # something's wrong
RC_FATAL = 20  # complete or severe failure


#### PORT #################################

class port:
	"""
	ARexx port base class.
	IMPORTANT: Don't create objects of this class directly, use one of
	the derived classes below (publicport or privateport)!!!
	"""
	def __init__(self,name):
		self.port = ARexxll.port(name)
		self.signal = self.port.signal
		if name:
			self.name = self.port.name
	def close(self):
		self.port.close()
	def wait(self):
		self.port.wait()
	def getmsg(self):
		return self.port.getmsg()
	def send(self,to,cmd,async=0):
		self.flush()                        # XXX necessary?
		if async:
			self.port.send(to,cmd,1)        # no return value
		else:
			rc,rc2,result = self.port.send(to,cmd,0)
			if rc:
				raise error, (rc,rc2)
			else:
				return result
	def flush(self):
		while self.port.getmsg(): pass
	def setstringmsgs(self,flag):
		self.port.setstringmsgs(flag)
	def settokenizeline(self,flag):
		self.port.settokenizeline(flag)
	


class privateport(port):
	"""
	Private ARexx port.
	Meant for sending messages to other ports only.
	"""
	def __init__(self):
		port.__init__(self,None)

	# currently, inherits all methods of port superclass unchanged


class publicport(port):
	"""
	Public ARexx port.
	For setting up your own host.
	"""
	def __init__(self,name='PYTHON'):
		name=string.upper(name)
		for c in name:
			if not c in '0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_.':
				raise ValueError,'invalid port name'
		port.__init__(self,name)

	# currently, inherits all methods of port superclass unchanged


#### HOST ##################################

class host(publicport):
	"""
	ARexx host class.
	For setting up your complete ARexx host system.
	Automatic parsing & dispatching of command lines.
	"""
	def __init__(self,name='PYTHON',cmds=None):
		publicport.__init__(self,name)
		self.commands = {}
		self.setcommand('HELP','COMMAND,STEM/K,VAR/K',None,std_help_func)
		self.cmderror='Unknown command'
		self.catch = 0
		if cmds: self.setcommands(cmds)
	def __repr__(self):
		if hasattr(self,'name'):
			return '<host instance, port '+self.name+'>'
		else:
			return '<host instance, defunct>'
	def close(self):
		if hasattr(self,'port'):
			self.port.close()
			del self.name
			del self.port
			del self.commands
	def setcommands(self,cmds):
		for (c,t,d,f) in cmds:
			self.setcommand(c,t,d,f)
	def setcommand(self,cmd,template,defaults,func):
		if template==None: parser=None
		else: parser=Dos.ArgParser(template)
		self.commands[string.upper(cmd)]=(parser,func)
		if defaults: self.setdefaults(cmd,defaults)
	def setdefaults(self,cmd,defaults):
		parser=self.commands[cmd][0]
		if parser.defaults.keys()==defaults.keys():
			parser.defaults=defaults
		else:
			raise ValueError,'incompatible defaults'
	def defaults(self,cmd):
		return self.commands[cmd][0].defaults
	def catchExceptions(self,yes):
		self.catch = yes
	def dispatch(self):
		m=self.port.getmsg()
		res=-1
		if m and m.msg:
			i=string.index(m.msg+' ',' ')
			cmd=string.upper(m.msg[:i])
			args=m.msg[i+1:]
			try:
				try:
					(parser,func) = self.commands[cmd]
				except KeyError:
					m.rc=RC_FATAL; m.rc2=self.cmderror
				else:
					if parser:
						res=func(self,m,cmd,parser.parse(args))
					else:
						res=func(self,m,cmd,args)
			except Dos.error,str:
				m.rc=RC_ERROR; m.rc2=str[0]	# ReadArgs() probably failed
			except:
				if self.catch:
					m.rc=RC_ERROR; m.rc2='Unhandled exception '+sys.exc_type+' : '+sys.exc_value
				else:
					print '*** Unhandled exception during ARexx cmd dispatch ***'
					raise sys.exc_type,sys.exc_value,sys.exc_traceback # re-raise exception
			m.reply()
		return res
	def run(self):
		self.flush()
		while 1:
			self.wait()
			if not self.dispatch(): break
	def flush(self):
		while self.dispatch()!=-1: pass       # overrides publicport.flush



def std_help_func(host,msg,cmd,args):
	var = args['VAR']
	stem = args['STEM']
	if args['COMMAND']:
		try:
			# return template for command
			helpc=string.upper(args['COMMAND'])
			parser=host.commands[helpc][0]
			if parser:
				result=[parser.template]
			else:
				result=['ARGS/F']
		except KeyError:
			msg.rc=RC_ERROR; msg.rc2='No help on command (unknown)'
			return 1
	else:
		# return no. of commands followed by the commands.
		cmds = host.commands.keys()
		cmds.sort()
		result = [`len(host.commands)`]+cmds
	if var:
		msg.setvar(var,string.join(result))
	elif stem:
		if len(result)==1:
			msg.setvar(stem+'COUNT','1')
			msg.setvar(stem+'1',result[0])
		else:
			msg.setvar(stem+'COUNT',result[0])
			del result[0]
			i=1
			for c in result:
				msg.setvar(stem+`i`,c); i=i+1
	else:
		msg.result=string.join(result)
	return 1

def std_debug_func(host,msg,cmd,args):
	print 'DEBUG FUNCTION'
	print 'HOST =',host
	print 'MSG  =',msg
	print 'CMD  =',cmd
	print 'ARGS =',args
	print 'RETURNING \'the result\''
	msg.result='the result'
	return 1


##### MISC UTILITY FUNCTIONS ###########################33

def SendARexxMsg(Port, Message):
 return privateport().send(Port, Message)

def CallARexxFunc(Func, *Args):
 return SendARexxMsg('REXX',
	'"Return '+Func+'('+reduce(lambda x,y: x+','+`y`,Args,'')[1:]+')')


