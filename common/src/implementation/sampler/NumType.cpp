#include <cstdio>
#include <cmath>
#include <math.h>
#include "sampler/NumType.hpp"


NumType::~NumType(){}

double_t lb = std::nextafter(0.0,1.0);
std::uniform_real_distribution<double_t> DoubleNumType::unif(lb,
	std::nextafter(1.0,std::numeric_limits<double>::max()));
DoubleNumType::DoubleNumType(double_t num_):number(num_){}

DoubleNumType::~DoubleNumType(){}

DoubleNumType* DoubleNumType::copy(){
	return new DoubleNumType(this->number);
}

DoubleNumType* DoubleNumType::add(NumType* other){
	return new DoubleNumType(this->number + ((DoubleNumType*)other)->number);
}

void DoubleNumType::addSelf(NumType* other){
	this->number += ((DoubleNumType*)other)->number;
}

void DoubleNumType::addPowerOfTwoSelf(Int n){
	this->number += pow(2,n);
}

void DoubleNumType::addDoubleSelf(double_t other){
	this->number += other;
}

void DoubleNumType::addLongDoubleSelf(long double other){
	this->number += other;
}

void DoubleNumType::addProductWithPowerOfTwoSelf(NumType* other, Int n){
	this->number += ((DoubleNumType*)other)->number*pow(2,n);
}

void DoubleNumType::addProductWithDoubleSelf(NumType* other, double_t other2){
	this->number += ((DoubleNumType*)other)->number*other2;
}

void DoubleNumType::addProductWithLongDoubleSelf(NumType* other, long double other2){
	this->number += ((DoubleNumType*)other)->number*other2;
}

DoubleNumType* DoubleNumType::multiply(NumType* other){
	return new DoubleNumType(this->number * ((DoubleNumType*)other)->number);
}

void DoubleNumType::multiplySelf(NumType* other){
	this->number *= ((DoubleNumType*)other)->number;
}

DoubleNumType* DoubleNumType::multiplyByDouble(double_t other){
	return new DoubleNumType(this->number * other);
}

void DoubleNumType::multiplyByDoubleSelf(double_t other){
	this->number *= other;
}

DoubleNumType* DoubleNumType::multiplyByLongDouble(long double other){
	return new DoubleNumType(this->number * other);
}

void DoubleNumType::multiplyByLongDoubleSelf(long double other){
	this->number *= other;
}

DoubleNumType* DoubleNumType::multiplyByPowerOfTwo(Int n){
	return new DoubleNumType(this->number * pow(2,n));
}

void DoubleNumType::multiplyByPowerOfTwoSelf(Int n){
	this->number *= pow(2,n);
}

DoubleNumType* DoubleNumType::divideBy(NumType* other){
	return new DoubleNumType(this->number / ((DoubleNumType*)other)->number);
}

void DoubleNumType::divideBySelf(NumType* other){
	this->number = this->number / ((DoubleNumType*)other)->number;
}

void DoubleNumType::set(NumType* other){
	this->number = ((DoubleNumType*)other)->number;
}

void  DoubleNumType::setPowerOfTwo(Int n){
	this->number = pow(2,n);
}

void  DoubleNumType::setDouble(double_t other){
	this->number = other;
}

void  DoubleNumType::setLongDouble(long double other){
	this->number = other;
}

void DoubleNumType::setRandom(NumType* other, RandomBits *rb){
	//the upper bound of unif is not inclusive. Technically we can set it to be the double number just greater than
	//this->number using some stl functions, but for simplicity we ignore it. this->number+1 can (and does) 
	//give errors because some number between this->number and this->number+1 gets sampled sometimes
	double_t ub = std::nextafter(((DoubleNumType*)other)->number,std::numeric_limits<double>::max());
	DoubleNumType::unif = std::uniform_real_distribution<double_t>(lb,ub);
	this->number = rb->getRandReal(DoubleNumType::unif);
}

bool DoubleNumType::lessThan(NumType* other){
	return (this->number) < (((DoubleNumType*)other)->number);
}

bool DoubleNumType::isPositive(){
	return (this->number) > 0;
}

bool DoubleNumType::isNonNegative() {
	return (this->number) >= 0;
}

bool DoubleNumType::isCloseToOne(double_t eps) {
	//printf("\n%g\n",this->number);
	return (std::abs((this->number)-1)<=eps);
}

bool DoubleNumType::isZero(){
	return (this->number)==0;
}

DoubleNumType* DoubleNumType::getRandom(RandomBits *rb){
	//the upper bound of unif is not inclusive. Technically we can set it to be the double number just greater than
	//this->number using some stl functions, but for simplicity we ignore it. this->number+1 can (and does) 
	//give errors because some number between this->number and this->number+1 gets sampled sometimes
	//DoubleNumType::unif.param(std::uniform_real_distribution<double_t>::param_type(1, this->number));
	double_t ub = std::nextafter(this->number,std::numeric_limits<double>::max());
	DoubleNumType::unif = std::uniform_real_distribution<double_t>(lb,ub);
	return new DoubleNumType(rb->getRandReal(DoubleNumType::unif));
	//return new DoubleNumType(DoubleNumType::rb->getRandReal(DoubleNumType::unif)*(this->number));
}

void DoubleNumType::print(){
	printf("%lf",this->number);
}

/*=======================================================================================*/

long double lbl = std::nextafter(0.0l,1.0l);
std::uniform_real_distribution<long double> LongDoubleNumType::unif(lb,
	std::nextafter(1.0l,std::numeric_limits<long double>::max()));
