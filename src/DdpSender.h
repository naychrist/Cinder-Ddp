#pragma once

#include "UdpSession.h"
#include "UdpClient.h"
#include "cinder/app/AppNative.h"

namespace ddp
{

    // DDP protocol header definitions
    typedef unsigned char byte;
    #define DDP_PORT        4048
    #define MAX_DBUFLEN     1512
    #define DDP_MAX_DATALEN (480*3)


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

        void setup( std::string _ip, bool _requirePush = false);
        void update( unsigned char * _data, int _length, int _offset = 0);
        
        void connect();
        void close();
        bool getIsConnected();

        void push();
        void getStatus();


    private:

        ddp_hdr_struct* ddp_header;
        ddp_hdr_struct* ddp_header_query;
        ddp_hdr_struct* ddp_header_push;
        byte            dbuf[MAX_DBUFLEN];

        std::string     ip;
        UdpClientRef    client;
        UdpSessionRef   udpSession;

        void setOffset(ddp_hdr_struct* _header, int _offset);
        int  setLength(ddp_hdr_struct* _header, int _length);
        
        void onConnect( UdpSessionRef session );
        void onError( std::string err, size_t bytesTransferred );
        void onWrite( size_t bytesTransferred );
        void onRead( ci::Buffer buffer );
        void onReadComplete();


    };//class Sender
}
