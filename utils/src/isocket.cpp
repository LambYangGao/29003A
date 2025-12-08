#include <iostream>  
#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <fcntl.h>  
#include <assert.h>  
#include "isocket.h" 
  
#define errputs puts  
  
#ifndef MAX_BUF_SIZE  
#define MAX_BUF_SIZE        1024  
#endif  
  
#ifndef MAX_STR_LEN  
#define MAX_STR_LEN         254  
#endif  
  
#define P(x) cout << (x) << endl
#define F(str, file) \
    { \
        char cmd[MAX_BUF_SIZE]; \
        sprintf(cmd, "echo \'%s\' >> %s", str, file); \
        system(cmd); \
          
#define ASSERT_NOT(x) \
    if (x) \
    { \
        perror("ERROR"); \
        exit(1); \
    }  
  
iSocket::iSocket()
{  
      
}  
iSocket::~iSocket()  
{     
    
}  
  
SOCKET iSocket::socket(int domain /* = AF_INET */,int type /* = SOCK_STREAM */, int protocol /* = 0 */)
{  
    SOCKET skt =  ::socket(domain, type, protocol);  
#ifdef WIN32  
	if(skt == INVALID_SOCKET)
	{
		skt = SOCKET_NULL;
	}
#endif // #ifdef WIN32 
	return skt;
}  
  
void iSocket::close(SOCKET _fd)  
{  
   if (_fd!=SOCKET_NULL)
   {
#ifdef WIN32  
	   ::closesocket(_fd);  
#else  
	   ::close(_fd);  
#endif // #ifdef WIN32  
   }

   
}  
  
void iSocket::shutdown(SOCKET _fd,int how /*= SHUT_RDWR*/)  
{  
    if (_fd != SOCKET_NULL)  
    {  
        ::shutdown(_fd, how);  
    }  
}  
  
int iSocket::getlocalname(SOCKET _fd,struct sockaddr* localaddr)   
{  
    if (_fd == SOCKET_NULL) {
        return SOCKET_ERROR;
    }
  
    int len = sizeof(struct sockaddr);  
    return ::getsockname(_fd, localaddr, (socklen_t *)&len);  
}  
  
int iSocket::getpeername(SOCKET _fd,struct sockaddr* peeraddr)   
{  
    if (_fd == SOCKET_NULL) {
        return SOCKET_ERROR;
    }
    int len = sizeof(struct sockaddr);  
    return ::getpeername(_fd, peeraddr, (socklen_t *)&len);
}  
#ifdef WIN32  
#define   OPT_DATATYPE char*
#else  
#define   OPT_DATATYPE int*
#endif // #ifdef WIN32  

void iSocket::reuse_addr(SOCKET _fd,bool reuse /*= true*/)  
{  
#ifdef WIN32  
    char open;  
#else  
    int open;  
#endif // #ifdef WIN32  
      
    open = reuse ? 1 : 0;  
    socklen_t optlen = sizeof(open);  
    ::setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &open, optlen);  
}
uint64_t iSocket::sockaddr2uint64(sockaddr_in & addr)
{
	uint64_t unPeerAddr = 0;
#ifdef WIN32
	unPeerAddr = (uint64_t)ntohl(addr.sin_addr.s_addr);
	unPeerAddr = (unPeerAddr << 32)| ntohs(addr.sin_port);
#else
	unPeerAddr = addr.sin_addr.s_addr;
	unPeerAddr = (unPeerAddr << 32)|(addr.sin_port);
#endif
	return unPeerAddr;
}
void iSocket::settcpnodelay(SOCKET _fd,bool on)
{
#ifdef WIN32  
	char optval;  
#else  
	int optval;  
#endif // #ifdef WIN32  

	optval = on ? 1 : 0;  
	socklen_t optlen = sizeof(optval);  
	::setsockopt(_fd, IPPROTO_TCP, TCP_NODELAY,
		&optval, optlen);
}
void iSocket::setkeeplive(SOCKET _fd,bool on)
{
#ifdef WIN32  
	char optval;  
#else  
	int optval;  
#endif // #ifdef WIN32  

	optval = on ? 1 : 0;
	socklen_t optlen = sizeof(optval);  
	::setsockopt(_fd, SOL_SOCKET, SO_KEEPALIVE,
		&optval, optlen);
}


