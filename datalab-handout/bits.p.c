#line 142 "bits.c"
int bitXor(int x, int y) {

  int result=  ~(x & y);
  return ~(~(result & x)&(~(result & y)));
}
#line 153
int tmin(void) {
  return 0x1<<31;
}
#line 164
int isTmax(int x) {

  return !((x+1)^(~x)) & !!(~x);
}
#line 175
int allOddBits(int x) {
  int mask=0x55;
  x=x | mask;
  x=x |( mask<<8);
  x=x |( mask<<16);
  x=x |( mask<<24);
  return !(~x);
}
#line 190
int negate(int x) {
  return (~x+1);
}
#line 203
int isAsciiDigit(int x) {
  int higher=x&(~0xF);
  int higherResult=higher^0x30;
  int lower=x&0xF;
  int largerTen=0xA;
  int largerTwelve=0xC;
  int doLargerTen=!((lower & largerTen)^largerTen);
  int doLargerTwelve=!((lower & largerTwelve)^largerTwelve);
  return !higherResult&!(doLargerTen|doLargerTwelve);
}
#line 220
int conditional(int x, int y, int z) {
  return 2;
}
#line 230
int isLessOrEqual(int x, int y) {
  return 2;
}
#line 242
int logicalNeg(int x) {
  return 2;
}
#line 257
int howManyBits(int x) {
  return 0;
}
#line 272
unsigned float_twice(unsigned uf) {
  return 2;
}
#line 284
unsigned float_i2f(int x) {
  return 2;
}
#line 299
int float_f2i(unsigned uf) {
  return 2;
}