LongDoubleNumType::LongDoubleNumType(double_t num_):number(num_){}

LongDoubleNumType::~LongDoubleNumType(){}

LongDoubleNumType* LongDoubleNumType::copy(){
	return new LongDoubleNumType(this->number);
}

LongDoubleNumType* LongDoubleNumType::add(NumType* other){
	return new LongDoubleNumType(this->number + ((LongDoubleNumType*)other)->number);
}

void LongDoubleNumType::addSelf(NumType* other){
	this->number += ((LongDoubleNumType*)other)->number;
}

void LongDoubleNumType::addPowerOfTwoSelf(Int n){
	this->number += pow(2,n);
}

void LongDoubleNumType::addDoubleSelf(double_t other){
	this->number += other;
}

void LongDoubleNumType::addLongDoubleSelf(long double other){
	this->number += other;
}

void LongDoubleNumType::addProductWithPowerOfTwoSelf(NumType* other, Int n){
	this->number += ((LongDoubleNumType*)other)->number*pow(2,n);
}

void LongDoubleNumType::addProductWithDoubleSelf(NumType* other, double_t other2){
	this->number += ((LongDoubleNumType*)other)->number*other2;
}

void LongDoubleNumType::addProductWithLongDoubleSelf(NumType* other, long double other2){
	this->number += ((LongDoubleNumType*)other)->number*other2;
}


LongDoubleNumType* LongDoubleNumType::multiply(NumType* other){
	return new LongDoubleNumType(this->number * ((LongDoubleNumType*)other)->number);
}

void LongDoubleNumType::multiplySelf(NumType* other){
	this->number *= ((LongDoubleNumType*)other)->number;
}

LongDoubleNumType* LongDoubleNumType::multiplyByDouble(double_t other){
	return new LongDoubleNumType(this->number * other);
}

void LongDoubleNumType::multiplyByDoubleSelf(double_t other){
	this->number *= other;
}

LongDoubleNumType* LongDoubleNumType::multiplyByLongDouble(long double other){
	return new LongDoubleNumType(this->number * other);
}

void LongDoubleNumType::multiplyByLongDoubleSelf(long double other){
	this->number *= other;
}


LongDoubleNumType* LongDoubleNumType::multiplyByPowerOfTwo(Int n){
	return new LongDoubleNumType(this->number * pow(2,n));
}

void LongDoubleNumType::multiplyByPowerOfTwoSelf(Int n){
	this->number *= pow(2,n);
}

LongDoubleNumType* LongDoubleNumType::divideBy(NumType* other){
	return new LongDoubleNumType(this->number / ((LongDoubleNumType*)other)->number);
}

void LongDoubleNumType::divideBySelf(NumType* other){
	this->number = this->number / ((LongDoubleNumType*)other)->number;
}

void LongDoubleNumType::set(NumType* other){
	this->number = ((LongDoubleNumType*)other)->number;
}

void  LongDoubleNumType::setPowerOfTwo(Int n){
	this->number = pow(2,n);
}

void  LongDoubleNumType::setDouble(double_t other){
	this->number = other;
}

void  LongDoubleNumType::setLongDouble(long double other){
	this->number = other;
}

void LongDoubleNumType::setRandom(NumType* other,RandomBits *rb){
	//the upper bound of unif is not inclusive. Technically we can set it to be the double number just greater than
	//this->number using some stl functions, but for simplicity we ignore it. this->number+1 can (and does) 
	//give errors because some number between this->number and this->number+1 gets sampled sometimes
	long double ubl = std::nextafter(((LongDoubleNumType*)other)->number,std::numeric_limits<long double>::max());
	LongDoubleNumType::unif = std::uniform_real_distribution<long double>(lbl,ubl);
	this->number = rb->getRandReal(LongDoubleNumType::unif);
}

bool LongDoubleNumType::lessThan(NumType* other){
	return (this->number) < (((LongDoubleNumType*)other)->number);
}

bool LongDoubleNumType::isPositive(){
	return (this->number) > 0;
}

bool LongDoubleNumType::isNonNegative() {
	return (this->number) >= 0;
}

bool LongDoubleNumType::isCloseToOne(double_t eps) {
	//printf("\n%g\n",this->number);
	return (std::abs((this->number)-1)<=eps);
}

bool LongDoubleNumType::isZero(){
	return (this->number)==0;
}

LongDoubleNumType* LongDoubleNumType::getRandom(RandomBits *rb){
	//the upper bound of unif is not inclusive. Technically we can set it to be the longdouble number just greater than
	//this->number using some stl functions, but for simplicity we ignore it. this->number+1 can (and does) 
	//give errors because some number between this->number and this->number+1 gets sampled sometimes
	//LongDoubleNumType::unif.param(std::uniform_real_distribution<long double>::param_type(1, this->number));
	//return new LongDoubleNumType(LongDoubleNumType::rb->getRandReal(LongDoubleNumType::unif)*(this->number-1)+1);
	long double ubl = std::nextafter(this->number,std::numeric_limits<long double>::max());
	LongDoubleNumType::unif = std::uniform_real_distribution<long double>(lbl,ubl);
	return new LongDoubleNumType(rb->getRandReal(LongDoubleNumType::unif));
	//return new LongDoubleNumType(LongDoubleNumType::rb->getRandReal(LongDoubleNumType::unif)*(this->number));
}


void LongDoubleNumType::print(){
	printf("%Lf",this->number);
}