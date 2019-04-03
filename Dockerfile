FROM ubuntu:18.04
RUN apt-get update && apt-get install -y build-essential libpqxx-dev libtbb-dev
RUN mkdir /code
WORKDIR /code
ADD . /code/
