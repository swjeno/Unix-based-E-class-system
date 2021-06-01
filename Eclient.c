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

void* MainMenu(void* arg) {//서버 실행시 가장 먼저 보여지는 메인 메뉴 함수
	char select[BUF_SIZE];
	printf("==============================\n");
	printf("        KPU e-class\n");
	printf("==============================\n");
	printf("          메뉴 선택\n");
	printf(" 1. 문제 보내기\n");
	printf(" 2. 문제 풀기\n");
	printf(" 3. 채팅방 입장\n");
	printf(" 0. 종료\n");
	printf("==============================\n");
	printf("메뉴 선택(번호만 입력하시오) : ");
	fgets(select, sizeof(int), stdin);
	if (!strcmp(select, "1\n")) {
		Send_question(arg);
	}
	else if (!strcmp(select, "2\n")) {
		RecvfromServ(arg);
	}
	else if (!strcmp(select, "3\n")) {
		//채팅방 입장
		Chat(arg);
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
void Chat(void* arg) {//채팅방, 클라이언트측
	int sock = *((int*)arg);
	void* thread_return;
	printf("채팅방에서 나가려면 Q, 혹은 q를 입력해주세요\n");
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

	printf("문제를 입력하세요 : ");
	fgets(Qst, BUF_SIZE, stdin);
	printf("정답을 입력하세요 : ");
	fgets(Ans1, BUF_SIZE, stdin);
	printf("해설을 입력하세요 : ");
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
	printf("문제 입력이 완료되었습니다.\n");
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


	printf("-----------문제 풀이 시작----------------");
	while (1) {
		fd1 = read(sock, Question, sizeof(Question)); //문제 받아오기
		fd2 = read(sock, Answer1_1, sizeof(Answer1_1));	//정답 받아오기
		fd3 = read(sock, Answer2_1, sizeof(Answer2_1)); //해설 받아오기
		fd4 = read(sock, &all_cnt, sizeof(all)); //푼 횟수 받아오기
		fd5 = read(sock, &cor_cnt, sizeof(cor)); // 정답 횟수 받아오기
		if ((fd1 == -1) || (fd2 == -1) || (fd3 == -1) || (fd4 == -1) || (fd5 == -1)) { //read() error 처리
			error_handling("read() error!");
			break;
		}
		cnt = 1;
		printf("%d번 문제. ", cnt);
		cnt++;
		printf("%s", Question); // 문제 출력
		printf("\n");

		if (all_cnt == 0) { // 분모가 0이 나오는 경우 처리
			all_cnt++;
			Cor_per = (double)(cor_cnt / all_cnt)*100;
			printf("문제 정답률 :  %3f%%\n", Cor_per);
			all_cnt--;
		}
		else {
			Cor_per = (double)(cor_cnt / all_cnt) * 100;
			printf("문제 정답률 :  %3f%%\n", Cor_per);
		}

		printf("정답 : ");
		fgets(Answer1_2, BUF_SIZE, stdin); //답안 작성
		if (!strcmp(Answer1_1, Answer1_2)) //오답인 경우
		{
			printf("오답입니다.");
			all_cnt++; // 전체 푼 수만 1 증가
			printf("정답 : %s\n 해설 : %s", Answer1_1, Answer2_1);
		}
		else { //정답인 경우
			printf("정답입니다.\n");
			all_cnt++;
			cor_cnt++; // 전체 푼 수와 정답 수 모두 1씩 증가
			printf("정답 : %s\n 해설 : %s", Answer1_1, Answer2_1);
		}
		sprintf(all, "%d", all_cnt);
		sprintf(cor, "%d", cor_cnt);
		while (1) {
			printf("1. 다음 문제로\n2. 문제 풀이 종료\n");
			fgets(req2, BUF_SIZE, stdin);
			if (!strcmp(req2, "1\n")) {
				sprintf(Sust, "%s", "유지");
				if (write(sock, Sust, sizeof(Sust)) == -1) {
					error_handling("write() error!");
				}
				break; // while문 종료
			}

			else if (!strcmp(req2, "2\n")) {
				sprintf(Quit, "%s", "종료");
				fs1 = write(sock, all, sizeof(all));
				fs2 = write(sock, cor, sizeof(cor));
				if ((fs1 == -1) || (fs2 == -1))
					error_handling("write() error!");
				if (write(sock, Quit, sizeof(Quit)) == -1)
					error_handling("write() error!");
				MainMenu(arg);
				return NULL; //종료
			}
			else
				printf("재입력 바랍니다.\n");

		}
		fs1 = write(sock, all, sizeof(all));
		fs2 = write(sock, cor, sizeof(cor));
		if ((fs1 == -1) || (fs2 == -1))
			error_handling("write() error!");

		if (read(sock, nextorend, sizeof(nextorend)) == -1)
			error_handling("read() error!");

		if (!strcmp(nextorend, "남은 문제가 없어 종료합니다."))
			return NULL;
		else
			break;
	}
}
