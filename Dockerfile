FROM amazonlinux:latest
LABEL application=testingharness

RUN echo "nameserver 1.1.1.1" > /etc/resolv.conf && \
    echo "nameserver 8.8.8.8" >> /etc/resolv.conf && \
    yum install -y gtk3-devel

# RUN groupadd -g 1000 app && \
#   adduser -u 1000 -g app app

WORKDIR /usr/testingharness

COPY ./build ./
RUN mv lib* /usr/local/lib && \
    echo "/usr/local/lib" > /etc/ld.so.conf.d/testingharness.conf && \
    ldconfig

CMD [ "./TestingHarness", "-q tcp://activemq:61616", "-f Scripts/TestMasterpoint.txt"]