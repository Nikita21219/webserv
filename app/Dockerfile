FROM gcc:12 as build

WORKDIR /app

COPY srcs/ .

RUN apt-get update && \
    apt-get install -y && \
    apt-get install vim -y && \
    make

WORKDIR /app

CMD ["./webserv", "configs/1.conf"]

EXPOSE 4242
EXPOSE 2121
