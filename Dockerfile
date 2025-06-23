FROM padator/kencc

# 9base for rc (TODO: delete once we can bootstrap a working bin/rc)
RUN apt-get install -y 9base

WORKDIR /src

# Now let's build from source
COPY . .

# 386
RUN mkdir -p ROOT/386/lib
RUN . ./env.sh && mk && mk kernel

# arm
RUN cp mkconfig.pi mkconfig
RUN mkdir -p ROOT/arm/lib
RUN . ./env.sh && mk && mk kernel
