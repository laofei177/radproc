data = filename.'.txt'

set key autotitle columnhead # si no stats falla
stats data nooutput
COLUMNS = int(STATS_columns)

set ylabel 'Log(E_{NT} / erg s^{-1})'
set xlabel 'Log(z / pc)'

set terminal svg size 800,600 fname 'Verdana' fsize 10
set output filename.'.svg'

plot for [i=2:COLUMNS] data using 1:i with line
