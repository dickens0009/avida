/*
 *  util/BitArray.cc
 *  avida-core
 *
 *  Copyright 1999-2014 Michigan State University. All rights reserved.
 *
 *
 *  This file is part of Avida.
 *
 *  Avida is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License
 *  as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 *
 *  Avida is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License along with Avida.
 *  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Authors: Charles Ofria <ofria@msu.edu>, Bess Walker, David M. Bryson <david@programerror.com>
 *
 */

#ifndef AvidaUtilBitArray_h
#define AvidaUtilBitArray_h

#include "avida/core/Types.h"

#include <cassert>
#include <iostream>


// Class: BitArray
// Desc: These classes handle an arbitrarily large array, and optimizes the operations on those bits to be as fast as possible.
//

// Constructors:
//  BitArray()                            -- Assume a size zero array.
//  BitArray(int in_size)                 -- Create an uninitialized array.
//  BitArray(const BitArray & in_array)  -- Copy Constructor

// Assignment and equality test:
//  BitArray & operator=(const BitArray & in_array)
//  bool operator==(const BitArray & in_array) const

// Sizing:
//  int GetSize() const
//  void Resize(const int new_size)
//  void ResizeClear(const int new_size)

// Accessors:
//  void Set(int index, bool value)
//  bool Get(int index) const
//  bool operator[](int index) const
//  cBitProxy operator[](int index)
//  void Clear()
//  void SetAll()

// Printing:
//  void Print(ostream & out=cout) const
//  void PrintOneIDs(ostream & out=cout) const

// Bit play:
//  int CountBits()   -- Count 1s -- fast for sparse arrays.
//  int CountBits2()  -- Count 1s -- fast for arbitary arrays.
//  int FindBit1(int start_bit)   -- Return pos of first 1 after start_bit 

// Boolean math functions:
//  BitArray NOT() const
//  BitArray AND(const BitArray & array2) const
//  BitArray OR(const BitArray & array2) const
//  BitArray NAND(const BitArray & array2) const
//  BitArray NOR(const BitArray & array2) const
//  BitArray XOR(const BitArray & array2) const
//  BitArray EQU(const BitArray & array2) const
//  BitArray SHIFT(const int shift_size) const   -- positive for left shift, negative for right shift

//  const BitArray & NOTSELF()
//  const BitArray & ANDSELF(const BitArray & array2)
//  const BitArray & ORSELF(const BitArray & array2)
//  const BitArray & NANDSELF(const BitArray & array2)
//  const BitArray & NORSELF(const BitArray & array2)
//  const BitArray & XORSELF(const BitArray & array2)
//  const BitArray & EQUSELF(const BitArray & array2)
//  const BitArray & SHIFTSELF(const int shift_size) const

// Arithmetic:
//  BitArray INCREMENTSELF()

// Operator overloads:
//  BitArray operator~() const
//  BitArray operator&(const BitArray & ar2) const
//  BitArray operator|(const BitArray & ar2) const
//  BitArray operator^(const BitArray & ar2) const
//  BitArray operator>>(const int) const
//  BitArray operator<<(const int) const
//  const BitArray & operator&=(const BitArray & ar2)
//  const BitArray & operator|=(const BitArray & ar2)
//  const BitArray & operator^=(const BitArray & ar2)
//  const BitArray & operator>>=(const int)
//  const BitArray & operator<<=(const int)
//  BitArray & operator++()     // prefix ++
//  BitArray & operator++(int)  // postfix ++



namespace Avida {
  namespace Util {
    
    // The following is an internal class used by BitArray It does not keep track of size, so this value must be passed in.
    class RawBitArray {
    private:
      unsigned int * bit_fields;
      
      // Disallow default copy constructor and operator=
      // (we need to know the number of bits we're working with!)
      RawBitArray(const RawBitArray&);
      const RawBitArray & operator=(const RawBitArray&);

      inline int GetNumFields(const int num_bits) const { return 1 + ((num_bits - 1) >> 5); }
      inline int GetField(const int index) const { return index >> 5; }
      inline int GetFieldPos(const int index) const { return index & 31; }
    public:
      RawBitArray() : bit_fields(NULL) { ; }
      ~RawBitArray() {
        if (bit_fields != NULL) {
          delete [] bit_fields;
        }
      }

      void Zero(const int num_bits) {
        const int num_fields = GetNumFields(num_bits);
        for (int i = 0; i < num_fields; i++) {
          bit_fields[i] = 0;
        }    
      }

