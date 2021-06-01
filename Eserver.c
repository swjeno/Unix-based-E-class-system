#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <fcntl.h>

#define BUF_SIZE 100
#define MAX_CLNT 256
#define NAME_SIZE 20

void* handle_clnt(void* arg);//������ ä�ü��� �ڵ鸵
void* send_msg(void* arg);//Ŭ���̾�Ʈ���� ä���� ������ ���
void send_msg_serv(char* msg, int len);//���� ä�� �������� ��ü Ŭ���̾�Ʈ���� ��ȭ���� �������ִ� �Լ�
void error_handling(char* msg);//���� �ڵ鸵
void Chat(void* arg);//���� Ŭ���̾�Ʈ �� ä�� ��� �Լ�
void* MainMenu(void* arg);///���� �޴� ���
void* Notice(void* arg);//��������
void* recv_msg(void* arg);//�޽��� ����
void* Recv_Qst(void* arg);//���� �ޱ�
void* Save_dat(void* arg);//���� ����
void* Sendtocl(void* arg);//���� ����

int clnt_cnt = 0;
int clnt_socks[MAX_CLNT];
pthread_mutex_t mutx;
char name[NAME_SIZE] = "[Professor]";


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
void* handle_clnt(void* arg)//������ ���� ��ü Ŭ���̾�Ʈ���� �޽��� ����
{
	int clnt_sock = *((int*)arg);
	int str_len = 0, i;
	char msg[BUF_SIZE];

	while ((str_len = read(clnt_sock, msg, sizeof(msg))) != 0)
		send_msg_serv(msg, str_len);

	pthread_mutex_lock(&mutx);
	for (i = 0; i < clnt_cnt; i++)   // remove disconnected client
	{
		if (clnt_sock == clnt_socks[i])
		{
			while (i++ < clnt_cnt - 1)
				clnt_socks[i] = clnt_socks[i + 1];
			break;
		}
	}
	clnt_cnt--;
	pthread_mutex_unlock(&mutx);
	close(clnt_sock);
	return NULL;
}
void Chat(void* arg) {//ä�ù�, ����(����)��
	int sock = *((int*)arg);
	void* thread_return;
	printf("ä�ù濡�� �������� Q, Ȥ�� q�� �Է����ּ���\n");
	pthread_t snd_thread, rcv_thread;
	pthread_create(&snd_thread, NULL, send_msg, (void*)&sock);
	pthread_create(&rcv_thread, NULL, recv_msg, (void*)&sock);
	pthread_join(snd_thread, &thread_return);
	pthread_join(rcv_thread, &thread_return);
}
void send_msg_serv(char* msg, int len)   // send to all
{
	int i;
	pthread_mutex_lock(&mutx);
	for (i = 0; i < clnt_cnt; i++)
		write(clnt_socks[i], msg, len);
	pthread_mutex_unlock(&mutx);
}
void* send_msg(void* arg) {   // send thread main
	char msg[BUF_SIZE];
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
	printf(" 1. �������� �Է�\n");
	printf(" 2. ���� �ޱ�\n");
	printf(" 3. ���� Ȯ�� �� ����\n");
	printf(" 4. ���� ������\n");
	printf(" 5. ä�ù� ����\n");
	printf(" 0. ����\n");
	printf("==============================\n");
	printf("�޴� ����(��ȣ�� �Է��Ͻÿ�) : ");
	fgets(select, sizeof(int), stdin);
	if (!strcmp(select, "1\n")) {
		Notice(arg); //�������� �Է�
	}
	else if (!strcmp(select, "2\n")) {
		Recv_Qst(arg);
	}
	else if (!strcmp(select, "3\n")) {
		Save_dat(arg);
	}
	else if (!strcmp(select, "4\n")) {
		Sendtocl(arg);
	}
	else if (!strcmp(select, "5\n")) {
		Chat(arg);//ä�ù� ����
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
void* Notice(void* arg) {//���������� �Է��ϴ� �Լ�
	char notice[BUF_SIZE];
	int sock = *((int*)arg);
	char name_msg[NAME_SIZE + BUF_SIZE];
	printf("���������� �Է����ּ��� : ");
	fgets(notice, BUF_SIZE, stdin);
	if (!strcmp(notice, "q\n") || !strcmp(notice, "Q\n"))
	{
		MainMenu(arg);
		exit(0);
	}
	sprintf(name_msg, "%s %s %s", name, "�������� : ", notice);
	write(sock, name_msg, strlen(name_msg));
	MainMenu(arg);
	return NULL;
}

void* Recv_Qst(void* arg) {
	char quit[BUF_SIZE];
	int clnt_sock = *((int*)arg);
	int fd1, fd2, fd3;
	int recv1, recv2, recv3;
	char Quest[BUF_SIZE];  //������ ������ ����
	char Asw1[BUF_SIZE], Asw2[BUF_SIZE];  //����� �ؼ��� ������ ����
	fd1 = open("Question.txt", O_CREAT | O_WRONLY | O_TRUNC);
	fd2 = open("Answer1.txt", O_CREAT | O_WRONLY | O_TRUNC);
	fd3 = open("Answer2.txt", O_CREAT | O_WRONLY | O_TRUNC);
	if (fd1 == -1)
		error_handling("open(1) error!");
	if (fd2 == -1)
		error_handling("open(2) error!");
	if (fd3 == -1)
		error_handling("open(3) error!");
	recv1 = read(clnt_sock, Quest, sizeof(Quest));
	if (recv1 == -1) {
		error_handling("read() error!");
		return NULL;
	}
	recv2 = read(clnt_sock, Asw1, sizeof(Asw1));
	if (recv2 == -1) {
		error_handling("read() error!");
		return NULL;
	}
	recv3 = read(clnt_sock, Asw2, sizeof(Asw2));
	if (recv3 == -1) {
		error_handling("read() error!");
		return NULL;
	}
	if (write(fd1, Quest, sizeof(Quest)) == -1) {
		error_handling("������ �Էµ��� �ʾҽ��ϴ�.\n");
	}
	if (write(fd2, Asw1, sizeof(Asw1)) == -1) {
		error_handling("������ �Էµ��� �ʾҽ��ϴ�.\n");
	}
	if (write(fd3, Asw1, sizeof(Asw2)) == -1) {
		error_handling("�ؼ��� �Էµ��� �ʾҽ��ϴ�.\n");
	}
	printf("�ԷµǾ����ϴ�. ���� ȭ������ �������� Q Ȥ�� q �� �Է��ϼ��� : ");
	fgets(quit, BUF_SIZE, stdin);
	if (!strcmp(quit, "q\n") || !strcmp(quit, "Q\n"))
	{
		close(fd1);
		close(fd2);
		close(fd3);
		MainMenu(arg);
		exit(0);
	}
	return NULL;
}

void* Save_dat(void* arg) { //�ҷ��� ������ ����
	int clnt_sock = *((int*)arg);
	int num = 1, all_cnt = 0, cor_cnt = 0;
	char all[BUF_SIZE] , cor[BUF_SIZE];
	char Qst_name[BUF_SIZE], Ans1_name[BUF_SIZE], Ans2_name[BUF_SIZE], All_name[BUF_SIZE], Cor_name[BUF_SIZE];
	int fd1, fd2, fd3;
	int fs1, fs2, fs3, fs4, fs5;
	char Qst[BUF_SIZE], Ans1[BUF_SIZE], Ans2[BUF_SIZE];
	char YorN[10];

	fd1 = open("Question_tmp.txt", O_RDONLY);
	fd2 = open("Answer1_tmp.txt", O_RDONLY);
	fd3 = open("Answer2_tmp.txt", O_RDONLY);
	if ((fd1 == -1) || (fd2 == -1) || (fd3 == -1))
		error_handling("open() error!");
	printf("����/����/�ؼ�\n");

	if (read(fd1, Qst, sizeof(Qst)) == -1) //���� ���
		error_handling("������ �о���� ���߽��ϴ�.");
	printf("���� : %s\n", Qst);

	if (read(fd2, Ans1, sizeof(Ans1)) == -1) //���� ���
		error_handling("������ �о���� ���߽��ϴ�.");
	printf("���� : %s\n", Ans1);

	if (read(fd3, Ans2, sizeof(Ans2)) == -1) //�ؼ� ���
		error_handling("�ؼ��� �о���� ���߽��ϴ�.");
	printf("�ؼ� : %s\n", Ans2);

	// ������ �о�� ������ �������� �Ǵ� 
	printf("������ �����Ͻðڽ��ϱ�?   YES / NO");
	fgets(YorN, BUF_SIZE, stdin);

	if (!strcmp(YorN, "YES\n"))
	{
		close(fd1);
		close(fd2);
		close(fd3);
		open("Question_tmp.txt", O_TRUNC);
		open("Answer1_tmp.txt", O_TRUNC);
		open("Answer2_tmp.txt", O_TRUNC);
		close(fd1);
		close(fd2);
		close(fd3); // ����Ǿ� �ִ� ���� ���� ���� ��ü ����
	}
	else {
		printf("���ϸ� : Question%d.txt�� �Է��ϼ���\n", num);
		fgets(Qst_name, sizeof(BUF_SIZE), stdin);
		printf("���ϸ� : Answer1_%d.txt�� �Է��ϼ���\n", num);
		fgets(Ans1_name, sizeof(BUF_SIZE), stdin);
		printf("���ϸ� : Answer2_%d.txt�� �Է��ϼ���\n", num);
		fgets(Ans2_name, sizeof(BUF_SIZE), stdin); //���ϸ� �޾ƿ���
		printf("���ϸ� : All%d.txt�� �Է��ϼ���\n", num);
		fgets(All_name, sizeof(BUF_SIZE), stdin);//��ü Ǭ Ƚ�� �ʱ� ���� 0
		printf("���ϸ� : Cor%d.txt�� �Է��ϼ���\n", num);
		fgets(Cor_name, sizeof(BUF_SIZE), stdin); // ��ü ���� �� �ʱ� ���� 0 
		num++;
		fs1 = open(Qst_name, O_CREAT | O_WRONLY | O_TRUNC);
		fs2 = open(Ans1_name, O_CREAT | O_WRONLY | O_TRUNC);
		fs3 = open(Ans2_name, O_CREAT | O_WRONLY | O_TRUNC);
		fs4 = open(All_name, O_CREAT | O_WRONLY | O_TRUNC);
		fs5 = open(Cor_name, O_CREAT | O_WRONLY | O_TRUNC); //txt ���� ���� �� ������ �Է�
		if ((fs1 == -1) || (fs2 == -1) || (fs3 == -1) || (fs4 == -1) || (fs5 == -1))
			error_handling("open() error!");
		write(fs1, Qst, sizeof(Qst));
		write(fs2, Ans1, sizeof(Ans1));
		write(fs3, Ans2, sizeof(Ans2));
		sprintf(all, "%d", all_cnt);
		sprintf(cor, "%d", cor_cnt);
		write(fs4, all, sizeof(all));
		write(fs5, cor, sizeof(cor)); // ���Ͽ� ������ �Է�
		printf("������ �Ϸ�Ǿ����ϴ�.");
	}

	close(fs1);
	close(fs2);
	close(fs3);
	close(fs4);
	close(fs5); // ���� �ݱ�
	MainMenu(arg);
	return NULL;
}

void* Sendtocl(void* arg) {
	int clnt_sock = *((int*)arg);
	int num = 1;
	char Question[BUF_SIZE], Answer1[BUF_SIZE], Answer2[BUF_SIZE], All[BUF_SIZE], Cor[BUF_SIZE];
	char Qst[BUF_SIZE], Ans1[BUF_SIZE], Ans2[BUF_SIZE], Al[BUF_SIZE], Cr[BUF_SIZE];
	char re_Al[BUF_SIZE], re_Cr[BUF_SIZE], re_Qst[BUF_SIZE], check[BUF_SIZE];
	char Consist, Quit_msg[BUF_SIZE];
	int ft1, ft2, ft3, ft4, ft5;
	int fd1, fd2, fd3, fd4, fd5;
	int fa1, fa2, fa3, fa4, fa5;
	int fb1, fb2, fb3, fb4, fb5;
	int fc1, fc2;
	int fe1, fe2;
	int fk1, fk2;

	int fp;

	while (1) {
		printf("���ϸ� : Question%d.txt�� �Է��ϼ���.\n", num);
		fgets(Question, BUF_SIZE, stdin);
		printf("���ϸ� : Answer1_%d.txt�� �Է��ϼ���.\n", num);
		fgets(Answer1, BUF_SIZE, stdin);
		printf("���ϸ� : Answer2_%d.txt�� �Է��ϼ���.\n", num);
		fgets(Answer2, BUF_SIZE, stdin);
		printf("���ϸ� : All%d.txt�� �Է��ϼ���.\n", num);
		fgets(All, BUF_SIZE, stdin);
		printf("���ϸ� : Cor%d.txt�� �Է��ϼ���.\n", num);
		fgets(Cor, BUF_SIZE, stdin);


		ft1 = open(Question, O_RDONLY);
		ft2 = open(Answer1, O_RDONLY);
		ft3 = open(Answer2, O_RDONLY);
		ft4 = open(All, O_RDONLY);
		ft5 = open(Cor, O_RDONLY);
		if ((ft1 == -1) || (ft2 == -1) || (ft3 == -1) || (ft4 == -1) || (ft5 == -1))
			error_handling("open() error!");

		close(ft4);
		close(ft5);

		fa1 = read(ft1, Qst, sizeof(Qst));
		fa2 = read(ft2, Ans1, sizeof(Ans1));
		fa3 = read(ft3, Ans2, sizeof(Ans2));
		fa4 = read(ft4, Al, sizeof(Al));
		fa5 = read(ft5, Cr, sizeof(Cr)); // ���� ���� ���ۿ� ����
		if ((fa1 == -1) || (fa2 == -1) || (fa3 == -1) || (fa4 == -1) || (fa5 == -1))
			error_handling("read() error!");

		fb1 = write(clnt_sock, Qst, sizeof(Qst));
		fb2 = write(clnt_sock, Ans1, sizeof(Ans1));
		fb3 = write(clnt_sock, Ans2, sizeof(Ans2));
		fb4 = write(clnt_sock, Al, sizeof(Qst));
		fb5 = write(clnt_sock, Cr, sizeof(Cr));
		if ((fb1 == -1) || (fb2 == -1) || (fb3 == -1) || (fb4 == -1) || (fb5 == -1))
			error_handling("write() error!");

		fc1 = read(clnt_sock, re_Al, sizeof(re_Al));
		fc2 = read(clnt_sock, re_Cr, sizeof(re_Cr));
		if ((fc1 == -1) || (fc2 == -1))
			error_handling("read() error!");

		fk1 = open(All, O_CREAT | O_WRONLY | O_TRUNC);
		fk2 = open(Cor, O_CREAT | O_WRONLY | O_TRUNC);
		if ((fk1 == -1) || (fk2 == -1))
			error_handling("open() error!");

		fe1 = write(fk1, re_Al, sizeof(re_Al));
		fe2 = write(fk2, re_Cr, sizeof(re_Cr));
		if ((fe1 == -1) || (fe2 == -1))
			error_handling("write() error!");

		if (read(clnt_sock, check, sizeof(check)) == -1)
			error_handling("read() error!");

		if (!strcmp(check, "����"))
			break;
		else
			return NULL;

		num++;
		printf("���� ������ ���� ������ Ȯ���ϱ� ���� Question%d.txt�� �Է��ϼ���.", num);
		fgets(re_Qst, BUF_SIZE, stdin);
		fp = open(re_Qst, O_RDONLY);
		if (fp == -1) {
			sprintf(Quit_msg, "%s", "���� ������ ���� �����մϴ�.\n");
			write(clnt_sock, Quit_msg, sizeof(Quit_msg));
			return NULL;
		}

	}
}

int main(int argc, char* argv[])
{
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;
	int clnt_adr_sz;
	pthread_t t_id;
	pthread_t menu;
	if (argc != 2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

	pthread_mutex_init(&mutx, NULL);
	serv_sock = socket(PF_INET, SOCK_STREAM, 0);

	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_adr.sin_port = htons(atoi(argv[1]));

	if (bind(serv_sock, (struct sockaddr*) & serv_adr, sizeof(serv_adr)) == -1)
		error_handling("bind() error");
	if (listen(serv_sock, 5) == -1)
		error_handling("listen() error");

	while (1)
	{
		clnt_adr_sz = sizeof(clnt_adr);
		clnt_sock = accept(serv_sock, (struct sockaddr*) & clnt_adr, &clnt_adr_sz);

		pthread_mutex_lock(&mutx);
		clnt_socks[clnt_cnt++] = clnt_sock;
		pthread_mutex_unlock(&mutx);
		printf("Connected client IP: %s \n", inet_ntoa(clnt_adr.sin_addr));
		pthread_create(&t_id, NULL, handle_clnt, (void*)&clnt_sock);
		pthread_create(&menu, NULL, MainMenu, (void*)&clnt_sock);
		pthread_detach(t_id);
		pthread_detach(menu);
	}
	close(serv_sock);
	return 0;
}