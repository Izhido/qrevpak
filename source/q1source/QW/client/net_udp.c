/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// net_main.c

// >>> FIX: For Nintendo Wii using devkitPPC / libogc
// Including the device-specific network layer header:
#include <network.h>
// <<< FIX

#include "quakedef.h"

#include <sys/types.h>
// >>> FIX: For Nintendo Wii using devkitPPC / libogc
// Removing non-present headers:
//#include <sys/socket.h>
//#include <netinet/in.h>
//#include <netdb.h>
// <<< FIX
#include <sys/param.h>
// >>> FIX: For Nintendo Wii using devkitPPC / libogc
// Removing non-present headers:
//#include <sys/ioctl.h>
//#include <sys/uio.h>
//#include <arpa/inet.h>
// <<< FIX
#include <errno.h>

#if defined(sun)
#include <unistd.h>
#endif

#ifdef sun
#include <sys/filio.h>
#endif

#ifdef NeXT
#include <libc.h>
#endif

// >>> FIX: For Nintendo Wii using devkitPPC / libogc
// Adding stuff supposed to be present on the previously removed headers:
#define MAXHOSTNAMELEN	256
// <<< FIX

// >>> FIX: For Nintendo Wii using devkitPPC / libogc
// New variable containing the current IP address of the device:
extern char sys_ipaddress_text[16];
// <<< FIX

netadr_t	net_local_adr;

netadr_t	net_from;
sizebuf_t	net_message;
// >>> FIX: For Nintendo Wii using devkitPPC / libogc
// Unfortunately, this particular variable name clashes with a network lib function in libogc. 
// Renaming:
//int			net_socket;			// non blocking, for receives
int			net_socket_holder;			// non blocking, for receives
// <<< FIX
int			net_send_socket;	// blocking, for sends

// >>> FIX: For Nintendo Wii using devkitPPC / libogc
// The current network library has a limit in the number of bytes that can be sent/received:
//#define	MAX_UDP_PACKET	8192
#define	MAX_UDP_PACKET	4096
// <<< FIX
byte		net_message_buffer[MAX_UDP_PACKET];

int gethostname (char *, int);
int close (int);

//=============================================================================

void NetadrToSockadr (netadr_t *a, struct sockaddr_in *s)
{
	memset (s, 0, sizeof(*s));
	s->sin_family = AF_INET;

	*(int *)&s->sin_addr = *(int *)&a->ip;
	s->sin_port = a->port;
}

void SockadrToNetadr (struct sockaddr_in *s, netadr_t *a)
{
	*(int *)&a->ip = *(int *)&s->sin_addr;
	a->port = s->sin_port;
}

qboolean	NET_CompareBaseAdr (netadr_t a, netadr_t b)
{
	if (a.ip[0] == b.ip[0] && a.ip[1] == b.ip[1] && a.ip[2] == b.ip[2] && a.ip[3] == b.ip[3])
		return true;
	return false;
}


qboolean	NET_CompareAdr (netadr_t a, netadr_t b)
{
	if (a.ip[0] == b.ip[0] && a.ip[1] == b.ip[1] && a.ip[2] == b.ip[2] && a.ip[3] == b.ip[3] && a.port == b.port)
		return true;
	return false;
}

char	*NET_AdrToString (netadr_t a)
{
	static	char	s[64];
	
	sprintf (s, "%i.%i.%i.%i:%i", a.ip[0], a.ip[1], a.ip[2], a.ip[3], ntohs(a.port));

	return s;
}

char	*NET_BaseAdrToString (netadr_t a)
{
	static	char	s[64];
	
	sprintf (s, "%i.%i.%i.%i", a.ip[0], a.ip[1], a.ip[2], a.ip[3]);

	return s;
}