      void Ones(const int num_bits) {
        const int num_fields = GetNumFields(num_bits);
        for (int i = 0; i < num_fields; i++) {
          bit_fields[i] = ~0;
        }    
        const int last_bit = GetFieldPos(num_bits);
        if (last_bit > 0) {
          bit_fields[num_fields - 1] &= (1 << last_bit) - 1;
        }
      }

      RawBitArray(const int num_bits) {
        const int num_fields = GetNumFields(num_bits);
        bit_fields = new unsigned int[ num_fields ];
        Zero(num_bits);
      }

      // The Copy() method and the Copy Constructor must both be told how many
      // bits they are working with.
      void Copy(const RawBitArray & in_array, const int num_bits);
      RawBitArray(const RawBitArray & in_array, const int num_bits)
        : bit_fields(NULL)
      {
        Copy(in_array, num_bits);
      }

      // For fast bit operations, we're not going to setup operator[]; instead
      // we're going to have a GetBit and a SetBit commamd.  For this raw version
      // we're also going to assume that the index is within range w/o any special
      // checks.
      bool GetBit(const int index) const{
        const int field_id = GetField(index);
        const int pos_id = GetFieldPos(index);
        return (bit_fields[field_id] & (1 << pos_id)) != 0;
      }

      void SetBit(const int index, const bool value) {
        const int field_id = GetField(index);
        const int pos_id = GetFieldPos(index);
        const int pos_mask = 1 << pos_id;

        if (value == false) {
          bit_fields[field_id] &= ~pos_mask;
        } else {
          bit_fields[field_id] |= pos_mask;
        }
      }

      bool IsEqual(const RawBitArray & in_array, int num_bits) const;

      void Resize(const int old_bits, const int new_bits);
      void ResizeSloppy(const int new_bits);
      void ResizeClear(const int new_bits);

      // Two different technique of bit counting...
      int CountBits(const int num_bits) const; // Better for sparse arrays
      int CountBits2(const int num_bits) const; // Better for dense arrays

      // Other bit-play
      int FindBit1(const int num_bits, const int start_pos) const;
      Apto::Array<int> GetOnes(const int num_bits) const;
      void ShiftLeft(const int num_bits, const int shift_size); // Helper: call SHIFT with positive number instead
      void ShiftRight(const int num_bits, const int shift_size); // Helper: call SHIFT with negative number instead

      void Print(const int num_bits, std::ostream& out = std::cout) const {
        for (int i = 0; i < num_bits; i++) {
          out << GetBit(i);
        }
      }
      
      // prints in the accepted human readable low-to-hight = right-to-left format, taking bit 0 as low bit
      void PrintRightToLeft(const int num_bits, std::ostream& out = std::cout) const {
        for (int i = num_bits - 1; i >= 0; i--) {
          out << GetBit(i);
        }
      }

      void PrintOneIDs(const int num_bits, std::ostream& out = std::cout) const {
        for (int i = 0; i < num_bits; i++) {
          if (GetBit(i) == true) {
      out << i << " ";
          }
        }
      }

      // Fast bool operators where we uses this bit array as one of the 
      // inputs and the place to store the results.
      void NOT(const int num_bits);
      void AND(const RawBitArray & array2, const int num_bits);
      void OR(const RawBitArray & array2, const int num_bits);
      void NAND(const RawBitArray & array2, const int num_bits);
      void NOR(const RawBitArray & array2, const int num_bits);
      void XOR(const RawBitArray & array2, const int num_bits);
      void EQU(const RawBitArray & array2, const int num_bits);
      void SHIFT(const int num_bits, const int shift_size);  // positive numbers for left and negative for right (0 does nothing)
      void INCREMENT(const int num_bits);

      // Fast bool operators where we load all of the inputs and store the
      // results here.
      void NOT(const RawBitArray & array1, const int num_bits);
      void AND(const RawBitArray & array1, const RawBitArray & array2,
         const int num_bits);
      void OR(const RawBitArray & array1, const RawBitArray & array2,
        const int num_bits);
      void NAND(const RawBitArray & array1, const RawBitArray & array2,
          const int num_bits);
      void NOR(const RawBitArray & array1, const RawBitArray & array2,
         const int num_bits);
      void XOR(const RawBitArray & array1, const RawBitArray & array2,
         const int num_bits);
      void EQU(const RawBitArray & array1, const RawBitArray & array2,
         const int num_bits);
      void SHIFT(const RawBitArray & array1, const int num_bits, const int shift_size);
      void INCREMENT(const RawBitArray & array1, const int num_bits);  // implemented for completeness, but unused by BitArray
    };

