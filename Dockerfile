FROM ghcr.io/userver-framework/ubuntu-24.04-userver:latest

LABEL org.opencontainers.image.authors="dmitriy-korotov"

COPY ./third_party/ ./third_party

RUN chmod +x ./third_party/install.sh
RUN sh ./third_party/install.sh
