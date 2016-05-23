/*
	MirrorServerDepreciated.cpp
	(c)2005 Palestar Inc, Richard Lyle
*/

#define NETWORK_DLL
#include "File/FileDisk.h"
#include "Network/FileSocket.h"
#include "Network/MirrorServer.h"



const int		SEND_FILE_QUEUE = 1;			// how many files to put into the send buffer before waiting for handshakes
const dword		SEND_FILE_CHUNK_SIZE = 1024 * 4;

//----------------------------------------------------------------------------

enum OldMessages
{
	CLIENT_LOGIN			= 0x0,
	CLIENT_RECV_CRC			= 0x1,		// Server is sending the catalog crc
	CLIENT_RECV_CATALOG		= 0x2,		// Server is sending catalog
	CLIENT_RECV_FILE		= 0x3,		// Server is sending file (depreciated)
	CLIENT_RECV_FILE2		= 0x8,		// server is send a file
	CLIENT_RECV_FILE3		= 0x9,		// server is send a file
	CLIENT_RECV_DONE		= 0x4,		// server is done sending client all files
	CLIENT_FILE_LOCK		= 0x6,		// SERVER_LOCK_FILE
	CLIENT_FILE_RECEIVED	= 0x7,		// Server has received the file	

	SERVER_LOGIN			= 0x80,		// Client login
	SERVER_SEND_CRC			= 0x81,		// Client is requesting the catalog crc
	SERVER_SEND_CATALOG		= 0x83,		// Client requesting catalog
	SERVER_SEND_FILES		= 0x84,		// Client is requesting file (depreciated)
	SERVER_SEND_FILES2		= 0x89,		// client is requesting files with handshake
	SERVER_SEND_FILES3		= 0x8a,		// client is requesting files
	SERVER_RECV_FILE		= 0x85,		// Client is sending a file
	SERVER_LOCK_FILE		= 0x86,		// Client requests server lock a file
	SERVER_UNLOCK_FILE		= 0x87,		// Client is unlocking a file
	SERVER_FILE_RECEIVED	= 0x88,		// client has received the file

	HANDSHAKE				= 0xff,
};

struct OldItem
{
	CharString	name;			// file name
	dword		crc;			// file time
	dword		size;			// size of file in bytes

	OldItem & operator=( const OldItem & copy )
	{
		name = copy.name;
		crc = copy.crc;
		size = copy.size;
		return *this;
	}
};

inline OutStream & operator<<(OutStream &output, const OldItem & item )
{
	output << item.name;
	output << item.crc;
	output << item.size;
	return output;
}


