#line 142 "bits.c"
int bitXor(int x, int y) {


  int result=  ~(x & y);
  return ~(~(result & x)&(~(result & y)));
}
#line 154
int tmin(void) {

  return 0x1<<31;
}
#line 166
int isTmax(int x) {


  return !((x+1)^(~x)) & !!(~x);
}
#line 178
int allOddBits(int x) {

  int mask=0x55;
  x=x | mask;
  x=x |( mask<<8);
  x=x |( mask<<16);
  x=x |( mask<<24);
  return !(~x);
}
#line 194
int negate(int x) {

  return (~x+1);
}
#line 208
int isAsciiDigit(int x) {
#line 213
  int higher=x&(~0xF);
  int higherResult=higher^0x30;

  int lower=x&0xF;
  int largerTen=0xA;
  int largerTwelve=0xC;

  int doLargerTen=!((lower & largerTen)^largerTen);

  int doLargerTwelve=!((lower & largerTwelve)^largerTwelve);
  return !higherResult&!(doLargerTen|doLargerTwelve);
}
#line 232
int conditional(int x, int y, int z) {


  return (y<<(!!x+~0x0)<<!x)+(z<<(!x+~0x0)<<(!!x));
}
#line 244
int isLessOrEqual(int x, int y) {

  int signX=x>>31;
  int signY=y>>31;

  int signResult=  signX^signY;
#line 253
  return (!(x^y))|((!signResult) & !!((x+(~y+1))>>31))|!((signX+1)^signY);
}
#line 264
int logicalNeg(int x) {


  x |= x >> 16;
  x |= x >> 8;
  x |= x >> 4;
  x |= x >> 2;
  x |= x >> 1;


  return (~x&1);
}
#line 288
int howManyBits(int x) {
  return 0;
}
#line 303
unsigned float_twice(unsigned uf) {

  unsigned inf=0x7F800000;

  unsigned expo=uf&inf;
  unsigned frac=(uf&0x7FFFFF);
  unsigned sign=uf&(0x80000000);

  if ((!(expo^inf))||( !(expo^0) && !frac)) 
    return uf;

  else if (!(expo^0)&&!!frac) {
    return (uf^frac)|(frac<<1);
  }
  else {
    unsigned newValue=(uf^expo)|(expo+0x00800000);

    if (!((newValue&inf)^inf)) 
      newValue=sign|inf;
    return newValue;
  }
}
#line 334
 unsigned float_i2f(int x) {
   int expo;int sign;int newValue;int frac;int rest;
   unsigned absX;unsigned signBitMask;
   signBitMask=0x80000000;
   if (!x) 
     return 0x00000000;
   sign = x & signBitMask;
   absX=sign?(~x+1):x;
   expo = 31+127;


   while (!(absX&signBitMask)&&(expo>127)) {
     absX<<=1;
     expo-=1;
   }
   absX<<=1;
   frac=(absX>>9)&(0x7FFFFF);
   rest=(absX&0x1FF);
   if (rest>>8) {
     if (rest^0x100) 
       frac+=0x1;
     else if ((absX&0x200)) {
       frac+=0x1;
     }
   }
   newValue=frac+(expo<<23)+sign;
   return newValue;
 }
#line 375
int float_f2i(unsigned uf) {
  int roundIndex;
  int sign=  uf & 0x80000000;
  int inf=  0x7F800000;
  unsigned expo=((  uf & inf)>>23);
  int expoAfterBias=  expo-127;
  int frac=(  uf&0x7FFFFF)|(1<<23);
  int restBitMask;
  int restBit;
  if (!(expo^0xFF)|| expoAfterBias>31) 
    return 0x80000000u;
  else if (!expo||expoAfterBias<0) 
    return 0;
  if (expoAfterBias<23) {
    roundIndex=23-expoAfterBias;
    restBitMask=(1<<roundIndex)-1;
    restBit=frac&restBitMask;
    frac=frac>>roundIndex;
    if (restBit>(1<<(roundIndex-1))) 
      frac+=1;
  }
  else {
    roundIndex=expoAfterBias-23;
    frac=frac<<roundIndex;
  }
  if (sign) 
    return -frac;
  return frac;
}
