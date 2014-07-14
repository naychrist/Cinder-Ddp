#pragma once

#include "UdpSession.h"
#include "UdpClient.h"

#include "cinder/Utilities.h"

namespace ddp
{
    using namespace std;
    using namespace ci;
    using namespace ci::app;

    // DDP protocol header definitions
    typedef unsigned char byte;
    #define DDP_PORT    4048
    #define MAX_DBUFLEN 1512


    // DDP header format
    // header is 10 bytes
    struct ddp_hdr_struct {
        byte flags1;
        byte reserved1;
        byte type;
        byte device;
        byte offset1;
        byte offset2;
        byte offset3;
        byte offset4;
        byte len1;
        byte len2;
    };

    
    class Sender
    {
    public:

        ddp_hdr_struct* ddp_header, ddp_header_query, ddp_header_push;
        byte            dbuf[MAX_DBUFLEN];

        string          ip;
        UdpClientRef    client;
        UdpSessionRef   udpSession;

        
        void setup( AppNative *_app, string _ip, bool _requirePush = false)
        {
            ip = _ip;
            
            client = UdpClient::create( _app->io_service() );

            client->connectConnectEventHandler( &ddp::Sender::onConnect, this );
            client->connectErrorEventHandler( &ddp::Sender::onError, this );
            client->connectResolveEventHandler( [ & ]()
                                                {
                                                    console()<< "Endpoint resolved"<<endl;
                                                } );

            // create packet header
            ddp_header = (struct ddp_hdr_struct *) &dbuf;
            _requirePush ? ddp_header->flags1 = 0x41 : ddp_header->flags1 = 0x41;       //tood: v1 || v1 + PUSH
            ddp_header->reserved1 = 0x00;
            ddp_header->type = 0x00;
            ddp_header->device = 0x01;


        }

        
        void update( unsigned char * _data, int _length, int _offset = 0)
        {
            setLength(_length);
            setOffset(_offset);

            if ( _data && udpSession && udpSession->getSocket()->is_open() ) {

                //send packet
                memcpy(&dbuf[0]+10,_data,_length);

                Buffer buffer(10+_length);
                buffer.copyFrom(&dbuf, 10+ _length);
                udpSession->write( buffer );


                for (int i=0; i<10+_length; i++) {

                    if (i==9) {
                        cout<<(int)dbuf[i]<<endl;
                    }
                    else {
                        cout<<(int)dbuf[i]<<",";
                    }
                }
                cout<<endl<<endl;

            }
        }
        
        void connect()
        {
            console() << "Connecting to: " + ip + ":" + toString( DDP_PORT ) <<endl;
            client->connect( ip, DDP_PORT);
        }
        
        void close()
        {
            //todo: disconnect
        }

        void write()
        {
            if ( udpSession && udpSession->getSocket()->is_open() ) {
                string test = "test";
                Buffer buffer = UdpSession::stringToBuffer( test );
                udpSession->write( buffer );
            }
        }

        /*

        void getStatus()
        {

            ddp_hdr_struct  *header;
            //todo: create header
            Buffer buffer(10);
            buffer.copyFrom(&header, 10);

            udpSession->write( buffer );
            udpSession->read();

        }

        void onRead( ci::Buffer buffer )
        {
            console()<< toString( buffer.getDataSize() ) << " bytes read" <<endl;
            string response	= UdpSession::bufferToString( buffer );
            console()<< response <<endl;

        }

        void onReadComplete()
        {
            console()<< "Read complete" <<endl;
        }

        */

    private:

        void setOffset(int _offset)
        {
            ddp_header->offset1 = (_offset >> 24) & 0xff;
            ddp_header->offset2 = (_offset >> 16) & 0xff;
            ddp_header->offset3 = (_offset >> 8) & 0xff;
            ddp_header->offset4 = _offset & 0xff;
        }

        void setLength(int _length)
        {
            if (_length > 1440) _length = 1440;
            ddp_header->len1 = (_length >> 8) & 0xff;
            ddp_header->len2 = _length & 0xff;
        }

        
#pragma Mark - Events
        
        void onConnect( UdpSessionRef session )
        {
            console()<<"Should be connected..."<<endl;
            udpSession = session;
            udpSession->connectErrorEventHandler( &ddp::Sender::onError, this );
            udpSession->connectWriteEventHandler( &ddp::Sender::onWrite, this );

            udpSession->getSocket()->set_option(boost::asio::socket_base::broadcast(true));

//            udpSession->connectReadCompleteEventHandler( &ddp::Sender::onReadComplete, this );
//            udpSession->connectReadEventHandler( &ddp::Sender::onRead, this );

        }
        
        void onError( std::string err, size_t bytesTransferred )
        {
            string text = "Error";
            if ( !err.empty() ) text += ": " + err;
            console() << text <<endl;
        }
        
        void onWrite( size_t bytesTransferred )
        {
            console() << toString( bytesTransferred ) + " bytes written"<<endl;
        }


    };//class Sender
}
