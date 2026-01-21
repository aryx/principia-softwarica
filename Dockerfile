# Build principia on Ubuntu Linux (amd64 or arm64) for 386 (pc) and arm (pi)
FROM padator/goken

WORKDIR /principia
COPY . .

# 386
RUN cp mkconfig.pc mkconfig
RUN mk && mk install 
RUN mk kernel

RUN apt-get install -y --no-install-recommends dosfstools mtools
RUN mk disk
# to extract 9qemu and dosdisk.img do:
#    docker run -u $(id -u):$(id -g) --rm -v "$PWD:/out" principia   sh -c "cp kernel/COMPILE/9/pc/9qemu /out && cp dosdisk.img /out"
# then to use qemu to run plan9 do:
#    qemu-system-i386 -smp 4 -m 512 -kernel ./9qemu -hda ./dosdisk.img

# arm
RUN cp mkconfig.pi mkconfig
RUN mk && mk install
RUN mk kernel
