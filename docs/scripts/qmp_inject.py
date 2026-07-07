#!/usr/bin/env python3
# claude: this script was written by Claude Code (2026-07-07)
"""Inject USB keyboard and mouse input into a headless 9pi/QEMU guest via QMP.

QEMU's Machine Protocol (QMP) lets you drive the *same* input subsystem that
the graphical window feeds, so you can test the usb-kbd/usb-mouse HID path with
no human at the window. This is how the dwcotg USB-HID fixes were verified (see
docs/qemu_arm_debug.txt, "Making USB HID work under QEMU"): a single send-key of
'd','e','f' must echo exactly "def" (not a runaway "dddeeefff"), and wiggling
the mouse while 'cat /dev/mouse' runs must print one 'm x y b msec' record per
motion event.

Boot QEMU with a QMP unix socket and the HID devices, e.g.:

  qemu-system-arm -M raspi1ap \
    -device loader,file=.../9pi,addr=0x8000,cpu-num=0,force-raw=on \
    -drive file=qemu-sd.img,if=sd,format=raw \
    -serial null -serial mon:stdio -display none \
    -device usb-kbd -device usb-mouse \
    -qmp unix:/tmp/q.sock,server,nowait

Usage: qmp_inject.py [socket] [seconds-to-wait-for-enumeration]

The protocol is line-oriented JSON: read the greeting, send qmp_capabilities,
then send-key (keyboard) and input-send-event (mouse). It is stable across QEMU
versions, so this is easy to port to any language.
"""
import json, socket, sys, time

SOCK = sys.argv[1] if len(sys.argv) > 1 else "/tmp/q.sock"
WAIT = int(sys.argv[2]) if len(sys.argv) > 2 else 10


def connect(path):
    for _ in range(50):
        try:
            s = socket.socket(socket.AF_UNIX)
            s.connect(path)
            return s
        except OSError:
            time.sleep(0.2)
    sys.exit("could not connect to QMP socket %s" % path)


def main():
    s = connect(SOCK)
    f = s.makefile("rwb")

    def cmd(obj):
        f.write((json.dumps(obj) + "\n").encode())
        f.flush()
        return f.readline()

    f.readline()                                   # QMP greeting
    cmd({"execute": "qmp_capabilities"})

    def sendkeys(*qcodes):
        # qcodes pressed together as a chord, e.g. sendkeys("shift","3") -> '#'
        keys = [{"type": "qcode", "data": k} for k in qcodes]
        cmd({"execute": "send-key", "arguments": {"keys": keys}})

    def mouserel(dx, dy):
        # usb-mouse turns relative-axis events into HID reports
        for axis, val in (("x", dx), ("y", dy)):
            cmd({"execute": "input-send-event", "arguments": {"events": [
                {"type": "rel", "data": {"axis": axis, "value": val}}]}})

    def mousebtn(button, down):
        cmd({"execute": "input-send-event", "arguments": {"events": [
            {"type": "btn", "data": {"button": button, "down": down}}]}})

    print("waiting %ds for USB enumeration..." % WAIT)
    time.sleep(WAIT)

    # keyboard: three single taps must echo exactly "def"
    for k in ("d", "e", "f", "ret"):
        sendkeys(k)
        time.sleep(0.4)

    # mouse: six wiggles -- watch 'cat /dev/mouse' in the guest
    # (this minimal boot.rc does not bind the mouse dev, so first run:
    #  bind -a '#m' /dev ; cat /dev/mouse )
    for _ in range(6):
        mouserel(20, 10)
        time.sleep(0.3)

    print("done")


if __name__ == "__main__":
    main()
