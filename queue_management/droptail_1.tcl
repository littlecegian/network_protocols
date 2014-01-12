#Create a simulator object
set ns [new Simulator]

#Define different colors for data flows (for NAM)
$ns color 1 Green
$ns color 2 Yellow

set f0 [open droptail_scenario1_receiver1.gr w]
set f1 [open droptail_scenario1_receiver2.gr w]
set f3 [open droptail_scenario1_total.gr w]
set f2 [open tcp_drop1.tr w]
set nf [open tcp_drop1.nam w]
$ns namtrace-all $nf
$ns trace-all $f2

#Define a 'finish' procedure
proc finish {} {
 	global ns f2 nf f0 f1 f3
 	$ns flush-trace
	
	# close $nf
	close $f0
	close $f1
	close $f2
	close $f3

	#Execute NAM on the trace file
 	exec nam tcp_drop1.nam &

 	exec xgraph droptail_scenario1_receiver1.gr droptail_scenario1_receiver2.gr -geometry 800x400 &
 	exec xgraph droptail_scenario1_total.gr -geometry 800x400 &

 	close $nf
 	exit 0
}

proc record {} {
    global ns sink sink1 f0 f1 f3
	#Get an instance of the simulator
	set ns [Simulator instance]
	#Set the time after which the procedure should be called again
        set time 0.5
	#How many bytes have been received by the traffic sinks?
        set bw0 [$sink set bytes_]
        set bw1 [$sink1 set bytes_]
	#Get the current time
        set now [$ns now]
	#Calculate the bandwidth (in MBit/s) and write it to the files
        puts $f0 "$now [expr $bw0/$time*8/1000000]"
        puts $f1 "$now [expr $bw1/$time*8/1000000]"
        puts $f3 "$now [expr ($bw0 + $bw1)/$time*8/1000000]"
	#Reset the bytes_ values on the traffic sinks
        $sink set bytes_ 0
        $sink1 set bytes_ 0
	#Re-schedule the procedure
        $ns at [expr $now+$time] "record"
}


#Create six nodes
set r1 [$ns node]
set r2 [$ns node]
set src1 [$ns node]
set src2 [$ns node]
set recv1 [$ns node]
set recv2 [$ns node]

#Create links between the nodes
$ns duplex-link $r1 $r2 1Mb 10ms DropTail
$ns duplex-link $src1 $r1 10Mb 1ms DropTail
$ns duplex-link $src2 $r1 10Mb 1ms DropTail
$ns duplex-link $r2 $recv1 10Mb 1ms DropTail
$ns duplex-link $r2 $recv2 10Mb 1ms DropTail
$ns queue-limit $r1 $r2 20

#Give node position (for NAM)
$ns duplex-link-op $src1 $r1 orient right-down
$ns duplex-link-op $src2 $r1 orient right-up
$ns duplex-link-op $r1 $r2 orient right
$ns duplex-link-op $r2 $recv1 orient right-up
$ns duplex-link-op $r2 $recv2 orient right-down

#Setup a TCP connection
set tcp [new Agent/TCP/Reno]
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
set tcp1 [new Agent/TCP/Reno]
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


#Schedule events for the FTP agents
$ns at 0.0 "record"
$ns at 0.0 "$ftp start"
$ns at 0.0 "$ftp1 start"
$ns at 181.0 "$ns detach-agent $src1 $tcp; $ns detach-agent $recv1 $sink; $ftp stop"
$ns at 181.0 "$ns detach-agent $src2 $tcp1; $ns detach-agent $recv2 $sink1; $ftp1 stop"

#Call the finish procedure after 5 seconds of simulation time
$ns at 181.0 "finish"

#Run the simulation
$ns run