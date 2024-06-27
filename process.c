/*
21CS30058 Voddula Karthik Reddy
21CS10057 Sanskar Mittal
*/

#include "vm.h"

int send_message_mmu(int qid, struct mmumsgbuf_send *qbuf)
{
	int result, length;

	length = sizeof(struct mmumsgbuf_send) - sizeof(long);

	if ((result = msgsnd(qid, qbuf, length, 0)) == -1)
	{
		perror("Error in sending message");
		exit(1);
	}

	return (result);
}
int read_message_mmu(int qid, long type, struct mmumsgbuf_recv *qbuf)
{
	int rval, len;
	len = sizeof(struct mmumsgbuf_recv) - sizeof(long);

	if ((rval = msgrcv(qid, qbuf, len, type, 0)) == -1)
	{
		perror("Error in receiving message");
		exit(1);
	}

	return (rval);
}

int pg_no[MAX_PAGES];
int no_of_pages;

void getallpages(char *refs)
{

	char *token;
	token = strtok(refs, "|");
	while (token != NULL)
	{
		pg_no[no_of_pages] = atoi(token);
		no_of_pages++;
		token = strtok(NULL, "|");
	}
}

int message_to_scheduler(int qid, struct mymsgbuf *qbuf)
{
	int result, length;

	/* The length is essentially the size of the structure minus sizeof(mtype) */
	length = sizeof(struct mymsgbuf) - sizeof(long);

	if ((result = msgsnd(qid, qbuf, length, 0)) == -1)
	{
		perror("Error in sending message");
		exit(1);
	}

	return (result);
}
int read_message_from_scheduler(int qid, long type, struct mymsgbuf *qbuf)
{
	int result, length;

	/* The length is essentially the size of the structure minus sizeof(mtype) */
	length = sizeof(struct mymsgbuf) - sizeof(long);

	if ((result = msgrcv(qid, qbuf, length, type, 0)) == -1)
	{
		perror("Error in receiving message");
		exit(1);
	}

	return (result);
}

int main(int argc, char *argv[])
{
	if (argc < 5)
	{
		perror("Need 5 arguments {id,mq1,mq3,ref_string}\n");
		exit(1);
	}
	int id, mq1_k, mq3_k;
	int mq1, mq3;
	id = atoi(argv[1]);
	mq1 = atoi(argv[2]);
	mq3 = atoi(argv[3]);
	no_of_pages = 0;
	getallpages(argv[4]);

	printf("Process : PID= %d\n", id);

	// message to scheduler
	mymsgbuf msg_send;
	msg_send.mtype = TOSCH;
	msg_send.id = id;
	message_to_scheduler(mq1, &msg_send);
	////

	mymsgbuf msg_recv;
	read_message_from_scheduler(mq1, FROMSCH + id, &msg_recv);

	mmumsgbuf_send mmu_send;
	mmumsgbuf_recv mmu_recv;
	int cpg = 0; // counter for page number array
	while (cpg < no_of_pages)
	{
		// sending msg to mmu the page number
		printf("Process : Sent request for %d page number\n", pg_no[cpg]);
		mmu_send.mtype = TOMMU;
		mmu_send.id = id;
		mmu_send.pageno = pg_no[cpg];
		send_message_mmu(mq3, &mmu_send);

		read_message_mmu(mq3, FROMMMU + id, &mmu_recv);
		if (mmu_recv.frameno >= 0)
		{
			printf("Process : Frame number from MMU received for process %d: %d\n", id, mmu_recv.frameno);
			cpg++;
		}
		else if (mmu_recv.frameno == -1) // here cpg will not be incremented
		{
			printf("Process : Page fault occured for process %d\n", id);
			// read_message(mq1, FROMSCH + id, &msg_recv);
		}
		else if (mmu_recv.frameno == -2)
		{
			printf("Process : Process Invalid page reference for process %d terminating ...\n", id);
			exit(1);
		}
	}
	printf("Process : Process %d Terminated successfully\n", id);
	mmu_send.pageno = -9;
	mmu_send.id = id;
	mmu_send.mtype = TOMMU;
	send_message_mmu(mq3, &mmu_send);

	exit(1);
	return 0;
}
