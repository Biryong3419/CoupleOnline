#include <Turboc.h>//stdio, stdlib, conio, time, windows ������ �������� ������ϰ� ��ɵ��� �����س� ��������̴�.
#include <process.h>
#include <string.h>
#include <stdio.h>

#pragma comment(lib, "winmm.lib")         //BGM�� ���� ��ó����
                                 
#define LEFT 75 
#define RIGHT 77
#define UP 72
#define DOWN 80
#define ESC 27
#define BUF_SIZE 100
#define MAX_CLNT 256
#define NAME_SIZE 20

unsigned WINAPI HandleClient(void* arg);//������ �Լ�
unsigned WINAPI SendMsg(void* arg);//������ �����Լ�
unsigned WINAPI RecvMsg(void* arg);//������ �����Լ�
void ErrorHandling(char* msg);
void SendMsg(char* msg, int len);//�޽��� ������ �Լ�

int clientCount = 0;
SOCKET clientSocks[MAX_CLNT];//Ŭ���̾�Ʈ ���� ������ �迭
HANDLE hMutex;//���ؽ�
char name[NAME_SIZE] = "[DEFAULT]";
char msg[BUF_SIZE];

int difficulty = 4; // ���� ���̵� ��������
enum Status { HIDDEN, FLIP, TEMPFLIP };//ī�� ����������, ������������, �Ͻ������� �����������¸� �����Ѵ�.
struct tag_Cell //�� ĭ������ �����͸� �����ϴ� ����ü
{
	int Num;
	Status St;
};
struct userdb //���������� �����ϴ� ����ü(���߿��Ḯ��Ʈ)
{
	char dbusername[20];
	int dbscore;
	struct userdb* next;
	struct userdb* prev;

};
userdb *head; //�������� head(ó��)
userdb *tail; //�������� tail���(������)
tag_Cell arCell[6][6];//�Լ� �ִ�迭���� �����Ѵ�
					  //--------------��������
int sound = 1;               //���α׷� ������ ���� BGMƲ������
int startflag = 1;            //gameStart�Լ��� �ѹ��� �����ϱ� ����
int nx, ny;//xy��
int count;//ī���ÿ�
int score;//���� ��Ͽ� ����
int highscore;//�ְ��� ���������Q����
char username[20];//�����̸� �Ͻ�����
int gamecount = 0;//���� ���ھ� �Ͻ�����
int firstdatainsert = 0;//������ �� �������ִ��� �ƴϸ� ���� ó�� �����ϴ���.
int userscore;
int overlap; //db�� ����ִ� ���������ƴ���
			 //--------------�Լ�����
void printrank();//��ŷ����Ʈ ���(������)
void init();//���߿��Ḯ��Ʈ ��� �����Ҵ� �� �ʱ�ȭ(������)
int searchdb(char *username);//db�����Ϳ��� �ߺ��Ǵ� �г������ִ��� ã��(������)
void InitGame();//���ӽ���(����)
void dbsave();//���Ͽ� ������ ���(������);
void dbload();//�������κ��� ������ �ε�(������);
int inserttodb(char *username, int userscore);//���߿��Ḯ��Ʈ�� �����ͻ����Լ�(������)
void menu();//�޴� ����Լ�(������)
void DrawScreen(BOOL bHint);//ȭ��׸����Լ�(����)
void GetTempFlip(int *tx, int *ty);//��������ī���� ��ǥ�� �ݳ�(����)
int GetRemain();//���� ī��� ���ϱ�(����)
void introducinggame();//���ӼҰ�(������);
//void gameStart(); //���ӽ��۽� ���÷��� �κ�(���)

