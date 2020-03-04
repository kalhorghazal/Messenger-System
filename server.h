#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>

#define LOCAL_ADDRESS_IP "127.0.0.1"
#define MAX_PENDING 10
#define MAX_NUMBER_OF_GROUPS 10
#define MAX_NUMBER_OF_USERS 20
#define HEARTBEAT_TIMEOUT 30
#define LOGIN_COMMAND "login"
#define GET_GROUP_LIST_COMMAND "get_group_list"
#define JOIN_GROUP_COMMAND "join"
#define CREATE_GROUP_COMMAND "create_group"
#define START_SECRET_CHAT_COMMAND "start_secret_chat_with"
#define LEAVE_GROUP "leave_group"

#define LEAVE_SECRET_CHAT "leave_secret_chat"
#define BASE_PORT 5000
#define BASE_GROUP_PORT 5050
#define TRUE 1
#define FALSE 0
#define EQUAL 0
#define NUMBER_OF_ARGUMENTS 2
#define ONE 1
#define ZERO 0
#define MAXLINE 1024
#define EMPTY ""
#define UNSUCCESSFUL 0
#define SUCCESSFUL 1

//char* group_messages[MAX_NUMBER_OF_GROUPS];
char group_names[MAX_NUMBER_OF_GROUPS][MAXLINE];
int* group_sockets;
int* group_ports;

int* user_sockets;
int* user_ports ;
char users[MAX_NUMBER_OF_USERS][MAXLINE];
int server_port;

struct sockaddr_in get_broadcast_address(int port);
int get_udp_socket();
int get_tcp_socket();
void set_broadcast_options(int sockfd);
void bind_socket(int sockfd, struct sockaddr_in servaddr);
void set_timeout_option(int rcv_sock, int seconds);
void free_group(int group_index);
int remove_inactive_groups(int heartbeat_port);
int get_port(int sockfd, struct sockaddr_in sin);
void initialize_users();
void initialize_groups();
int handle_login_user(int sockfd, int user_index);
int handle_get_group_list(int sockfd);
int set_group_socket(int group_index);
int handle_create_group(int sockfd);
int handle_join_group(int sockfd);
int handle_start_secret_chat(int sockfd);
void handle_commands(int sockfd, int user_index, char* command);
int run_server(int server_port);
char* itoa(int val);


int max(int x, int y);
void print(char* buf);

#endif