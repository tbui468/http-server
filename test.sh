#!/bin/bash

./http_server 8888 > /dev/null 2>&1 &

sleep 0.5

./http_client http://localhost:8888/http_server > /dev/null 2>&1

if cmp -s "output" "http_server"; then
    echo "Send File: Passed"
else
    echo "Send File: Failed"
fi

./http_client http://localhost:8888/abc > /dev/null 2>&1

if cmp -s "output" "filenotfound"; then
    echo "File Not Found: Passed"
else
    echo "File Not Found: Failed"
fi

./http_client htp://localhost:8888/http_server > /dev/null 2>&1

if cmp -s "output" "invalidprotocol"; then
    echo "Invalid Protocol: Passed"
else
    echo "Invalid Protocol: Failed"
fi

./http_client http://cat:8888/http_server > /dev/null 2>&1

if cmp -s "output" "noconnection"; then
    echo "No Connection: Passed"
else
    echo "No Connection: Failed"
fi

pid=$(lsof -t -i :8888)
kill $pid
