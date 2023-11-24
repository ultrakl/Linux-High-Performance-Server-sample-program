#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H

/*14章locker.h的位置*/
#include "/home/kuli/cpp/chapter_14/locker.h"
#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/uio.h>
#include <stdarg.h>
#include <unistd.h>

class http_conn {
    static const int FILENAME_LEN = 200;
    static const int READ_BUFFER_SIZE = 2048;
    static const int WRITE_BUFFER_SIZE = 1024;
    enum METHOD {
        GET = 0,
        POST,
        HEAD,
        PUT,
        DELETE,
        TRACE,
        OPTIONS,
        CONNECT,
        PATCH
    };
    enum CHECK_STATE {
        CHECK_STATE_REQUESTLINE = 0,
        CHECK_STATE_HEADER,
        CHECK_STATE_CONTENT
    };
    enum HTTP_CODE {
        NO_REQUEST,
        GET_REQUEST,
        BAD_REQUEST,
        NO_RESOURCE,
        FORBIDDEN_REQUEST,
        FILE_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTION
    };
    enum LINE_STATUS { LINE_OK = 0, LINE_BAD, LINE_OPEN };
  public:
    http_conn() {}
    ~http_conn() {}
  public:
    void init(int sockfd, const sockaddr_in& addr);
    /*关闭连接*/
    void close_conn(bool real_close = true);
    /*处理客户请求*/
    void process();
    bool read();
    bool write();
  private:
    void init();
    /*解析HTTP请求*/
    HTTP_CODE process_read();
    /*填充HTTP应答*/
    bool process_write(HTTP_CODE ret);

    /*下面这一组函数被process_read调用以分析HTTP请求*/
    HTTP_CODE parse_request_line(char* text);
    HTTP_CODE parse_headers(char* text);
    HTTP_CODE parse_content(char* text);
    HTTP_CODE do_request();
    char* get_line() {return m_read_buf + m_start_line;}
    LINE_STATUS parse_line();

    /*下面这一组函数被process_write调用以填充HTTP应答*/
    void unmap();
    bool add_response(const char* format, ...);
    bool add_content(const char* content);
    bool add_status_line(int status, const char* title);
    bool add_headers(int content_length);
    bool add_content_length(int content_length);
    bool add_linger();
    bool add_blank_line();
  public:
    static int m_epollfd;
    static int m_user_count;
  private:
    /*该HTTP连接的socket和对方的socket地址*/
    int m_sockfd;
    sockaddr_in m_address;
    char m_read_buf[READ_BUFFER_SIZE];
    /*标识读缓冲已经读入的客户数据的最后一个字节的下一个位置*/
    int m_read_idx;
    /*当前正在分析的字符在读缓冲区中的位置*/
    int m_checked_idx;
    /*当前正在解析行的起始位置*/
    int m_start_line;
    char m_write_buf[WRITE_BUFFER_SIZE];
    /*写缓冲区待发送的字节数*/
    int m_write_idx;

    /*主状态机当前所处的状态*/
    CHECK_STATE m_check_state;
    METHOD m_method;

    /*客户请求的目标文件的完整路径， 其内容等于doc_root + m_url, doc_root是网站根目录*/
    char m_real_file[FILENAME_LEN];
    /*客户请求的目标文件的文件名*/
    char* m_url;
    char* m_version;
    char* m_host;
    int m_content_length;
    bool m_linger;
    /*客户请求的目标文件被mmap到内存中的起始位置*/
    char* m_file_address;
    /*目标文件的状态*/
    struct stat m_file_stat;
    /*采用writev来执行写操作， m_iv_count表示被写内存块的数量*/
    struct iovec m_iv[2];
    int m_iv_count;
};

// template<typename T, typename... Args>

#endif