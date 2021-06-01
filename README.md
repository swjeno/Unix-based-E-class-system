# Unix-based-E-class-system
Network program class task

PM, Lead Programmer 임무 수행
PM : 프로젝트 구상, 계획, 일정관리
Lead Programmer : 교재(윤성우의 열혈 TCP/IP 소켓 프로그래밍)코드 변형 및 새 함수 제작 
  eserver.c - handle_clnt(), send_msg(), send_msg_serv(), error_handling(), Chat(), MainMenu(), Notice(), recv_msg()
  eclient.c - send_msg(), recv_msg(), error_handling(), Chat(), MainMenu() 함수 제작 및 교재 코드 변경 
  makefile - 전체
  
makefile 없이 개별 컴파일 방법
gcc -o eserver eserver.c -lpthread
gcc -o eclient eclient.c -lpthread

개발기간 2021.04.22 ~ 2021.06.01
