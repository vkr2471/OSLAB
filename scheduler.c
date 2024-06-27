/*
21CS30058 Voddula Karthik Reddy
21CS10057 Sanskar Mittal
*/
#include "vm.h"

int send_message(int qid, struct mymsgbuf *qbuf)
{
	int result, length;

	length = sizeof(struct mymsgbuf) - sizeof(long);

	if ((result = msgsnd(qid, qbuf, length, 0)) == -1)
	{
		perror("Error in sending message");
		exit(1);
	}

	return (result);
}

int read_message(int qid, long type, struct mymsgbuf *qbuf)
{
	int rval, len;

	len = sizeof(struct mymsgbuf) - sizeof(long);

	if ((rval = msgrcv(qid, qbuf, len, type, 0)) == -1)
	{
		perror("Error in receiving message");
		exit(1);
	}

	return (rval);
}
int k;
int read_message_mmu(int qid, long type, mmutosch *qbuf)
{
	int result, length;

	/* The length is essentially the size of the structure minus sizeof(mtype) */
	length = sizeof(mmutosch) - sizeof(long);

	if ((result = msgrcv(qid, qbuf, length, type, 0)) == -1)
	{
		perror("Error in receiving message");
		exit(1);
	}

	return (result);
}
int main(int argc, char *argv[])
{
	int master_pid;

	k = atoi(argv[3]);
	master_pid = atoi(argv[4]);

	mymsgbuf msg_send, msg_recv;

	int mq1 = atoi(argv[1]);
	int mq2 = atoi(argv[2]);
	if (mq1 == -1)
	{
		perror("Message Queue1 creation failed");
		exit(1);
	}
	if (mq2 == -1)
	{
		perror("Message Queue2 creation failed");
		exit(1);
	}
	printf("Total no of process = %d\n", k);

	int terminated_process = 0;
	while (1)
	{
		read_message(mq1, FROMPROCESS, &msg_recv);
		int curr_id = msg_recv.id;

		msg_send.mtype = TOPROCESS + curr_id;
		send_message(mq1, &msg_send);
		mmutosch mmu_recv;
		read_message_mmu(mq2, 0, &mmu_recv);
		if (mmu_recv.mtype == PAGEFAULT_HANDLED)
		{
			msg_send.mtype = FROMPROCESS;
			msg_send.id = curr_id;
			send_message(mq1, &msg_send);
		}
		else if (mmu_recv.mtype == TERMINATED)
		{
			terminated_process++;
		}
		else
		{
			perror("Shceduler : Invalid Msg from mmu\n");
			exit(1);
		}
		if (terminated_process == k)
			break;
	}
	kill(master_pid, SIGUSR1);
	pause();
	printf("Scheduler:Scheduler exited\n");
	exit(1);
}