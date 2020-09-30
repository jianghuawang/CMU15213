/* 
 * CS:APP Data Lab 
 * 
 * <Please put your name and userid here>
 * Jianghua Wang
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Sme of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.o

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting an integer by more
     than the word size.

EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implent floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operators (! ~ & ^ | + << >>)
     that you are allowed to use for your implementation of the function. 
     The max operator count is checked by dlc. Note that '=' is not 
     counted; you may use as many of these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */


#endif
//1
/* 
 * bitXor - x^y using only ~ and & 
 *   Example: bitXor(4, 5) = 1
 *   Legal ops: ~ &
 *   Max ops: 14
 *   Rating: 1
 */
int bitXor(int x, int y) {
  // Using Demorgan's law (not(A or B)==not A and not B) and and reverse it again
  // operations:8
  int result = ~(x & y);
  return ~(~(result & x)&(~(result & y)));
}
/* 
 * tmin - return minimum two's complement integer 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
int tmin(void) {
  //operations:1
  return 0x1<<31;
}
//2
/*
 * isTmax - returns 1 if x is the maximum, two's complement number,
 *     and 0 otherwise 
 *   Legal ops: ! ~ & ^ | +
 *   Max ops: 10
 *   Rating: 2
 */
int isTmax(int x) {
  //using !!(~x) to exclude the case that x = -1 ( ~(-1)will be 0, and !!(0)will be 0)
  //operations:8
  return !((x+1)^(~x)) & !!(~x);
}
/* 
 * allOddBits - return 1 if all odd-numbered bits in word set to 1
 *   Examples allOddBits(0xFFFFFFFD) = 0, allOddBits(0xAAAAAAAA) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 2
 */
int allOddBits(int x) {
  //operations:9
  int mask=0x55;
  x=x | mask;
  x=x | (mask<<8);
  x=x | (mask<<16);
  x=x | (mask<<24);
  return !(~x);
}
/* 
 * negate - return -x 
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int negate(int x) {
  //operations:2
  return (~x+1);
}
//3
/* 
 * isAsciiDigit - return 1 if 0x30 <= x <= 0x39 (ASCII codes for characters '0' to '9')
 *   Example: isAsciiDigit(0x35) = 1.
 *            isAsciiDigit(0x3a) = 0.
 *            isAsciiDigit(0x05) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 3
 */
int isAsciiDigit(int x) {
  //operations:14
  //better approach:after finding that 0x38<=x, check if x==0x38 or x==0x39.
  
  //1. extract bit 4-31 and find if these higher bit contains only 3
  int higher=x&(~0xF);
  int higherResult=higher^0x30;
  //2. extracct bit 0-3
  int lower=x&0xF;
  int largerTen=0xA;
  int largerTwelve=0xC;
  //3. check if x is 10 or 11 or 14
  int doLargerTen=!((lower & largerTen)^largerTen);
  //4. check if x is 12 or 13 or 14
  int doLargerTwelve=!((lower & largerTwelve)^largerTwelve);
  return !higherResult&!(doLargerTen|doLargerTwelve);
}
/* 
 * conditional - same as x ? y : z 
 *   Example: conditional(2,4,5) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
int conditional(int x, int y, int z) {
  //operations:15
  //when x is 0,shift y left -1(31) and 1, when x is 1, shift z left -1(31) and 1
  return (y<<(!!x+~0x0)<<!x)+(z<<(!x+~0x0)<<(!!x));
}
/* 
 * isLessOrEqual - if x <= y  then return 1, else return 0 
 *   Example: isLessOrEqual(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isLessOrEqual(int x, int y) {
  //operations: 18
  int signX=x>>31;
  int signY=y>>31;
  //check wheather x and y are same sign
  int signResult = signX^signY;
  //1. check if x and y are equal 
  //2. if x and y are same sign, then check if x-y>0 
  //3. check if x is negative and y is positive
  return (!(x^y))|((!signResult) & !!((x+(~y+1))>>31))|!((signX+1)^signY);
}
//4
/* 
 * logicalNeg - implement the ! operator, using all of 
 *              the legal operators except !
 *   Examples: logicalNeg(3) = 0, logicalNeg(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4 
 */
int logicalNeg(int x) {
  //operation:12
  //bit smearing (making all the bits that after the first bit that is 1 equal to 1)
  x |= x >> 16;
  x |= x >> 8;
  x |= x >> 4;
  x |= x >> 2;
  x |= x >> 1;

  //only 0 will have the last digit be 1 after fliping
  return (~x&1);
}
/* howManyBits - return the minimum number of bits required to represent x in
 *             two's complement
 *  Examples: howManyBits(12) = 5
 *            howManyBits(298) = 10
 *            howManyBits(-5) = 4
 *            howManyBits(0)  = 1
 *            howManyBits(-1) = 1
 *            howManyBits(0x80000000) = 32
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 90
 *  Rating: 4
 */
int howManyBits(int x) {
  return 0;
}
//float
/* 
 * float_twice - Return bit-level equivalent of expression 2*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned float_twice(unsigned uf) {
  //mask to extract exponential part 
  unsigned inf=0x7F800000;

  unsigned expo=uf&inf;
  unsigned frac=(uf&0x7FFFFF);
  unsigned sign=uf&(0x80000000);
  //handle NaN and Infinity and +-0
  if((!(expo^inf)) || (!(expo^0) && !frac))
    return uf;
  //handle denormalized value
  else if(!(expo^0)&& !!frac){
    return (uf^frac)|(frac<<1);
  }
  else{
    unsigned newValue=(uf^expo)|(expo+0x00800000);
    //check if overflow or underflow
    if(!((newValue&inf)^inf))
      newValue=sign|inf;
    return newValue;
  }
}
/* 
 * float_i2f - Return bit-level equivalent of expression (float) x
 *   Result is returned as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point values.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
 unsigned float_i2f(int x) {
   int expo,sign,newValue,frac,rest;
   unsigned absX,signBitMask;
   signBitMask=0x80000000;
   if(!x)  
     return 0x00000000;
   sign = x & signBitMask;
   absX=sign?(~x+1):x;
   expo = 31+127;
   //find the first 1 in x by right shift until meeting the first 1
   //find expoonential 
   while(!(absX&signBitMask)&&(expo>127)){
     absX<<=1;
     expo-=1;
   }
   absX<<=1;
   frac=(absX>>9)&(0x7FFFFF);
   rest=(absX&0x1FF);
   if(rest>>8){
     if(rest^0x100)
       frac+=0x1;
     else if((absX&0x200)){
       frac+=0x1;
     }
   }
   newValue=frac+(expo<<23)+sign;
   return newValue;
 }

/* 
 * float_f2i - Return bit-level equivalent of expression (int) f
 *   for floating point argument f.
 *   Argument is passed as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point value.
 *   Anything out of range (including NaN and infinity) should return
 *   0x80000000u.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
int float_f2i(unsigned uf) {
  int sign = uf & 0x80000000;
  int inf = 0x7F800000;
  int expo = ((uf & inf)>>23);
  int frac = uf&0x700000;
  if(expo^0xFF || expo>=32)
    return 0x80000000u;
  else if(!expo|| expo<-1)
    return 0;
  
}