/*
=============
NET_StringToAdr

idnewt
idnewt:28000
192.246.40.70
192.246.40.70:28000
=============
*/
qboolean	NET_StringToAdr (char *s, netadr_t *a)
{
	struct hostent	*h;
	struct sockaddr_in sadr;
	char	*colon;
	char	copy[128];
	
	
	memset (&sadr, 0, sizeof(sadr));
	sadr.sin_family = AF_INET;
	
	sadr.sin_port = 0;

	strcpy (copy, s);
	// strip off a trailing :port if present
	for (colon = copy ; *colon ; colon++)
		if (*colon == ':')
		{
			*colon = 0;
			sadr.sin_port = htons(atoi(colon+1));	
		}
	
	if (copy[0] >= '0' && copy[0] <= '9')
	{
		*(int *)&sadr.sin_addr = inet_addr(copy);
	}
	else
	{
// >>> FIX: For Nintendo Wii using devkitPPC / libogc
// Switching to the equivalent function in the library:
		//if (! (h = gethostbyname(copy)) )
		if (! (h = net_gethostbyname(copy)) )
// <<< FIX
			return 0;
		*(int *)&sadr.sin_addr = *(int *)h->h_addr_list[0];
	}
	
	SockadrToNetadr (&sadr, a);

	return true;
}

