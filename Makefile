all: process.c mmu.c master.c scheduler.c
	gcc master.c -o master -lm
	gcc scheduler.c -o scheduler 
	gcc mmu.c -o mmu 
	gcc process.c -o process 

clean:
	rm process master scheduler mmu