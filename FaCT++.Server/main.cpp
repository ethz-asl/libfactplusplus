/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2005-2007 by Matthew Horridge, Dmitry Tsarkov

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/


#include <iostream>
#include <string>
#include <sstream>

#ifdef __WIN32
#	include <winsock.h>
#else
#	include <sys/types.h>
#	include <sys/socket.h>
#	include <netinet/in.h>
#	include <arpa/inet.h>
#	include <unistd.h>
#endif

#include "DIGinterface.h"

DIGInterface digInterface;

int portNumber = 3490;
#define TYPE_POST = 2;
#define TYPE_GET = 3;


std::string GET_RESPONSE = "<html><head></head><body><p>FaCT++ DIG Server running.</p></body></html>";

/********************************************************************************
*
* Reads an HTTP header (and possibly part of the content) from a client which
* is connected through the speicified socket.
*
* socket - the socket descriptor
* Header - a std::string in which the HTTP header will be placed
* Content - a std::string in which any read content will be placed
*
* returns - false if the header was read, true if error occured.
*
********************************************************************************/

bool readHeader ( int socket, std::string& Header, std::string& Content )
{
	const int BUFFER_SIZE = 64;
	char buffer[BUFFER_SIZE];
	int endOfHeaderIndex = -1;
	int read = 0;
	std::string temp;

	while ( endOfHeaderIndex == -1 )
	{
		memset(buffer, 0, BUFFER_SIZE);
		int rec = (int) recv ( socket, buffer, BUFFER_SIZE-1, 0 );
		if ( rec <= 0 )
		{
			std::cout << "ReadHeader: Connection closed by client." << std::endl;
			return true;
		}
		read += rec;
		temp += buffer;
		endOfHeaderIndex = temp.find("\r\n\r\n");
	}

	endOfHeaderIndex += 3;	// take into account EOL
	Header = temp.substr ( 0, endOfHeaderIndex );
	// part of the content is in the TEMP now; save it into CONTENT
	int readContentLength = read - endOfHeaderIndex;
	if ( readContentLength > 0 )
		Content = temp.substr ( endOfHeaderIndex, readContentLength );

	return false;
}

/********************************************************************************
*
* Determines an HTTP request is a POST request.
*
* Header - a std::string that contains the HTTP header
* returns - true if the request is a POST request or false if the request is
* not a POST request.
*
********************************************************************************/

inline bool isPost ( const std::string& Header )
{
	int endOfFirstLine = Header.find("\r\n");
	int indexOfPost = Header.find("POST");
	return indexOfPost != -1 && indexOfPost < endOfFirstLine;
}


/********************************************************************************
*
* Gets an integer HTTP header value from the specified header.
*
* name - the name of the header value
* Header - a std::string that contains the header
*
* returns - an integer that represents the value of the header.
*
********************************************************************************/

int getIntHeaderValue ( const std::string& name, const std::string& Header )
{
	int nameIndex = Header.find(name);
	if ( nameIndex == -1 )
		return -1;

	int colonIndex = Header.find ( ":", nameIndex );
	int endOfLineIndex = Header.find ( "\r\n", colonIndex );
	int valueLength = endOfLineIndex - colonIndex - 1;
	return atoi(Header.substr(colonIndex + 1, valueLength).c_str());
}

/********************************************************************************
*
* Reads the content of the specified content length from the specified client.
* @return true if error occured
* This method should be called after readHeader.
*
********************************************************************************/
bool readContent ( int socket, std::string& Content, int length )
{
	const int BUFFER_SIZE = 2048;
	char buffer[BUFFER_SIZE];
	int read = Content.length();

	while ( read < length )
	{
		memset ( buffer, 0, BUFFER_SIZE );
		int rec = (int) recv (socket, buffer, BUFFER_SIZE-1, 0);
		if ( rec <= 0 )
		{
			std::cout << "ReadContent: Connection closed by client." << std::endl;
			return true;
		}
		read += rec;
		Content += buffer;
	}
	return false;
}

/// send CONTENT of length TOSEND to a SOCKET. @return true if error occures
bool sendLine ( int socket, const char* content, unsigned int toSend )
{
	while ( toSend > 0 )
	{
		int sent = (int) send ( socket, content, toSend, 0);
		if ( sent <= 0 )
		{
			std::cout << "SendLine: Connection closed by client." << std::endl;
			return true;
		}
		content += sent;
		toSend -= sent;
	}
	return false;
}

/********************************************************************************
 *
 * Sends the response held at Content to the client, writing out the necessary
 * HTTP headers such as Content-Length.
 *
 ********************************************************************************/
