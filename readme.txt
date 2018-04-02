
Open source programs are good, but clean and well documented programs
are even better!
Principia softwarica is a serie of books explaining essential ``meta'' programs
(programs dealing with other programs. a.k.a system software):

 - kernel/shell/windows = programs that manage and launch other programs
 - network = programs to help other programs communicate with each other
 - editor = program to write programs
 - compiler/assembler/linker/generators = programs that generate other programs
 - interpreter = program that interprets and run another program
 - machine = program that emulates the hardware and run another (binary) program
 - debugger = program that commands and inspect another program
 - profiler = program that generates statistics about another program
 - builder = program to help build programs (calling compiler/asm/linker/...)
 - vcs = program to store the history and manage evolutions of your program
 - libc/libxx = programs to be used by other programs
 - browser = program to help you find information on how to write programs :)
 - games = programs to help you relax between two coding sessions

We chose to present mostly programs from the Plan 9 operating system
because they are small, elegant, open source, and portable.

In many of those programs there are support for both the x86 and ARM
architectures. We chose x86 because it's the most common desktop
machine. We chose ARM because it is far simpler and elegant than x86
(RISC are simpler than CISC) and we chose ARM over MIPS because ARM is
almost as simple as MIPS but it's alive as it is the most common
mobile machine (it's in every phone and also in the raspberry PI!).

related work:
 - UNIX (Ken Thompson et al.)
   http://minnie.tuhs.org/cgi-bin/utree.pl
 - GNU (Richard Stallman et al.)
   http://www.gnu.org/
 - XINU (Douglas Comer)
   http://minnie.tuhs.org/cgi-bin/utree.pl?file=Xinu7
 - Minix (Andrew Tannenbaum et al.)
   http://minnie.tuhs.org/cgi-bin/utree.pl?file=Minix1.1
 - Oberon (Niklaus Wirth)
   http://www.projectoberon.com/
 - STEPS (Alan Kay et al.)
   http://vpri.org/html/writings.php
 - TECS (Nisan and Schocken)
   http://www.nand2tetris.org/
 - MMIX (Donald Knuth)
   http://www-cs-faculty.stanford.edu/~uno/mmix-news.html
 - TempleOS (Terry A. Davis)
   http://www.templeos.org/
 - RECC collection (Robert Elder)
   http://recc.robertelder.org/ 
 - short canonical programs:
   http://aosabook.org/blog/2014/01/call-for-technical-reviewers-for-500-lines-or-less/
   https://github.com/aosabook/500lines
 - A tiny hand crafted CPU emulator, C compiler, and Operating System
   https://github.com/rswier/swieros
 - ...

Here are other series of software (application software)
not covered by principa softwarica:
 - office: word processor, spreadsheet, presentations, 
   pixel or vectorized drawing programs, publishing, database
   (Alto Bravo, appleII Visicalc, Powerpoint?, MacPaint, Publisher?, ingres?)
 - personal information manager: contact, calendar, alarm, notes, calc, ...
   (Newton)
 - digital art revolution: photo, music, video, books
   (iLife)
 - communication: email, chat, forums, social network
   (???)
 - information/digital age: web browser/server, digital store, search engine
   (???)
 - mobile software: maps, weather, camera, chat, phone, ...
   (iPhone)
 - games: arcade, shooter, rts, ...

A few other meta programs are included in a separate project (pfff):
 - codemap/codegraph/codequery: programs to help understand other programs
 - sgrep/spatch: programs to search and transform other programs
 - scheck: program to verify other programs
