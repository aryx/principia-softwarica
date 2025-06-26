int x;
int buf[10];

void main() {
  int* y;
  y = &x;

  y = buf;
  buf[1] = 2;
}
