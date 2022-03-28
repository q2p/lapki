set term png
set output "initial.png"
plot "initial.txt" with linespoints
set output "interpolated.png"
plot "interpolated.txt" with linespoints
