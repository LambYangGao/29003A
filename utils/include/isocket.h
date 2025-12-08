
#ifndef _SOCK_H_JKL23895U23N524NM235R90124H1U238XHY901P4J3HY845_
#define _SOCK_H_JKL23895U23N524NM235R90124H1U238XHY901P4J3HY845_

#ifdef _WIN32
#include <WinSock2.h>
#pragma comment(lib,"Ws2_32.lib")
#include <Windows.h>
#else
#include <netdb.h>
#include <sys/time.h>
#include  <unistd.h>
#include <sys/ioctl.h>
//#include <stropts.h> 
#include  <sys/types.h>       /* basic system data types */
#include  <sys/socket.h>      /* basic socket definitions */
#include  <netinet/in.h>      /* sockaddr_in{} and other Internet defns */
#include  <arpa/inet.h>       /* inet(3) functions */
#include  <sys/select.h>       /* select function*/
//#include "Osal.h"
#define TCP_NODELAY         0x0001
#define SOCKET int
#define SOCKET_ERROR -1
#endif // #ifdef _WIN32

#ifdef _WIN32
typedef int socklen_t;
// #ifndef socklen_t
// #define socklen_t int
// #endif

#ifndef	ssize_t
#define ssize_t long
#endif

#ifndef pid_t
#define pid_t int
#endif


#ifndef SHUT_RD
#define SHUT_RD			SD_RECEIVE
#define SHUT_WR			SD_SEND
#define SHUT_RDWR		2
#endif

#endif // #ifdef _WIN32
#define SOCKET_NULL -1
#define MAX_REQUEST 255
#include <stdint.h>

class  iSocket
{
public:
	// 构造与解除
	iSocket();								// 创建一个空的iSocket对象
// 	bool create(int type = SOCK_STREAM, int protocol = 0, int domain = PF_INET);
// 											// 为空的对象创建socket描述符
	~iSocket();								// 关闭连接和套接字
	
	static SOCKET socket(int domain = AF_INET,int type = SOCK_STREAM, int protocol = 0);	
											// 先关闭原有的连接和套接字，然后创建新的socket
	static void close(SOCKET _fd);						// 关闭套接字
	static void shutdown(SOCKET _fd,int how = SHUT_RDWR);	// 关闭连接

	static int getlocalname(SOCKET _fd,struct sockaddr* localaddr) ;
	static int getpeername(SOCKET _fd,struct sockaddr* peeraddr) ;
	static uint64_t sockaddr2uint64(sockaddr_in & addr);
	// 操作
	static void reuse_addr(SOCKET _fd,bool reuse = true);
	/// Enable/disable TCP_NODELAY (disable/enable Nagle's algorithm).
	static void settcpnodelay(SOCKET _fd,bool on);
	static void setkeeplive(SOCKET _fd,bool on);

	static void setsndtimeo(SOCKET _fd,int sec);
	static void setrcvtimeo(SOCKET _fd,int sec);
	static void setsndbuf(SOCKET _fd,int KB);
	static void setrcvbuf(SOCKET _fd,int KB);

	static int bind(SOCKET _fd,struct sockaddr* addr);
	static int bind(SOCKET _fd,unsigned short port = 0, const char* addr = NULL, short family = AF_INET);
	
	static int listen(SOCKET _fd,int backlog = SOMAXCONN);
	
	static SOCKET accept(SOCKET _fd,struct sockaddr* addr);
	
	static int connect(SOCKET _fd,struct sockaddr* addr);
	static int connect(SOCKET _fd,const char* addr, unsigned short port, short family = AF_INET);
	static int connect(SOCKET _fd,uint64_t addr, short family = AF_INET);

	static int send_loop(SOCKET _fd, char* buf, int size);
	static int recv_loop( SOCKET _fd,char* buf, int size);
	static int sendto_loop(SOCKET _fd, char* buf, int size, unsigned short port, const char* addr);
	static int sendto_loop(SOCKET skt,char *data, int len, uint64_t peeraddr);



	static int send_some(SOCKET _fd,const void* buf, int size);
	static int recv_some(SOCKET _fd, void* buf, int size);
	static int recv_some_noblock(SOCKET _fd, void* buf, int size, struct sockaddr* from);
	static int sendto_some(SOCKET _fd,const void* buf, int size, struct sockaddr* to);
	static int sendto_some(SOCKET _fd,const void* buf, int size, unsigned short port, const char* addr);
	static int recvfrom_some(SOCKET _fd, void* buf, int size, unsigned short* port, char* addr);
	static int recvfrom_some(SOCKET _fd,void* buf, int size, struct sockaddr* from);

	static void set_block(SOCKET _fd,bool bblock );
	static int  getreadablelen(SOCKET _fd,int *len);
	static int  select(int fds,fd_set& writefds,int millsec);
public:
	/*
	 * 工具
	 */
	// 装载和卸载socket库
	static bool load_library();
	static bool unload_library();

	static struct hostent* gethost(const char* name) { return ::gethostbyname(name); }
	static struct hostent* gethost(const char* addr, int len, int family = AF_INET) 
	{ return ::gethostbyaddr(addr, len, family); }
	
	static struct servent* getserv(const char* name, const char* protocol = NULL) 
	{ return ::getservbyname(name, protocol); }
	static struct servent* getserv(int port, const char* protocol = NULL) 
	{ return ::getservbyport(port, protocol); } 
 
	static bool is_dot_dec_ip(const char* ipstr);
	
private:
	// 实现
	
	
private:
	// 禁止的操作
	iSocket(const iSocket& rhs);
	iSocket& operator =(const iSocket& rhs);

};
#define SAFE_CLOSE_SOCKET(s)if(s!=SOCKET_NULL){iSocket::shutdown(s);iSocket::close(s);s = SOCKET_NULL;}
#endif // #ifndef _SOCK_H_JKL23895U23N524NM235R90124H1U238XHY901P4J3HY845_