void MirrorServer::onReceiveDepreciated( dword client, byte message, InStream & input )
{
	log( CharString().format("Client %u sent depreciated message, message = %d", client, message ) );

	switch( message )
	{
	case SERVER_LOGIN:
		{
			dword job;
			input >> job;
			CharString id;
			input >> id;
			CharString md5;
			input >> md5;

			if ( login( client, id, md5 ) )
			{
				// log a message
				log( CharString().format("Client %u logged in as '%s'", client, (const char *)id ) );
				OutStream output( send( client, CLIENT_LOGIN ) );
				output << job << true;
			}
			else
			{
				log( CharString().format("Client %u failed to login!", client) );
				OutStream output( send( client, CLIENT_LOGIN ) );
				output << job << false;
			}
		}
		break;
	case SERVER_SEND_CRC:
		{
			dword catalogCRC = 0;

			dword job;
			input >> job;

			lock();
			CatalogIt it = m_Catalog.head();
			while( it.valid() )
			{
				catalogCRC += (*it).crc;
				it.next();
			}
			unlock();

			// the client determines if an error occured if the value is negative, clear to high bit so it doesn't think an error occured
			catalogCRC &= 0x7fffffff;		
			OutStream output( send( client, CLIENT_RECV_CRC ) );
			output << job << catalogCRC;
		}
		break;
	case SERVER_SEND_CATALOG:
		{
			lock();

			Tree< CharString, OldItem > catalog;

			CatalogIt it = m_Catalog.head();
			while( it.valid() )
			{
				OldItem item;
				item.name = (*it).name;
				item.crc = (*it).crc;
				item.size = (*it).size;

				catalog[ item.name ] = item;
				it.next();
			}

			log( CharString().format("Client %u requesting catalog", client) );
			OutStream output( send( client, CLIENT_RECV_CATALOG ) );
			output << catalog;

			unlock();
		}
		break;
	case SERVER_SEND_FILES:
		{
			Array< CharString > fileList;
			input >> fileList;

			log( CharString().format("Client %u requested %d files", client, fileList.size()) );

			dword bytesSent = 0;
			if ( canDownload( client ) )
			{
				log( CharString().format("Sending %u files to client %u", fileList.size(), client ) );

				int fileQueue = 0;
				for(int i=0;i<fileList.size();i++)
				{
					lock();
					CatalogIt find = m_Catalog.find( CharString( fileList[i] ) );
					if ( find.valid() )
					{
						OldItem item;
						item.name = (*find).name;
						item.crc = (*find).crc;
						item.size = (*find).size;

						unlock();

						// load and compress this file
						CharString localFile( m_Context.mirror + CharString(item.name) );
						if ( FileDisk::fileExists( localFile ) )
						{
							FileDisk file;
							if ( file.open( localFile, FileDisk::READ ) )
							{
								// get the latest size of the file
								item.size = file.size();
								
								OutStream output( send( client, CLIENT_RECV_FILE ) );
								output << item;

								// read the file into a buffer for sending
								byte * pBuffer = new byte[ item.size ];
								file.read( pBuffer, item.size  );

								try {
									output.write( pBuffer, item.size );
									output.flush();
								}
								catch( FileSocket::FileError )
								{}

								// release the local buffer
								delete [] pBuffer;

								fileQueue++;
								if ( fileQueue >= SEND_FILE_QUEUE )
								{
									fileQueue--;

									// wait for entire file to be received by the client
									byte fileReceived;
									input >> fileReceived;

									// stop sending if byte received is not the correct code
									if ( fileReceived != SERVER_FILE_RECEIVED )
										break;
								}

								// update the total bytes sent
								bytesSent += item.size;
							}
							else
								log( CharString().format("Failed to open local file %s!", (const char *)localFile) );
						}
						else
							log( CharString().format("Failed to find local file %s!", (const char *)localFile) );
					}
					else
					{
						unlock();
						log( CharString().format("Client requested missing file %s!", (const char *)fileList[i]) );
					}

					// wait for any files currently in the send buffer to be received
					while ( fileQueue > 0 )
					{
						fileQueue--;

						byte fileReceived;
						input >> fileReceived;

						if ( fileReceived != SERVER_FILE_RECEIVED )
							break;
					}
				}

				// send done message to client
				send( client, CLIENT_RECV_DONE );

				log( CharString().format("Sent %u files (%u bytes) to client %u", fileList.size(), bytesSent, client ) );
			}
			else
			{
				log( CharString().format("Client %u cannot download!", client) );
				removeClient( client );
			}
		}
		break;
	case SERVER_SEND_FILES2:
		{
			Array< CharString > fileList;
			input >> fileList;

			log( CharString().format("Client %u requested %d files", client, fileList.size()) );

			dword bytesSent = 0;
			if ( canDownload( client ) )
			{
				log( CharString().format("Sending %u files to client %u", fileList.size(), client ) );

				bool bDisconnected = false;
				for(int i=0;i<fileList.size() && !bDisconnected;i++)
				{
					lock();
					CatalogIt find = m_Catalog.find( CharString(fileList[i]) );
					if ( find.valid() )
					{
						OldItem item;
						item.name = (*find).name;
						item.crc = (*find).crc;
						item.size = (*find).size;

						unlock();

						// load and compress this file
						CharString localFile( m_Context.mirror + CharString(item.name) );
						if ( FileDisk::fileExists( localFile ) )
						{
							FileDisk file;
							if ( file.open( localFile, FileDisk::READ ) )
							{
								// get the latest size of the file
								item.size = file.size();
								// create the output stream
								OutStream output( send( client, CLIENT_RECV_FILE2 ) );
								// send the item we're sending
								output << item;

								// read the file into a buffer for sending
								byte * pBuffer = new byte[ item.size ];
								file.read( pBuffer, item.size  );
								
								// use a try/catch so that we can delete the buffer if the client disconnects
								try {
									// send the file data in chunks
									dword nSent = 0;
									while( nSent < item.size )
									{
										dword nRemain = item.size - nSent;
										dword nSend = nRemain > SEND_FILE_CHUNK_SIZE ? SEND_FILE_CHUNK_SIZE : nRemain;

										// send the number of bytes were sending
										output << nSend;
										// send the file data
										output.write( pBuffer + nSent, nSend );
										output.flush();

										nSent += nSend;

										// get the handshake packet from the client for each chunk sent
										byte handShake;
										input >> handShake;

										if ( handShake != HANDSHAKE )
										{
											log( CharString().format("Client %u failed handshake while sending %s...", 
												client, (const char *)localFile ) );
											break;
										}
									}
								}
								catch( FileSocket::FileError )
								{
									log( CharString().format("Client %u disconnected while sending %s...", client, (const char *)localFile) );
									bDisconnected = true;
								}

								// release the local buffer
								delete [] pBuffer;

								// update the total bytes sent
								bytesSent += item.size;
							}
							else
								log( CharString().format("Failed to open local file %s!", (const char *)localFile) );
						}
						else
							log( CharString().format("Failed to find local file %s!", (const char *)localFile) );
					}
					else
					{
						unlock();
						log( CharString().format("Client requested missing file %s!", (const char *)fileList[i]) );
					}
				}	// end for

				// send done message to client
				send( client, CLIENT_RECV_DONE );

				log( CharString().format("Sent %u files (%u bytes) to client %u", fileList.size(), bytesSent, client ) );
			}
			else
			{
				log( CharString().format("Client %u cannot download!", client) );
				removeClient( client );
			}
		}
		break;
	case SERVER_SEND_FILES3:
		{
			Array< CharString > fileList;
			input >> fileList;

			log( CharString().format("Client %u requested %d files", client, fileList.size()) );

			dword bytesSent = 0;
			if ( canDownload( client ) )
			{
				log( CharString().format("Sending %u files to client %u", fileList.size(), client ) );

				bool bDisconnected = false;
				for(int i=0;i<fileList.size() && !bDisconnected;i++)
				{
					lock();
					CatalogIt find = m_Catalog.find( CharString( fileList[i] ) );
					if ( find.valid() )
					{
						OldItem item;
						item.name = (*find).name;
						item.crc = (*find).crc;
						item.size = (*find).size;
						unlock();

						// load and compress this file
						CharString localFile( m_Context.mirror + CharString(item.name) );
						if ( FileDisk::fileExists( localFile ) )
						{
							FileDisk file;
							if ( file.open( localFile, FileDisk::READ ) )
							{
								// get the latest size of the file
								item.size = file.size();
								// create the output stream
								OutStream output( send( client, CLIENT_RECV_FILE3 ) );
								// send the item we're sending
								output << item;

								// read the file into a buffer for sending
								byte * pBuffer = new byte[ item.size ];
								file.read( pBuffer, item.size  );
								
								// use a try/catch so that we can delete the buffer if the client disconnects
								try {
									// send the file data in chunks
									dword nSent = 0;
									while( nSent < item.size )
									{
										dword nRemain = item.size - nSent;
										dword nSend = nRemain > SEND_FILE_CHUNK_SIZE ? SEND_FILE_CHUNK_SIZE : nRemain;

										// send the number of bytes were sending
										output << nSend;
										// send the file data
										output.write( pBuffer + nSent, nSend );
										output.flush();

										nSent += nSend;
									}
								}
								catch( FileSocket::FileError )
								{
									log( CharString().format("Client %u disconnected while sending %s...", client, (const char *)localFile) );
									bDisconnected = true;
								}

								// release the local buffer
								delete [] pBuffer;

								// update the total bytes sent
								bytesSent += item.size;
							}
							else
								log( CharString().format("Failed to open local file %s!", (const char *)localFile) );
						}
						else
							log( CharString().format("Failed to find local file %s!", (const char *)localFile) );
					}
					else
					{
						unlock();
						log( CharString().format("Client requested missing file %s!", (const char *)fileList[i]) );
					}
				}	// end for

				// send done message to client
				send( client, CLIENT_RECV_DONE );

				log( CharString().format("Sent %u files (%u bytes) to client %u", fileList.size(), bytesSent, client ) );
			}
			else
			{
				log( CharString().format("Client %u cannot download!", client) );
				removeClient( client );
			}
		}
		break;
	default:
		log( CharString().format("bad data type %s, removing client!", (const char *)clientAddress(client)) );
		removeClient( client );
		break;
	}
}

//----------------------------------------------------------------------------
// EOF
