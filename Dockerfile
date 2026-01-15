# Build principia on Ubuntu Linux for 386 (pc) and arm (pi)
FROM padator/goken

WORKDIR /principia
# Now let's build from source
COPY . .

# 386
RUN cp mkconfig.pc mkconfig
RUN mk && mk install && mk kernel
# arm
RUN cp mkconfig.pi mkconfig
RUN mk && mk install && mk kernel
