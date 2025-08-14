#
#	Multiuser Chat Server. (6-apr-96) by Irmen de Jong.
#		Extended by Frank Niessink
#
#	Implements a mud-like "chat" server to which multiple users can connect,
#	by simple telnet. Some simple commands are supported.
#	Feelings are supported, but not fully. No adverbs yet.
#
#	Usage: python chatserv.py [hostname] [portnumber]
#

import select
import string
import sys
import time
import socket

WelcomeMsg = "\r\nTELNET CHAT SERVICE...\r\n"

class connection:
	def __init__(self,sock,address,name=None):
		self.sock=sock
		self.myname=name
		self.address=address

	def __del__(self):
		self.sock.close()
		self.sock=None

	def setname(self,name):
		self.myname=name

	def name(self):
		return self.myname

	def send(self,msg):
		self.sock.send(msg)
	def senderr(self,errmsg):
		self.sock.send(mkerr(errmsg))
	def sendnote(self,msg):
		self.sock.send(mknote(msg))


# The following list contain all open connections.
connections=[]

#Feeling types:
SIMP=0; DEFA=1; PERS=2; QUAD=3; PREV=4; PHYS=5; DEUX=6; SHRT=7

#feelings.. taken from LPC mud soul. Not yet fully supported.
feelings = {
"hmm":		[0,0," hmm$ \nHOW \nAT"," at"],
"burp":		[1,["rudely"],""," at"],
"wink":		[1,["suggestively"],""," at"],
"smile":	[1,["happily"],""," at"],
"point":	[1,0,""," at"],
"grin":		[1,["evilly"],""," at"],
"laugh":	[1,0,""," at"],
"nod":		[1,["solemnly"],""," at"],
"wave":		[1,["happily"],""," at"],
"cackle":	[1,["with glee"],""," at"],
"chuckle":	[1,["politely"],""," at"],
"bow":		[1,0,""," to"],
"glare":	[1,["stonily"],""," at"],
"giggle":	[1,["merrily"],""," at"],
"groan":	[1,0,""," at"],
"grunt":	[1,0,""," at"],
"growl":	[1,0,""," at"],
"snarl":	[1,0,""," at"],
"recoil":	[1,["with fear"],""," from"],
"moan":		[1,0,""," at"],
"howl":		[1,["in pain"],""," at"],
"puke":		[1,0,""," on"],
"sneeze":	[1,["loudly"],""," at"],
"spit":		[1,0,""," on"],
"stare":	[1,0,""," at"],
"whistle":	[1,["appreciatively"],""," at"],
"applaud":	[1,0,""," at"],
"agree":	[1,0,""," with"],
"disagree":	[1,0,""," with"],
"fart":		[1,0,""," at"],
"dance":	[1,0,""," with"],
"purr":		[1,0,""," at"],
"listen":	[1,0,""," to"],
"apologize":	[1,0,""," to"],
"complain":	[1,0,""," about"],
"beg":		[2,0," beg$ \nHOW"," beg$ \nWHO for mercy \nHOW"],
"shake":	[2,0," shake$ \nYOUR head \nHOW"," shake$ hands with \nWHO \nHOW"],
"grimace":	[0,0," \nHOW make$ an awful face \nAT"," at"],
"stomp":	[2,0," stomp$ \nYOUR foot \nHOW"," stomp$ on \nPOSS foot \nHOW"],
"snigger":	[1,["jeeringly"],""," at"],
"watch":	[3,["carefully"]," watch the surroundings \nHOW", " watches the surroundings \nHOW"," watch \nWHO \nHOW"," watches \nWHO \nHOW",],
"scratch":	[3,["on the head"]," scratch \nMYself \nHOW"," scratches \nMYself \nHOW"," scratch \nWHO \nHOW"," scratches \nWHO \nHOW",],
"tap":		[2,["impatiently",0,"on the shoulder"]," tap$ \nYOUR foot \nHOW"," tap$ \nWHO \nWHERE"],
"curse":	[2,0," curse$ \nWHAT \nHOW"," curse$ \nWHO \nHOW"],
"sing":		[0,0," \nHOW sing$ \nWHAT \nAT"," to"],
"mumble":	[0,0," mumble$ \nWHAT \nHOW \nAT"," to"],
"scream":	[0,["loudly"]," scream$ \nWHAT \nAT \nHOW"," at"],
"yell":		[0,["in a high pitched voice"]," yell$ \nHOW \nWHAT \nAT"," at"],
"hide":		[0,0," hide$ \nHOW behind \nWHO"],
"pounce":	[5,["playfully"],""],
"kneel":	[0,0," \nHOW fall$ on \nYOUR knees \nAT"," in front of"],
"roll":		[0,["to the ceiling"]," roll$ \nYOUR eyes \nHOW"],
"boggle":	[0,0," boggle$ \nHOW at the concept"],
"cheer":	[7,["enthusiastically"],""],
"wiggle":	[0,0," wiggle$ \nYOUR bottom \nHOW"],
"flip":		[0,0," flip$ \nHOW head over heels"],
"cry":		[6,0," cry \nHOW"," cries \nHOW"],
"sob":		[7,0,""],
"sweat":	[7,0,""],
"gurgle":	[7,0,""],
"grumble":	[7,0,""],
"die":		[6,0," fall \nHOW down and play dead"," falls \nHOW to the ground, dead"],
"stumble":	[7,0,""],
"bounce":	[7,0,""],
"yawn":		[7,0,""],
"sulk":		[7,["in the corner"],""],
"strut":	[7,["proudly"],""],
"sniff":	[7,0,""],
"snore":	[7,0,""],
"snicker":	[7,0,""],
"smirk":	[7,0,""],
"jump":		[0,["in aggravation"] ," jump$ up and down \nHOW"],
"fume":		[7,0 ,""],
"faint":	[7,0,""],
"shrug":	[7,0,""],
"pout":		[7,0,""],
"hiccup":	[7,0,""],
"frown":	[7,0,""],
"gasp":		[7,["in astonishment"],""],
"think":	[7,["carefully"],""],
"ponder":	[7,["over some problem"],""],
"clap":		[7,0,""],
"sigh":		[7,0,""],
"cough":	[7,["noisily"],""],
"shiver":	[7,["from the cold"],""],
"blush":	[6,0," blush \nHOW"," blushes \nHOW"]
}

