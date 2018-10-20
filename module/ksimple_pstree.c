#include <linux/module.h> 
#include <linux/kernel.h> 
#include <linux/init.h> 
#include <net/sock.h> 
#include <net/netlink.h> 
#include <linux/skbuff.h> 
 
 
static struct sock *netlink_sock;
 
static void udp_reply(int pid,int seq,void *payload) 
{ 
struct sk_buff *skb;
struct nlmsghdr *nlh;
int size=strlen(payload)+1;
int len = NLMSG_SPACE(size);
void *data;
int ret;
 
skb = alloc_skb(len, GFP_ATOMIC);
if (!skb) 
return;
nlh= nlmsg_put(skb, pid, seq, 0, size, 0);
nlh->nlmsg_flags = 0;
data=NLMSG_DATA(nlh);
memcpy(data, payload, size);
NETLINK_CB(skb).portid = 0; /* from kernel */ 
NETLINK_CB(skb).dst_group = 0; /* unicast */ 
ret=netlink_unicast(netlink_sock, skb, pid, MSG_DONTWAIT);
if (ret <0) 
{ 
printk("send failed\n");
return;
} 
return;
 
nlmsg_failure: /* Used by NLMSG_PUT */ 
if (skb) 
kfree_skb(skb);
} 
 
 
/* Receive messages from netlink socket. */ 
static void udp_receive(struct sk_buff *skb) 
{ 
u_int pid, seq;
void *data;
struct nlmsghdr *nlh;

if(!skb)
	return;
 
nlh = (struct nlmsghdr *)skb->data;
pid = NETLINK_CREDS(skb)->pid;
seq = nlh->nlmsg_seq;
data = NLMSG_DATA(nlh);
printk("recv skb from user space pid:%d seq:%d\n",pid,seq);
printk("data is :%s\n",(char *)data);
udp_reply(pid,seq,data);
return ;
} 
 
static int __init kudp_init(void) 
{
    struct netlink_kernel_cfg nkc;
    nkc.groups = 0;
    nkc.flags = 0;
    nkc.input = udp_receive;
    nkc.cb_mutex = NULL;
    nkc.bind = NULL;
    nkc.unbind = NULL;
    nkc.compare = NULL;
    netlink_sock = netlink_kernel_create(&init_net, NETLINK_USERSOCK, &nkc);
    if(!netlink_sock)
	    printk(KERN_ERR, "[netlink] create netlink socket error \n");
    printk("initialize successfully\n");
    return 0;
} 
 
static void __exit kudp_exit(void) 
{ 
sock_release(netlink_sock->sk_socket);
printk("netlink driver remove successfully\n");
} 
module_init(kudp_init);
module_exit(kudp_exit);
 
MODULE_DESCRIPTION("kudp echo server module");
MODULE_AUTHOR("Joey Cheng<jemicheng@gmail.com>");
MODULE_LICENSE("GPL");
MODULE_ALIAS("QT2410:kudp echo server module");