void iSocket::setsndtimeo(SOCKET _fd,int sec)
{
	sec *= 1000;
	setsockopt(_fd,SOL_SOCKET,SO_SNDTIMEO,(OPT_DATATYPE)&sec,sizeof(int));
}
void iSocket::setrcvtimeo(SOCKET _fd,int sec)
{
	sec *= 1000;
	setsockopt(_fd,SOL_SOCKET,SO_RCVTIMEO,(OPT_DATATYPE)&sec,sizeof(int));
}
void iSocket::setsndbuf(SOCKET _fd,int KB)
{
	int nBuf = 1024*KB;
	setsockopt(_fd,SOL_SOCKET,SO_SNDBUF,(OPT_DATATYPE)&nBuf,sizeof(int)); 
}
void iSocket::setrcvbuf(SOCKET _fd,int KB)
{
	int nBuf = 1024*KB;
	setsockopt(_fd,SOL_SOCKET,SO_RCVBUF,(OPT_DATATYPE)&nBuf,sizeof(int)); 
}

int iSocket::bind(SOCKET _fd,struct sockaddr* addr)  
{  
    if (_fd == SOCKET_NULL)
    {
        return SOCKET_ERROR;
    }
	int addrlen = sizeof(struct sockaddr);
    return ::bind(_fd, addr, addrlen);  
}  

int iSocket::bind(SOCKET _fd,unsigned short port /*= 0*/, const char* addr /*= NULL*/, short family /*= AF_INET*/)  
{  
    if (_fd == SOCKET_NULL)  
    {
        return SOCKET_ERROR;  
    }  
      
    sockaddr_in my_addr = { sizeof(struct sockaddr_in) };  
    my_addr.sin_family = family;  
    my_addr.sin_port = htons(port);  
    if (addr == NULL)
    {
        my_addr.sin_addr.s_addr = INADDR_ANY;
    }
    else   
    {   
        if (is_dot_dec_ip(addr))   
        {   
            my_addr.sin_addr.s_addr = inet_addr(addr);   
        }   
        else   
        {   
            hostent* he = ::gethostbyname(addr);   
            if (he == NULL)   
            {   
                return SOCKET_ERROR;   
            }   
            memcpy(&my_addr.sin_addr, he->h_addr_list[0], he->h_length);   
        }   
    }  
      
    int addrlen = sizeof(struct sockaddr_in);  
    int result = ::bind(_fd, (struct sockaddr*)&my_addr, addrlen);  
    return result;  
}  
  
int iSocket::listen(SOCKET _fd,int backlog /*= SOMAXCONN*/)  
{  
    if (_fd == SOCKET_NULL) {
        return SOCKET_ERROR;
    }
    int result = ::listen(_fd, backlog);  
      
    return result;  
}  

// void setNonBlockAndCloseOnExec(int sockfd)
// {
// 	// non-block
// 	int flags = ::fcntl(sockfd, F_GETFL, 0);
// 	flags |= O_NONBLOCK;
// 	int ret = ::fcntl(sockfd, F_SETFL, flags);
// 	// FIXME check
// 
// 	// close-on-exec
// 	flags = ::fcntl(sockfd, F_GETFD, 0);
// 	flags |= FD_CLOEXEC;
// 	ret = ::fcntl(sockfd, F_SETFD, flags);
// 	// FIXME check
// 
// 	(void)ret;
// }


SOCKET iSocket::accept(SOCKET _fd, struct sockaddr* addr )  
{  
 
      
    if (_fd == SOCKET_NULL) {
        return SOCKET_NULL;
    }
	socklen_t addrlen = sizeof(struct sockaddr);
    SOCKET skt_new = ::accept(_fd, addr,&addrlen);  
#ifdef WIN32
	if (skt_new == INVALID_SOCKET)
	{
		skt_new = SOCKET_NULL;
	}
#else
	if (skt_new != SOCKET_NULL)
	{
// 		int flags = ::fcntl(skt_new, F_GETFL, 0);
// 		flags |= O_NONBLOCK;//暂时不能设置成非柱塞
// 		int ret = ::fcntl(skt_new, F_SETFL, flags);
		// FIXME check

		// close-on-exec
		int flags = ::fcntl(skt_new, F_GETFD, 0);
		flags |= FD_CLOEXEC;
		int ret = ::fcntl(skt_new, F_SETFD, flags);
	}

#endif
	return skt_new;
}  
  

