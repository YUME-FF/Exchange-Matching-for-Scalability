# erss-hwk4-zf70-hb174

## Create database
```
sudo su - postgres
psql
\password postgres
#enter new password we use
create database exchange;
```
## Start connection
```
cd src/server
make clean
make
./main

cd ../client
make clean
make
./client anything test.xml
```

## check data in postgres
```
sudo su - postgres
psql
#enter password
\c exchange
\d
select * from account;
select * from position;
```
## Test functionality

1. Start the server:
```
./run.sh localhost 1 1
```
2. Start the client:
```
cd src/testing
make clean
make
./functionality
```


## Test scalability

1. Start the server:
```
./run.sh <host> <core numbers> <ThreadPoolSize>

e.g: 
./run.sh localhost 1 1
```
2. Start the client:
```
cd src/testing
make clean
make
./scalability <client numbers> <request number per client> <host> <port>

e.g:
./scalability 1 1000 localhost 12345
```