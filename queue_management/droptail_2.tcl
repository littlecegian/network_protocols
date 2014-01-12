#Create a simulator object
set ns [new Simulator]

#Define different colors for data flows (for NAM)
$ns color 1 Blue
$ns color 2 Red
$ns color 3 Yellow

#Open the NAM trace file
set f0 [open droptail_scenario2_receiver1.gr w]
set f1 [open droptail_scenario2_receiver2.gr w]
set f2 [open droptail_scenario2_receiver3.gr w]
set f3 [open droptail_scenario2_total.gr w]
set nam_trace_fd [open tcp_drop2.nam w]
$ns namtrace-all $nam_trace_fd


# open the measurement output files
set trace_fd [open tcp_drop2.tr w]
$ns trace-all $trace_fd

#Define a 'finish' procedure
proc finish {} {
    global ns nam_trace_fd trace_fd f0 f1 f2 f3
    
    #close the nam trace file
    $ns flush-trace
    #Close the NAM trace file
    close $nam_trace_fd
    #Close the Trace file
    close $trace_fd
    close $f0
    close $f1
    close $f2
    close $f3

    exec xgraph droptail_scenario2_receiver1.gr droptail_scenario2_receiver2.gr droptail_scenario2_receiver3.gr -geometry 800x400 &
    exec xgraph droptail_scenario2_total.gr -geometry 800x400 &

    #Execute NAM on the trace file
    exec nam tcp_drop2.nam &
    exit 0
}

proc record {} {
    global ns sink1 sink2 sink3 f0 f1 f2 f3
    #Get an instance of the simulator
    set ns [Simulator instance]
    #Set the time after which the procedure should be called again
        set time 0.5
    #How many bytes have been received by the traffic sinks?
        set bw0 [$sink1 set bytes_]
        set bw1 [$sink2 set bytes_]
        set bw2 [$sink3 set bytes_]
    #Get the current time
        set now [$ns now]
    #Calculate the bandwidth (in MBit/s) and write it to the files
        puts $f0 "$now [expr $bw0/$time*8/1000000]"
        puts $f1 "$now [expr $bw1/$time*8/1000000]"
        puts $f2 "$now [expr $bw2/$time*8/1000000]"
        puts $f3 "$now [expr ($bw0 + $bw1 + $bw2)/$time*8/1000000]"
    #Reset the bytes_ values on the traffic sinks
        $sink1 set bytes_ 0
        $sink2 set bytes_ 0
        $sink3 set bytes_ 0
    #Re-schedule the procedure
        $ns at [expr $now+$time] "record"
}

#Create six nodes
set h1 [$ns node]
set h2 [$ns node]
set h3 [$ns node]
set r1 [$ns node]
set r2 [$ns node]
set h4 [$ns node]
set h5 [$ns node]
set h6 [$ns node]

#Create links between the nodes
$ns duplex-link $h1 $r1 10Mb 1ms DropTail
$ns duplex-link $h2 $r1 10Mb 1ms DropTail
$ns duplex-link $h3 $r1 10Mb 1ms DropTail
$ns duplex-link $r1 $r2 1Mb 10ms DropTail
$ns duplex-link $r2 $h4 10Mb 1ms DropTail
$ns duplex-link $r2 $h5 10Mb 1ms DropTail
$ns duplex-link $r2 $h6 10Mb 1ms DropTail
$ns queue-limit $r1 $r2 20




#Give node position (for NAM)
$ns duplex-link-op $h1 $r1 orient right-down
$ns duplex-link-op $h2 $r1 orient right
$ns duplex-link-op $h3 $r1 orient right-up
$ns duplex-link-op $r1 $r2 orient right
$ns duplex-link-op $r2 $h4 orient right-up
$ns duplex-link-op $r2 $h5 orient right
$ns duplex-link-op $r2 $h6 orient right-down


#Setup a TCP connection
set tcp1 [new Agent/TCP/Reno]
$tcp1 set class_ 1
$ns attach-agent $h1 $tcp1
$tcp1 set fid_ 1

set sink1 [new Agent/TCPSink]
$ns attach-agent $h4 $sink1
$ns connect $tcp1 $sink1

#Setup a TCP connection
set tcp2 [new Agent/TCP/Reno]
$tcp2 set class_ 2
$ns attach-agent $h2 $tcp2
$tcp2 set fid_ 2

set sink2 [new Agent/TCPSink]
$ns attach-agent $h5 $sink2
$ns connect $tcp2 $sink2

#Setup a UDP connection
set udp [new Agent/UDP]
$tcp1 set class_ 3
$ns attach-agent $h3 $udp
$tcp1 set fid_ 3

set sink3 [new Agent/LossMonitor]
$ns attach-agent $h6 $sink3
$ns connect $udp $sink3

#Setup a CBR over UDP connection
set cbr [new Application/Traffic/CBR]
$cbr attach-agent $udp
$cbr set type_ CBR
$cbr set packet_size_ 100
$cbr set rate_ 1Mb
$cbr set random_ false

#Setup a FTP over TCP connection
set ftp1 [new Application/FTP]
$ftp1 attach-agent $tcp1
$ftp1 set type_ FTP

#Setup a FTP over TCP connection
set ftp2 [new Application/FTP]
$ftp2 attach-agent $tcp2
$ftp2 set type_ FTP

$ns at 0.0 "record"
$ns at 0 "$ftp1 start"
$ns at 0 "$ftp2 start"
$ns at 0 "$cbr start"
$ns at 180 "$ftp1 stop"
$ns at 180 "$cbr stop"
$ns at 180 "$ftp2 stop"
$ns at 181 "$ns detach-agent $h1 $tcp1; $ns detach-agent $h4 $sink1"
$ns at 181 "$ns detach-agent $h2 $tcp2; $ns detach-agent $h5 $sink2"
$ns at 181 "$ns detach-agent $h3 $udp; $ns detach-agent $h6 $sink3"
$ns at 181 "finish"

$ns run