// Returns true if we can't bind the address locally--in other words, 
// the IP is NOT one of our interfaces.
qboolean NET_IsClientLegal(netadr_t *adr)
{
	struct sockaddr_in sadr;
	int newsocket;

#if 0
	if (adr->ip[0] == 127)
		return false; // no local connections period

	NetadrToSockadr (adr, &sadr);

	if ((newsocket = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
		Sys_Error ("NET_IsClientLegal: socket:", strerror(errno));

	sadr.sin_port = 0;

	if( bind (newsocket, (void *)&sadr, sizeof(sadr)) == -1) 
	{
		// It is not a local address
		close(newsocket);
		return true;
	}
	close(newsocket);
	return false;
#else
	return true;
#endif
}


//=============================================================================

qboolean NET_GetPacket (void)
{
	int 	ret;
	struct sockaddr_in	from;
	int		fromlen;

	fromlen = sizeof(from);
// >>> FIX: For Nintendo Wii using devkitPPC / libogc
// 1) Unfortunately, this particular variable name clashes with a network lib function in libogc.
// 2) The function being called exists, but with a different name
// 3) Some data in the call must be set in a different way
// 4) Return value & errno are not the usual values
// Fixing:
	//ret = recvfrom (net_socket, net_message_buffer, sizeof(net_message_buffer), 0, (struct sockaddr *)&from, &fromlen);
	//if (ret == -1) {
	//	if (errno == EWOULDBLOCK)
	//		return false;
	//	if (errno == ECONNREFUSED)
	//		return false;
	Q_memset(&from, 0, fromlen);
	ret = net_recvfrom (net_socket_holder, net_message_buffer, sizeof(net_message_buffer), 0, (struct sockaddr *)&from, &fromlen);
	if (ret < 0) {
		if (ret == -EWOULDBLOCK)
			return false;
		if (ret == -ECONNREFUSED)
			return false;
// <<< FIX
		Sys_Printf ("NET_GetPacket: %s\n", strerror(errno));
		return false;
	}

	net_message.cursize = ret;
	SockadrToNetadr (&from, &net_from);

	return ret;
}

//=============================================================================

void NET_SendPacket (int length, void *data, netadr_t to)
{
	int ret;
	struct sockaddr_in	addr;

	NetadrToSockadr (&to, &addr);

// >>> FIX: For Nintendo Wii using devkitPPC / libogc
// 1) Unfortunately, this particular variable name clashes with a network lib function in libogc.
// 2) The function being called exists, but with a different name
// Renaming:
	//ret = sendto (net_socket, data, length, 0, (struct sockaddr *)&addr, sizeof(addr) );
	//if (ret == -1) {
	//	if (errno == EWOULDBLOCK)
	//		return;
	//	if (errno == ECONNREFUSED)
	//		return;
	int addr_size = 8;
	ret = net_sendto (net_socket_holder, data, length, 0, (struct sockaddr *)&addr, addr_size );
	if (ret < 0) {
		if (ret == -EWOULDBLOCK)
			return;
		if (ret == -ECONNREFUSED)
			return;
// <<< FIX
		Sys_Printf ("NET_SendPacket: %s\n", strerror(errno));
	}
}

//=============================================================================

int UDP_OpenSocket (int port)
{
	int newsocket;
	struct sockaddr_in address;
	qboolean _true = true;
	int i;

// >>> FIX: For Nintendo Wii using devkitPPC / libogc
// Switching to the equivalent function in the library:
	//if ((newsocket = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	//	Sys_Error ("UDP_OpenSocket: socket:", strerror(errno));
	//if (ioctl (newsocket, FIONBIO, (char *)&_true) == -1)
	//	Sys_Error ("UDP_OpenSocket: ioctl FIONBIO:", strerror(errno));
	if ((newsocket = net_socket (PF_INET, SOCK_DGRAM, IPPROTO_IP)) < 0)
		Sys_Error ("UDP_OpenSocket: socket: %s", errno, strerror(errno));
	if (net_ioctl (newsocket, FIONBIO, (char *)&_true) < 0)
		Sys_Error ("UDP_OpenSocket: ioctl FIONBIO: %s", strerror(errno));
// <<< FIX
	address.sin_family = AF_INET;
//ZOID -- check for interface binding option
	if ((i = COM_CheckParm("-ip")) != 0 && i < com_argc) {
		address.sin_addr.s_addr = inet_addr(com_argv[i+1]);
		Con_Printf("Binding to IP Interface Address of %s\n",
				inet_ntoa(address.sin_addr));
	} else
		address.sin_addr.s_addr = INADDR_ANY;
	if (port == PORT_ANY)
		address.sin_port = 0;
	else
		address.sin_port = htons((short)port);
// >>> FIX: For Nintendo Wii using devkitPPC / libogc
// Do not bind the socket if port == 0 (part 1):
	if(port > 0) 
	{
// <<< FIX
// >>> FIX: For Nintendo Wii using devkitPPC / libogc
// Switching to the equivalent function in the library:
	//if( bind (newsocket, (void *)&address, sizeof(address)) == -1)
	if( net_bind (newsocket, (void *)&address, sizeof(address)) < 0)
// <<< FIX
		Sys_Error ("UDP_OpenSocket: bind: %s", strerror(errno));
// >>> FIX: For Nintendo Wii using devkitPPC / libogc
// Do not bind the socket if port == 0 (part 2):
	}
// <<< FIX

	return newsocket;
}

void NET_GetLocalAddress (void)
{
// >>> FIX: For Nintendo Wii using devkitPPC / libogc
// Since there's no gethosname() or getsockname() in here, paste the IP address text:
	//char	buff[MAXHOSTNAMELEN];
	//struct sockaddr_in	address;
	//int		namelen;

	//gethostname(buff, MAXHOSTNAMELEN);
	//buff[MAXHOSTNAMELEN-1] = 0;

	//NET_StringToAdr (buff, &net_local_adr);

	//namelen = sizeof(address);
	//if (getsockname (net_socket, (struct sockaddr *)&address, &namelen) == -1)
	//	Sys_Error ("NET_Init: getsockname:", strerror(errno));
	//net_local_adr.port = address.sin_port;
	NET_StringToAdr (sys_ipaddress_text, &net_local_adr);
	net_local_adr.port = 80;
// <<< FIX

	Con_Printf("IP address %s\n", NET_AdrToString (net_local_adr) );
}

/*
====================
NET_Init
====================
*/
void NET_Init (int port)
{
	//
	// open the single socket to be used for all communications
	//
// >>> FIX: For Nintendo Wii using devkitPPC / libogc
// Unfortunately, this particular variable name clashes with a network lib function in libogc. 
// Renaming:
	//net_socket = UDP_OpenSocket (port);
	net_socket_holder = UDP_OpenSocket (port);
// <<< FIX

	//
	// init the message buffer
	//
	net_message.maxsize = sizeof(net_message_buffer);
	net_message.data = net_message_buffer;

	//
	// determine my name & address
	//
	NET_GetLocalAddress ();

	Con_Printf("UDP Initialized\n");
}

/*
====================
NET_Shutdown
====================
*/
void	NET_Shutdown (void)
{
// >>> FIX: For Nintendo Wii using devkitPPC / libogc
// Unfortunately, this particular variable name clashes with a network lib function in libogc. 
// Renaming. Also, switching to the equivalent function in the library:
	//close (net_socket);
	net_close (net_socket_holder);
// <<< FIX
}

