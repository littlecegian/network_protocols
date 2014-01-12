BEGIN { 
	total_mbps = 0;
}
{
	total_mbps += $2;
};

END{
duration = 800;
print "Number of records is " NR;
print "Output: ";
print "Transmission: N" fromNode "->N" 4;
print "  - Thoughput = "  total_mbps/duration " Mbps."; 
};