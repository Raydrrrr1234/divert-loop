/* block_inICMP.c

   loop of FreeBSD divert socket

   Ruikang Dai 10/4/2015
*/

#define DEBUG

#include "divertlib.h"

#define maxbuflen 1520

int main(int argc, char *argv[])
{ int i, len, divsock;
  u_short iphlen, tcphlen;
  int udpsock, DivPort;
  struct sockaddr_in sin, sin1;
  struct ip *iph;
  struct tcphdr *tcph;
  struct udphdr *udph;
  struct icmp *icmph;
  unsigned char buf[maxbuflen+1];
  int addrsize=sizeof (struct sockaddr);
  int j;
  if (argc!=2)
  { puts("usage : divert-loop [div port]");
    return 1;
  };

  DivPort=atoi(argv[1]);
  printf("DivPort=%d\n", DivPort);

  if ((divsock=initDivSock(DivPort))<=0)
  { printf("can not get divert socket for port %d, divsock=%d\n",
            DivPort, divsock);
    exit(1);
  };

  for (i=1; ;i++)
  {
/* if ((len=readDiv(divsock, buf, maxbuflen, (struct sockaddr *) &sin))>0)
 */
    if ((len=recvfrom(divsock, buf, maxbuflen, 0, 
		      (struct sockaddr *) &sin, &addrsize))>0)
    {

      iph=(struct ip *) buf;
      iphlen=iph->ip_hl<<2;
#ifdef DEBUG
      if (sin.sin_addr.s_addr==INADDR_ANY) /* outgoing */
      { printf("\n%d : Out\t\t\t\t\t\t\t\t==>\n", i);
      }
      else /* incoming */
      { printf("\n%d : In from %s:%d\t\t\t\t\t\t<==\n",
                i, inet_ntoa(sin.sin_addr), ntohs(sin.sin_port));
      };
      printf("\tsrc IP:%s\n", inet_ntoa(iph->ip_src));
      printf("\tdst IP:%s\n", inet_ntoa(iph->ip_dst));
      printf("\tproto :%d\n", iph->ip_p);
	printf("---------IP header--------\n");
	for (j = 0; j < iph->ip_hl*4; j+=4) 
		printf ("%.2x %.2x %.2x %.2x\n", buf[j], buf[j+1], buf[j+2],buf[j+3]);
	printf("\n---------ICMP header---------\n");
	for (j = (iph->ip_hl)*4; j < (iph->ip_hl)*4+sizeof(struct icmp); j+=4) 
		printf ("%.2x %.2x %.2x %.2x\n", buf[j], buf[j+1], buf[j+2],buf[j+3]);
#endif
	printf("%d", iph->ip_p);
	if (1 != iph->ip_p)  
	{
      		sendto(divsock, buf, len, 0, (struct sockaddr *) &sin,
	     sizeof(struct sockaddr));
	}
    else
	{	// ICMP request to destination
		if (inet_addr("172.16.201.128") == iph->ip_src.s_addr && 8 == buf[(iph->ip_hl)*4])
		sendto(divsock, buf, len, 0, (struct sockaddr *) &sin,
	     sizeof(struct sockaddr));
		// ICMP reply to local
		if (inet_addr("172.16.201.128") == iph->ip_dst.s_addr && 0 == buf[(iph->ip_hl)*4])
		sendto(divsock, buf, len, 0, (struct sockaddr *) &sin,
	     sizeof(struct sockaddr));
	}
    }
  };
  return 0;
}
