There are 4 cases
1. scenario 1 Droptail: droptail_1.tcl
2. scemario 2 Droptail: droptail_2.tcl
3. scenario 1 RED: red1.tcl
4  scenario 2 RED: red2.tcl

There are 2 throughput calculator
for scenario 1: throughput1.awk
for scenario 2: throughput2.awk

Result:
1. case1 output tcp_drop1
sink1 average throughput=500.09kbps
sink2=499.92kbps

2. case2 output tcp_drop2
sink1 average throughput=0bps
sink2=0bps
sink3=1000kbps

3. case3 output tcp_red1
sink1 average throughput=480.56kbps
sink2=518.61kbps

4. case4 output tcp_red2
sink1 average throughput=19.47kbps
sink2=6.82kbps
sink3=973.71kbps

