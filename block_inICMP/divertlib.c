/* divertlib.c

   convenient library of FreeBSD divert socket

   Ruikang Dai 10/4/2015
*/

#include "divertlib.h"

int initDivSock (u_short divport)
/* return >0 : Ok
          -1 : error
*/
{
  struct sockaddr_in	addr;
  int			sockfd;

  if (divport)
  { sockfd = socket(PF_INET, SOCK_RAW, IPPROTO_DIVERT);
    if (sockfd == -1)
    {
#ifdef DEBUG
      fprintf(stderr, "Unable to create divert socket\n");
#endif /* DEBUG */
puts("unable to create divert socket");
      return -1;
    }

/* bind socket */
    addr.sin_family	= AF_INET;
    addr.sin_addr.s_addr= INADDR_ANY;
    addr.sin_port	= htons(divport);

    if (bind( sockfd,
	     (struct sockaddr *) &addr,
	     sizeof addr) == -1)
    {
#ifdef DEBUG
      fprintf(stderr, "Unable to bind divert socket : %d\n", divport);
#endif /* DEBUG */
puts("unable to bind divert socket");
      return -1;
    }
    return sockfd;
  }
  else return -1;
}

int readDiv(int sockd, u_char *buf, int buflen, struct sockaddr *saddr)
/* return >0 : number of bytes read
          else : error
*/
{ int len;
  int addrsize=sizeof (struct sockaddr);
  len = recvfrom(sockd, buf, buflen, 0, saddr, &addrsize);

  return len;
}

int writeDiv(int sockd, u_char *buf, int buflen, struct sockaddr *saddr)
/* return actural number of bytes writen
*/
{ int len;
  len = sendto(sockd, buf, buflen, 0, saddr, sizeof (struct sockaddr));

  return len;
}

