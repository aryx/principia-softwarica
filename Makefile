all:
	mk

build-docker:
	docker build -t "principia" .
