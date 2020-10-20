// $Id: libfns.cpp,v 1.4 2015-07-03 14:46:41-07 - - $
// Trystan Nguyen trtanguy
// Leonard Tolentino lemtolen

#include "libfns.h"
#include "util.h"

//
// This algorithm would be more efficient with operators
// *=, /=2, and is_odd.  But we leave it here.
//

bigint pow (const bigint& base_arg, const bigint& exponent_arg) {
   bigint base (base_arg);
   bigint exponent (exponent_arg);

   static const bigint ZERO = bigint(0);
   static const bigint ONE = bigint(1);
   static const bigint TWO = bigint(2);

   static const bigint max = bigint(4294967295);
   bigint val (exponent_arg);
   if (val < 0) val = val * (ZERO - ONE);
   if (val > max) throw ydc_exn ("Exponent too long");

   //DEBUGF ('^', "base = " << base << ", exponent = " << exponent);
   if (base == ZERO) return ZERO;
   bigint result = ONE;
   if (exponent < ZERO) {
      base = ONE / base;
      exponent = - exponent;
   }
   while (exponent > ZERO) {
      if (exponent % TWO == ONE) {
         result = result * base;
         exponent = exponent - 1;
      }else {
         base = base * base;
         exponent = exponent / 2;
      }
   }
   //DEBUGF ('^', "result = " << result);
   return result;
}

