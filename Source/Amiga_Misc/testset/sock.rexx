/*****************************************************************************

	ARexx script showing how to use Python's socket host.

******************************************************************************/

options results

address PY_SOCK

GETHOSTNAME
hostname=RESULT
say "Running on host" hostname

say "Getting daytime: "
GETSERVBYNAME 'daytime'
CONNECTSOCKET hostname RESULT
sock=RESULT
READSOCKET sock 100
say RESULT
CLOSESOCKET sock

say "Echo service:"

GETSERVBYNAME 'echo'
CONNECTSOCKET hostname RESULT
sock=RESULT
WRITESOCKETCR sock 'foobar 1'
READSOCKET sock 100
say 'got back:' RESULT
WRITESOCKETCR sock 'foobar 2'
READSOCKET sock 100
say 'got back:' RESULT
CLOSESOCKET sock

say "Finger root@localhost:"
GETSERVBYNAME 'finger'
CONNECTSOCKET hostname RESULT
sock=RESULT
WRITESOCKETCR sock 'root'
cont=0
do while cont=0
	READSOCKET sock 1000
	cont=rc
	if cont=0 then do
		address; WRITECH(STDOUT,RESULT); address
	end
end

CLOSESOCKET sock

say "Complete."