int iSocket::connect(SOCKET _fd,struct sockaddr* addr)  
{  
    if (_fd == SOCKET_NULL)
    {
        return SOCKET_ERROR;
    }
    int addrlen = sizeof(struct sockaddr);
    return ::connect(_fd, addr, addrlen);  
}  
  
int  iSocket::connect(SOCKET _fd,const char* addr, unsigned short port, short family /*= AF_INET*/)  
{  
    if (_fd == SOCKET_NULL)
    {
        return SOCKET_ERROR;
    }
    if (addr == NULL) {
        return SOCKET_ERROR;
    }
    sockaddr_in my_addr;  
    my_addr.sin_family = family;  
    my_addr.sin_port = htons(port);  
       
    if (is_dot_dec_ip(addr))   
    {   
        my_addr.sin_addr.s_addr = inet_addr(addr);   
    }   
    else   
    {   
        hostent* he = ::gethostbyname(addr);   
        if (he == NULL)   
        {   
            return SOCKET_ERROR;   
        }   
        memcpy(&my_addr.sin_addr, he->h_addr_list[0], he->h_length);   
    }  
      
    int addrlen = sizeof(struct sockaddr);  
    int result = ::connect(_fd, (struct sockaddr*)&my_addr, addrlen);  
      
    return result;  
}  
int iSocket::connect(SOCKET _fd,uint64_t addr, short family /*= AF_INET*/)
{
    if (_fd == SOCKET_NULL) {
        return SOCKET_ERROR;
    }
    if (addr == 0) {
        return SOCKET_ERROR;
    }
	sockaddr_in my_addr;  
	my_addr.sin_family = family;  
	my_addr.sin_port = htons((int)addr);  
	int nIP = (int)(addr>>32);
	if (nIP!=0)
	{
		my_addr.sin_addr.s_addr  = htonl(nIP);
	}
	else
	{
		my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	}
	
	int addrlen = sizeof(struct sockaddr);  
	int result = ::connect(_fd, (struct sockaddr*)&my_addr, addrlen);  

	return result ;  
}
void iSocket::set_block(SOCKET _fd,bool block /*= true*/)
{  
      
    if (_fd == SOCKET_NULL) {
        return;
    }
#ifdef WIN32  
    u_long b = block ? 0 : 1;  
    ::ioctlsocket(_fd, FIONBIO, &b);
#else  
    int flag;  
    flag = ::fcntl(_fd, F_GETFL, 0);  
    if (block) {
        flag &= ~O_NONBLOCK;
    }
    else {
        flag |= O_NONBLOCK;
    }
    ::fcntl(_fd, F_SETFL, flag);
#endif  
}  
int  iSocket::getreadablelen(SOCKET _fd,int *len)
{
	if (_fd ==SOCKET_NULL)
	{
		return SOCKET_ERROR;
	}
	int ret = SOCKET_ERROR;
#ifdef WIN32
	ret = ::ioctlsocket(_fd,FIONREAD,(u_long*)len);
#else
	ret = ::ioctl(_fd,FIONREAD,len);
#endif
	return ret ;
}
int  iSocket::select(int fds,fd_set& writefds,int millsec)
{
	int ret = SOCKET_ERROR;
	timeval Timeout = {3,0};
#ifdef WIN32 
	ret = ::select(fds,NULL,&writefds, NULL, &Timeout);
#else
	ret = ::select(fds+1,NULL,&writefds, NULL, &Timeout);
#endif
	return ret;
}
int iSocket::send_loop(SOCKET _fd, char * data, int len)
{
	if (!data || len<=0)
	{
		return SOCKET_ERROR;
	}
	int nRet = 0;
	int nLeft = len;
	do 
	{
		nRet = send_some(_fd,data,nLeft);
		if (SOCKET_ERROR == nRet)
		{
			//int nErrorCode = WSAGetLastError(); 
			return SOCKET_ERROR;
		}
		else if (nRet == 0)
		{
		}
		else
		{
			data= data+nRet;
			nLeft = nLeft - nRet;
		}
	} while (nLeft>0);
	return len - nLeft;
}
int iSocket::recv_loop( SOCKET _fd,char* data, int len)
{
	if (!data || len<=0)
	{
		return SOCKET_ERROR;
	}
	int nRet = 0;
	int nLeft = len;
	do 
	{
		nRet = recv_some(_fd,data,nLeft);
        
		if (SOCKET_ERROR == nRet)
		{
// 			int nErrorCode = WSAGetLastError();
// 			if (WSAETIMEDOUT == nErrorCode)
// 			{
// 				break;
// 			}
			return SOCKET_ERROR;
		}
		else if (nRet == 0)
		{
			break;
		}
		else
		{
			nLeft = nLeft - nRet;
			data = data + nRet;
		}

	} while (nLeft >0);


	return len - nLeft;
}

