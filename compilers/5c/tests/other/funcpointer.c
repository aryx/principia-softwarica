int (*f)(int);

int foo(int x) {
  return x + 2;
}

void main() {
  f = &foo;
  f(3);
}
