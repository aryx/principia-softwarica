// The skeletons Principia Softwarica's programs share, for tinybox's
// code map (~/github/ocaml-elm-playground: its docs/manual/codemap.md,
// and the guidelines, docs/claude_notes/codemapconfig_guidelines.md).
// A shape is a function of files, its bones' anchors given.
{
  // a chain: [anchor, role, say] each, the first calling (or handing its
  // data to) the second, ... -- a path across files, C and assembly (the
  // books' "Software architecture" sections often print one: 5c's
  // main -> compile -> yyparse -> codgen -> gen -> ...); the first
  // step's say is unused
  chain(name, steps):: {
    name: name,
    bones: [{ at: s[0], role: s[1] } for s in steps],
    joints: [{ from: steps[i][0], to: steps[i + 1][0], say: steps[i + 1][2] } for i in std.range(0, std.length(steps) - 2)],
  },

  // a Plan 9 command: its main (the flags read between ARGBEGIN and
  // ARGEND, the arguments after), then its core, a chain as above
  // ([anchor, role, say] each, the first called by main); main may be
  // 'x.c:def:threadmain' for a libthread program (rio)
  cmd(name, main, core, main_role='main: the flags (ARGBEGIN), then the arguments')::
    self.chain(name, [[main, main_role, '']] + core),

  // a directory's parts as whole units (a path each, no anchor): a
  // layered picture, [path, role] each, and joints [from, to, say]
  parts(name, bones, joints):: {
    name: name,
    bones: [{ at: b[0], role: b[1] } for b in bones],
    joints: [{ from: j[0], to: j[1], say: j[2] } for j in joints],
  },
}
