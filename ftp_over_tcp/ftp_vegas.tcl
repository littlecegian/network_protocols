#Create a simulator object
set ns [new Simulator]

#Define different colors for data flows (for NAM)
$ns color 1 Green
$ns color 2 Yellow

set f2 [open tracefile_vegas_12.5.tr w]
set nf [open out.nam w]
$ns namtrace-all $nf
$ns trace-all $f2

#Define a 'finish' procedure
proc finish {} {
 	global ns f2 nf
 	$ns flush-trace
	
	# close $nf
	close $f2

	#Execute NAM on the trace file
 	exec nam out.nam &

 	close $nf
 	exit 0
}

#Create five nodes
set r1 [$ns node]
set r2 [$ns node]
set src1 [$ns node]
set src2 [$ns node]
set recv1 [$ns node]
set recv2 [$ns node]

#Create links between the nodes
$ns duplex-link $r1 $r2 1Mb 5ms DropTail
$ns duplex-link $src1 $r1 10Mb 5ms DropTail
$ns duplex-link $src2 $r1 10Mb 12.5ms DropTail
$ns duplex-link $r2 $recv1 10Mb 5ms DropTail
$ns duplex-link $r2 $recv2 10Mb 12.5ms DropTail

#Give node position (for NAM)
$ns duplex-link-op $src1 $r1 orient right-down
$ns duplex-link-op $src2 $r1 orient right-up
$ns duplex-link-op $r1 $r2 orient right
$ns duplex-link-op $r2 $recv1 orient right-up
$ns duplex-link-op $r2 $recv2 orient right-down

#Setup a TCP connection
set tcp [new Agent/TCP/Vegas]
$tcp set class_ 2
$ns attach-agent $src1 $tcp
set sink [new Agent/TCPSink]
$ns attach-agent $recv1 $sink
$ns connect $tcp $sink
$tcp set fid_ 1

#Setup a FTP over TCP connection
set ftp [new Application/FTP]
$ftp attach-agent $tcp
$ftp set type_ FTP

#Setup a TCP connection
set tcp1 [new Agent/TCP/Vegas]
$tcp1 set class_ 2
$ns attach-agent $src2 $tcp1
set sink1 [new Agent/TCPSink]
$ns attach-agent $recv2 $sink1
$ns connect $tcp1 $sink1
$tcp1 set fid_ 2

#Setup a FTP over TCP connection
set ftp1 [new Application/FTP]
$ftp1 attach-agent $tcp1
$ftp1 set type_ FTP

#Start logging the received bandwidth
# $ns at 0.0 "record"

#Schedule events for the FTP agents
$ns at 0.0 "$ftp start"
$ns at 0.0 "$ftp1 start"
$ns at 400.0 "$ns detach-agent $src1 $tcp; $ns detach-agent $recv1 $sink; $ftp stop"
$ns at 400.0 "$ns detach-agent $src2 $tcp1; $ns detach-agent $recv2 $sink1; $ftp1 stop"

#Call the finish procedure after 5 seconds of simulation time
$ns at 400.0 "finish"

#Run the simulation
$ns run