
int foo();

static int myfunc() {
  return 3;
}

int bar() {
  int x;
  x = myfunc();
  x = x + foo();
  return x;
}

int _main() {
  return bar();
}
