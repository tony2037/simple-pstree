#include <unistd.h>
#include <stdlib.h>
#include <stdio.h> 
#include <string.h> 
#include <malloc.h> 
#include <sys/socket.h> 
#include <linux/netlink.h> 

#define MAX_PAYLOAD 1024


int main(int agrc, char** argv){
    char *opt = (char *)malloc(strlen(argv[1]) + 1);
    memcpy(opt, &(argv[1][1]), strlen(argv[1]));

    printf("Option: %s  strlen:%d\n", opt, (int)strlen(opt));
    
    struct sockaddr_nl src_addr, dest_addr;
    struct nlmsghdr *nlh = NULL;
    struct iovec iov;
    int sock_fd;
    struct msghdr msg; 

    sock_fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_USERSOCK);
    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = getpid(); /* self pid */ 
    src_addr.nl_groups = 0; /* not in mcast groups */ 
    printf("%d",bind(sock_fd, (struct sockaddr*)&src_addr, sizeof(struct sockaddr)));
    
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.nl_family = AF_NETLINK;
    dest_addr.nl_pid = 0; /* For Linux Kernel */ 
    dest_addr.nl_groups = 0; /* unicast */ 
    
    nlh=(struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));

    memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
    /* Fill the netlink message header */ 
    nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
    nlh->nlmsg_pid = getpid(); /* self pid */ 
    nlh->nlmsg_flags = 0;
    /* Fill in the netlink message payload */
    //strcpy(NLMSG_DATA(nlh), opt);
    memcpy(NLMSG_DATA(nlh), opt, strlen(opt)+1);
    printf("payload: %s\n", (char *)NLMSG_DATA(nlh));
    
    
    memset(&msg, 0, sizeof(msg));
    iov.iov_base = (void *)nlh;
    iov.iov_len = nlh->nlmsg_len;
    msg.msg_name = (void *)&dest_addr;
    msg.msg_namelen = sizeof(dest_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    
   int send_result = sendmsg(sock_fd, &msg, 0);
   if (send_result != 0) {
   	perror("sendmsg");
   }
   printf("sendmsg result: %d\n", send_result);
    
    printf("Waiting for message from kernel\n");
    
    /* Read message from kernel */ 
    recvmsg(sock_fd, &msg, 0);
    

    printf("Received message payload: \n%s\n", (char *)NLMSG_DATA(msg.msg_iov->iov_base));
    close(sock_fd);
    free(opt);
    return 0; 
}
