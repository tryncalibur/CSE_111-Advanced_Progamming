// $Id: ubigint.cpp,v 1.16 2019-04-02 16:28:42-07 - - $
// Trystan Nguyen trtanguy
// Leonard Tolentino lemtolen

#include <cctype>
#include <cstdlib>
#include <exception>
#include <stack>
#include <stdexcept>
#include <vector>

using namespace std;

#include "ubigint.h"
#include "relops.h"
//#include "debug.h"

ubigint::ubigint (unsigned long that) {
   while (that > 0){
      ubig_value.push_back(that % 10);
      that/= 10;
   }
   if (ubig_value.size() == 0) ubig_value.push_back(0);
   //DEBUGF ('~', this << " -> " << ubig_value);
}

ubigint::ubigint (const string& that) {
   for (char digit: that) {
      if (not isdigit (digit)) {
         throw invalid_argument ("ubigint::ubigint(" + that + ")");
      }
      ubig_value.emplace(ubig_value.begin(), (digit - '0'));
   }

   while (ubig_value.back() == 0 && ubig_value.size() > 1) {
      ubig_value.pop_back();
      }
   //DEBUGF ('~', "that = \"" << that << "\"");
}

ubigint ubigint::operator+ (const ubigint& that) const {
   ubigint re = ubigint();
   udigit_t carry = 0;
   udigit_t valueHolder = 0;
   unsigned int i = 0;;
   
   // Traverse both number sequence
   for (i = 0; i<ubig_value.size() && i<that.ubig_value.size(); ++i){
      // Do Operation 
      valueHolder = ubig_value[i] + that.ubig_value[i] + carry;

      // Determine Carry Over
      if (valueHolder >= 10){
         valueHolder -= 10;
         carry = 1;
      }
      else {carry = 0;}

      // Push Value
      re.ubig_value.push_back(valueHolder);
   }
   
   // Traverse remaining of bigger number sequence
   if (ubig_value.size() > that.ubig_value.size()){
      for (;i < ubig_value.size(); ++i){
         valueHolder = ubig_value[i] + carry;
            if (valueHolder >= 10){
               valueHolder -=10;
               carry = 1;
            }
            else {carry = 0;}
            re.ubig_value.push_back(valueHolder);
      }
   }
   else if (that.ubig_value.size() > ubig_value.size()){
      for (;i < that.ubig_value.size(); ++i){
         valueHolder = that.ubig_value[i] + carry;
            if (valueHolder >= 10){
               valueHolder -=10;
               carry = 1;
            }
            else {carry = 0;}
            re.ubig_value.push_back(valueHolder);
      }
   }
   if (carry != 0) re.ubig_value.push_back(carry);

   // Clean Leading zero
   while (re.ubig_value.back() == 0 && re.ubig_value.size() > 1) {
      re.ubig_value.pop_back();
   }

   return re;   
}

ubigint ubigint::operator- (const ubigint& that) const {
   if (*this < that) throw domain_error ("ubigint::operator-(a<b)");

   ubigint re = ubigint();
   udigit_t carry = 0;
   udigit_t placeHolder = 0;
   unsigned int i = 0;
   
   // Traverse both number sequence
   for (i = 0; i<ubig_value.size() && i<that.ubig_value.size(); ++i){
      // Do Operation and Carry
      if (ubig_value[i] >= that.ubig_value[i] + carry){
         placeHolder = ubig_value[i] - that.ubig_value[i] - carry;
         carry = 0;
      }
      else{
          placeHolder = ubig_value[i] + 10 - that.ubig_value[i] - carry;
          carry = 1;
      }

      // Push to result
      re.ubig_value.push_back(placeHolder);
   }

   // Traverse rest of bigger sequence
   if (ubig_value.size() > that.ubig_value.size()){
      for (;i < ubig_value.size(); ++i){
         if (ubig_value[i] > 0 || carry == 0) {
            placeHolder = ubig_value[i] - carry;
            carry = 0;
         }
         else{
            placeHolder = ubig_value[i] + 10 - carry;
            carry = 1;
         }
         re.ubig_value.push_back(placeHolder);
      }
   }
   else if (that.ubig_value.size() > ubig_value.size()){
      for (;i < that.ubig_value.size(); ++i){
         if (that.ubig_value[i] > 0 || carry == 0) {
            placeHolder = that.ubig_value[i] - carry;
            carry = 0;
         }
         else{
            placeHolder = that.ubig_value[i] + 10 - carry;
            carry = 1;
         }
         re.ubig_value.push_back(placeHolder);
      }
   }

   // Clean leading zero
   while (re.ubig_value.back() == 0 && re.ubig_value.size() > 1){
      re.ubig_value.pop_back();
      }
   return re;
}

