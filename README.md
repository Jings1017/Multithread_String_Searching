# Mulitthreaded-string-search 

## objectives

understand multi-thread programming</br>
understand client-server model</br>

## pre-commit

$ astyle  --style=bsd --indent=spaces=4</br>

## server

parameter format</br>
./server -r Root -p Port -n Thread_Number</br>
Root: the path  of the files</br>
Port: the port which the server is listening to</br>
Thread_Number: number of threads in the thread pool</br>
$ ./server -r testdir -p 12345 -n 4</br>

## client

parameter format</br>
./client -h LocalHost -p Port</br>
LocalHost: server would be running on localhost(127.0.0.1)</br>
Port: the port which the server is listening to</br>
$ ./client -h 127.0.0.1 -p 12345</br>
