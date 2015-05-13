
Open source is good, but clean and well documented source is even better!

Principia softwarica consists of a small set of essential ``meta'' programs,
that is programs which are dealing with other programs:
 - kernel/shell/windows = programs that manage and launch other programs
 - network = programs to help other programs communicate with each other
 - editor = program to write programs
 - compiler/assembler/linker/generators = programs that generate other programs
 - interpreter = program that interprets and run another program
 - machine = program that emulates the hardware and run another (binary) program
 - debugger = program that commands another program
 - profiler = program that generates statistics about another program
 - builder = program to help build programs (calling compiler/asm/linker/...)
 - libc/libxx = program to be used by other programs
todo:
 - browser = program to help you find information on how to write programs :)
 - games = programs to help you relax between two coding sessions
 - vcs = program to store the history and manage evolutions of your program

We chose to present mostly programs from the Plan 9 operating system
because they are small, elegant, open source, and portable.

In many of those programs there are support for both the x86 and ARM
architectures. We chose x86 because it's the most common desktop
machine (and also it's not that bad according to Linus:
http://yarchive.net/comp/linux/x86.html). We chose ARM because it is
far simpler and elegant than x86 (RISC are usually better than CISC).
We chose ARM over MIPS because ARM is almost as simple as MIPS but
it's far more alive as it is the most common mobile machine (it's in
every phone and also in the raspberry PI!).

related work:
 - GNU (Richard Stallman et al.)
 - STEPS (Alan Kay et al.)
 - TECS (Nisan and Schocken)
 - Minix (Andrew Tannenbaum et al.)
 - XINU (Douglas Comer)
 - MMIX (Donald Knuth)
 - TempleOS
 - ...

Here are other series of software not covered by principa softwarica:
 - office: word processor, spreadsheet, pixel or vectorized drawing programs
   (Alto Bravo, appleII Visicalc, MacPaint)
 - communication: email, chat, forums, social network
   (???)
 - information/digital age: web browser/server, digital store, search engine
   (???)
 - personal information manager: contact, calendar, clock, alarm, notes, calc,..
   (Newton)
 - digital art revolution: photo, music, video, books
   (iLife)
 - mobile software: maps, weather, camera, chat, phone, ...
   (iPhone)
 - games: arcade, shooter, rts, ...

A few other meta programs are included in a separate project, pfff:
 - codemap/codegraph/codequery: programs to help understand other programs.
 - sgrep/spatch: programs to search and transform other programs.
 - scheck: program to verify other programs.

This project is dedicated to professor Chazarain and his motto
"Implanter pour mieux comprendre"! (implement to better understand)
