// $Id: bigint.cpp,v 1.78 2019-04-03 16:44:33-07 - - $
// Trystan Nguyen trtanguy
// Leonard Tolentino lemtolen

#include <cstdlib>
#include <exception>
#include <stack>
#include <stdexcept>
using namespace std;

#include "bigint.h"
//#include "debug.h"
#include "relops.h"

bool isNeg;

bigint::bigint (long that): uvalue (that), is_negative (that < 0) {
   //DEBUGF ('~', this << " -> " << uvalue)
}

bigint::bigint (const ubigint& uvalue_, bool is_negative_):
                uvalue(uvalue_), is_negative(is_negative_) {
}

bigint::bigint (const string& that) {
   is_negative = that.size() > 0 and that[0] == '_';
   uvalue = ubigint (that.substr (is_negative ? 1 : 0));
}

bigint bigint::operator+ () const {
   return *this;
}

bigint bigint::operator- () const {
   return {uvalue, not is_negative};
}

bigint bigint::operator+ (const bigint& that) const {
   bigint result = bigint();
   // Both numbers have same sign
   if (is_negative == that.is_negative){
      result.is_negative = is_negative;
      result.uvalue = uvalue + that.uvalue;
   }
   // Numbers have different signs
   else{
      if (uvalue > that.uvalue){
         result.uvalue = uvalue - that.uvalue;
         result.is_negative = is_negative;
      }
      else if (uvalue < that.uvalue){
         result.uvalue = that.uvalue - uvalue;
         result.is_negative = that.is_negative;
      }
      // If result is zero
      else{
         result.uvalue = 0;
         result.is_negative = false;
      }
   }
   return result;
}

bigint bigint::operator- (const bigint& that) const {
   bigint result = bigint();
   
   // If signs are different
   if (is_negative != that.is_negative){
      result.uvalue = uvalue + that.uvalue;
      result.is_negative = is_negative;
   }
   // If signs are same
   else{
      if (uvalue > that.uvalue){
         result.uvalue = uvalue - that.uvalue;
         result.is_negative = is_negative;
      }
      else if (uvalue < that.uvalue){
         result.uvalue = that.uvalue - uvalue;
         result.is_negative = !(that.is_negative);
      }
      //If result is zero
      else{
         result.uvalue = 0;
         result.is_negative = false;
      }
   }

   return result;
}


bigint bigint::operator* (const bigint& that) const {
   bigint result = bigint();
   result.uvalue = uvalue * that.uvalue;
   // Determine Sign
   if (is_negative == that.is_negative) {result.is_negative = false;}
   else {result.is_negative = true;}
   
   return result;
}

bigint bigint::operator/ (const bigint& that) const {
   bigint result = uvalue / that.uvalue;
   // Determine Sign
   if (is_negative == that.is_negative) {result.is_negative = false;}
   else {result.is_negative = true;}
   return result;
}

bigint bigint::operator% (const bigint& that) const {
   bigint result = uvalue % that.uvalue;
   return result;
}

bool bigint::operator== (const bigint& that) const {
   return is_negative == that.is_negative and uvalue == that.uvalue;
}

bool bigint::operator< (const bigint& that) const {
   if (is_negative != that.is_negative) return is_negative;
   return is_negative ? uvalue > that.uvalue
                      : uvalue < that.uvalue;
}

ostream& operator<< (ostream& out, const bigint& that) {
   isNeg = that.is_negative;
   return out << (that.is_negative ? "-" : "")
              << that.uvalue;
}

