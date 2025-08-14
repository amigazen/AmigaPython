import ARexx
import socket

# dictionary of used sockets, filedes->socket
sockets={}

# One function to handle the socket commands,
# this is convenient because one 'try' block covers all.
def cmd_dispatcher(host,msg,cmd,args):
	try:
		if cmd=='GETHOSTBYNAME':
			msg.result=socket.gethostbyname(args['HOST'])
		elif cmd=='GETHOSTNAME':
			msg.result=socket.gethostname()
		elif cmd=='GETHOSTBYADDR':
			(msg.result,aliaslist,ipaddrlist) = socket.gethostbyaddr(args['IPADDR'])
		elif cmd=='GETSERVBYNAME':
			if args['TCP']: prot='tcp'
			if args['UDP']: prot='udp'
			msg.result=`socket.getservbyname(args['SERVICE'],prot)`
		elif cmd=='CONNECTSOCKET':
			sock=socket.socket(socket.AF_INET,socket.SOCK_STREAM)
			sock.connect(args['HOST'],args['PORT'])
			sockets[sock.fileno()]=sock
			msg.result=`sock.fileno()`
		elif cmd=='CLOSESOCKET':
			fd=args['SOCKET']
			sockets[fd].close()
			del sockets[fd]
		elif cmd=='READSOCKET':
			msg.result=sockets[args['SOCKET']].recv(args['LEN'])
			if not msg.result: msg.rc=ARexx.RC_WARN
		elif cmd=='WRITESOCKET':
			msg.result=`sockets[args['SOCKET']].send(args['DATA'])`
		elif cmd=='WRITESOCKETCR':
			msg.result=`sockets[args['SOCKET']].send(args['DATA']+'\n')`

	except socket.error,string:
		msg.rc=ARexx.RC_ERROR
		if type(string)==type(''): msg.rc2=string
		else: msg.rc2=string[1]
	return 1

cmds = [
('GETHOSTBYNAME','HOST/A',None,cmd_dispatcher),
('GETHOSTNAME','',None,cmd_dispatcher),
('GETHOSTBYADDR','IPADDR/A',None,cmd_dispatcher),
('GETSERVBYNAME','SERVICE/A,TCP/S,UDP/S',{'TCP':-1,'UDP':0},cmd_dispatcher),
('CONNECTSOCKET','HOST/A,PORT/N/A',None,cmd_dispatcher),
('CLOSESOCKET','SOCKET/A/N',None,cmd_dispatcher),
('READSOCKET','SOCKET/A/N,LEN/A/N',None,cmd_dispatcher),
('WRITESOCKET','SOCKET/A/N,DATA/A/F',None,cmd_dispatcher),
('WRITESOCKETCR','SOCKET/A/N,DATA/A/F',None,cmd_dispatcher)
]

h=ARexx.host('PY_SOCK',cmds)

print 'Python ARexx host. Host is',h.name

h.run()
