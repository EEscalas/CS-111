#! /usr/bin/gnuplot
#
# purpose:
#	 generate data reduction graphs for the multi-threaded list project
#
# input: lab2_list.csv
#	1. test name
#	2. # threads
#	3. # iterations per thread
#	4. # lists
#	5. # operations performed (threads x iterations x (ins + lookup + delete))
#	6. run time (ns)
#	7. run time per operation (ns)
#
# output:
#	lab2b_1.png ... throughput vs number of threads for mutex and spin-lock synchronized list operations
#	lab2b_2.png ... mean time per mutex wait and mean time per operation for mutex-synchronized list operations
#	lab2b_3.png ... successful iterations vs threads for each synchronization method
#	lab2b_4.png ... throughput vs number of threads for mutex synchronized partitioned lists
#	lab2b_5.png ... throughput vs number of threads for spin-lock synchronized partitioned lists
#
# Note:
#	Managing data is simplified by keeping all of the results in a single
#	file.  But this means that the individual graphing commands have to
#	grep to select only the data they want.
#

# general plot parameters
set terminal png
set datafile separator ","

set title "Scalability-1: Throughput of Synchronized Lists"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "Throughput (operations/sec)"
set logscale y 10
set output 'lab2b_1.png'
set key center top
plot \
	"< grep list-none-m lab2b_list_2.csv" using ($2):(1000000000/($7)) \
	title 'list ins/lookup/delete w/mutex' with linespoints lc rgb 'orange', \
	"< grep list-none-s lab2b_list_1.csv" using ($2):(1000000000/($7)) \
	title 'list ins/lookup/delete w/spin' with linespoints lc rgb 'violet'


set title "Scalability-2: Per-operation Times for Mutex-Protected List Operations"
set xlabel "Threads"
set logscale x 2
unset xrange
set xrange [0.75:]
set ylabel "Mean time/operation (ns)"
set logscale y
set output 'lab2b_2.png'
set key left top
plot \
     "lab2b_list_2.csv" using ($2):($7) \
     title 'completion time' with linespoints lc rgb 'orange', \
     "lab2b_list_2.csv" using ($2):($8) \
     title 'wait for lock' with linespoints lc rgb 'red'
     
set title "Scalability-3: Correct Synchronization of Partitioned Lists"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "Successful Iterations"
set logscale y 10
set output 'lab2b_3.png'
set key left top

# note that unsuccessful runs should have produced no output
plot \
     "< grep list-id-none lab2b_list.csv" using ($2):($3) \
	title 'yield=id' with points lc rgb 'red', \
	"< grep list-id-m lab2b_list.csv" using ($2):($3) \
	title 'Mutex' with points lc rgb 'green', \
	"< grep list-id-s lab2b_list.csv" using ($2):($3) \
	title 'Spin-Lock' with points lc rgb 'blue'

set title "Scalability-4: Throughput of Mutex Synchronized Partitioned Lists"
set xlabel "Threads"
set logscale x 2
unset xrange
set xrange [0.75:]
set ylabel "Throughput (operations/sec)"
set logscale y
set output 'lab2b_4.png'
set key left top
plot \
	"lab2b_list_4-1.csv" using ($2):(1000000000/($7)) \
	title 'lists=1' with linespoints lc rgb 'violet', \
	"lab2b_list_4-4.csv" using ($2):(1000000000/($7)) \
	title 'lists=4' with linespoints lc rgb 'green', \
	"lab2b_list_4-8.csv" using ($2):(1000000000/($7)) \
	title 'lists=8' with linespoints lc rgb 'blue', \
	"lab2b_list_4-16.csv" using ($2):(1000000000/($7)) \
	title 'lists=16' with linespoints lc rgb 'orange'

set title "Scalability-5: Throughput of Spin-Lock Synchronized Partitioned Lists"
set xlabel "Threads"
set logscale x 2
unset xrange
set xrange [0.75:]
set ylabel "Throughput (operations/sec)"
set logscale y
set output 'lab2b_5.png'
set key left top
plot \
     	"lab2b_list_5-1.csv" using ($2):(1000000000/($7)) \
	title 'lists=1' with linespoints lc rgb 'violet', \
	"lab2b_list_5-4.csv" using ($2):(1000000000/($7)) \
	title 'lists=4' with linespoints lc rgb 'green', \
	"lab2b_list_5-8.csv" using ($2):(1000000000/($7)) \
	title 'lists=8' with linespoints lc rgb 'blue', \
	"lab2b_list_5-16.csv" using ($2):(1000000000/($7)) \
	title 'lists=16' with linespoints lc rgb 'orange'