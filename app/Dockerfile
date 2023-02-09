FROM gcc:12 as build

WORKDIR /app

COPY srcs/ .

RUN apt-get update && \
    apt-get install -y && \
    make

FROM ubuntu:latest

RUN groupadd -r webserv && useradd -r -g webserv webserv

USER webserv

WORKDIR /app

COPY --from=build /app/webserv .

CMD ["./webserv", "tools/configs/1.conf"]

EXPOSE 4242
EXPOSE 2121
