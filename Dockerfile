# Build principia on Ubuntu Linux for 386 (pc) and arm (pi)

FROM padator/kencc

# 9base for rc (TODO: delete once we can have a working rc in kencc)
RUN apt-get install -y 9base

WORKDIR /src

# Now let's build from source
COPY . .

# 386
RUN cp mkconfig.pc mkconfig
RUN . ./env.sh && mk && mk install && mk kernel

# arm
RUN cp mkconfig.pi mkconfig
RUN . ./env.sh && mk && mk install && mk kernel