int iSocket::sendto_loop(SOCKET _fd, char* data, int len, unsigned short port, const char* addr)
{
	if (!data || len<=0)
	{
		return SOCKET_ERROR;
	}
	int nRet = 0;
	int nLeft = len;
	do 
	{
		nRet = sendto_some(_fd,data,nLeft,port,addr);
		if (SOCKET_ERROR == nRet)
		{
			//int nErrorCode = WSAGetLastError();
			return SOCKET_ERROR;
		}
		else if (nRet == 0)
		{
			
		}
		else
		{
			data= data+nRet;
			nLeft = nLeft - nRet;
		}
	} while (nLeft>0);
	return len - nLeft;
}
int iSocket::sendto_loop(SOCKET skt,char *data, int len, uint64_t peeraddr)
{
	if (!data || len<=0)
	{
		return SOCKET_ERROR;
	}
	int nRet = 0;
	int nLeft = len;
	sockaddr_in DstAddr;
	DstAddr.sin_family = AF_INET;
	DstAddr.sin_addr.s_addr = htonl((int)(peeraddr>>32));
	DstAddr.sin_port = htons(peeraddr);
	do 
	{
		nRet = sendto_some(skt,data,nLeft,(sockaddr *)&DstAddr);
		if (SOCKET_ERROR == nRet)
		{
			//int nErrorCode = WSAGetLastError();
			return SOCKET_ERROR;
		}
		else if (nRet == 0)
		{
			
		}
		else
		{
			data= data+nRet;
			nLeft = nLeft - nRet;
		}
	} while (nLeft>0);
	return len - nLeft;
}
int iSocket::send_some(SOCKET _fd,const void* buf, int size)  
{   
	int flags = 0;
#ifndef WIN32
#if (defined(__APPLE__) && defined(__MACH__))
	flags = SO_NOSIGPIPE;
#else
	flags = MSG_NOSIGNAL;
#endif
#endif
	
    ssize_t new_size;
    new_size = ::send(_fd, (const char*)buf, size, flags);
    return new_size ;
}  
  
int iSocket::recv_some(SOCKET _fd,void* buf, int size)
{  
	int flags = 0;
    ssize_t new_size;
    new_size = ::recv(_fd, (char*)buf, size, flags);
    return new_size ;// 注意：连接被优雅地断开时new_size == 0  
}  
  
int iSocket::recv_some_noblock(SOCKET _fd, void* buf, int size, struct sockaddr* from)
{
	int flags = MSG_DONTWAIT;
	ssize_t new_size;
    socklen_t addrlen = sizeof(sockaddr_in);

	new_size = ::recvfrom(_fd, (char*)buf, size, flags, from, (socklen_t*)&addrlen);

    if (new_size < 0)
    {
        if (errno == EWOULDBLOCK || errno == EAGAIN)
        {
            return 0; // 没有数据
        } 
        else
        {
            std::cerr << "recvfrom failed:" << strerror(errno) << std::endl;
            return -1;
        }
    }
    return new_size;
}

