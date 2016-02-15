#include "DdpSender.h"
#include "cinder/Utilities.h"

namespace ddp
{
    using namespace std;
    using namespace ci;
    using namespace ci::app;

    void Sender::setup( string _ip, bool _requirePush)
    {
        ip = _ip;

        client = UdpClient::create( ci::app::getWindow()->getApp()->io_service() );

        client->connectConnectEventHandler( &ddp::Sender::onConnect, this );
        client->connectErrorEventHandler( &ddp::Sender::onError, this );
        client->connectResolveEventHandler( [ & ]()
        {
            console()<< "Endpoint resolved"<<endl;
        } );

        // create packet header
        ddp_header = (struct ddp_hdr_struct *) &dbuf;
        _requirePush ? ddp_header->flags1 = 0x40 : ddp_header->flags1 = 0x41;       //v1 || v1 + push
        ddp_header->reserved1 = 0x00;
        ddp_header->type = 0x00;
        ddp_header->device = 0x01;

        if (_requirePush)
        {
            ddp_header_push = new ddp_hdr_struct;
            ddp_header_push->flags1 = 0x41;       //v1 + push
            ddp_header_push->reserved1 = 0x00;
            ddp_header_push->type = 0x00;
            ddp_header_push->device = 0x01;
            setLength(ddp_header_push, 0);
            setOffset(ddp_header_push, 0);

        }


    }


    void Sender::update( unsigned char * _data, int _length, int _offset)
    {
        int length = setLength(ddp_header, _length);
        setOffset(ddp_header, _offset);

        if ( _data && getIsConnected() ) {

            //send packet
            memcpy(&dbuf[0]+10,_data,length);

            BufferRef buffer = Buffer::create(10+_length);
            buffer->copyFrom(&dbuf, 10+ length);
            udpSession->write( buffer );

        }
    }

    void Sender::connect()
    {
        console() << "Connecting to: " + ip + ":" + toString( DDP_PORT ) <<endl;
        client->connect( ip, DDP_PORT);
    }

    void Sender::close()
    {
        if ( getIsConnected() )
        {
            udpSession->getSocket()->close();
            console() << "Closed: " + ip + ":" + toString( DDP_PORT ) <<endl;
        }
    }

    bool Sender::getIsConnected()
    {
        return ( udpSession && udpSession->getSocket()->is_open() );
    }

    void Sender::push()
    {
        if (ddp_header_push && getIsConnected() )
        {
            BufferRef buffer = Buffer::create(10);
            buffer->copyFrom(&ddp_header_push[0], 10);
            udpSession->write( buffer );

        }
        else
        {
            console() << "Error: DDP not set up to require push!" <<endl;
        }

    }

    void Sender::getStatus()
    {
        if ( getIsConnected() )
        {
            if (!ddp_header_query)
            {
                ddp_header_query = new ddp_hdr_struct;
                ddp_header_query->flags1 = 0x43;       //v1 + push + query
                ddp_header_query->reserved1 = 0x00;
                ddp_header_query->type = 0x00;
                ddp_header_query->device = 251; //read status
                setLength(ddp_header_push, 0);
                setOffset(ddp_header_push, 0);
            }
            BufferRef buffer = Buffer::create(10);
            buffer->copyFrom(&ddp_header_query[0], 10);

            udpSession->write( buffer );
            udpSession->read();
        }


    }

    void Sender::setOffset(ddp_hdr_struct* _header, int _offset)
    {
        _header->offset1 = (_offset >> 24) & 0xff;
        _header->offset2 = (_offset >> 16) & 0xff;
        _header->offset3 = (_offset >> 8) & 0xff;
        _header->offset4 = _offset & 0xff;
    }

    int Sender::setLength(ddp_hdr_struct* _header, int _length)
    {
        if (_length > DDP_MAX_DATALEN) {
            _length = DDP_MAX_DATALEN;
            console() << "Error: shortened DDP packet data length!" << endl;
        }
        _header->len1 = (_length >> 8) & 0xff;
        _header->len2 = _length & 0xff;
        return _length;
    }

    void Sender::onConnect( UdpSessionRef session )
    {
        console()<<"Should be connected..."<<endl;
        udpSession = session;
        udpSession->connectErrorEventHandler( &ddp::Sender::onError, this );
        udpSession->connectWriteEventHandler( &ddp::Sender::onWrite, this );

        udpSession->getSocket()->set_option(asio::socket_base::broadcast(true));

        udpSession->connectReadCompleteEventHandler( &ddp::Sender::onReadComplete, this );
        udpSession->connectReadEventHandler( &ddp::Sender::onRead, this );

    }

    void Sender::onError( std::string err, size_t bytesTransferred )
    {
        string text = "Error";
        if ( !err.empty() ) text += ": " + err;
        console() << text <<endl;
    }

    void Sender::onWrite( size_t bytesTransferred )
    {
//        console() << toString( bytesTransferred ) + " bytes written"<<endl;
    }

    void Sender::onRead( ci::BufferRef buffer )
    {
        console()<< toString( buffer->getSize() ) << " bytes read" <<endl;
        string response	= UdpSession::bufferToString( buffer );
        console()<< response <<endl;

    }

    void Sender::onReadComplete()
    {
        console()<< "Read complete" <<endl;
    }
}//namespace ddp