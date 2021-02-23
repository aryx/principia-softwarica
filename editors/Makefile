# Mostly a copy of mkfile
# I use a Makefile for efuns because my mk/rc is currently broken
# because kencc/bin/rc is compiled for 32 bits and I have no time to fix it

SYNCFLAGS=-md5sum_in_auxfile -less_marks -lang ocaml

VERSION=5
DOC=Editor

SRC_ORIG=Editor.nw Editor_extra.nw

EFUNS=/home/pad/github/fork-efuns

#todo: to lpize at some point:
# dircolors.ml, shell.ml, outline_mode.ml, prog_modes/*, etc

#pad: I've indented some of the files I created and put them under the
# file from which they come from in the original Efuns from LeFessant.
SRC= \
  commons/store.ml\
  commons/store.mli\
  \
  core/efuns.ml\
   core/var.ml\
   core/parameter_option.ml\
   core/attr.ml\
   core/hook.ml\
   core/action.ml\
   core/globals.ml\
   core/error.ml\
  core/text.ml\
  core/ebuffer.ml\
  core/keymap.ml\
  core/window.ml\
  core/frame.ml\
  core/top_window.ml\
  \
  features/misc_features.ml\
   features/edit.ml\
   features/move.ml\
   features/copy_paste.ml\
   features/scroll.ml\
   features/mouse.ml\
   features/highlight.ml\
   features/indent.ml\
   features/color.ml\
   features/structure.ml\
   features/electric.ml\
   features/transform.ml\
  features/minibuffer.ml\
  features/system.ml\
  features/select.ml\
  features/search.ml\
  features/interactive.ml\
  features/multi_buffers.ml\
  features/multi_frames.ml\
  features/abbrevs.ml\
  features/compil.ml\
  \
  config/default_config.ml\
  main.ml\
  \
  major_modes/dired.ml\
  \
  minor_modes/minor_mode_sample.ml\
  minor_modes/abbrevs_mode.ml\
  minor_modes/TODO_more/accents_mode.ml\
  minor_modes/fill_mode.ml\
  minor_modes/paren_mode.ml\
  minor_modes/tab_mode.ml\
  \
  prog_modes/makefile_mode.ml\
  prog_modes/TODO_more/ocaml_toplevel.ml\
  \
  ipc/server.ml\
  efuns_client.ml\
  \
  todo/dynamic/eval.ml\
  todo/misc/efuns_filebrowser.ml\
  todo/misc/efuns_texbrowser.ml\

SRC_VIEWS=$(SRC:%=$(EFUNS)/%)

include ~/semgrep-internals/Makefile.common

