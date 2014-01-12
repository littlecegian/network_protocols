BEGIN {
  recvdSize = 0
  recvdSize2 = 0
  stopTime = 180
  startTime = 30
}
{
  event = $1
  time = $2
  node_id = $4
  type= $5
  pkt_size = $6

  # Update total received packets' size and store packets arrival time
  if (event == "r" && type == "tcp" && time >= startTime && time <= stopTime && node_id == 4) {
    recvdSize += pkt_size
  }
  else if (event == "r" && type == "tcp" && time >= startTime && time <= stopTime && node_id == 5) {
    recvdSize2 += pkt_size
  }
}  
END {
  printf("\nAverage Throughput[kbps] of src1 -> recv1 = %.2f\n",(recvdSize/(stopTime-startTime))*(8/1000))
  printf("Average Throughput[kbps] of src2 -> recv2 = %.2f\n",(recvdSize2/(stopTime-startTime))*(8/1000))
  printf("Total Throughput[kbps] of the system = %.2f\n\n",((recvdSize + recvdSize2)/(stopTime-startTime))*(8/1000))
}

