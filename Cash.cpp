#include "stdafx.h"
#include <iostream>
#include "Cash.h"

using namespace std;

Cash::Cash(int dol, int q, int dime, int n, int p) {
	dollars = dol;
	quarters = q;
	dimes = dime;
	nickels = n;
	pennies = p;
	cash_total = calculate();
}
Cash::Cash() {
	dollars = 0;
	quarters = 0;
	dimes = 0;
	nickels = 0;
	pennies = 0;
	cash_total = 0;
}
double Cash::calculate() {
	double temp_dollars = pennies * 0.01;
	temp_dollars = temp_dollars + (nickels *0.05);
	temp_dollars = temp_dollars + (dimes * 0.1);
	temp_dollars = temp_dollars + (quarters * 0.25);
	temp_dollars = temp_dollars + (dollars);
	return temp_dollars;
}

void Cash::setPennies(int x) {
	pennies = x;
}

void Cash::setDimes(int x) {
	dimes = x;
}

void Cash::setNickels(int x) {
	nickels = x;
}

void Cash::setQuarters(int x) {
	quarters = x;
}

void Cash::setDollars(int x) {
	dollars = x;
}

int Cash::getPennies() {
	return pennies;
}

int Cash::getDimes() {
	return dimes;
}

int Cash::getNickels() {
	return nickels;
}

int Cash::getQuarters() {
	return quarters;
}

int Cash::getDollars() {
	return dollars;
}

void Cash::zero() {
	dollars = 0;
	quarters = 0;
	dimes = 0;
	nickels = 0;
	pennies = 0;
	cash_total = 0;
}