ubigint ubigint::operator* (const ubigint& that) const {
   ubigint sum = ubigint();
   udigit_t carry = 0;
   udigit_t temp = 0;

   // Traverse each digit of second num (multiplier)
   for (size_t i = 0; i < that.ubig_value.size(); ++i){
      ubigint subsum = ubigint();
      carry = 0;

      // Indent Subsum
      for (size_t z = 0; z < i; ++z){
         subsum.ubig_value.push_back(0);
      }

      // Traverse digits of first num
      for(size_t k = 0; k < ubig_value.size(); ++k){
         temp = ubig_value[k] * that.ubig_value[i] + carry;
         if (temp >= 10){
            carry = temp / 10;
            temp %= 10;
         }
         else {carry = 0;}
         subsum.ubig_value.push_back(temp);
      }
      if (carry != 0) {subsum.ubig_value.push_back(carry);}

      sum = sum + subsum;
   }

   // Clean Leading zero
   while (sum.ubig_value.back() == 0 && sum.ubig_value.size() > 1) {
      sum.ubig_value.pop_back();
   }

   return sum;
}

void ubigint::multiply_by_2() {
   udigit_t temp = 0;
   udigit_t carry = 0;

   for (size_t i = 0; i < ubig_value.size(); ++i){
      temp = (ubig_value[i] * 2) + carry;
      if (temp >= 10){
         carry = temp / 10;
         temp = temp % 10;
      }
      else carry = 0;
      ubig_value[i] = temp;
   }
   if (carry != 0) ubig_value.push_back(carry);
}

void ubigint::divide_by_2() {
   unsigned int carry = 0;
   int temp = 0;
   for(int i = ubig_value.size()-1; i >= 0; i--){
      temp = static_cast<int> (ubig_value[i] + carry) / 2;
      carry = ((ubig_value[i] + carry) % 2) * 10;
      ubig_value[i] = temp;
   }
   while (ubig_value.back() == 0 && ubig_value.size() > 1) {
      ubig_value.pop_back();
   }
}


struct quo_rem { ubigint quotient; ubigint remainder; };
quo_rem udivide (const ubigint& dividend, const ubigint& divisor_) {
   // NOTE: udivide is a non-member function.
   ubigint divisor {divisor_};
   ubigint zero = ubigint(0);
   if (divisor == zero) throw domain_error ("udivide by zero");
   ubigint power_of_2 = ubigint(1);
   ubigint quotient = ubigint(0);
   ubigint remainder {dividend}; // left operand, dividend

   while (divisor < remainder) {
      divisor.multiply_by_2();
      power_of_2.multiply_by_2();
   }


   while (power_of_2 != zero) {
      if (divisor <= remainder) {
         remainder = remainder - divisor;
         quotient = quotient + power_of_2;
      }
      
      divisor.divide_by_2();
      power_of_2.divide_by_2();

      //cout << "Q: " << quotient << endl;
      //cout << "P: " << power_of_2 << endl;
   }

   quo_rem re = {quotient, remainder};
   return re;
}

ubigint ubigint::operator/ (const ubigint& that) const {
   ubigint re = udivide (*this, that).quotient;
   while (re.ubig_value.back() == 0 && re.ubig_value.size() > 1) {
      re.ubig_value.pop_back();
   }
   return re;
}

ubigint ubigint::operator% (const ubigint& that) const {
   ubigint re = udivide (*this, that).remainder;
   while (re.ubig_value.back() == 0 && re.ubig_value.size() > 1) {
      re.ubig_value.pop_back();
   }
   return re;
}

bool ubigint::operator== (const ubigint& that) const {
   // Check Size
   if (ubig_value.size() != that.ubig_value.size())
      return false;
   
   // Check Values
   for (size_t i = 0; i < ubig_value.size(); ++i){
      if (ubig_value[i] != that.ubig_value[i])
         return false;
   }
   return true;
}

bool ubigint::operator< (const ubigint& that) const {
   // Check Size
   if (ubig_value.size() < that.ubig_value.size())
      return true;
   if (ubig_value.size() > that.ubig_value.size())
      return false;

   // Check Values
   for (int i = ubig_value.size()-1; i >= 0; --i){
      if (ubig_value[i] > that.ubig_value[i]) return false;
      if (ubig_value[i] < that.ubig_value[i]) return true;
   }
   return false;
}

ostream& operator<< (ostream& out, const ubigint& that) { 
   
   int n = 0;
   if (isNeg) n = 1;
   int count = 0;;

   for (int i = that.ubig_value.size()-1; i >= 0; --i){
      if (count + n == 69){
         out << "\\\n";
         n = 0;
         count = 0;
      }

      out << (that.ubig_value[i] + 0);
      ++count;
   }

   if (that.ubig_value.size() == 0){out << 0;}
   
   return out;
}

