#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <fcntl.h>

#define BUF_SIZE 100
#define NAME_SIZE 20

void* send_msg(void* arg);
void* recv_msg(void* arg);
void error_handling(char* msg);
void Chat(void* arg);
void* MainMenu(void* arg);
void* Send_question(void* arg);
void* RecvfromServ(void* arg);

char name[NAME_SIZE] = "[DEFAULT]";
char msg[BUF_SIZE];

int main(int argc, char* argv[])
{
	int sock;
	struct sockaddr_in serv_addr;
	//pthread_t snd_thread, rcv_thread;
	pthread_t t_id;
	void* thread_return;
	if (argc != 4) {
		printf("Usage : %s <IP> <port> <name>\n", argv[0]);
		exit(1);
	}

	sprintf(name, "[%s]", argv[3]);
	sock = socket(PF_INET, SOCK_STREAM, 0);

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_addr.sin_port = htons(atoi(argv[2]));

	if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
		error_handling("connect() error");
	pthread_create(&t_id, NULL, MainMenu, (void*)&sock);
	pthread_join(t_id, &thread_return);
	//pthread_create(&snd_thread, NULL, send_msg, (void*)&sock);
	//pthread_create(&rcv_thread, NULL, recv_msg, (void*)&sock);
	//pthread_join(snd_thread, &thread_return);
	//pthread_join(rcv_thread, &thread_return);
	close(sock);
	return 0;
}

void* send_msg(void* arg)   // send thread main
{
	int sock = *((int*)arg);
	char name_msg[NAME_SIZE + BUF_SIZE];
	while (1)
	{
		fgets(msg, BUF_SIZE, stdin);
		if (!strcmp(msg, "q\n") || !strcmp(msg, "Q\n"))
		{
			MainMenu(arg);
		}
		sprintf(name_msg, "%s %s", name, msg);
		write(sock, name_msg, strlen(name_msg));
	}
	return NULL;
}

void* recv_msg(void* arg)   // read thread main
{
	int sock = *((int*)arg);
	char name_msg[NAME_SIZE + BUF_SIZE];
	int str_len;
	while (1)
	{
		str_len = read(sock, name_msg, NAME_SIZE + BUF_SIZE - 1);
		if (str_len == -1)
			return (void*)-1;
		name_msg[str_len] = 0;
		fputs(name_msg, stdout);
	}
	return NULL;
}

void error_handling(char* msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}

void* MainMenu(void* arg) {//���� ����� ���� ���� �������� ���� �޴� �Լ�
	char select[BUF_SIZE];
	printf("==============================\n");
	printf("        KPU e-class\n");
	printf("==============================\n");
	printf("          �޴� ����\n");
	printf(" 1. ���� ������\n");
	printf(" 2. ���� Ǯ��\n");
	printf(" 3. ä�ù� ����\n");
	printf(" 0. ����\n");
	printf("==============================\n");
	printf("�޴� ����(��ȣ�� �Է��Ͻÿ�) : ");
	fgets(select, sizeof(int), stdin);
	if (!strcmp(select, "1\n")) {
		Send_question(arg);
	}
	else if (!strcmp(select, "2\n")) {
		RecvfromServ(arg);
	}
	else if (!strcmp(select, "3\n")) {
		//ä�ù� ����
		Chat(arg);
	}
	else if (!strcmp(select, "0\n")) {
		printf("e-class�� �����մϴ�.\n");
		return 0;
	}
	else {
		printf("�߸� �Է��ϼ̽��ϴ�.\n");
		MainMenu(arg);
	}
}
void Chat(void* arg) {//ä�ù�, Ŭ���̾�Ʈ��
	int sock = *((int*)arg);
	void* thread_return;
	printf("ä�ù濡�� �������� Q, Ȥ�� q�� �Է����ּ���\n");
	pthread_t snd_thread, rcv_thread;
	pthread_create(&snd_thread, NULL, send_msg, (void*)&sock);
	pthread_create(&rcv_thread, NULL, recv_msg, (void*)&sock);
	pthread_join(snd_thread, &thread_return);
	pthread_join(rcv_thread, &thread_return);
}

void* Send_question(void* arg) {
	int sock = *((int*)arg);
	int fd1, fd2, fd3;
	int Send1, Send2, Send3;
	char Qst[BUF_SIZE], Ans1[BUF_SIZE], Ans2[BUF_SIZE];

	printf("������ �Է��ϼ��� : ");
	fgets(Qst, BUF_SIZE, stdin);
	printf("������ �Է��ϼ��� : ");
	fgets(Ans1, BUF_SIZE, stdin);
	printf("�ؼ��� �Է��ϼ��� : ");
	fgets(Ans2, BUF_SIZE, stdin);

	Send1 = write(sock, Qst, strlen(Qst));
	if (Send1 == -1) {
		error_handling("write() error!");
		return NULL;
	}
	Send2 = write(sock, Ans1, strlen(Qst));
	if (Send2 == -1) {
		error_handling("write() error!");
		return NULL;
	}
	Send3 = write(sock, Ans2, strlen(Qst));
	if (Send3 == -1) {
		error_handling("write() error!");
		return NULL;
	}
	printf("���� �Է��� �Ϸ�Ǿ����ϴ�.\n");
	MainMenu(arg);
	return NULL;
}

