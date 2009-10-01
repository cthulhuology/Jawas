#!/bin/sh
openssl genrsa -des3 -out server.key 1024
chmod go-r server.key
openssl req -new -key server.key -out server.csr
chmod go-r server.csr
openssl x509 -req -days 365 -in server.csr -signkey server.key -out server.crt
chmod go-r server.cst
cat server.key server.crt > server.pem
chmod go-r server.pem