int iSocket::sendto_some( SOCKET _fd,const void* buf, int size, struct sockaddr* to)
{  
	int flags = 0;
#ifndef WIN32
#if (defined(__APPLE__) && defined(__MACH__))
	flags = SO_NOSIGPIPE;
#else
	flags = MSG_NOSIGNAL;
#endif
#endif
    ssize_t new_size;
	socklen_t addrlen = sizeof( sockaddr_in);
    new_size = ::sendto(_fd, (const char*)buf, size, flags, to, addrlen);
    return new_size ;
}  
  
int iSocket::sendto_some(SOCKET _fd,const void* buf, int size, unsigned short port, const char* addr)  
{
    sockaddr_in my_addr = { sizeof(struct sockaddr_in) };  
    my_addr.sin_family = AF_INET;  
    my_addr.sin_port = htons(port);  
    if (addr == NULL) {
        my_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    }
    else   
    {   
        //if (is_dot_dec_ip(addr))   
        {   
            my_addr.sin_addr.s_addr = inet_addr(addr);   
        }   
//         else   
//         {   
//             hostent* he = ::gethostbyname(addr);   
//             if (he == NULL)   
//             {   
//                 return SOCKET_ERROR;   
//             }   
//             memcpy(&my_addr.sin_addr, he->h_addr_list[0], he->h_length);   
//         }   
    }
    return sendto_some(_fd,buf, size, (struct sockaddr*)&my_addr);  
}  
  
int iSocket::recvfrom_some(SOCKET _fd,void* buf, int size, struct sockaddr* from)  
{  
	int flags = 0;
    ssize_t new_size;  
	socklen_t addrlen = sizeof( sockaddr_in);
    new_size = ::recvfrom(_fd, (char*)buf, size, flags, from, (socklen_t *)&addrlen);
    return new_size;  
}  
  
 int iSocket::recvfrom_some( SOCKET _fd,void* buf, int size, unsigned short* port, char* addr)  
 {  
     struct sockaddr_in from;  
     ssize_t new_size;
 	new_size = recvfrom_some(_fd,buf, size, (struct sockaddr*)&from);
 	if (new_size != SOCKET_ERROR)
 	{
        if (port != NULL) {
            *port = ntohs(from.sin_port);
        }
        if (addr != NULL) {
            strcpy(addr, inet_ntoa(from.sin_addr));
        }
 	}  
     return new_size;  
 }  
  
bool iSocket::load_library()  
{  
#ifdef WIN32  
    unsigned short wVersion = MAKEWORD(2,2);  
    WSADATA wsaData;  
    if (WSAStartup(wVersion,&wsaData) != 0)  
    {  
        return false;  
    }  
#endif // #ifdef WIN32  
    return true;  
}  
  
bool iSocket::unload_library()  
{  
#ifdef WIN32  
    if (WSACleanup() != 0)  
    {  
        return false;  
    }  
#endif // #ifdef WIN32  
    return true;  
}   
   
bool iSocket::is_dot_dec_ip(const char* ipstr)   
{   
    assert(ipstr != NULL);   
   
    // 检查长度   
    if (strlen(ipstr) < 7 || strlen(ipstr) > 15)   
    {   
        return false;   
    }   
   
    // 检查字符是否合法   
    int ndot = 0;   
    const char* p;   
    for (p = ipstr; *p != '\0'; ++p)   
    {   
        if (*p != '.' && (*p < '0' || *p > '9'))   
        {   
            return false;   
        }   
        if (*p == '.')   
        {   
            ++ndot;   
        }   
    }   
   
    // 检查句点是否合理   
    if (ndot != 3)   
    {   
        return false;   
    }   
    if (ipstr[0] == '.' || ipstr[strlen(ipstr) - 1] == '.')   
    {   
        return false;   
    }   
    if (strstr(ipstr, "..") != NULL)   
    {   
        return false;   
    }   
   
    // 检查每个数字是否在0-255之间   
    char* buf = (char*)malloc(strlen(ipstr) + 1);   
    strcpy(buf, ipstr);   
    p = strtok(buf, ".");   
    while (p != NULL)   
    {   
        int n = atoi(p);   
        if (n > 255)   
        {   
            free(buf);   
            return false;   
        }   
   
        p = strtok(NULL, ".");   
    }   
    free(buf);
   
    return true;   
}  