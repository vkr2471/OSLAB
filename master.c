/*
21CS30058 Voddula Karthik Reddy
21CS10057 Sanskar Mittal
*/

#include "vm.h"

void print_freelist(freelist *ptr)
{
	int i;
	printf("Free List: ");
	for (i = 0; i <= ptr->current; i++)
	{
		printf("%d ", ptr->flist[i]);
	}
	printf("\n");
}


void wrapup(int status);



void print_PCB(pcb p)
{
	printf("PID = %d m = %d f_cnt = %d\n", p.pid, p.m, p.f_cnt);
}


int readyid, msgq2id, msgq3id, ptbid, freelid;
int pcbid;
int k, m, f;
int flg = 0;

void print_ptb(ptbentry *ptr, int m)
{
	int i, j;
	for (i = 0; i < k; i++)
	{
		printf("PID = %d\n", i);
		for (j = 0; j < m; j++)
		{
			printf("Page %d: Frame %d, Valid %d\n", j, ptr[i * m + j].frameno, ptr[i * m + j].isvalid);
		}
	}
}


void createFreeList()
{
	int i;

	freelid = shmget(IPC_PRIVATE, sizeof(freelist) + f * sizeof(int), 0666 | IPC_CREAT | IPC_EXCL);
	if (freelid == -1)
	{
		printf("cannot create freelist\n");
		wrapup(EXIT_FAILURE);
	}

	freelist *ptr = (freelist *)(shmat(freelid, NULL, 0));
	if (ptr==NULL)
	{
		printf("cannot attach to freelist\n");
		wrapup(EXIT_FAILURE);
	}
	for (i = 0; i < f; i++)
	{
		ptr->flist[i] = i;
	}
	ptr->current = f - 1;

	if (shmdt(ptr) == -1)
	{
		printf("cannot detach from freelist\n");
	
		wrapup(EXIT_FAILURE);
	}
}

void createPageTables()
{
	int i;
	ptbid = shmget(IPC_PRIVATE, m * sizeof(ptbentry) * k, 0666 | IPC_CREAT | IPC_EXCL);
	if (ptbid == -1)
	{
		
		wrapup(EXIT_FAILURE);
	}

	ptbentry *ptr = (ptbentry *)(shmat(ptbid, NULL, 0));
	if (*(int *)ptr == -1)
	{
		
		wrapup(EXIT_FAILURE);
	}

	for (i = 0; i < k * m; i++)
	{
		ptr[i].frameno = -1;
		ptr[i].isvalid = 0;
	}

	if (shmdt(ptr) == -1)
	{
		
		wrapup(EXIT_FAILURE);
	}
}

void freeRes()
{

	msgctl(msgq2id, IPC_RMID, NULL);

	msgctl(msgq3id, IPC_RMID, NULL);

	shmctl(freelid, IPC_RMID, NULL);

	shmctl(pcbid, IPC_RMID, NULL);
	shmctl(ptbid, IPC_RMID, NULL);

	msgctl(readyid, IPC_RMID, NULL);
}

void createMessageQueues()
{

	readyid = msgget(IPC_PRIVATE, 0666 | IPC_CREAT | IPC_EXCL);
	if (readyid == -1)
	{
		perror("ready-msgget");
		wrapup(EXIT_FAILURE);
	}

	msgq2id = msgget(IPC_PRIVATE, 0666 | IPC_CREAT | IPC_EXCL);
	if (msgq2id == -1)
	{
		perror("msgq2-msgget");
		wrapup(EXIT_FAILURE);
	}

	msgq3id = msgget(IPC_PRIVATE, 0666 | IPC_CREAT | IPC_EXCL);
	if (msgq3id == -1)
	{
		perror("msgq3-msgget");
		wrapup(EXIT_FAILURE);
	}
}

