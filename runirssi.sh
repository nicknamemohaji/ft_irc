docker pull irssi:latest

docker container rm  my-running-irssi
docker run -it --name my-running-irssi -e TERM -u $(id -u):$(id -g) \
    --log-driver=none \
	--network=host \
    -v $HOME/.irssi:/home/user/.irssi:ro \
    irssi

<<<<<<< HEAD
# /CONNECT host.docker.internal 6667 verystrongpassword user
# /CONNECT irc.ubuntu.com 6667 user
=======
# /CONNECT host.docker.internal 6667 verystrongpassw
>>>>>>> 6f4c7ecfdbbe292b0b8a05cc54e03f5bbdc9d2f7
