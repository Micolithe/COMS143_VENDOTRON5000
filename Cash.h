#pragma once
#ifndef CASH_H_
#define CASH_H_
#endif

class Cash {
private:
	int dollars;
	int quarters;
	int dimes;
	int nickels;
	int pennies;
	double cash_total;
public:
	Cash::Cash(int, int, int, int, int);
	Cash::Cash();
	double calculate();
	void setPennies(int x);
	void setDimes(int x);
	void setNickels(int x);
	void setQuarters(int x);
	void setDollars(int x);
	void zero();
	int getPennies();
	int getDimes();
	int getNickels();
	int getQuarters();
	int getDollars();

};