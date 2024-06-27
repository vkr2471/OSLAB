/*
21CS30058 Voddula Karthik Reddy
21CS10057 Sanskar Mittal
*/

#include "vm.h"

int count = 0;

FILE *resultfile;
int i;
int pcbid,ptbid, freelid,msgq2id, msgq3id;
int m,k;
int *pffreq;

void reply(int id, int frameno)
{
	struct mmutopbuf mbuf;
	mbuf.mtype = id + MMUTOPRO;
	mbuf.frameno = frameno;
	int length = sizeof(struct msgbuf) - sizeof(long);
	int rst = msgsnd(msgq3id, &mbuf, length, 0);
	if (rst == -1)
	{
		perror("error in sending reply");
		exit(EXIT_FAILURE);
	}
}
int getRequest(int* id)
{
	struct msgbuf mbuf;
	int length;
	length = sizeof(struct msgbuf) - sizeof(long);
	memset(&mbuf, 0, sizeof(mbuf));

	int rst = msgrcv(msgq3id, &mbuf, length, PSEND_TYPE, 0);
	if (rst == -1)
	{
		if(errno == EINTR)
			return -1;
		perror("Error in receiving message");
		exit(EXIT_FAILURE);
	}
	*id = mbuf.id;
	return mbuf.pageno;
}

void notifySched(int type)
{
	struct mmutosch mbuf;
	mbuf.mtype = type;
	int length = sizeof(struct msgbuf) - sizeof(long);
	int rst = msgsnd(msgq2id, &mbuf, length, 0);
	if (rst == -1)
	{
		perror("msgsnd");
		exit(EXIT_FAILURE);
	}
	//printf("Sent signal to sched = %d\n",type);

}
pcb *pcbptr;
ptbentry *ptbptr;
freelist *free_list_ptr;

int pfc(int id, int pageno)
{
	int i;
	if (free_list_ptr->current == -1 || pcbptr[i].f_cnt <= pcbptr[i].f_allo)
	{
		int min = INT_MAX, mini = -1;
		int donor = 0;
		for (i = 0; i < pcbptr[i].m; i++)
		{
			if (ptbptr[id * m + i].isvalid == 1)
			{
				if (ptbptr[id * m + i].count < min)
				{
					min = ptbptr[id * m + i].count;
					donor = ptbptr[id * m + i].frameno;
					mini = i;
				}
			}
		}
		ptbptr[id * m + mini].isvalid = 0;
		return donor;
	}
	else
	{
		int fn = free_list_ptr->flist[free_list_ptr->current];
		free_list_ptr->current -= 1;
		return fn;
	}
}

void releasepages(int i)
{

	int k = 0;
	for (k = 0; k < pcbptr[i].m; i++)
	{
		if (ptbptr[i * m + k].isvalid == 1)
		{
			free_list_ptr->flist[free_list_ptr->current + 1] = ptbptr[i * m + k].frameno;
			free_list_ptr->current += 1;
		}
	}
	
}

int serviceMRequest()
{
	pcbptr = (pcb*)(shmat(pcbid, NULL, 0));

	ptbptr = (ptbentry*)(shmat(ptbid, NULL, 0));

	free_list_ptr = (freelist*)(shmat(freelid, NULL, 0));

	int id = -1, pageno;
	pageno = getRequest(&id);
	if(pageno == -1 && id == -1)
	{
		return 0;
	}
	int i = id;
	if (pageno == PROCESS_OVER)
	{
		releasepages(id);
		notifySched(TERMINATED);
		return 0;
	}
	count ++;
	printf("MMU : Page reference : cnt:%d,id:%d,pgno:%d\n",count,id,pageno);
	fprintf(resultfile,"Page reference : (%d,%d,%d)\n",count,id,pageno);
	if (pcbptr[id].m < pageno || pageno < 0)
	{
		printf("MMU : Invalid Page Reference : id:%d, pgno:%d\n",id,pageno);
		fprintf(resultfile,"Invalid Page Reference : %d %d\n",id,pageno);
		reply(id, INVALID_PAGE_REF);
		printf("MMU : Process %d: TRYING TO ACCESS INVALID PAGE REFERENCE %d\n", id, pageno);
		releasepages(id);
		notifySched(TERMINATED);


	}
	else
	{
		if (ptbptr[i * m + pageno].isvalid == 0)
		{

			printf("MMU: Page Fault : (%d, %d)\n",id,pageno);
			fprintf(resultfile,"Page Fault : (%d, %d)\n",id,pageno);
			pffreq[id] += 1;
			reply(id, -1);
			int fno = pfc(id, pageno);
			ptbptr[i * m + pageno].isvalid = 1;
			ptbptr[i * m + pageno].count = count;
			ptbptr[i * m + pageno].frameno = fno;
			
			notifySched(PAGEFAULT_HANDLED);
		}
		else
		{
			reply(id, ptbptr[i * m + pageno].frameno);
			ptbptr[i * m + pageno].count = count;
			//FRAME FOUND
		}
	}
	shmdt(pcbptr);
	
	shmdt(ptbptr) ;
	
	shmdt(free_list_ptr);
	return 1;
	
}
int flg = 1;
void handletgerm(int sig)
{
	flg = 0;
}

int main(int argc, char const *argv[])
{

	msgq2id = atoi(argv[1]);
	msgq3id = atoi(argv[2]);
	ptbid = atoi(argv[3]);
	freelid = atoi(argv[4]);
	pcbid = atoi(argv[5]);
	m = atoi(argv[6]);
	k = atoi(argv[7]);
	signal(SIGUSR2, handletgerm);
	pffreq = (int *)malloc(k*sizeof(int));
	for(i=0;i<k;i++)
	{
		pffreq[i] = 0;
	} 
	resultfile = fopen("result.txt","w");
	while(flg)
	{
		serviceMRequest();
	}
	printf("Page fault Count for each Process:\n");	
	fprintf(resultfile,"Page fault Count for each Process:\n");
	printf("pNo\tFreq\n");
	fprintf(resultfile,"PNo\tFreq\n");
	for(i = 0;i<k;i++)
	{
		printf("%d\t%d\n",i,pffreq[i]);
		fprintf(resultfile,"%d\t%d\n",i,pffreq[i]);
	}
	fclose(resultfile);
	return 0;
}