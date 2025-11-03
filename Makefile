all:
	mk

build-docker:
	docker build -t "principia" .

###############################################################################
# Developer targets
###############################################################################

# See https://github.com/aryx/codemap and https://github.com/aryx/fork-efuns
visual:
	codemap -screen_size 3 -efuns_client efuns_client -emacs_client /dev/null .
