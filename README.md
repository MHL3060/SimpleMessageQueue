#Simple Message Queue


this just a pet project to stream data from client to server or server broadcast to client for now.

There are a lot of things need to be implemented before it can be useful. 
like routing, persistence. 

Currently, I use this one to stream NRSC5 HD radio stream from my raspberry pi to my another raspberry speaker

This is just a very simple Message Queue. It is able to receive wav file and play it 
on the server side.

This project depends on libjasson, libao library.

the apache avro library will be build if it doesn't exist on the machine.


## Flow.

the message format

the first 4 bytes is reserved for payload size in network order.
the the payload size. 
right after the payload there is a 7 byte magic payload 
{ 0xBA, 0xDB, 0xEE, 0xFB, 0xAD, 0xF0, 0x0D}

[payloadsize (4bytes), payload, magicpayload(7bytes)]


the payload must a avro byte stream. the avro schema current defined in message.h
```json
{
  "namespace": "simple.message.queue.avro",
  "type": "record",
  "name" : "message",
  "fields": [
    {
      "name": "header",
      "type": {"type": "map", "values": "string"}
    },
   {
      "name": "type",
      "type": "int"
   },
    {
      "name": "payload",
      "type": "bytes"
    }
  ]
}
```
with avro language neutral payload, we can create client in different languages.


you may need to install `gcc(or clang)`, `cmake`, `libjasson-dev` and `libao-dev` first


how to build

install cmake
brew install cmake

mkdir build

cd build

cmake ..
make 

the binary is inside the build/src

There are two files exists.
server and client

for streaming wav file from server to multiple client

cat {file.wav} |./server

src/client -h host 
 in this case the server broadcast the TCP packets to all its clients and the client will 
 play the wav file.
 
 send it to the server
 
  ./server
  
  in the client side 
  ./client -h serverIp -t 3 -f flie.wav
  
  in this case. the client will send the packets to server and server will play the wav file.
  
  
