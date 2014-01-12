BEGIN {
	router=1; dest1=4; dest2=5; duration; 
	total_bits_1 = total_bits_2 = total_bits_3 = 0;
	count1 = count2 = count3 = 0;
	start1 = start2 = start3 = end1 = end2 = end3 = 0;
}

/^r/ && $2 > 100 && $3 == router && $4 == dest1 && $5 == "tcp" {
    total_bits_1 += 8*$6;
    if ( count1 == 0 ) {
    	start1 = $2; 
    	count1++;
    } 
    else {
    	end1 = $2;
    };
};

/^r/ && $2 > 100 && $3 == router && $4 == dest2 && $5 == "tcp" {
    total_bits_2 += 8*$6;
    if ( count2 == 0 ) {
    	start2 = $2; 
    	count2++;
    } 
    else {
    	end2 = $2;
    };
};

/^r/ && $2 > 100 && $3 == router && ($4 == dest1 || $4 == dest2) && $5 == "tcp" {
    total_bits_3 += 8*$6;
    if ( count3 == 0 ) {
    	start3 = $2; 
    	count3++;
    } 
    else {
    	end3 = $2;
    };
};


END{
	print "Src1 to Recv1 ";
	duration = end1 - start1;
	print "duration of transmission is " duration " seconds";
	print "  - S1 - R1 bits transferred in the duration = " total_bits_1 " bits";
	throughput1 = total_bits_1/duration/1e3
	print "  - Thoughput of src1 - dest1 = "  throughput1 " kbps.";

	print "Src2 to Recv2";
	duration = end2 - start2;
	print "duration of transmission is " duration " seconds";
	print "  - S2 - R2 bits transferred in the duration = " total_bits_2 " bits";
	throughput2 = total_bits_2/duration/1e3
	print "  - Thoughput of src2 - dest2 = "  throughput2 " kbps.";
	print " Ratio of two throughputs is " throughput1/throughput2;
	print ""; 

	duration = end3 - start3;
	print "total duration of transmission is" duration " seconds";
	print "  - Total bits transferred in the duration = " total_bits_3 " bits";
	throughput3 = total_bits_3/duration/1e6
	print "Total Throughput is " throughput3 " mbps"; 
};