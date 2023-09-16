FROM ubuntu:18.04

ARG USER_ID=1000
ARG GROUP_ID=1000
ARG ROOT_PW='docker'

ENV DEBIAN_FRONTEND=noninteractive

RUN apt -y update && apt -y upgrade
RUN dpkg --add-architecture i386
RUN apt -y update
RUN apt -y install build-essential gcc-multilib git nano vim neofetch htop wget

RUN echo "root:$ROOT_PW" | chpasswd

RUN addgroup --gid $GROUP_ID www
RUN adduser --disabled-password --gecos '' --uid $USER_ID --gid $GROUP_ID www
RUN mkdir -p /home/www
RUN chown -R www: /home/www
# start as non root user to avoid permissions conflict with the files created inside the container
USER www
WORKDIR /home/www
COPY . .
