#NAME: Elena Escalas
#EMAIL: elenaescalas@g.ucla.edu
#ID: 704560697

CC = gcc
RM = rm -f

build: default

default:
	@$(CC) -pthread -lrt -g -o lab2_list lab2_list.c SortedList.c

graphs:
	@cat lab2b_list.csv | grep 'list-none-s,[1248]\?[46]\?,1000,1,' > lab2b_list_1.csv

	@cat lab2b_list.csv | grep 'list-none-m,[1248]\?[46]\?,1000,1,' > lab2b_list_2.csv

	@cat lab2b_list.csv | grep 'list-none-m,[1248]\?[2]\?,1000,1,' > lab2b_list_4-1.csv
	@cat lab2b_list.csv | grep 'list-none-m,[1248]\?[2]\?,1000,4,' > lab2b_list_4-4.csv
	@cat lab2b_list.csv | grep 'list-none-m,[1248]\?[2]\?,1000,8,' > lab2b_list_4-8.csv
	@cat lab2b_list.csv | grep 'list-none-m,[1248]\?[2]\?,1000,16,' > lab2b_list_4-16.csv

	@cat lab2b_list.csv | grep 'list-none-s,[1248]\?[2]\?,1000,1,' > lab2b_list_5-1.csv
	@cat lab2b_list.csv | grep 'list-none-s,[1248]\?[2]\?,1000,4,' > lab2b_list_5-4.csv
	@cat lab2b_list.csv | grep 'list-none-s,[1248]\?[2]\?,1000,8,' > lab2b_list_5-8.csv
	@cat lab2b_list.csv | grep 'list-none-s,[1248]\?[2]\?,1000,16,' > lab2b_list_5-16.csv
	@gnuplot lab2b_list.gp

profile:
	-rm -f ./raw.gperf
	LD_PRELOAD=/u/cs/ugrad/elena/lib/libprofiler.so.0 CPUPROFILE=./raw.gperf ./lab2_list --threads=\
12 --iterations=1000 --sync=s
	pprof --text ./lab2_list ./raw.gperf > profile.out
	pprof --list=threadTask ./lab2_list ./raw.gperf >> profile.out
	rm -f ./raw.gperf

dist:
	@tar -cf lab2b-704560697.tar.gz README Makefile lab2_list.c SortedList.h SortedLi\
st.c lab2b_list.csv lab2b_list.gp profile.out lab2b_1.png lab2b_2.png lab2b_3.png lab2b_4.png lab2b_5.p\
ng

clean:
	@$(RM) lab2_list profile.out *.csv *.png

tests:
# Timing Mutex Waits
	./lab2_list --threads=1 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=2 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=24 --iterations=1000 --sync=m >> lab2b_list.csv

# Addressing the Underlying Problem
# No Sync w/ yields
	-./lab2_list --threads=1 --iterations=1 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=1 --iterations=2 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=1 --iterations=4 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=1 --iterations=8 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=1 --iterations=16 --lists=4 --yield=id >> lab2b_list.csv

	-./lab2_list --threads=4 --iterations=1 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=4 --iterations=2 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=4 --iterations=4 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=4 --iterations=8 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=4 --iterations=16 --lists=4 --yield=id >> lab2b_list.csv

	-./lab2_list --threads=8 --iterations=1 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=8 --iterations=2 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=8 --iterations=4 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=8 --iterations=8 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=8 --iterations=16 --lists=4 --yield=id >> lab2b_list.csv

	-./lab2_list --threads=12 --iterations=1 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=2 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=4 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=8 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=16 --lists=4 --yield=id >> lab2b_list.csv

	-./lab2_list --threads=16 --iterations=1 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=16 --iterations=2 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=16 --iterations=4 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=16 --iterations=8 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=16 --iterations=16 --lists=4 --yield=id >> lab2b_list.csv

# Sync Mutex w/ yields
#	./lab2_list --threads=1 --iterations=10 --lists=4 --yield=id --sync=m >> lab2b_list.csv
#	./lab2_list --threads=1 --iterations=20 --lists=4 --yield=id --sync=m >> lab2b_list.csv
#	./lab2_list --threads=1 --iterations=40 --lists=4 --yield=id --sync=m >> lab2b_list.csv
#	./lab2_list --threads=1 --iterations=80 --lists=4 --yield=id --sync=m >> lab2b_list.csv

	./lab2_list --threads=4 --iterations=10 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=20 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=40 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=80 --lists=4 --yield=id --sync=m >> lab2b_list.csv

	./lab2_list --threads=8 --iterations=10 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=20 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=40 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=80 --lists=4 --yield=id --sync=m >> lab2b_list.csv

	./lab2_list --threads=12 --iterations=10 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=20 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=40 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=80 --lists=4 --yield=id --sync=m >> lab2b_list.csv

	./lab2_list --threads=16 --iterations=10 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=20 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=40 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=80 --lists=4 --yield=id --sync=m >> lab2b_list.csv