void createPCBs()
{
	int i;

	pcbid = shmget(IPC_PRIVATE, sizeof(pcb) * k, 0666 | IPC_CREAT | IPC_EXCL);
	if (pcbid == -1)
	{
		perror("master:cannot create pcbid\n");
		wrapup(EXIT_FAILURE);
	}

	pcb *ptr = (pcb *)(shmat(pcbid, NULL, 0));
	if (ptr == NULL)
	{
		perror("master: cannot attach to pcb\n");
		wrapup(EXIT_FAILURE);
	}

	int totpages = 0;
	for (i = 0; i < k; i++)
	{
		ptr[i].pid = i;
		ptr[i].m = rand() % m + 1;
		ptr[i].f_allo = 0;
		totpages += ptr[i].m;
	}
	int allo_frame = 0;
	printf("tot = %d, k = %d, f=  %d\n", totpages, k, f);
	int max = 0, maxi = 0;
	for (i = 0; i < k; i++)
	{
		ptr[i].pid = -1;
		int allo = (int)round(ptr[i].m * (f - k) / (float)totpages) + 1;
		if (ptr[i].m > max)
		{
			max = ptr[i].m;
			maxi = i;
		}
		allo_frame = allo_frame + allo;
		ptr[i].f_cnt = allo;
	}
	ptr[maxi].f_cnt += f - allo_frame;

	for (i = 0; i < k; i++)
	{
		print_PCB(ptr[i]);
	}

	if (shmdt(ptr) == -1)
	{
		perror("freel-shmdt");
		wrapup(EXIT_FAILURE);
	}
}

void wrapup(int status)
{
	freeRes();
	exit(status);
}

void createProcesses()
{
	pcb *ptr = (pcb *)(shmat(pcbid, NULL, 0));
	if (ptr == NULL)
	{
		perror("master: cannot attach to pcb\n");
		wrapup(EXIT_FAILURE);
	}

	int i, j;
	for (i = 0; i < k; i++)
	{
		int reference_string_len = rand() % (8 * ptr[i].m) + 2 * ptr[i].m + 1;
		char reference_string[m * 20 * 40];
		printf("reference_string_length = %d\n", reference_string_len);
		int l = 0;
		for (j = 0; j < reference_string_len; j++)
		{
			int r;
			r = rand() % ptr[i].m;
			float p = (rand() % 100) / 100.0;
			if (p < 0.2)
			{
				r = rand() % (1000 * m) + ptr[i].m;
			}
			l += sprintf(reference_string + l, "%d|", r);
		}
		printf("Ref string = %s\n", reference_string);
		if (fork() == 0)
		{
			char buf1[20], buf2[20], buf3[20];
			sprintf(buf1, "%d", i);
			sprintf(buf2, "%d", readyid);
			sprintf(buf3, "%d", msgq3id);
			execlp("./process", "./process", buf1, buf2, buf3, reference_string, (char *)(NULL));
			exit(0);
		}

		usleep(250 * 1000);
	}
}
int pid, spid, mpid;

void leave(int sig)
{

	sleep(1);
	kill(spid, SIGTERM);
	kill(mpid, SIGUSR2);
	sleep(2);
	flg = 1;
}
int main(int argc, char const *argv[])
{
	srand(time(NULL));
	signal(SIGUSR1, leave);
	signal(SIGINT, wrapup);
	if (argc < 4)
	{
		printf("master k m f\n");
		wrapup(EXIT_FAILURE);
	}
	k = atoi(argv[1]);
	m = atoi(argv[2]);
	f = atoi(argv[3]);
	pid = getpid();
	//making sure atleast one fram is available per process
	if (k <= 0 || m <= 0 || f <= 0 || f < k)
	{
		printf("Invalid input\n");
		wrapup(EXIT_FAILURE);
	}

	createPageTables();
	createFreeList();
	createPCBs();
	createMessageQueues();

	if ((spid = fork()) == 0)
	{
		char buff[5][20];
		sprintf(buff[1], "%d", readyid);
		sprintf(buff[2], "%d", msgq2id);
		sprintf(buff[3], "%d", k);
		sprintf(buff[4], "%d", pid);
		execlp("./scheduler", "./scheduler", buff[1], buff[2], buff[3], buff[4], (char *)(NULL));
		exit(0);
	}

	if ((mpid = fork()) == 0)
	{
		char buff[8][20];
		sprintf(buff[1], "%d", msgq2id);
		sprintf(buff[2], "%d", msgq3id);
		sprintf(buff[3], "%d", ptbid);
		sprintf(buff[4], "%d", freelid);
		sprintf(buff[5], "%d", pcbid);
		sprintf(buff[6], "%d", m);
		sprintf(buff[7], "%d", k);
		execlp("./mmu", "./mmu", buff[1], buff[2], buff[3], buff[4], buff[5], buff[6], buff[7], (char *)(NULL));
		exit(0);
	}
	printf("generating processed\n");
	//** create all the auxliray process
	createProcesses();
	if (flg == 0)
		pause();
	freeRes();

	return 0;
}