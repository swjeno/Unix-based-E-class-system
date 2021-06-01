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

void* handle_clnt(void* arg);//기존의 채팅서버 핸들링
void* send_msg(void* arg);//클라이언트에게 채팅이 가능한 기능
void send_msg_serv(char* msg, int len);//기존 채팅 서버에서 전체 클라이언트에게 대화내용 전달해주는 함수
void error_handling(char* msg);//에러 핸들링
void Chat(void* arg);//기존 클라이언트 측 채팅 기능 함수
void* MainMenu(void* arg);///메인 메뉴 출력
void* Notice(void* arg);//공지사항
void* recv_msg(void* arg);//메시지 수신
void* Recv_Qst(void* arg);//문제 받기
void* Save_dat(void* arg);//문제 저장
void* Sendtocl(void* arg);//문제 전송

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
void* handle_clnt(void* arg)//쓰레드 통해 전체 클라이언트에게 메시지 전송
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
void Chat(void* arg) {//채팅방, 서버(교수)측
	int sock = *((int*)arg);
	void* thread_return;
	printf("채팅방에서 나가려면 Q, 혹은 q를 입력해주세요\n");
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
void* MainMenu(void* arg) {//서버 실행시 가장 먼저 보여지는 메인 메뉴 함수
	char select[BUF_SIZE];
	printf("==============================\n");
	printf("        KPU e-class\n");
	printf("==============================\n");
	printf("          메뉴 선택\n");
	printf(" 1. 공지사항 입력\n");
	printf(" 2. 문제 받기\n");
	printf(" 3. 문제 확인 및 저장\n");
	printf(" 4. 문제 보내기\n");
	printf(" 5. 채팅방 입장\n");
	printf(" 0. 종료\n");
	printf("==============================\n");
	printf("메뉴 선택(번호만 입력하시오) : ");
	fgets(select, sizeof(int), stdin);
	if (!strcmp(select, "1\n")) {
		Notice(arg); //공지사항 입력
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
		Chat(arg);//채팅방 실행
	}
	else if (!strcmp(select, "0\n")) {
		printf("e-class를 종료합니다.\n");
		return 0;
	}
	else {
		printf("잘못 입력하셨습니다.\n");
		MainMenu(arg);
	}
}
void* Notice(void* arg) {//공지사항을 입력하는 함수
	char notice[BUF_SIZE];
	int sock = *((int*)arg);
	char name_msg[NAME_SIZE + BUF_SIZE];
	printf("공지사항을 입력해주세요 : ");
	fgets(notice, BUF_SIZE, stdin);
	if (!strcmp(notice, "q\n") || !strcmp(notice, "Q\n"))
	{
		MainMenu(arg);
		exit(0);
	}
	sprintf(name_msg, "%s %s %s", name, "공지사항 : ", notice);
	write(sock, name_msg, strlen(name_msg));
	MainMenu(arg);
	return NULL;
}

void* Recv_Qst(void* arg) {
	char quit[BUF_SIZE];
	int clnt_sock = *((int*)arg);
	int fd1, fd2, fd3;
	int recv1, recv2, recv3;
	char Quest[BUF_SIZE];  //문제를 저장할 버퍼
	char Asw1[BUF_SIZE], Asw2[BUF_SIZE];  //정답과 해설을 저장할 버퍼
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
		error_handling("문제가 입력되지 않았습니다.\n");
	}
	if (write(fd2, Asw1, sizeof(Asw1)) == -1) {
		error_handling("정답이 입력되지 않았습니다.\n");
	}
	if (write(fd3, Asw1, sizeof(Asw2)) == -1) {
		error_handling("해설이 입력되지 않았습니다.\n");
	}
	printf("입력되었습니다. 메인 화면으로 나가려면 Q 혹은 q 를 입력하세요 : ");
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

void* Save_dat(void* arg) { //불러온 데이터 저장
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
	printf("문제/정답/해설\n");

	if (read(fd1, Qst, sizeof(Qst)) == -1) //문제 출력
		error_handling("문제를 읽어오지 못했습니다.");
	printf("문제 : %s\n", Qst);

	if (read(fd2, Ans1, sizeof(Ans1)) == -1) //정답 출력
		error_handling("정답을 읽어오지 못했습니다.");
	printf("정답 : %s\n", Ans1);

	if (read(fd3, Ans2, sizeof(Ans2)) == -1) //해설 출력
		error_handling("해설을 읽어오지 못했습니다.");
	printf("해설 : %s\n", Ans2);

	// 문제를 읽어보고 문제로 저장할지 판단 
	printf("문제를 저장하시겠습니까?   YES / NO");
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
		close(fd3); // 저장되어 있던 문제 관련 내용 전체 삭제
	}
	else {
		printf("파일명 : Question%d.txt를 입력하세요\n", num);
		fgets(Qst_name, sizeof(BUF_SIZE), stdin);
		printf("파일명 : Answer1_%d.txt를 입력하세요\n", num);
		fgets(Ans1_name, sizeof(BUF_SIZE), stdin);
		printf("파일명 : Answer2_%d.txt를 입력하세요\n", num);
		fgets(Ans2_name, sizeof(BUF_SIZE), stdin); //파일명 받아오기
		printf("파일명 : All%d.txt를 입력하세요\n", num);
		fgets(All_name, sizeof(BUF_SIZE), stdin);//전체 푼 횟수 초기 설정 0
		printf("파일명 : Cor%d.txt를 입력하세요\n", num);
		fgets(Cor_name, sizeof(BUF_SIZE), stdin); // 전체 정답 수 초기 설정 0 
		num++;
		fs1 = open(Qst_name, O_CREAT | O_WRONLY | O_TRUNC);
		fs2 = open(Ans1_name, O_CREAT | O_WRONLY | O_TRUNC);
		fs3 = open(Ans2_name, O_CREAT | O_WRONLY | O_TRUNC);
		fs4 = open(All_name, O_CREAT | O_WRONLY | O_TRUNC);
		fs5 = open(Cor_name, O_CREAT | O_WRONLY | O_TRUNC); //txt 파일 오픈 후 데이터 입력
		if ((fs1 == -1) || (fs2 == -1) || (fs3 == -1) || (fs4 == -1) || (fs5 == -1))
			error_handling("open() error!");
		write(fs1, Qst, sizeof(Qst));
		write(fs2, Ans1, sizeof(Ans1));
		write(fs3, Ans2, sizeof(Ans2));
		sprintf(all, "%d", all_cnt);
		sprintf(cor, "%d", cor_cnt);
		write(fs4, all, sizeof(all));
		write(fs5, cor, sizeof(cor)); // 파일에 데이터 입력
		printf("저장이 완료되었습니다.");
	}

	close(fs1);
	close(fs2);
	close(fs3);
	close(fs4);
	close(fs5); // 파일 닫기
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
		printf("파일명 : Question%d.txt를 입력하세요.\n", num);
		fgets(Question, BUF_SIZE, stdin);
		printf("파일명 : Answer1_%d.txt를 입력하세요.\n", num);
		fgets(Answer1, BUF_SIZE, stdin);
		printf("파일명 : Answer2_%d.txt를 입력하세요.\n", num);
		fgets(Answer2, BUF_SIZE, stdin);
		printf("파일명 : All%d.txt를 입력하세요.\n", num);
		fgets(All, BUF_SIZE, stdin);
		printf("파일명 : Cor%d.txt를 입력하세요.\n", num);
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
		fa5 = read(ft5, Cr, sizeof(Cr)); // 파일 내용 버퍼에 저장
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

		if (!strcmp(check, "유지"))
			break;
		else
			return NULL;

		num++;
		printf("다음 파일의 존재 유무를 확인하기 위해 Question%d.txt를 입력하세요.", num);
		fgets(re_Qst, BUF_SIZE, stdin);
		fp = open(re_Qst, O_RDONLY);
		if (fp == -1) {
			sprintf(Quit_msg, "%s", "남은 문제가 없어 종료합니다.\n");
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