# exit : graceful exit
def exit(code):
	global connections
	del connections
	raise SystemExit,code

# stripcrlf: strip CR/LF or LF from string
def stripcrlf(str):
	if len(str)>2:
		if (str[-2:]=='\r\n') or (str[-2:]=='\n\r'): return str[:-2]
	if len(str)>1:
		if str[-1]=='\n': return str[:-1]
	return str

# broadcast: send a msg to all connections, except self.
def broadcast(self_idx,msg):
	i=0
	for c in connections:
		if i!=self_idx: c.sock.send(msg)
		i=i+1

# shutdown: close connection & send msg to quitter & others
def shutdown(i,msg,brmsg):
	if brmsg: broadcast(i,brmsg)
	connections[i].send(msg)
	log(i,"QUIT")
	del connections[i]

# log: log message from connection #i (to screen)
def log(i,msg):
	if connections[i].name():
		print msg+'\t'+connections[i].name()+'\t'+`connections[i].address`,time.ctime(time.time())
	else:
		print msg+"\t<???>\t"+`connections[i].address`,time.ctime(time.time())

# mkerr: convert string into error string
# mknote: convert string into note string
def mkerr(msg):
	return "** ERROR: "+msg+" **\r\n"
def mknote(msg):
	return "** NOTE: "+msg+" **\r\n"

# do_feeling: execute feeling 'data'. i=index of active socket.
def do_feeling(i,data):
	line=string.split(data)
	feeling=string.lower(line[0])[1:]
	if len(line)>1:
		args=line[1:]
	else:
		args=None

	if feeling in feelings.keys():
		adv=""
		if feelings[feeling][1]:
			adv=' '+feelings[feeling][1][0]
		connections[i].send("You "+feeling+adv+".\r\n")
		if feeling[-2:]=='ch' or feeling[-1]=='s' or feeling[-2:]=='sh':
			feeling=feeling+'e'
		if feeling[-1]=='y':
			feeling=feeling[:-1]+"ie"
		broadcast(i,connections[i].name()+' '+feeling+'s'+adv+".\r\n")
	else:
		connections[i].senderr("Unknown feeling")


# do_chatter: broadcast normal chatter
def do_chatter(i, data):
	broadcast(i,connections[i].name()+" says: `"+data+"'\r\n")

# do_to: send chatter to only one person
def do_to(i, args):
	if args: 
		destname=args[0]
	else:
		connections[i].senderr("No destination provided")
		return
	data=string.join(args[1:])
	if destname in map(lambda x:x.name(),connections):
		dest=map(lambda x:x.name(),connections).index(destname)
		connections[dest].send(connections[i].name()+" says to you: `"+data+"'\r\n")
	else:
		connections[i].senderr("Unknown name")

# do_help: send a help message
def do_help(i, args):
	if args and args[0]=="feelings":
		connections[i].send("** FEELINGS **\r\n"
		"Currently, the following feeling commands have been defined:\r\n")
		s=""
		fls=feelings.keys()
		fls.sort()
		for f in fls:
			if (len(s)+len(f))>=76:
				connections[i].send(s+"\r\n")
				s=""
			s=s+f+' '
		connections[i].send(s+"\n")
	else:
		connections[i].send("** CHAT SERVICE ONLINE HELP **\r\n"
		"All normal text entered is sent to all other people, as if you 'said' it.\r\n"
		"Lines starting with ' are a special feelings line. See @help feelings.\r\n"
		"Lines starting with @ are regarded as a special command line:\r\n"
		"@help topic - help on topic\r\n"
		"@to foo bar - say 'bar' to 'foo' only\r\n"
		"@who        - who's there\r\n"
		"@name foo   - set your name to 'foo'\r\n"
		"@quit       - exit chat service\r\n"
		"@eject who passwd - eject someone (operator cmd)\r\n"
		"@shutdown passwd) - shutdown server (operator cmd)\r\n")

