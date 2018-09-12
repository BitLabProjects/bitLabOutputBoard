#include <mbed.h>

DigitalOut myled(A0);

int main()
{
  // put your setup code here, to run once:
  while (1)
  {
    myled = 1;
    wait(0.4);
    myled = 0;
    wait(0.4);
  }
}