bool sendResponse ( int socket, const std::string& Content )
{
	std::stringstream buf;
	buf << "HTTP/1.1 200 OK\r\nContent-Type: application/xml\r\n"
		<< "Content-Length: " << Content.length() << "\r\n\r\n";
	const std::string& header = buf.str();

	if ( sendLine ( socket, header.c_str(), header.length() ) )
		return true;
	if ( sendLine ( socket, Content.c_str(), Content.length() ) )
		return true;
	return false;
}

void Usage ( void )
{
	std::cout << "\nUsage: FaCT++.Server [-port <port>]\n where <port> is an HTTP port used for DIG connecntion.";
	exit(1);
}

/// read request from SOCKET to HEADER:CONTENT, answer if necessary
void
singleTransaction ( int socket, std::string& header, std::string& content )
{
	// try to read header of a request
	if ( readHeader ( socket, header, content ) )
		return;

	// if not a post -- nothing to do
	if ( !isPost(header) )
	{
		// The request isn't a POST request - send back some HTML
		// saying FaCT++ is running as a DIG server.
		sendResponse ( socket, GET_RESPONSE );
		return;
	}

	// post request: read content
	int contentLenght = getIntHeaderValue ( "Content-Length", header );
	if ( contentLenght < 0 )
		return;
	if ( readContent ( socket, content, contentLenght ) )
		return;

	// We don't want to send any leading white space, so
	// find the first non whitespace
	const char* p = content.c_str();
	const char* p_end = p+content.length();
	for ( ; p < p_end; ++p )
		if ( *p != ' ' && *p != '\t' && *p != '\n' && *p != '\r' )
			break;

	std::string reasonerResponse;
	digInterface.processQuery ( p, reasonerResponse );
	sendResponse ( socket, reasonerResponse );
}

/// create a connection, then make a recieve-send transaction
void
runSingleSession ( int sockfd, std::string& header, std::string& content )
{
#ifdef __WIN32
	typedef int socklen_t;	// simplify ACCEPT call
#endif
	sockaddr their_addr;
	socklen_t sin_size = sizeof(their_addr);
	int new_fd = accept ( sockfd, &their_addr, &sin_size );

	header.clear();
	content.clear();
	singleTransaction ( new_fd, header, content );

	// Response is sent, so close the connection
	// (different depending on whether the platform is Windows or not)
#ifdef __WIN32
	closesocket(new_fd);
#else
	close(new_fd);
#endif
}

int main ( int argc, char * const argv[] )
{
	// check for the given value of the port
	if ( argc == 3)
	{
		if ( strncmp ( argv[1], "-port", 5 ) )
			Usage();
		else
			portNumber = atoi(argv[2]);
	}
	else if ( argc != 1 )
		Usage();

	// If the platform is Windows then we need to initialise Winsock
#ifdef __WIN32
	WSADATA wsaData; // if this doesn't work
	//	WSAData wsaData; // then try this instead
	if ( WSAStartup ( MAKEWORD(1,1), &wsaData ) != 0 )
	{
		std::cerr << "WSAStartup failed.\n";
		exit(1);
	}
#endif

	int sockfd;
	struct sockaddr_in my_addr;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(portNumber);
	my_addr.sin_addr.s_addr = INADDR_ANY; // Use localhost
	memset(&(my_addr.sin_zero),'\0', 8);

	// Attempt to bind to the required port - if there is an error, print an
	// error message and exit with an error code.
	if ( bind ( sockfd, (struct sockaddr*)&my_addr, sizeof(struct sockaddr) ) == -1 )
	{
		std::cout << "ERROR: Could not bind to port " << portNumber << std::endl;
		exit(1);
	}

#ifndef __WIN32
	int reuseSocket = 1;
	if ( setsockopt ( sockfd, SOL_SOCKET, SO_REUSEADDR, &reuseSocket, sizeof(int) ) == -1 )
	{
		perror("setsockopt");
		exit(1);
	}
#endif
	if ( listen ( sockfd, 5 ) != -1 )
	{
		char hostName[64];
		memset(hostName, 0, 64);
		gethostname(hostName, 64);
		std::cout << "HTTP Interface Copyright (C) Matthew Horridge 2005" << std::endl;
		std::cout << "FaCT++ running on " << hostName << " port " << portNumber << std::endl;
	}

	// allocate header and content lines once
	std::string header, content;

	for(;;)
		runSingleSession ( sockfd, header, content );

	return 0;
}