    class BitArray {
    private:
      RawBitArray bit_array;
      int array_size;

      // Setup a bit proxy so that we can use operator[] on bit arrays as a lvalue.
      class cBitProxy {
      private:
        BitArray & array;
        int index;
      public:
        cBitProxy(BitArray & _array, int _idx) : array(_array), index(_idx) {;}

        inline cBitProxy & operator=(bool b);    // lvalue handling...
        inline operator bool() const;            // rvalue handling...
      };
      friend class cBitProxy;
    public:
      BitArray() : array_size(0) { ; }
      BitArray(int in_size) : bit_array(in_size), array_size(in_size) { ; }
      BitArray(const BitArray & in_array)
        : bit_array(in_array.bit_array, in_array.array_size)
        , array_size(in_array.array_size) { ; }
      BitArray(const RawBitArray & in_array, int in_size)
        : bit_array(in_array, in_size)
        , array_size(in_size) { ; }

      BitArray & operator=(const BitArray & in_array) {
        bit_array.Copy(in_array.bit_array, in_array.array_size);
        array_size = in_array.array_size;
        return *this;
      }

      bool operator==(const BitArray & in_array) const {
        if (array_size != in_array.array_size) return false;
        return bit_array.IsEqual(in_array.bit_array, array_size);
      }

      int GetSize() const { return array_size; }

      void Set(int index, bool value) {
        assert(index < array_size);
        bit_array.SetBit(index, value);
      }

      bool Get(int index) const {
        assert(index < array_size);
        return bit_array.GetBit(index);
      }

      bool operator[](int index) const { return Get(index); }
      cBitProxy operator[](int index) { return cBitProxy(*this, index); }

      void Clear() { bit_array.Zero(array_size); }
      void SetAll() { bit_array.Ones(array_size); }
      

      void Print(std::ostream& out = std::cout) const { bit_array.Print(array_size, out); }
      void PrintRightToLeft(std::ostream& out = std::cout) const { bit_array.PrintRightToLeft(array_size, out); }
      void PrintOneIDs(std::ostream& out = std::cout) const
        { bit_array.PrintOneIDs(array_size, out); }
      void Resize(const int new_size) {
        bit_array.Resize(array_size, new_size);
        array_size = new_size;
      }
      void ResizeClear(const int new_size) {
        bit_array.ResizeClear(new_size);
        array_size = new_size;
      }
      int CountBits() const { return bit_array.CountBits(array_size); }
      int CountBits2() const { return bit_array.CountBits2(array_size); }

      int FindBit1(int start_bit=0) const
        { return bit_array.FindBit1(array_size, start_bit); }
      Apto::Array<int> GetOnes() const { return bit_array.GetOnes(array_size); }

      // Boolean math functions...
      BitArray NOT() const {
        BitArray out_array;
        out_array.bit_array.NOT(bit_array, array_size);
        out_array.array_size = array_size;
        return out_array;
      }

      BitArray AND(const BitArray & array2) const {
        assert(array_size == array2.array_size);
        BitArray out_array;
        out_array.bit_array.AND(bit_array, array2.bit_array, array_size);
        out_array.array_size = array_size;
        return out_array;
      }

      BitArray OR(const BitArray & array2) const {
        assert(array_size == array2.array_size);
        BitArray out_array;
        out_array.bit_array.OR(bit_array, array2.bit_array, array_size);
        out_array.array_size = array_size;
        return out_array;
      }

      BitArray NAND(const BitArray & array2) const {
        assert(array_size == array2.array_size);
        BitArray out_array;
        out_array.bit_array.NAND(bit_array, array2.bit_array, array_size);
        out_array.array_size = array_size;
        return out_array;
      }

      BitArray NOR(const BitArray & array2) const {
        assert(array_size == array2.array_size);
        BitArray out_array;
        out_array.bit_array.NOR(bit_array, array2.bit_array, array_size);
        out_array.array_size = array_size;
        return out_array;
      }

      BitArray XOR(const BitArray & array2) const {
        assert(array_size == array2.array_size);
        BitArray out_array;
        out_array.bit_array.XOR(bit_array, array2.bit_array, array_size);
        out_array.array_size = array_size;
        return out_array;
      }

