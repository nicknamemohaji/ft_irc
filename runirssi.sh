docker pull irssi:latest

docker container rm  my-running-irssi
docker run -it --name my-running-irssi -e TERM -u $(id -u):$(id -g) \
    --log-driver=none \
	--network=host \
    -v $HOME/.irssi:/home/user/.irssi:ro \
    irssi

# /CONNECT host.docker.internal 6667 verystrongpassw