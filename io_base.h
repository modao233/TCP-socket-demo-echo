ssize_t readn(int fd, void *vptr, size_t n){
    size_t nleft = n;
    ssize_t nread;
    char *ptr = vptr;
    while(nleft > 0){
        if((nread = read(fd, ptr, nleft))<0){
            if(errno == EINTR)
                nread = 0;//重新调用read
            else return -1;
        }
        else if(nread == 0)
            break;
        nleft -= nread;
        ptr += nread;
    }
    return n-nleft;
}

ssize_t writen(int fd, void *vptr, size_t n){
    size_t nleft = n;
    ssize_t nwriten;
    char *ptr = vptr;
    while(nleft > 0){
        if((nwriten = write(fd, ptr, nleft))<=0){
            if(nwriten < 0 && errno == EINTR)
                nwriten = 0;//重新调用write
            else return -1;
        }
        
        nleft -= nwriten;
        ptr += nwriten;
    }
    return n-nleft;
}

/*readline慢速版：每个字符调用一次read，效率低下 */
/*readline较快速版：使用缓冲区，但非stdio的缓冲区，而是自己构建的可见的缓冲区 */
#define MAXLINE 1024
//使用静态变量实现跨相继函数调用的状态维护，使函数不可重入或者说非线程安全
static int read_cnt;
static char *read_ptr;
static char read_buf[MAXLINE];
static ssize_t my_read(int fd, char *ptr){
    //缓冲区为空，则每次读入最多MAXLINE个字符
    if(read_cnt <= 0){
        again:
            if((read_cnt = read(fd, read_buf, sizeof(read_buf))) < 0){
                if(errno == EINTR)
                    goto again;
                return -1;
            }else if(read_cnt == 0)
                return 0;
            read_ptr = read_buf;
    }
    //一次返回一个字符
    read_cnt -= 1;
    *ptr = *read_ptr++;
    return 1;
}
ssize_t readline(int fd, void *vptr, size_t maxlen){
    ssize_t n, rc;
    char c, *ptr;

    ptr = vptr;
    for(n = 1; n < maxlen; n++){
        if(rc = my_read(fd, &c) == 1){
            *ptr++ = c;
            if(c == '\n')
                break;
        }else if(rc == 0){
            *ptr = 0;
            return n - 1;
        }else
            return -1;
    }
    *ptr = 0;
    return n;
}
//便于调用者查看在当前文本行之后是否收到了新的数据，展露内部缓冲区的状态
ssize_t readlinebuf(void **vptrptr){
    if(read_cnt)
        *vptrptr = read_ptr;
    return read_cnt;
}
