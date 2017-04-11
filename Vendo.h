#pragma once
#ifndef VENDO_H_
#define VENDO_H_
#endif
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdio>
#include <vector>

using namespace std;

struct s_product {
	int v_loc_id;
	int v_loc_horiz;
	int v_loc_vert;
	string v_name;
	double v_price;
	int v_qty;
};

class Vendo {
private:
	double v_lifetime_sales;
	vector<s_product> v_product;
	int v_plist_size;
	double v_cash_on_hand;
	int v_lifetime_qty_sales;
	int v_service_passwd;
	bool b_jammed;
public:
	Vendo::Vendo(int);
	double getLifetimeSales(); //lifetime sales dollars
	int getLifetimeQtySales();// lifetime sales qty
	bool servicePasswordCheck(int p); //is this the correct svc pwd
	bool serviceMenuChoice(int v_choice); //returns false when its time to exit
	bool checkEmpty();
	void vendMenu();
	void displayStats();
	void displayProductList();
	void displayProductList(double v_cashola);
	bool vendMenuChoice(int v_choice);
	//bool printMenu();
	double collectCash(); //get money get paid
	void clearJam(); //sets jam=false but only if the system is not in error
	void setJam(); //sets jam = true
	void addSalesQty(int sq); //dispense method 1 - add to lifetime qty
	void addSalesDollars(double sd);
	void addCashOnHand(double c);
	bool dispenseItem(int v_prod);
	bool dispenseItem(double cash);
	bool checkJam();
	bool theVendoGambit();
	double getCashOnHand();
	int serviceMenu(int);
};