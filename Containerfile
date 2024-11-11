# Start with the official Ubuntu Minimal base image
FROM ubuntu

# Set environment variables
ENV LANG=en_US.UTF-8
ENV LC_ALL=en_US.UTF-8

# Update Ubuntu and install essential packages
RUN apt-get update && apt-get upgrade -y && \
            apt-get install -y \
            bash \
            sudo \
            git \
            curl \
            rsync \
            openssh-server \
            cmake \
            llvm \
            llvm-dev \
            clang \
            lld \
            lldb && \
#guix && \
            rm -rf /var/lib/apt/lists/*

# Create dev user
RUN useradd -m user && echo "user:password" | chpasswd && \
    usermod -aG sudo user   # Add user to sudo group

# Create SSH directory for the new user and set perms
RUN mkdir /home/user/.ssh && chown user:user /home/user/.ssh && chmod 700 /home/user/.ssh

# Copy the public key into the container for the new user
COPY id_rsa.pub /home/user/.ssh/authorized_keys

# Set permissions for the authorized_keys file
RUN chown user:user /home/user/.ssh/authorized_keys && chmod 600 /home/user/.ssh/authorized_keys

# Create SSH directory for root and set permissions
RUN mkdir -p /root/.ssh && chmod 700 /root/.ssh

# Copy the public key into the container
COPY id_rsa.pub /root/.ssh/authorized_keys

# Set permissions for the authorized_keys file
RUN chmod 600 /root/.ssh/authorized_keys

# Configure SSH
RUN mkdir /var/run/sshd  # Create directory for SSH daemon

# Generate SSH host keys
RUN ssh-keygen -A  # Generate SSH keys

# Set up SSH configuration
# TODO CHANGE PERMIT ROOT TO NO WHEN DONE DEVELOPING
RUN echo "StrictHostKeyChecking no" >> /etc/ssh/ssh_config && \
    echo "PermitRootLogin yes" >> /etc/ssh/sshd_config && \ 
    echo "AllowUsers user root" >> /etc/ssh/sshd_config && \
    echo "PasswordAuthentication no" >> /etc/ssh/sshd_config && \
    echo "PubkeyAuthentication yes" >> /etc/ssh/sshd_config

# Expose SSH port
EXPOSE 22

# Configure Guix for development (e.g., additional environment setup)
#RUN guix pull  # Update Guix to the latest version

# Start the SSH service and set the default shell
CMD ["/usr/sbin/sshd", "-D"]

# Install C-Logger from git
RUN mkdir -p /opt/c-logger && git clone https://github.com/N3ar/c-logger.git /opt/c-logger
RUN chown user:user /opt/c-logger
WORKDIR /opt/c-logger
RUN ./build.sh
WORKDIR build
RUN make && make install
RUN ldconfig

# Copy Private Key onboarded to GITHUB
WORKDIR /home/user
COPY id_rsa .ssh/id_rsa_github
RUN chmod 400 .ssh/id_rsa_github

# Create ssh_config file for user to clone from private repo
RUN echo "Host github.com" >> .ssh/config && \
    echo "  HostName github.com" >> .ssh/config && \
    echo "  IdentityFile ~/.ssh/id_rsa_github" >> .ssh/config && \
    echo "  IdentitiesOnly yes" >> .ssh/config && \
    echo "  AddKeysToAgent yes" >> .ssh/config 
RUN chmod 600 .ssh/config
RUN chown -R user:user .ssh

# Use gitconfig to specify user account to enable private repo auth
COPY gitconfig .gitconfig
RUN chown user:user .gitconfig

# Install FuncLog from git as user with ssh configurations
USER user
RUN git clone git@github.com:N3ar/funclog.git
RUN chown -R user:user funclog

# Build and copy into path
WORKDIR funclog
USER root
RUN ./build.sh -i
RUN ldconfig

