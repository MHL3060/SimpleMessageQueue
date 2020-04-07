#Simple Message Queue


this just a pet project to stream wav file for now.

This is just a very simple Message Queue. It is able to receive wav file and play it 
on the server side.





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
  
  