void main()
{
	WSADATA wsaData;
	SOCKET serverSock, clientSock;
	SOCKADDR_IN serverAddr, clientAddr;
	int clientAddrSize;
	HANDLE hThread;
	HANDLE hThread;
	HANDLE sendThread, recvThread;

	char myIp[100];
	char port[100];
	char inputName[100];
	char port[100];//��Ʈ ����
	init(); //�����Ҵ��ʱ�ȭ
	int ch;//���ӽ��۽� �Է¹޴� ����   
	int tx, ty;//
	int SwitchForRank;//��ŷ ����ġ���� ���� ����
	int whileflag = 1; // ��ü���� while���� ���� �÷��װ�
	char y_1;//y�� �Է¹ޱ� ���� ����
	char y_2;//y�� �Է¹ޱ� ���� ����2
	int gameflag = 1;//���ӽ��� �ݺ�while���� ���� �÷��װ�
	int setflag; // ���� �޴� case���� ���� �÷��װ�
	int forwhileflag = 1; //���� case���� ���� �÷��װ�
	int userwhileflag = 1;//user������ ������ while���� ���� �÷��װ�
	
	dbload();//db�ε�

	//gameStart(); //BGM����� ó��ȭ���� �ٹ� �Լ�

	while (whileflag) {   //1�� while��
		menu();//�ʱ� �޴� ���
		scanf("%d", &setflag);	
	
		switch (setflag) {//�޴� switch ��
		case 1:       //���� ����                         //ȣ��Ʈ ���Ӹ��
			printf("Input port number : "); //��Ʈ�� �Է¹���
			scanf("%s", port);
			if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) //������ ������ ����ϰڴٴ� ����� �ü���� ����
				ErrorHandling("WSAStartup() error!");

			hMutex = CreateMutex(NULL, FALSE, NULL);//�ϳ��� ���ؽ��� �����Ѵ�.
			serverSock = socket(PF_INET, SOCK_STREAM, 0); //�ϳ��� ������ �����Ѵ�.
			memset(&serverAddr, 0, sizeof(serverAddr));
			serverAddr.sin_family = AF_INET;
			serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
			serverAddr.sin_port = htons(atoi(port));
			if (bind(serverSock, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) //������ ������ ��ġ�Ѵ�.
				ErrorHandling("bind() error");
			if (listen(serverSock, 5) == SOCKET_ERROR)//������ �غ���¿� �д�.
				ErrorHandling("listen() error");
			clrscr(); //������ ���θ޴�ȭ�� ����� ��������
			while (1) {
				clientAddrSize = sizeof(clientAddr);
				clientSock = accept(serverSock, (SOCKADDR*)&clientAddr, &clientAddrSize);//�������� ���޵� Ŭ���̾�Ʈ ������ clientSock�� ����
				WaitForSingleObject(hMutex, INFINITE);//���ؽ� ����
				clientSocks[clientCount++] = clientSock;//Ŭ���̾�Ʈ ���Ϲ迭�� ��� ������ ���� �ּҸ� ����
				ReleaseMutex(hMutex);//���ؽ� ����
				hThread = (HANDLE)_beginthreadex(NULL, 0, HandleClient, (void*)&clientSock, 0, NULL);//HandleClient ������ ����, clientSock�� �Ű������� ����
				printf("Connected Client IP : %s\n", inet_ntoa(clientAddr.sin_addr));
			}
			userwhileflag = 1;
			gamecount = 0;
			overlap = 0;//�������� �ߺ����� ���� 0���� �ʱ�ȭ
			while (userwhileflag) {//�г��� �ߺ�Ȯ�� �� ���������� ���� �ݺ���
				printf("�г����� �Է��ϼ���\n");
				printf("����� �г����� : ");

				scanf("%s", username);//�����г��� �Է¹޴´�.
				if (searchdb(username) == 1) {
					clrscr();
					printf("%s �� ��ŷ�����Ϳ� �̹� �����ϴ� ������Դϴ�. ����Ͻ÷��� Y�� �Է����ּ��� : ", username);

					scanf(" %c", &y_1);

					if ((89 == y_1) || (121 == y_1)) {
						clrscr();
						userwhileflag = 0;
						overlap = 1;
					}
					else
						clrscr();
				}
				else {
					userwhileflag = 0;
					clrscr();
				}
			}
			randomize();
			InitGame();
			gameflag = 1;//������ �ѱ�� �����


			while (gameflag) {//2�� while��
				if (gamecount != 0)
					overlap = 1;
				gotoxy(nx * 8 + 3, ny * 3 + 2);
				ch = _getch();
				if (ch == 0xE0) {
					ch = _getch();
					switch (ch) {
					case LEFT:
						if (nx > 0) nx--;
						break;
					case RIGHT:
						if (nx < difficulty - 1) nx++;//
						break;
					case UP:
						if (ny > 0) ny--;
						break;
					case DOWN:
						if (ny < difficulty - 1) ny++;//
						break;
					}
				}
				else {
					switch (ch) {

					case ESC: // esc������ �� �ݺ��� ������Բ� 1��while������ ���ư��� ����ȭ�� �����
						clrscr();
						printf("������ ����˴ϴ�. ����Ͻ÷��� (Y/N) �����ּ��� : ");
						scanf(" %c", &y_2);

						if ((89 == y_2) || (121 == y_2)) {
							clrscr();
							gameflag = 0;

							break;
						}
						else
						{
							clrscr();
							DrawScreen(TRUE);

						}

						//exit(0);
						break;
					case ' ':
						if (arCell[nx][ny].St == HIDDEN) {
							GetTempFlip(&tx, &ty);
							if (tx == -1) {
								arCell[nx][ny].St = TEMPFLIP;
							}
							else {
								count++;
								if (arCell[tx][ty].Num == arCell[nx][ny].Num) {
									_putch('\a');
									arCell[tx][ty].St = FLIP;
									arCell[nx][ny].St = FLIP;
									if (GetRemain() == 0) {
										DrawScreen(FALSE);//drawscreen �Լ�ȣ��

										score = 100 - (count - ((difficulty*difficulty / 2) - (GetRemain() / 2))) * 2;//����

										highscore = inserttodb(username, score);//2�߿��Ḯ��Ʈ�� �����ͻ���

										if (score < 0) {//���ھ 0������ ���������ʵ��� ���ǹ����
											gotoxy(50, 12);   printf("����� ������ -> 0��<- �Դϴ�~~!");
										}
										else {
											gotoxy(50, 12); printf("����� ������      ->%d��<- �Դϴ�~~!", score);//���ھ����
											gotoxy(50, 14); printf("����� �ְ� ������ ->%d��<- �Դϴ�~~!", highscore);//�ְ����� ���
											delay(1000);
											gotoxy(50, 16); puts("�����մϴ�. �ٽ� �����մϴ�.");
										}delay(1000);
										dbsave();//��������
										gamecount++;


										InitGame();
									}
								}
								else {
									arCell[nx][ny].St = TEMPFLIP;
									DrawScreen(FALSE);
									delay(1000);
									arCell[tx][ty].St = HIDDEN;
									arCell[nx][ny].St = HIDDEN;
								}
							}
							DrawScreen(FALSE);
						}
						break;
					}
				}
			}
			closesocket(serverSock);//������ ������ ����.
			WSACleanup();//������ ������ �����ϰڴٴ� ����� �ü���� ����
		
			break;
		case 2:
			printf("Input server IP : ");
			gets(myIp);

			printf("Input server port : ");
			gets(port);

			printf("Input your name : ");
			gets(inputName);

			if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)// ������ ������ ����Ѵٰ� �ü���� �˸�
				ErrorHandling("WSAStartup() error!");

			sprintf(name, "[%s]", inputName);
			sock = socket(PF_INET, SOCK_STREAM, 0);//������ �ϳ� �����Ѵ�.

			memset(&serverAddr, 0, sizeof(serverAddr));
			serverAddr.sin_family = AF_INET;
			serverAddr.sin_addr.s_addr = inet_addr(myIp);
			serverAddr.sin_port = htons(atoi(port));

			if (connect(sock, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)//������ �����Ѵ�.
				ErrorHandling("connect() error");

			//���ӿ� �����ϸ� �� �� �Ʒ��� ����ȴ�.

			sendThread = (HANDLE)_beginthreadex(NULL, 0, SendMsg, (void*)&sock, 0, NULL);//�޽��� ���ۿ� �����尡 ����ȴ�.
			recvThread = (HANDLE)_beginthreadex(NULL, 0, RecvMsg, (void*)&sock, 0, NULL);//�޽��� ���ſ� �����尡 ����ȴ�.

			WaitForSingleObject(sendThread, INFINITE);//���ۿ� �����尡 �����ɶ����� ��ٸ���./
			WaitForSingleObject(recvThread, INFINITE);//���ſ� �����尡 �����ɶ����� ��ٸ���.
													  //Ŭ���̾�Ʈ�� ���Ḧ �õ��Ѵٸ� ���� �Ʒ��� ����ȴ�.
			closesocket(sock);//������ �����Ѵ�.
			WSACleanup();//������ ���� ��������� �ü���� �˸���.
				break;
			}
		case 3:
			clrscr();
			printf("1. ��ŷ����\n");//
			printf("2. ��ŷ�ʱ�ȭ\n");//
			scanf("%d", &SwitchForRank);
			switch (SwitchForRank) {
			case 1:
				printrank();
				break;
			case 2:
				head->next = tail; //����带 ���������� �̾������ν� �ʱ�ȭ
				dbsave();
				printf("�ʱ�ȭ �Ϸ�");
				delay(500);
				clrscr();
				break;
			}
			break;
		case 4:
			int temp;
			clrscr();
			printf("���̵��� �����մϴ�. (�⺻ ���̵��� 2=4x4)\n"); //���̵�����
			printf("���ϴ� ���̵��� ���ڸ� �Է����ּ���.(1=2x2, 2=4x4, 3=6x6)\n");//���̵� ����
			printf("----> ");

			while (forwhileflag) {//���̵������� ����ó��
				scanf("%d", &temp);
				if (temp >= 1 && temp <= 3) {
					difficulty = temp * 2;
					clrscr();
					break;
				}
				else {
					printf("�߸��Է��ϼ̽��ϴ�. 1~3������ ���ڸ� �Է����ּ���\n");
					printf("----> ");
				}

			}
			//���̵��� ���� �뷡���
			if (temp == 1)
				PlaySound(TEXT("Sin.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
			else if (temp == 2)
				PlaySound(TEXT("RockinNightStyle.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
			else
				PlaySound(TEXT("Nightmare.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
			break;
		case 5:
			whileflag = 0;//�ݺ��� ����
			break;
		}

	}

}

//�������� �ʱ�ȭ �ϴ� �Լ�
unsigned WINAPI HandleClient(void* arg) {
	SOCKET clientSock = *((SOCKET*)arg); //�Ű������ι��� Ŭ���̾�Ʈ ������ ����
	int strLen = 0, i;
	char msg[BUF_SIZE];

	while ((strLen = recv(clientSock, msg, sizeof(msg), 0)) != 0) { //Ŭ���̾�Ʈ�κ��� �޽����� ���������� ��ٸ���.
		if (!strcmp(msg, "q")) {
			send(clientSock, "q", 1, 0);
			break;
		}
		SendMsg(msg, strLen);//SendMsg�� ���� �޽����� �����Ѵ�.
	}

	printf("client left the chat\n");
	//�� ���� �����Ѵٴ� ���� �ش� Ŭ���̾�Ʈ�� �����ٴ� ����� ���� �ش� Ŭ���̾�Ʈ�� �迭���� �����������
	WaitForSingleObject(hMutex, INFINITE);//���ؽ� ����
	for (i = 0; i<clientCount; i++) {//�迭�� ������ŭ
		if (clientSock == clientSocks[i]) {//���� ���� clientSock���� �迭�� ���� ���ٸ�
			while (i++<clientCount - 1)//Ŭ���̾�Ʈ ���� ��ŭ
				clientSocks[i] = clientSocks[i + 1];//������ �����.
			break;
		}
	}
	clientCount--;//Ŭ���̾�Ʈ ���� �ϳ� ����
	ReleaseMutex(hMutex);//���ؽ� ����
	closesocket(clientSock);//������ �����Ѵ�.
	return 0;
}
unsigned WINAPI SendMsg(void* arg) {//���ۿ� �������Լ�
	SOCKET sock = *((SOCKET*)arg);//������ ������ �����Ѵ�.
	char nameMsg[NAME_SIZE + BUF_SIZE];
	while (1) {//�ݺ�
		fgets(msg, BUF_SIZE, stdin);//�Է��� �޴´�.
		if (!strcmp(msg, "q\n")) {//q�� �Է��ϸ� �����Ѵ�.
			send(sock, "q", 1, 0);//nameMsg�� �������� �����Ѵ�.
		}
		sprintf(nameMsg, "%s %s", name, msg);//nameMsg�� �޽����� �����Ѵ�.
		send(sock, nameMsg, strlen(nameMsg), 0);//nameMsg�� �������� �����Ѵ�.
	}
	return 0;
}

unsigned WINAPI RecvMsg(void* arg) {
	SOCKET sock = *((SOCKET*)arg);//������ ������ �����Ѵ�.
	char nameMsg[NAME_SIZE + BUF_SIZE];
	int strLen;
	while (1) {//�ݺ�
		strLen = recv(sock, nameMsg, NAME_SIZE + BUF_SIZE - 1, 0);//�����κ��� �޽����� �����Ѵ�.
		if (strLen == -1)
			return -1;
		nameMsg[strLen] = 0;//���ڿ��� ���� �˸��� ���� ����
		if (!strcmp(nameMsg, "q")) {
			printf("left the chat\n");
			closesocket(sock);
			exit(0);
		}
		fputs(nameMsg, stdout);//�ڽ��� �ֿܼ� ���� �޽����� ����Ѵ�.
	}
	return 0;
}

void SendMsg(char* msg, int len) { //�޽����� ��� Ŭ���̾�Ʈ���� ������.
	int i;
	WaitForSingleObject(hMutex, INFINITE);//���ؽ� ����
	for (i = 0; i<clientCount; i++)//Ŭ���̾�Ʈ ������ŭ
		send(clientSocks[i], msg, len, 0);//Ŭ���̾�Ʈ�鿡�� �޽����� �����Ѵ�.
	ReleaseMutex(hMutex);//���ؽ� ����
}
void ErrorHandling(char* msg) {
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}
void InitGame()
{
	int i, j;
	int x, y;
	nx = ny = 0;
	count = 0;

	memset(arCell, 0, sizeof(arCell));
	for (i = 1; i <= difficulty*difficulty / 2; i++) {
		for (j = 0; j<2; j++) {
			do {
				x = random(difficulty);//
				y = random(difficulty);//
			} while (arCell[x][y].Num != 0);
			arCell[x][y].Num = i;
		}
	}

	DrawScreen(TRUE);
	delay(2000);
	clrscr();
	DrawScreen(FALSE);
}

//ȭ���� �׸��� �Լ�
void DrawScreen(BOOL bHint)
{

	int x, y;
	int s, g;

	for (y = 0; y<difficulty; y++) {//
		for (x = 0; x<difficulty; x++) {//
			gotoxy(x * 8 + 2, y * 3 + 2);
			if (bHint == TRUE || arCell[x][y].St == FLIP) {
				gotoxy(wherex() - 1, wherey());
				s = x * 8 + 2;
				g = y * 3 + 2;
				gotoxy(s - 2, g - 1);
				printf("��������");
				gotoxy(s - 2, g);
				printf("��%2d  ��", arCell[x][y].Num);
				gotoxy(s - 2, g + 1);
				printf("��������");
			}
			else if (arCell[x][y].St == TEMPFLIP) {
				s = x * 8 + 2;
				g = y * 3 + 2;
				gotoxy(s - 2, g - 1);
				printf("��    ��");
				gotoxy(s - 2, g);
				printf("  %2d   ", arCell[x][y].Num);
				gotoxy(s - 2, g + 1);
				printf("��    ��");
			}
			else {
				s = x * 8 + 2;
				g = y * 3 + 2;
				gotoxy(s - 2, g - 1);
				printf("��������");
				gotoxy(s - 2, g);
				printf("�� �� ��");
				gotoxy(s - 2, g + 1);
				printf("��������");
			}
		}
	}

	gotoxy(50, 2); printf("����� �̸� : %s", username);
	gotoxy(50, 4); puts("Ŀ��Ű:�̵�. ����:������. Esc:����");
	gotoxy(50, 6); printf("�� �õ� ȸ�� : %d", count);
	gotoxy(50, 8); printf("���� �� ã�� �� : %d ", GetRemain());
	gotoxy(50, 10); printf("Ʋ�� Ƚ�� : %d ", count - ((difficulty*difficulty / 2) - (GetRemain() / 2)));

}

//�ӽ÷� �������� ĭ�� ��ȣ�� ����
void GetTempFlip(int *tx, int *ty)
{
	int i, j;
	for (i = 0; i<difficulty; i++) {
		for (j = 0; j<difficulty; j++) {
			if (arCell[i][j].St == TEMPFLIP) {
				*tx = i;
				*ty = j;
				return;
			}
		}
	}
	*tx = -1;
}

//���� ã������ ĭ�� ������ ����
int GetRemain()
{
	int i, j;
	int remain = difficulty*difficulty;//


	for (i = 0; i<difficulty; i++) {//
		for (j = 0; j<difficulty; j++) {//
			if (arCell[i][j].St == FLIP) {
				remain--;
			}

		}
	}
	return remain;
}
int searchdb(char *username)//�����г��� �Է½� ���ؼ� �ߺ����θ� ����ڿ��� Ȯ�����ְ� ������ ��Ͽ� ��������Ʈ�Ѵ�.
{
	userdb *tmp;
	tmp = head->next;
	while (strcmp(tmp->dbusername, username) != 0 && tmp != tail) //
	{
		tmp = tmp->next;
	}
	if (tmp == tail) {
		return 0;
	}
	else {
		overlap = 1;//������ �ߺ����·� ����
		return 1;
	}

}
void printrank()
{
	clrscr();
	userdb *tmp = head->next;    // tmp�� �Ӹ��� ���� ��� �ּҰ��� ����
	int ranknum = 1;
	char error = '��';

	if (tmp->dbusername[0] == error&&gamecount == 0) {
		printf("������ �������� �ʽ��ϴ�.");
		if (_getch() != NULL) {
			clrscr();
		}
	}
	else
	{
		if (firstdatainsert == 1) {
			tmp = tmp->next;

		}
		while (tmp != tail)     // ����� �� ( �������� )
		{

			printf("���� : %d   �г��� : %10s   ���� : %10d\n", ranknum++, tmp->dbusername, tmp->dbscore); // tmp�� ����Ű�� ��� �����͸� ���
			tmp = tmp->next;    // tmp �������� �̵�
		}
	}

	if (_getch() != NULL) {
		clrscr();

	}



}
int inserttodb(char *username, int userscore) {
	userdb *tmp = head->next;//�������� �������� �Ӹ�����
	userdb *node = (userdb*)malloc(sizeof(userdb));//�� ��� �����Ҵ�
	strcpy(node->dbusername, username);//�� ��忡 �̸�����
	node->dbscore = userscore;//�� ��忡 ���� ����
	if (head->next == tail)//��尡 ������(�Ӹ� ������ ������)
	{

		//�Ӹ��� ���� ���̿� �� ��� ����
		head->next = node;
		node->prev = head;
		node->next = tail;
		tail->prev = node;
		return 0;
	}
	else
	{
		if (overlap == 0)
		{
			while (1)//���ѷ���
			{

				if (tmp->dbscore < node->dbscore)//������ ����� ������ �� �������� ũ��
				{

					node->next = tmp;       // �����ǳ�� �տ� ����� ����
					node->prev = tmp->prev;
					tmp->prev->next = node;
					tmp->prev = node;
					return userscore;
				}
				if (tmp == tail)        // �� ����� ������ ���� ũ��
				{

					node->next = tail;      // ���� �տ� �� ��� ����
					node->prev = tail->prev;
					tail->prev->next = node;
					tail->prev = node;
					return userscore;
				}
				tmp = tmp->next;        // ������ ��� ������ �̵�
			}
		}
		else
		{
			while (1)
			{

				if (!strcmp(tmp->dbusername, node->dbusername))  //���������� ������
				{
					if (tmp->dbscore < node->dbscore) //�������������� ���������� ��ũ��
					{
						tmp->dbscore = node->dbscore;//������
						return tmp->dbscore;//�����Ѵ�.
					}
					else
					{
						return tmp->dbscore;//������ ��������ũ�� ���������� ����
					}

				}
				tmp = tmp->next;
			}
		}
	}
}

void dbload()
{
	char tempname[20];
	int tempscore = 0;
	char error_1 = '��';

	FILE *fpin;//���������ͺ����Ǽ���

	fpin = fopen("userdb.txt", "r");//userdb.txt ������ �о���δ�

	while (!feof(fpin)) //������ �� �о���϶����� while���� ����
	{
		userdb *node = (userdb*)malloc(sizeof(userdb)); // �� ��� �Ҵ�

		fscanf(fpin, "%s %d", tempname, &tempscore);
		if (tempname[0] == error_1)
		{

			firstdatainsert = 1;
		}
		strcpy(node->dbusername, tempname);
		node->dbscore = tempscore;
		if (head->next == tail)     // ��尡 ������ ( �Ӹ������� ������ )
		{


			// �Ӹ��� ���� ���̿� �� ������
			head->next = node;
			node->prev = head;
			node->next = tail;
			tail->prev = node;

		}
		else         // �ٸ���尡 ������
		{

			tail->prev->next = node; // ���� �ٷ� ���� �� ��� �߰�
			node->prev = tail->prev;    // �� ��� ������ ������ �������� ( �������� ���� ���̿� �� ��� �� )
			node->next = tail;   // �� ��� ������ ����
			tail->prev = node;   // ���� ������ �� ���

		}
		//node = node->next;
	}

	fclose(fpin);//�������� file ��Ʈ���� ������
}
void dbsave()//����db�� �����͸� ���������� �����Ѵ�
{
	char error_1 = '��';

	userdb *tmp = head->next;//tmp�� �Ӹ��� ���� ��� �ּҰ��� ����

	FILE *fpout;//���������ͺ����Ǽ���
	fpout = fopen("userdb.txt", "w");//fpout ���Ϻ��� �ʱ�ȭ
	while (tmp != tail) {//����� ��(��������)

		if (tmp->dbusername[0] != error_1) {

			if (tmp->next != tail) {

				fprintf(fpout, "%s %d\n", tmp->dbusername, tmp->dbscore);//����db�� ����Ѵ�

			}
			else {

				fprintf(fpout, "%s %d", tmp->dbusername, tmp->dbscore);
			}
		}

		tmp = tmp->next; //tmp �������� �̵�
	}
	fclose(fpout);//�������� file��Ʈ���� ������

}
void init() {
	head = (userdb*)malloc(sizeof(userdb)); // �Ӹ������Ҵ�
	tail = (userdb*)malloc(sizeof(userdb)); // ���������Ҵ�
	head->next = tail;      // �Ӹ������� ���� ����
	head->prev = head;      // �Ӹ������� �ڱ��ڽ� ����Ŵ
	tail->prev = head;      // ���������� �Ӹ�
	tail->next = tail;      // ���������� �ڱ��ڽ�



}
void menu() {
	printf("��¦�� ��Ƽ�÷��̡�\n");//ǥ�ÿ�
	printf("1. �游���\n");//���ӽ���while������������ ǥ������
	printf("2. �����ϱ�\n");//Ŭ���̾�Ʈ
//	printf("2. ���ӼҰ�\n");//
	//printf("3. ��ŷ\n");//
//	printf("4. ���̵�����\n");
	printf("3. ����\n");
	printf("��ȣ�� �Է����ּ��� : ");
}
