/* divert-loop.c--
   AUTHOR:Ruikang Dai
   DATE:Fri Oct  2 11:59:35 EDT 2015
*/
#include <stdio.h>
// These are optional header files
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <limits.h>
#include <getopt.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <signal.h>

#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <net/if.h>
#include <sys/param.h>
#include <netinet/types.h>
#include <netinet/icmp.h>
#include <netinet/ip_fw.h>

#define IPPROTO_DIVERT 254
#define BUFFERSIZE 65536
#define DEFAULT_PORT 54321

#ifdef FIREWALL
char *fw_icmp="divert-loop";
char *fw_chain="output";
struct ip_fw fw;
struct ip_fwuser ipfu;
struct ip_fwchange ipfc;
int fw_sock;
#endif
void intHandler (int signo) {
	if (setsockopt(fw_sock, IPPROTO_IP, IP_FW_DELETE, &ipfc, sizeof(ipfc))==-1) {
		perror("Could not remove rule!\n");
		exit(2);
	}
	close(fw_sock);
	exit(0);
}

int main(int argn, char **argv) {

	// Check number of command-line arguments
	if (argn != 2) {
		perror("No port information, use default 54321 port.");
	}
	int fd, rawfd, fdfw, ret, n;
	int on=1;
	struct sockaddr_in bindPort, sin;
	int sinlen;
	struct iphdr *hdr;
	unsigned char packet[BUFSIZE];
	struct in_addr addr;
	int i, direction;
	struct ip_mreq mreq;
	
	fd = socket(AF_INET, SOCK_RAW, IPPROTO_DIVERT);

	
	if (fd==−1) {
		fprintf(stderr,"%s:We could not open a divert socket\n",argv[0]);
		exit(1);
	}
	bindPort.sin_family=AF_INET;
	// Default port
	bindPort.sin_port=htons(DEFAULT_PORT);
	bindPort.sin_addr.s_addr=0;
	ret=bind(fd, &bindPort, sizeof(struct sockaddr_in));
	if (ret!=0) {
		close(fd);
		fprintf(stderr, "%s: Error bind(): %s",argv[0],strerror(ret));
		exit(2);
	}
#ifdef FIREWALL
	// Firewall setup:
	bzero(&fw, sizeof (struct ip_fw));
	fw.fw_proto=1; /* ICMP */
	fw.fw_redirpt=htons(bindPort.sin_port);
	fw.fw_spts[1]=0xffff;
	fw.fw_dpts[1]=0xffff;
	fw.fw_outputsize=0xffff;
	
	ipfu.ipfw=fw;
	memcpy(ipfu.label, fw_policy, strlen(fw_policy));
	
	/* fill in the fwchange structure */
	ipfc.fwc_rule=ipfu;
	memcpy(ipfc.fwc_label, fw_chain, strlen(fw_chain));
	/* open a socket */
  	if ((fw_sock=socket(AF_INET, SOCK_RAW, IPPROTO_RAW))==−1) {
  		fprintf(stderr, "%s: could not create a raw socket: %s\n", argv[0], strerror(errno));
		exit(2); 
	}
	/* write a rule into it */
	if (setsockopt(fw_sock, IPPROTO_IP, IP_FW_APPEND, &ipfc, sizeof(ipfc))==−1) {
    		fprintf(stderr, "%s could not set rule: %s\n", argv[0], strerror(errno));
		exit(2); 
	}
	signal(SIGINT, intHandler);
	// Firewall end
#endif	
	printf("%s: Waiting for data...\n",argv[0]);
	sinlen=sizeof(struct sockaddr_in);
	while(1) {
    		n=recvfrom(fd, packet, BUFSIZE, 0, &sin, &sinlen);
    		hdr=(struct iphdr*)packet;
    		printf("%s: The packet looks like this:\n",argv[0]);
        	for( i=0; i<40; i++) {
                	printf("%02x ", (int)*(packet+i));
                	if (!((i+1)%16)) printf("\n");
        	}
		printf("\n");
		addr.s_addr=hdr−>saddr;
    		printf("%s: Source address: %s\n",argv[0], inet_ntoa(addr));
    		addr.s_addr=hdr−>daddr;
    		printf("%s: Destination address: %s\n", argv[0], inet_ntoa(addr));
    		printf("%s: Receiving IF address: %s\n", argv[0], inet_ntoa(sin.sin_addr));
    		printf("%s: Protocol number: %i\n", argv[0], hdr−>protocol);
	}

#ifdef MULTICAST
   	if (IN_MULTICAST((ntohl(hdr−>daddr)))) {
	printf("%s: Multicast address!\n", argv[0]);
	addr.s_addr = hdr−>saddr;
	errno = 0;
	if (sin.sin_addr.s_addr == 0)
   	printf("%s: set_interface returns", argv[0]);
#endif
#ifdef REINJECT
   	printf("%s Reinjecting DIVERT %i bytes\n", argv[0], n);
   	n=sendto(fd, packet, n ,0, &sin, sinlen);
	printf("%s: %i bytes reinjected.\n", argv[0], n);
          if (n<=0)
            printf("%s: Oops: errno = %i\n", argv[0], errno);
          if (errno == EBADRQC)
            printf("errno == EBADRQC\n");
          if (errno == ENETUNREACH)
            printf("errno == ENETUNREACH\n");
#endif
	return 0;
}
