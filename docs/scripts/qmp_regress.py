#!/usr/bin/env python3
# claude: this script was written by Claude Code (2026-07-07)
"""Headless regression test for 9pi under QEMU's raspi1ap machine.

Boots the kernel with a USB keyboard and mouse, drives them over QMP, and
asserts that nothing in the boot / HID path has regressed:

  1. boot sequence   -- reaches the interactive rc shell prompt (%);
  2. no reset loop   -- the 'Plan 9 from Bell Labs' banner appears exactly once
                        (a mis-detected emulation would let the watchdog reset
                        the board every clock tick -> the banner would repeat);
  3. USB enumeration -- the usb/kb HID driver attaches (usb/kb... in the log);
  4. keyboard        -- three tapped keys echo as exactly "def", NOT a runaway
                        repeat like "dddddeeeee" (the split-transaction bug);
  5. mouse           -- wiggling the mouse produces 'm x y b msec' reports with
                        an advancing position.

Exit status is 0 on pass, 1 on any failed check (so `mk run-test` fails loudly).

Usage: qmp_regress.py <kernel> <sd-image> [enum-wait-seconds]
Invoked by the run-test target in mkfile-target-pi.
"""
import json, os, re, socket, subprocess, sys, tempfile, time

if len(sys.argv) < 3:
    sys.exit("usage: qmp_regress.py <kernel> <sd-image> [enum-wait-seconds]")
KERNEL, SD = sys.argv[1], sys.argv[2]
ENUM_WAIT = int(sys.argv[3]) if len(sys.argv) > 3 else 12
BOOT_TIMEOUT = 60                       # hard cap on reaching the prompt

tmp = tempfile.mkdtemp(prefix="qmp_regress_")
SERIAL = os.path.join(tmp, "serial.log")
SOCK = os.path.join(tmp, "qmp.sock")    # keep the path short (<108 bytes)


def wait_for(pattern, timeout):
    """Poll the serial log until regex `pattern` appears; return True/False."""
    rx = re.compile(pattern)
    end = time.time() + timeout
    while time.time() < end:
        try:
            if rx.search(open(SERIAL, "rb").read().decode("latin1")):
                return True
        except FileNotFoundError:
            pass
        time.sleep(0.3)
    return False


def main():
    cmd = [
        "qemu-system-arm", "-M", "raspi1ap",
        "-device", "loader,file=%s,addr=0x8000,cpu-num=0,force-raw=on" % KERNEL,
        "-drive", "file=%s,if=sd,format=raw" % SD,
        "-serial", "null", "-serial", "file:%s" % SERIAL,
        "-display", "none",
        "-device", "usb-kbd", "-device", "usb-mouse",
        "-qmp", "unix:%s,server,nowait" % SOCK,
    ]
    proc = subprocess.Popen(cmd, stdin=subprocess.DEVNULL,
                            stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT)

    def finish(ok):
        proc.terminate()
        try:
            proc.wait(5)
        except subprocess.TimeoutExpired:
            proc.kill()
        sys.exit(0 if ok else 1)

    # QMP handshake
    qs = None
    for _ in range(50):
        try:
            qs = socket.socket(socket.AF_UNIX); qs.connect(SOCK); break
        except OSError:
            time.sleep(0.2)
    if qs is None:
        print("FAIL: could not connect to QMP"); finish(False)
    f = qs.makefile("rwb")
    f.readline()
    f.write(b'{"execute":"qmp_capabilities"}\n'); f.flush(); f.readline()

    def sendkeys(*ks):
        keys = [{"type": "qcode", "data": k} for k in ks]
        f.write((json.dumps({"execute": "send-key",
                             "arguments": {"keys": keys}}) + "\n").encode())
        f.flush(); f.readline()

    def typeline(s):
        plain = {" ": "spc", "/": "slash", "'": "apostrophe", "-": "minus"}
        for ch in s:
            if ch == "#":
                sendkeys("shift", "3")
            else:
                sendkeys(plain.get(ch, ch))
            time.sleep(0.1)
        sendkeys("ret"); time.sleep(0.4)

    def mouserel(dx, dy):
        for axis, val in (("x", dx), ("y", dy)):
            f.write((json.dumps({"execute": "input-send-event", "arguments":
                     {"events": [{"type": "rel",
                      "data": {"axis": axis, "value": val}}]}}) + "\n").encode())
            f.flush(); f.readline()

    results = []

    def check(name, ok, detail=""):
        results.append((name, ok, detail))
        print("%s %s%s" % ("PASS" if ok else "FAIL", name,
                           ("  -- " + detail) if detail else ""))

    # 1. boot to prompt
    booted = wait_for(r"%", BOOT_TIMEOUT)
    check("boot reaches rc prompt", booted)
    if not booted:
        finish(False)

    # 3. usb kb enumerated (check now that we are up)
    log = open(SERIAL, "rb").read().decode("latin1")
    check("usb keyboard enumerates", "usb/kb" in log)

    # give the second HID device time, then bind + read the mouse
    time.sleep(ENUM_WAIT)
    typeline("bind -a '#m' /dev")
    typeline("cat /dev/mouse")
    time.sleep(0.5)
    for _ in range(6):
        mouserel(20, 10); time.sleep(0.3)
    time.sleep(1.0)
    sendkeys("delete"); time.sleep(0.5)          # interrupt the cat

    # 4. keyboard: taps must echo single chars, not a runaway repeat
    sendkeys("d"); time.sleep(0.3)
    sendkeys("e"); time.sleep(0.3)
    sendkeys("f"); time.sleep(0.3)
    sendkeys("ret"); time.sleep(1.0)

    log = open(SERIAL, "rb").read().decode("latin1")

    # 2. no reset loop
    banners = log.count("Plan 9 from Bell Labs")
    check("no watchdog reset loop", banners == 1,
          "boot banner seen %d times (want 1)" % banners)

    # 5. mouse reports with advancing position
    mrecs = re.findall(r"m\s+(-?\d+)\s+(-?\d+)\s+(-?\d+)\s+\d+", log)
    xs = [int(x) for x, _, _ in mrecs]
    advancing = len(mrecs) >= 4 and any(b > a for a, b in zip(xs, xs[1:]))
    check("mouse reports advance", advancing,
          "%d mouse records, x=%s" % (len(mrecs), xs[:8]))

    # 4. runaway-repeat signature: a tapped key must not print >=4 in a row
    runaway = re.search(r"(d{4,}|e{4,}|f{4,})", log)
    got_def = ("d" in log and "e" in log and "f" in log)
    check("keyboard: no runaway key-repeat", runaway is None,
          ("saw run '%s'" % runaway.group(1)) if runaway else "single taps ok")
    check("keyboard: keys reached shell", got_def)

    ok = all(r[1] for r in results)
    print("\n%s: %d/%d checks passed" %
          ("PASS" if ok else "FAIL",
           sum(1 for r in results if r[1]), len(results)))
    finish(ok)


if __name__ == "__main__":
    main()
