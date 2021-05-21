#include "sampler/GMPNumType.hpp"
#include <cstdio>
#include <iostream>

gmp_randstate_t MPZNumType::randState;

bool MPZNumType::initDone = false;

MPZNumType::MPZNumType(double_t num_):number(num_){
	std::cout<<"WARNING: MPZ class cannot store floating point values. Problems may occur for weighted sampling. Use mpq class instead\n";
}

MPZNumType::MPZNumType(mpz_class num_):number(num_){
		std::cout<<"WARNING: MPZ class cannot store floating point values. Problems may occur for weighted sampling. Use mpq class instead\n";
}

MPZNumType::~MPZNumType(){
	//no need to delete since not allocated dynamically?
}

void MPZNumType::initRandom(RandomBits* rb, Int nBits=1500){
	gmp_randinit_mt (randState);
	mpz_t temp;
	mpz_init(temp);
	mpz_set_str(temp,rb->GenerateRandomBits(nBits).c_str(),2);
	gmp_randseed(randState, temp);
	mpz_clear(temp);
	MPZNumType::initDone = true;
}

MPZNumType* MPZNumType::copy(){
	return new MPZNumType(this->number);
}

MPZNumType* MPZNumType::add(NumType* other){
	return new MPZNumType(this->number + ((MPZNumType*)other)->number);
}

void MPZNumType::addSelf(NumType* other){
	this->number += ((MPZNumType*)other)->number;
}

void MPZNumType::addPowerOfTwoSelf(Int n){
	mpz_t temp;
	mpz_init(temp);
	mpz_ui_pow_ui(temp,2,n);
	mpz_add(this->number.get_mpz_t(),this->number.get_mpz_t(),temp);
	mpz_clear(temp);
}

void MPZNumType::addProductWithPowerOfTwoSelf(NumType* other, Int n){
	mpz_t temp;
	mpz_init(temp);
	mpz_ui_pow_ui(temp,2,n);
	mpz_mul(temp,((MPZNumType*)other)->number.get_mpz_t(),temp);
	mpz_add(this->number.get_mpz_t(),this->number.get_mpz_t(),temp);
	mpz_clear(temp);
}

void MPZNumType::addDoubleSelf(double_t other){
	this->number += other;
}

void MPZNumType::addLongDoubleSelf(long double other){
	this->number += (double)other; //not defined for long dbl
}

void MPZNumType::addProductWithDoubleSelf(NumType* other, double_t other2){
	this->number += ((MPZNumType*)other)->number*other2;
}

void MPZNumType::addProductWithLongDoubleSelf(NumType* other, long double other2){
	this->number += ((MPZNumType*)other)->number*(double_t)other2;//not defined for lng
}

MPZNumType* MPZNumType::multiply(NumType* other){
	return new MPZNumType(this->number * ((MPZNumType*)other)->number);
}

void MPZNumType::multiplySelf(NumType* other){
	this->number *= ((MPZNumType*)other)->number;
}

MPZNumType* MPZNumType::multiplyByDouble(double_t other){
	return new MPZNumType(this->number * other);
}

void MPZNumType::multiplyByDoubleSelf(double_t other){
	this->number *= other;
}

MPZNumType* MPZNumType::multiplyByLongDouble(long double other){
	return new MPZNumType(this->number * (double_t)other);
}

void MPZNumType::multiplyByLongDoubleSelf(long double other){
	this->number *= (double_t) other; // not defined for long dbl
}

MPZNumType* MPZNumType::multiplyByPowerOfTwo(Int n){
	mpz_t temp;
	mpz_init(temp);
	mpz_ui_pow_ui(temp,2,n);
	MPZNumType* mpznew = new MPZNumType(1);
	mpz_mul(mpznew->number.get_mpz_t(),this->number.get_mpz_t(),temp);
	mpz_clear(temp);
	return mpznew;
}

void MPZNumType::multiplyByPowerOfTwoSelf(Int n){
	mpz_t temp;
	mpz_init(temp);
	mpz_ui_pow_ui(temp,2,n);
	mpz_mul(this->number.get_mpz_t(),this->number.get_mpz_t(),temp);
	mpz_clear(temp);
}

MPZNumType* MPZNumType::divideBy(NumType* other){
	return new MPZNumType(this->number / ((MPZNumType*)other)->number);
}

void MPZNumType::divideBySelf(NumType* other){
	this->number /= ((MPZNumType*)other)->number;
}

void MPZNumType::set(NumType* other){
	this->number = ((MPZNumType*)other)->number;
}

void  MPZNumType::setPowerOfTwo(Int n){
	mpz_t temp;
	mpz_init(temp);
	mpz_ui_pow_ui(temp,2,n);
	// not using mpz_set(this->number.get_mpz_t(),temp); since not sure if it will cause mem leaks. instead..
	this->number = mpz_class(temp);
	mpz_clear(temp);
}

void  MPZNumType::setDouble(double_t other){
	this->number = other;
}

void  MPZNumType::setLongDouble(long double other){
	this->number = (double_t) other; // not defined for long dbl
 }

void MPZNumType::setRandom(NumType* other,RandomBits * rb){
	if(!MPZNumType::initDone) MPZNumType::initRandom(rb);
	mpz_t temp;
	mpz_init(temp);
	mpz_urandomm(temp,randState, ((MPZNumType*)other)->number.get_mpz_t()); // returns num between 0 and (this->number - 1)
 	mpz_add_ui(temp,temp,1); //add 1 to make the range 1, this->number-1 inclusive
	this->number = mpz_class(temp);
	mpz_clear(temp);
}

bool MPZNumType::lessThan(NumType* other){
	return (this->number) < (((MPZNumType*)other)->number);
}

bool MPZNumType::isPositive(){
	return (this->number) > 0;
}

bool MPZNumType::isNonNegative() {
	return (this->number) >= 0;
}

bool MPZNumType::isCloseToOne(double_t eps) {
	printf("\nShould not need isCloseToOne() func. Exiting\n");
	exit(1);
}

bool MPZNumType::isZero(){
	return (this->number==0);
}

MPZNumType* MPZNumType::getRandom(RandomBits *rb){
	if(!MPZNumType::initDone) MPZNumType::initRandom(rb);
	mpz_t temp;
	mpz_init(temp);
	mpz_urandomm(temp,randState, this->number.get_mpz_t()); // returns num between 0 and (this->number - 1)
 	mpz_add_ui(temp,temp,1); //add 1 to make the range 1, this->number-1 inclusive
	MPZNumType* res = new MPZNumType(std::move(mpz_class(temp)));
	mpz_clear(temp);
	return res;
}


void MPZNumType::print(){
	printf("%s",this->number.get_str().c_str());
}