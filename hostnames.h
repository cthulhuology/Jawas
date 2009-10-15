// hostnames.h
//
// © 2009 David J. Goehrig
// All Rights Reserved
//

#ifndef __HOSTNAMES_H__
#define __HOSTNAMES_H__

typedef in_addr_t  IPAddress;

extern struct sockaddr_in Address;
extern size_t Address_len;

IPAddress* lookup(str host);
int attachTo(IPAddress addr, int port);

#endif