# do_who: show who are there
def do_who(i):
	str=""
	for c in connections:
		str=str+"  "+c.name()+"\r\n"
	connections[i].send(str)
	

# do_name: change name
def do_name(i,args):
	if not args:
		connections[i].senderr("No name")
	elif len(args[0]) < 2:
		connections[i].senderr("Name too short")
	elif args[0] in map(lambda x:x.name(),connections):
		connections[i].senderr("Name already in use")
	else:
		broadcast(-1,mknote(connections[i].name()+" now known as: "+args[0]))
		connections[i].setname(args[0])

# do_eject: eject someone, need password for this
def do_eject(i, args):
	if not password:
		connections[i].senderr("Sysop commands not enabled")
	elif len(args) != 2:
		connections[i].senderr("Usage: @eject name password")
	elif not (args[0] in map(lambda x:x.name(), connections)):
		connections[i].senderr("Unknown name")
	elif args[1] != password:
		connections[i].senderr("Wrong password")
	else:
		dest=map(lambda x:x.name(),connections).index(args[0])
		shutdown(dest,"You were ejected by "+connections[i].name()+".\n",mknote(connections[dest].name()+" WAS EJECTED BY "+connections[i].name()))
	
# do_shutdown: shut down the server (needs passwd)
def do_shutdown(i, args):
	if not password:
		connections[i].senderr("Sysop commands not enabled")
	elif len(args) != 1:
		connections[i].senderr("Usage: @shutdown password")
	elif args[0] != password:
		connections[i].senderr("Wrong password")
	else:
		broadcast(-1,mknote("Server shutdown by "+connections[i].name()))
		log(i,"SHUTDOWN");
		exit(0)

# do_command: execute command 'data'. i=index of active socket.
def do_command(i,data):
	line=string.split(data)
	cmd=string.lower(line[0])[1:]
	args=line[1:]

	if cmd=='name':
		do_name(i,args)
	elif cmd=="who":
		do_who(i)
	elif cmd=="quit":
		shutdown(i,"Quitting chat service.\n",mknote(connections[i].name()+" LEFT"))
	elif cmd=="to":
		do_to(i, args)
	elif cmd=="help":
		do_help(i, args)
	elif cmd=="eject":
		do_eject(i, args)
	elif cmd=="shutdown":
		do_shutdown(i, args)
	else:
		connections[i].senderr("Bad CMD, try @help")


def serverloop(host,port):
	global feelings

	print "\nStarting telnet chat service on",host,port,"..."

	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	s.bind(host, port)
	s.listen(1)

	while 1:
		socklist=map(lambda x:x.sock, connections)
		ins,outs,exs = select.select(socklist+[s],[],[],5)

		for sock in ins:
			if sock==s:
				sock,addr=s.accept()
				connections.append(connection(sock,addr))
				sock.send(WelcomeMsg+"Enter your name: ")

			else:
				i = socklist.index(sock)
				data=sock.recv(512)
				if data:
					data=string.strip(stripcrlf(data))

					# If we don't have a name yet, read the name.
					if not connections[i].name():
						if len(data)<2:
							shutdown(i,mkerr("Name too short, try again"),None)
						elif data in map(lambda x:x.name(),connections):
							shutdown(i,mkerr("Name already in use, try again"),None)
						else:
							connections[i].setname(data)
							broadcast(i,mknote(connections[i].name()+" ARRIVES"))
							connections[i].sendnote("Ok, enter chatter or @help for help")
							log(i,"CONNECT")
							
					elif data:
						if data[0]=='\'':
							do_feeling(i,data)
						elif data[0]=='@':
							do_command(i,data)
						else:
							do_chatter(i,data)

					if sock.fileno()>0: sock.send("> ")

				else:
					# EOF read or closed connection.
					shutdown(i,"Quitting chat service.\r\n",mknote(connections[i].name()+" LEAVES"))


def main(host,port):
	global connections, password 

	password=raw_input("Enter password (or just enter for no passwd): ");

	serverloop(host,port)
	print "Server shut down."
	del connections			# close all connections


if __name__=='__main__':

	argc=len(sys.argv)
	if argc==1:
		main(socket.gethostname(),9999)
	elif argc==2:
		main(socket.gethostname(),eval(sys.argv[1]))
	elif argc==3:
		main(sys.argv[1],eval(sys.argv[2]))
	else:
		print "Usage: python",sys.argv[0],"[hostname] [portnumber]"
