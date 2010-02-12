// hostnames.c
//
// © 2009 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "memory.h"
#include "str.h"
#include "log.h"
#include "hostnames.h"

struct sockaddr_in Address;
size_t Address_len;

IPAddress*
lookup(str host)
{
	size_t i;
	struct hostent* hst = gethostbyname(host->data);
	for (i = 0; hst->h_addr_list[i];++i);
	IPAddress* retval = (IPAddress*)reserve(hst->h_length * i);
	if (hst->h_length != sizeof(IPAddress)) error("Host name address to large!");
	for (i = 0; hst->h_addr_list[i]; ++i) {
		debug("Found ip address: %s",ipaddress(*(IPAddress*)hst->h_addr_list[i],80));
		memcpy(&retval[i],hst->h_addr_list[i],sizeof(IPAddress));
	}
	return retval;
}

int
attachTo(IPAddress addr, int port)
{
	memset(&Address,0,sizeof(Address));
#ifndef LINUX
	Address.sin_len = sizeof(struct sockaddr_in);
#endif
	Address.sin_port = htons(port);
	Address.sin_addr.s_addr = addr;	// Uint32 ipv4 htonl
	Address.sin_family = AF_INET;
	return 0;
}

str
ipaddress(IPAddress addr, int port)
{
	return $("%i.%i.%i.%i:%i",
		(0xff & addr),
		(0xff00 & addr) >> 8,
		(0xff0000 & addr) >> 16,
		(0xff000000 & addr) >> 24,
		port);
}