void* RecvfromServ(void* arg) {
	int sock = *((int*)arg);

	int fd1, fd2, fd3, fd4, fd5, fs1, fs2;
	int cnt;
	char req[BUF_SIZE], req2[BUF_SIZE];
	char all[BUF_SIZE], cor[BUF_SIZE];
	char Question[BUF_SIZE], Quit[BUF_SIZE], Sust[BUF_SIZE];
	char Answer1_1[BUF_SIZE], Answer1_2[BUF_SIZE];
	char Answer2_1[BUF_SIZE];
	char nextorend[BUF_SIZE];
	int all_cnt, cor_cnt;
	float Cor_per;


	printf("-----------���� Ǯ�� ����----------------");
	while (1) {
		fd1 = read(sock, Question, sizeof(Question)); //���� �޾ƿ���
		fd2 = read(sock, Answer1_1, sizeof(Answer1_1));	//���� �޾ƿ���
		fd3 = read(sock, Answer2_1, sizeof(Answer2_1)); //�ؼ� �޾ƿ���
		fd4 = read(sock, &all_cnt, sizeof(all)); //Ǭ Ƚ�� �޾ƿ���
		fd5 = read(sock, &cor_cnt, sizeof(cor)); // ���� Ƚ�� �޾ƿ���
		if ((fd1 == -1) || (fd2 == -1) || (fd3 == -1) || (fd4 == -1) || (fd5 == -1)) { //read() error ó��
			error_handling("read() error!");
			break;
		}
		cnt = 1;
		printf("%d�� ����. ", cnt);
		cnt++;
		printf("%s", Question); // ���� ���
		printf("\n");

		if (all_cnt == 0) { // �и� 0�� ������ ��� ó��
			all_cnt++;
			Cor_per = (double)(cor_cnt / all_cnt)*100;
			printf("���� ����� :  %3f%%\n", Cor_per);
			all_cnt--;
		}
		else {
			Cor_per = (double)(cor_cnt / all_cnt) * 100;
			printf("���� ����� :  %3f%%\n", Cor_per);
		}

		printf("���� : ");
		fgets(Answer1_2, BUF_SIZE, stdin); //��� �ۼ�
		if (!strcmp(Answer1_1, Answer1_2)) //������ ���
		{
			printf("�����Դϴ�.");
			all_cnt++; // ��ü Ǭ ���� 1 ����
			printf("���� : %s\n �ؼ� : %s", Answer1_1, Answer2_1);
		}
		else { //������ ���
			printf("�����Դϴ�.\n");
			all_cnt++;
			cor_cnt++; // ��ü Ǭ ���� ���� �� ��� 1�� ����
			printf("���� : %s\n �ؼ� : %s", Answer1_1, Answer2_1);
		}
		sprintf(all, "%d", all_cnt);
		sprintf(cor, "%d", cor_cnt);
		while (1) {
			printf("1. ���� ������\n2. ���� Ǯ�� ����\n");
			fgets(req2, BUF_SIZE, stdin);
			if (!strcmp(req2, "1\n")) {
				sprintf(Sust, "%s", "����");
				if (write(sock, Sust, sizeof(Sust)) == -1) {
					error_handling("write() error!");
				}
				break; // while�� ����
			}

			else if (!strcmp(req2, "2\n")) {
				sprintf(Quit, "%s", "����");
				fs1 = write(sock, all, sizeof(all));
				fs2 = write(sock, cor, sizeof(cor));
				if ((fs1 == -1) || (fs2 == -1))
					error_handling("write() error!");
				if (write(sock, Quit, sizeof(Quit)) == -1)
					error_handling("write() error!");
				MainMenu(arg);
				return NULL; //����
			}
			else
				printf("���Է� �ٶ��ϴ�.\n");

		}
		fs1 = write(sock, all, sizeof(all));
		fs2 = write(sock, cor, sizeof(cor));
		if ((fs1 == -1) || (fs2 == -1))
			error_handling("write() error!");

		if (read(sock, nextorend, sizeof(nextorend)) == -1)
			error_handling("read() error!");

		if (!strcmp(nextorend, "���� ������ ���� �����մϴ�."))
			return NULL;
		else
			break;
	}
}
