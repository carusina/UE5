#include "Common.h"

#define SERVERPORT 9000
#define BUFSIZE    512

// 윤년 여부를 판단하는 함수
int isLeapYear(int year) {
    if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))
        return 1;
    else
        return 0;
}

// 해당 월의 일수를 반환하는 함수
int daysInMonth(int year, int month) {
    int days[] = {31, 28 + isLeapYear(year), 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    return days[month - 1];
}

// 달력을 출력하는 함수
void Calendar(char buf[], char result[]) {
    int i, j, day;
    int startingDay;
    char cyear[5], cmonth[3], cday[5];

    strncpy(cyear, buf, 4);
    strncpy(cmonth, buf+5, 2);

    int year = atoi(cyear);
    int month = atoi(cmonth);

    // 해당 월의 1일이 무슨 요일인지 계산
    startingDay = 1;
    for (i = 1900; i < year; i++) {
        for (j = 1; j <= 12; j++) {
            startingDay = (startingDay + daysInMonth(i, j)) % 7;
        }
    }
    for (j = 1; j < month; j++) {
        startingDay = (startingDay + daysInMonth(year, j)) % 7;
    }

    // 달력 출력
    strcat(result, "SUN MON THU WED THU FRI SAT\n");
    for (i = 0; i < startingDay; i++) {
        strcat(result, "    ");
    }
    for (day = 1; day <= daysInMonth(year, month); day++) {
        sprintf(cday, "%3d", day); cday[3] = ' '; cday[4] = '\0';
        strcat(result, cday);
        
        startingDay++;
        if (startingDay % 7 == 0)
            strcat(result, " \n");
    }
    strcat(result, " \n");
}

int main(int argc, char *argv[])
{
	int retval;

	// 소켓 생성
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) err_quit("socket()");

	// bind()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(listen_sock, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit("listen()");

	// 데이터 통신에 사용할 변수
	SOCKET client_sock;
	struct sockaddr_in clientaddr;
	socklen_t addrlen;
	char buf[BUFSIZE + 1];

	while (1) {
		// accept()
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (struct sockaddr *)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET) {
			err_display("accept()");
			break;
		}

		// 접속한 클라이언트 정보 출력
		char addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
		printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
			addr, ntohs(clientaddr.sin_port));

		// 클라이언트와 데이터 통신
		while (1) {
			// 데이터 받기
			retval = recv(client_sock, buf, BUFSIZE, 0);
			if (retval == SOCKET_ERROR) {
				err_display("recv()");
				break;
			}
			else if (retval == 0)
				break;

			// 받은 데이터 출력
			buf[retval] = '\0';
			printf("[TCP/%s:%d] %s\n", addr, ntohs(clientaddr.sin_port), buf);

			//buf에 캘린더 저장
			char result[512];
			Calendar(buf, result);
	
			// 데이터 보내기
			retval = send(client_sock, result, retval, 0);
			if (retval == SOCKET_ERROR) {
				err_display("send()");
				break;
			}
		}

		// 소켓 닫기
		close(client_sock);
		printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n",
			addr, ntohs(clientaddr.sin_port));
	}

	// 소켓 닫기
	close(listen_sock);
	return 0;
}
