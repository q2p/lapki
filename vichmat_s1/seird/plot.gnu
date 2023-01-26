set term png
set output "result_seird.png"
set yrange [0:*]
plot "result.txt" using 1:2 with lines title 'Susceptible', \
               "" using 1:3 with lines title 'Exposed', \
               "" using 1:4 with lines title 'Infected', \
               "" using 1:5 with lines title 'Recovered', \
               "" using 1:6 with lines title 'Dead'
set output "result_ird.png"
plot "result.txt" using 1:4 with lines title 'Infected', \
               "" using 1:5 with lines title 'Recovered', \
               "" using 1:6 with lines title 'Dead'