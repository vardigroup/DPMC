#ifndef GMPNUMTYPE_H_
#define GMPNUMTYPE_H_

#include "sampler/RandomBits.hpp"
#include "sampler/NumType.hpp"
#include "gmp.h"
#include "gmpxx.h"

class MPZNumType : public NumType{
	public:
		static void initRandom(RandomBits* rb, Int nBits);
		
		MPZNumType(double_t);
		MPZNumType(mpz_class num_);
		~MPZNumType() override;
		MPZNumType* copy() override;
		
		MPZNumType* add(NumType*) override;
		void addSelf(NumType*) override;
		void addPowerOfTwoSelf(Int) override;
		void addDoubleSelf(double_t) override;
		void addLongDoubleSelf(long double)  override;
		void addProductWithPowerOfTwoSelf(NumType*, Int)  override;
		void addProductWithDoubleSelf(NumType*, double_t)  override;
		void addProductWithLongDoubleSelf(NumType*, long double)  override;

		MPZNumType* multiply(NumType*) override;
		void multiplySelf(NumType*) override;
		MPZNumType* multiplyByDouble(double_t) override;
		void multiplyByDoubleSelf(double_t) override;
		MPZNumType* multiplyByLongDouble(long double) override;
		void multiplyByLongDoubleSelf(long double) override;
		MPZNumType* multiplyByPowerOfTwo(Int) override;
		void multiplyByPowerOfTwoSelf(Int) override;
		
		MPZNumType* divideBy(NumType*) override;
		void divideBySelf(NumType*) override;
		
		void set(NumType* other) override;
		void setPowerOfTwo(Int) override;
		void setDouble(double_t) override;
		void setLongDouble(long double) override;
		void setRandom(NumType* other,RandomBits *) override;

		MPZNumType* getRandom(RandomBits *) override;

		void print() override;

		bool lessThan(NumType*) override;
		bool isPositive() override;
		bool isNonNegative() override;
		bool isCloseToOne(double_t) override;
		bool isZero() override;
	private:
		mpz_class number;
		static gmp_randstate_t randState;
		static bool initDone;
};

#endif