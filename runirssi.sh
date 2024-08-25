#!/bin/bash

# 기본값 설정
DEFAULT_USER="default_user"
DEFAULT_PORT="6667"
DEFAULT_PASSWORD="1234"

# 인자 처리
if [ $# -eq 2 ]; then
    PORT=$1
    PASSWORD=$2
    USER=$DEFAULT_USER
elif [ $# -eq 3 ]; then
    PORT=$1
    PASSWORD=$2
    USER=$3
else
    echo "Usage: $0 PORT PASSWORD [USERNAME]"
    echo "Example: $0 6667 1234"
    echo "         $0 6667 1234 sabyun"
    exit 1
fi

# 기존 컨테이너 제거 (만약 존재한다면)
docker container rm -f $USER 2>/dev/null

# Docker 컨테이너 실행
docker run -it --name $USER -e TERM \
    --log-driver=none \
    --network=host \
    -v $HOME/.irssi:/home/user/.irssi \
    irssi \
    irssi -c host.docker.internal -p $PORT -w $PASSWORD -n $USER

# 컨테이너 종료 후 삭제
docker container rm $USER

echo "irssi session for $USER ended and cleaned up."