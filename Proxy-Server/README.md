curl --request GET --proxy 127.0.0.1:8080 --local-port 20001 127.0.0.1:8082/1.data

curl --request GET -u username:password --proxy 127.0.0.1:20000 --local-port 20001-20010 127.0.0.1:19998/1.data