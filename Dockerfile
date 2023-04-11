FROM gcc:latest

RUN apt-get update && \
    apt-get install -y libpqxx-dev

WORKDIR /app

COPY . /app/

RUN cd src/server && \
    make

EXPOSE 12345

CMD ["./src/server/main"]