      BitArray EQU(const BitArray & array2) const {
        assert(array_size == array2.array_size);
        BitArray out_array;
        out_array.bit_array.EQU(bit_array, array2.bit_array, array_size);
        out_array.array_size = array_size;
        return out_array;
      }
      
      BitArray SHIFT(const int shift_size) const {
        BitArray out_array;
        out_array.bit_array.SHIFT(bit_array, array_size, shift_size);
        out_array.array_size = array_size;
        return out_array;
      }

      const BitArray & NOTSELF() {
        bit_array.NOT(array_size);
        return *this;
      }

      const BitArray & ANDSELF(const BitArray & array2) {
        assert(array_size == array2.array_size);
        bit_array.AND(array2.bit_array, array_size);
        return *this;
      }

      const BitArray & ORSELF(const BitArray & array2) {
        assert(array_size == array2.array_size);
        bit_array.OR(array2.bit_array, array_size);
        return *this;
      }

      const BitArray & NANDSELF(const BitArray & array2) {
        assert(array_size == array2.array_size);
        bit_array.NAND(array2.bit_array, array_size);
        return *this;
      }

      const BitArray & NORSELF(const BitArray & array2) {
        assert(array_size == array2.array_size);
        bit_array.NOR(array2.bit_array, array_size);
        return *this;
      }

      const BitArray & XORSELF(const BitArray & array2) {
        assert(array_size == array2.array_size);
        bit_array.XOR(array2.bit_array, array_size);
        return *this;
      }

      const BitArray & EQUSELF(const BitArray & array2) {
        assert(array_size == array2.array_size);
        bit_array.EQU(array2.bit_array, array_size);
        return *this;
      }
      
      const BitArray & SHIFTSELF(const int shift_size) {
        bit_array.SHIFT(array_size, shift_size);
        return *this;
      }
      
      BitArray & INCREMENTSELF() {
        bit_array.INCREMENT(array_size);
        return *this;
      }
      

      // Operator overloads...
      BitArray operator~() const { return NOT(); }
      BitArray operator&(const BitArray & ar2) const { return AND(ar2); }
      BitArray operator|(const BitArray & ar2) const { return OR(ar2); }
      BitArray operator^(const BitArray & ar2) const { return XOR(ar2); }
      BitArray operator<<(const int shift_size) const { return SHIFT(shift_size); }
      BitArray operator>>(const int shift_size) const { return SHIFT(-shift_size); }
      const BitArray & operator&=(const BitArray & ar2) { return ANDSELF(ar2); }
      const BitArray & operator|=(const BitArray & ar2) { return ORSELF(ar2); }
      const BitArray & operator^=(const BitArray & ar2) { return XORSELF(ar2); }
      const BitArray & operator<<=(const int shift_size) { return SHIFTSELF(shift_size); }
      const BitArray & operator>>=(const int shift_size) { return SHIFTSELF(-shift_size); }
      BitArray & operator++() { return INCREMENTSELF(); }  // prefix ++
      BitArray operator++(int) { BitArray ans = *this; operator++(); return ans;}  // postfix ++

      bool operator<(const BitArray& ar2) const;
      inline bool operator<=(const BitArray& ar2) const { return operator<(ar2) || operator==(ar2); }
      inline bool operator>(const BitArray& ar2) const { return !operator<=(ar2); }
      inline bool operator>=(const BitArray& ar2) const { return !operator<(ar2); }
      
    };

    std::ostream& operator << (std::ostream& out, const BitArray & bit_array);

    BitArray::cBitProxy & BitArray::cBitProxy::operator=(bool b)
    {
      array.Set(index, b);
      return *this;
    }


    BitArray::cBitProxy::operator bool() const
    {
      return array.Get(index);
    }

  };
};



// Basic BitArray Hashing Support
// --------------------------------------------------------------------------------------------------------------

// HASH_TYPE = BitArray
// We hash a bit array by calculating the sum of the squared values of the
// positions where bits are on, then modding this number by the size of 
// the hash table
namespace Apto {
  template <class T, int HashFactor> class HashKey;
  template <int HashFactor> class HashKey<Avida::Util::BitArray, HashFactor>
  {
  public:
    static int Hash(const Avida::Util::BitArray& key)
    {
      unsigned int out_hash = 0;
      for (int i = 0; i < key.GetSize(); i++) {
        if (key.Get(i)) { out_hash += i*i; }
      }
      return out_hash % HashFactor;
    }
  };
};


#endif