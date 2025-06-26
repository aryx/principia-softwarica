int bar(int x, int y) {
  return x + y;
}

int foo(int x) {
  return bar(x, x);
}

