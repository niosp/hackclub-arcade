FROM python:3.11-alpine
WORKDIR /server
RUN apk add --no-cache libpq-dev gcc musl-dev python3-dev linux-headers postgresql-client postgresql-dev nginx openrc
RUN pip install --no-cache-dir -r requirements.txt
COPY ./server.py .
RUN chmod +x ./startup.sh
CMD [ "/bin/sh", "/server/startup.sh" ]