# Sync Spin Lock w/ yields
#	./lab2_list --threads=1 --iterations=10 --lists=4 --yield=id --sync=s >> lab2b_list.csv
#	./lab2_list --threads=1 --iterations=20 --lists=4 --yield=id --sync=s >> lab2b_list.csv
#	./lab2_list --threads=1 --iterations=40 --lists=4 --yield=id --sync=s >> lab2b_list.csv
#	./lab2_list --threads=1 --iterations=80 --lists=4 --yield=id --sync=s >> lab2b_list.csv

	./lab2_list --threads=4 --iterations=10 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=20 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=40 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=80 --lists=4 --yield=id --sync=s >> lab2b_list.csv

	./lab2_list --threads=8 --iterations=10 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=20 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=40 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=80 --lists=4 --yield=id --sync=s >> lab2b_list.csv

	./lab2_list --threads=12 --iterations=10 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=20 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=40 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=80 --lists=4 --yield=id --sync=s >> lab2b_list.csv

	./lab2_list --threads=16 --iterations=10 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=20 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=40 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=80 --lists=4 --yield=id --sync=s >> lab2b_list.csv

# Sync Mutex w/o yields
#	./lab2_list --threads=1 --iterations=1000 --lists=1 --sync=m >> lab2b_list.csv
	./lab2_list --threads=1 --iterations=1000 --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --threads=1 --iterations=1000 --lists=8 --sync=m >> lab2b_list.csv
	./lab2_list --threads=1 --iterations=1000 --lists=16 --sync=m >> lab2b_list.csv

#	./lab2_list --threads=2 --iterations=1000 --lists=1 --sync=m >> lab2b_list.csv
	./lab2_list --threads=2 --iterations=1000 --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --threads=2 --iterations=1000 --lists=8 --sync=m >> lab2b_list.csv
	./lab2_list --threads=2 --iterations=1000 --lists=16 --sync=m >> lab2b_list.csv

#	./lab2_list --threads=4 --iterations=1000 --lists=1 --sync=m >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=1000 --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=1000 --lists=8 --sync=m >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=1000 --lists=16 --sync=m >> lab2b_list.csv

#	./lab2_list --threads=8 --iterations=1000 --lists=1 --sync=m >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=1000 --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=1000 --lists=8 --sync=m >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=1000 --lists=16 --sync=m >> lab2b_list.csv

	./lab2_list --threads=12 --iterations=1000 --lists=1 --sync=m >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=1000 --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=1000 --lists=8 --sync=m >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=1000 --lists=16 --sync=m >> lab2b_list.csv

# Sync Spin-Lock w/o yields
	./lab2_list --threads=1 --iterations=1000 --lists=1 --sync=s >> lab2b_list.csv
	./lab2_list --threads=1 --iterations=1000 --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --threads=1 --iterations=1000 --lists=8 --sync=s >> lab2b_list.csv
	./lab2_list --threads=1 --iterations=1000 --lists=16 --sync=s >> lab2b_list.csv

	./lab2_list --threads=2 --iterations=1000 --lists=1 --sync=s >> lab2b_list.csv
	./lab2_list --threads=2 --iterations=1000 --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --threads=2 --iterations=1000 --lists=8 --sync=s >> lab2b_list.csv
	./lab2_list --threads=2 --iterations=1000 --lists=16 --sync=s >> lab2b_list.csv

	./lab2_list --threads=4 --iterations=1000 --lists=1 --sync=s >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=1000 --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=1000 --lists=8 --sync=s >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=1000 --lists=16 --sync=s >> lab2b_list.csv

	./lab2_list --threads=8 --iterations=1000 --lists=1 --sync=s >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=1000 --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=1000 --lists=8 --sync=s >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=1000 --lists=16 --sync=s >> lab2b_list.csv

	./lab2_list --threads=12 --iterations=1000 --lists=1 --sync=s >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=1000 --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=1000 --lists=8 --sync=s >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=1000 --lists=16 --sync=s >> lab2b_list.csv

	./lab2_list --threads=16 --iterations=1000 --lists=1 --sync=s >> lab2b_list.csv
	./lab2_list --threads=24 --iterations=1000 --lists=1 --sync=s >> lab2b_list.csv
