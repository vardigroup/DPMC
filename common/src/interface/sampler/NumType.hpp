#ifndef NUMTYPE_H_
#define NUMTYPE_H_

#include <cstdint>
#include <cmath>
#include "sampler/RandomBits.hpp"

using Int = int_fast64_t;
using Float = long double;

class NumType{
	public:
		virtual ~NumType() = 0;
		virtual NumType* copy() = 0;
		
		virtual NumType* add(NumType*) = 0;
		virtual void addSelf(NumType*) = 0;
		virtual void addPowerOfTwoSelf(Int) = 0;
		virtual void addDoubleSelf(double_t) = 0;
		virtual void addLongDoubleSelf(long double) = 0;
		virtual void addProductWithPowerOfTwoSelf(NumType*, Int) =0;
		virtual void addProductWithDoubleSelf(NumType*, double_t) = 0;
		virtual void addProductWithLongDoubleSelf(NumType*, long double) = 0;

		virtual NumType* multiply(NumType*) = 0;
		virtual void multiplySelf(NumType*) = 0;
		virtual NumType* multiplyByDouble(double_t) = 0;
		virtual void multiplyByDoubleSelf(double_t) = 0;
		virtual NumType* multiplyByLongDouble(long double) = 0;
		virtual void multiplyByLongDoubleSelf(long double) = 0;
		virtual NumType* multiplyByPowerOfTwo(Int) = 0;
		virtual void multiplyByPowerOfTwoSelf(Int) = 0;
		
		virtual NumType* divideBy(NumType*) = 0;
		virtual void divideBySelf(NumType*) = 0;
		
		virtual void set(NumType* other) = 0;
		virtual void setPowerOfTwo(Int) = 0;
		virtual void setDouble(double_t) = 0;
		virtual void setLongDouble(long double) = 0;
		virtual void setRandom(NumType*,RandomBits *) = 0;

		virtual NumType* getRandom(RandomBits *) = 0; //returns a random number between 1 and this-number

		virtual bool lessThan(NumType*) = 0;
		virtual bool isPositive() = 0;
		virtual bool isNonNegative() = 0;
		virtual bool isCloseToOne(double_t) = 0;
		virtual bool isZero() = 0;

		virtual void print() = 0;
};


class DoubleNumType : public NumType{
	public:
		DoubleNumType(double_t);
		~DoubleNumType() override;
		DoubleNumType* copy() override;
		
		DoubleNumType* add(NumType*) override;
		void addSelf(NumType*) override;
		void addPowerOfTwoSelf(Int) override;
		void addDoubleSelf(double_t) override;
		void addLongDoubleSelf(long double)  override;
		void addProductWithPowerOfTwoSelf(NumType*, Int)  override;
		void addProductWithDoubleSelf(NumType*, double_t)  override;
		void addProductWithLongDoubleSelf(NumType*, long double)  override;

		DoubleNumType* multiply(NumType*) override;
		void multiplySelf(NumType*) override;
		DoubleNumType* multiplyByDouble(double_t) override;
		void multiplyByDoubleSelf(double_t) override;
		DoubleNumType* multiplyByLongDouble(long double) override;
		void multiplyByLongDoubleSelf(long double) override;
		DoubleNumType* multiplyByPowerOfTwo(Int) override;
		void multiplyByPowerOfTwoSelf(Int) override;
		
		DoubleNumType* divideBy(NumType*) override;
		void divideBySelf(NumType*) override;
		
		void set(NumType* other) override;
		void setPowerOfTwo(Int) override;
		void setDouble(double_t) override;
		void setLongDouble(long double) override;
		void setRandom(NumType*,RandomBits *) override;

		DoubleNumType* getRandom(RandomBits *) override;

		bool lessThan(NumType*) override;
		bool isPositive() override;
		bool isNonNegative() override;
		bool isCloseToOne(double_t) override;
		bool isZero() override;

		void print() override;
		// double_t number;
	private:
		double_t number;
		static std::uniform_real_distribution<double_t> unif;
};

class LongDoubleNumType : public NumType{
	public:
		LongDoubleNumType(double_t);
		~LongDoubleNumType() override;
		LongDoubleNumType* copy() override;
		
		LongDoubleNumType* add(NumType*) override;
		void addSelf(NumType*) override;
		void addPowerOfTwoSelf(Int) override;
		void addDoubleSelf(double_t) override;
		void addLongDoubleSelf(long double)  override;
		void addProductWithPowerOfTwoSelf(NumType*, Int)  override;
		void addProductWithDoubleSelf(NumType*, double_t)  override;
		void addProductWithLongDoubleSelf(NumType*, long double)  override;

		LongDoubleNumType* multiply(NumType*) override;
		void multiplySelf(NumType*) override;
		LongDoubleNumType* multiplyByDouble(double_t) override;
		void multiplyByDoubleSelf(double_t) override;
		LongDoubleNumType* multiplyByLongDouble(long double) override;
		void multiplyByLongDoubleSelf(long double) override;
		LongDoubleNumType* multiplyByPowerOfTwo(Int) override;
		void multiplyByPowerOfTwoSelf(Int) override;
		
		LongDoubleNumType* divideBy(NumType*) override;
		void divideBySelf(NumType*) override;
		
		void set(NumType* other) override;
		void setPowerOfTwo(Int) override;
		void setDouble(double_t) override;
		void setLongDouble(long double) override;
		void setRandom(NumType*,RandomBits *) override;

		LongDoubleNumType* getRandom(RandomBits *) override;

		bool lessThan(NumType*) override;
		bool isPositive() override;
		bool isNonNegative() override;
		bool isCloseToOne(double_t) override;
		bool isZero() override;

		void print() override;
	private:
		long double number;
		static std::uniform_real_distribution<long double> unif;
};

#endif