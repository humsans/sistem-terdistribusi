FROM ubuntu:22.04
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y \
    gcc \
    mpich \
    openssh-server \
    && rm -rf /var/lib/apt/lists/*

RUN mkdir /var/run/sshd && \
    echo 'root:root' | chpasswd && \
    sed -i 's/#PermitRootLogin prohibit-password/PermitRootLogin yes/' /etc/ssh/sshd_config && \
    ssh-keygen -t rsa -f /root/.ssh/id_rsa -q -N "" && \
    cat /root/.ssh/id_rsa.pub >> /root/.ssh/authorized_keys && \
    echo "StrictHostKeyChecking no" >> /root/.ssh/config

WORKDIR /app
CMD ["/usr/sbin/sshd", "-D"]