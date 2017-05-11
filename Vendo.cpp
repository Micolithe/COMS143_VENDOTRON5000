#include "stdafx.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdio>
#include <vector>
#include "Vendo.h"
#include "Cash.h"
#include <windows.h>
#include <time.h>
#include <ctime>

//#define BLOCK = wchar_t(178);


using namespace std;
//This is the Vendo Tron 5000 - The Ultimate in Vending Technology.
//Please use caution in your dealings with Vendo.

Vendo::Vendo(int pwd)
{
	srand(time(NULL)); //gotta seed rand with the time
	v_service_passwd = pwd;
	v_lifetime_sales = 0;
	v_lifetime_qty_sales = 0;
	v_cash_on_hand = 0;
	b_jammed = false;
	bool v_fatal = false;

	fstream f_change("VEND_CHANGE.CSV", fstream::in);
	f_change << std::noskipws;
	string v_str_pen;
	string v_str_nick;
	string v_str_dime;
	string v_str_quar;
	string v_str_doll;
	getline(f_change, v_str_doll, ',');
	getline(f_change, v_str_quar, ',');
	getline(f_change, v_str_dime, ','); 
	getline(f_change, v_str_nick, ',');
	getline(f_change, v_str_pen);
	//if (!f_change.good() || f_change.bad()) {
	//	v_fatal = true;
	//	cout << "[FATAL] Error reading the change file! Please check for VEND_CHANGE.CSV, that it exists & is readable..." << endl;
	//}
	f_change.close();
	change_dollars = stoi(v_str_doll);
	change_quarters = stoi(v_str_quar);
	change_dimes = stoi(v_str_dime);
	change_nickels = stoi(v_str_nick);
	change_pennies = stoi(v_str_pen);

	v_coin = Cash(change_dollars, change_quarters, change_dimes, change_nickels, change_pennies);

	fstream f_persist("VEND_PERSIST.CSV", fstream::in);
	f_persist << std::noskipws;
	string v_str_lifetime_qty_sales;
	string v_str_lifetime_sales;
	string v_str_cashonhand;
	getline(f_persist, v_str_lifetime_qty_sales, ','); 
	getline(f_persist, v_str_lifetime_sales, ',');
	getline(f_persist, v_str_cashonhand);

	//if (!f_persist.good() || f_persist.bad()) {
	//	v_fatal = true;
	//	cout << "[FATAL] Error reading the stats file! Please check for VEND_PERSIST.CSV, that it exists & is readable..." << endl;
	//}
	
	v_lifetime_sales = stod(v_str_lifetime_sales);
	v_lifetime_qty_sales = stoi(v_str_lifetime_qty_sales);
	v_cash_on_hand = stod(v_str_cashonhand);
	f_persist.close();
	fstream f_pricing("VEND_PRODUCTS.CSV", fstream::in);  //get product definition file
	f_pricing >> std::noskipws; //spaces are allowed its pretty important the program will crash & burn without this line.
	cout << "Opening product definitions file for reading: VEND_PRODUCTS.CSV" << endl;
	string v_loc_id_temp; //getline will only do strings so i need temp string vars for this.
	string v_name_temp;
	string v_price_temp;
	string v_qty_temp;
	int v_horiz_temp;
	int v_vert_temp;

	int loc = 0;
	double pr = 0;
	int q = 0;
	//Read the file, place each line into the struct
	if (!f_pricing.good() || f_pricing.bad()) {
		v_fatal = true;
		cout << "[FATAL] Error reading the pricing file! Please check for VEND_PRODUCTS.CSV, that it exists & is readable..." << endl;
		Sleep(10000);
	}
	while (f_pricing.good() || !f_pricing.eof())
	{
		getline(f_pricing, v_loc_id_temp, ','); // COLUMN 1: LOCATION
		getline(f_pricing, v_name_temp, ','); // COLUMN 2: NAME
		getline(f_pricing, v_price_temp, ','); // COLUMN 3: PRICE
		getline(f_pricing, v_qty_temp); // COLUMN 4: QTY

		if (v_loc_id_temp == "" || v_name_temp == "" || v_price_temp == "" || v_qty_temp == "") { //Sanity Check...
			continue; //extraneous blank row or missing data will cause a crash so dont do it.
		}
		try {
			loc = stoi(v_loc_id_temp); //bless your fucking heart whoever wrote stoi & stod
			pr = stod(v_price_temp);
			q = stoi(v_qty_temp);
		}
		catch (int e) {
			v_fatal = true;
			cout << "[FATAL] Unable to parse CSV properly..." << e << endl;
			Sleep(10000);
		}
		if (loc <= 0 || pr < 0 || q < 0) {
			v_fatal = true;
			cout << "[FATAL] Invalid data detected in product file. All numeric values should be positive." << endl;
		}
		v_horiz_temp = loc % 100;
		v_vert_temp = loc / 100;
		v_product.push_back({ loc, v_horiz_temp, v_vert_temp, v_name_temp, pr, q }); //store the line we just pulled by adding it to the vector
	};
	f_pricing.close();//OK we're done with the CSV lets close it because its nice to do that

	int p_vec_size = v_product.size();//how many lines did we read? that file could be any number of rows.

	cout << "File reading completed. Read " << p_vec_size << " rows." << endl << endl;

	//This is the Test It Block so that I can know I havent fucked up yet.
	for (int i = 0; i < p_vec_size; i++) {
		cout << "[PRODUCT] " << v_product[i].v_loc_id << " " << v_product[i].v_name << " "
			<< v_product[i].v_price << " " << v_product[i].v_qty << endl;
	}
	cout << endl << "Data load from CSV completed." << endl << endl;
	//End Initialization Phase...
	if (p_vec_size == 0) {
		v_fatal = true;
		cout << "[FATAL] The product file was empty. Cannot continue." << endl;
	}

	if (v_fatal == true) {
		cout << "[ABORT] Fatal error encountered while setting up. Please check the CSV file for mistakes." << endl;
		cout << "- Shutting down in 10 seconds" << endl;
		Sleep(10000);
		exit (-1); //Crash and burn
	}
	//We should check if all items are out of stock.
	int v_empty_products = 0;
	for (int i = 0; i < p_vec_size; i++) {
		if (v_product[i].v_qty == 0) {
			v_empty_products++;
			cout << "[ALERT] Product in slot " << v_product[i].v_loc_id << " is empty and cannot be sold..." << endl;
		}
	}
	if (v_empty_products >= p_vec_size) {
		cout << "ALL product slots are empty -- no sales can be made. Please call for service." << endl;
		this->setJam();
	}
	v_plist_size = p_vec_size;
	cout << "constructor now complete" << endl;
	writeTransactSess();
	Sleep(5000);
	system("CLS");
	
	
}

bool Vendo::servicePasswordCheck(int p) {
	if (p == v_service_passwd) {
		return true;
	}
	else {
		return false;
	}
}

double Vendo::getLifetimeSales() {
	return v_lifetime_sales;
}

int Vendo::getLifetimeQtySales() {
	return v_lifetime_qty_sales;
}

double Vendo::collectCash() {
	double cashstash = v_cash_on_hand;
	v_cash_on_hand = 0;
	change_dollars = 0;
	change_quarters = 0;
	change_nickels = 0;
	change_dimes = 0;
	change_pennies = 0;
	v_coin.zero();
	return cashstash;
}

void Vendo::clearJam()
{ 
	b_jammed = false;
}

void Vendo::addSalesQty(int sq)
{
	v_lifetime_qty_sales = v_lifetime_qty_sales + sq;
}

void Vendo::addSalesDollars(double sd)
{
	v_lifetime_sales = v_lifetime_sales + sd;
}

void Vendo::addCashOnHand(double c)
{
	v_cash_on_hand = v_cash_on_hand + c;
}

bool Vendo::dispenseItem(int v_prod)
{
	if (v_prod == 0) {
		return false;
	}
	int v_product_found_subscript = 0;
	double v_inserted_cash;
	double v_change;
	bool v_found = false;
	bool v_fuck_you = this->theVendoGambit(); //true = jammed
	for (int i = 0; i < v_plist_size; i++) {
		if (v_product[i].v_loc_id == v_prod) {
			v_found = true;
			v_product_found_subscript = i;
			break;
		}
	}
	if (!v_found) {
		cout << "No item found at this ID. Try again." << endl; //invalid id
		Sleep(5000);
		system("CLS");
		return false;
	}

	if (v_product[v_product_found_subscript].v_qty <= 0) { //none left
		cout << "There aren't any left. Try again." << endl;
		Sleep(5000);
		system("CLS");
		//cout << "Refunding $" << v_inserted_cash << endl;
		return false;
	}

	cout << "Insert cash! Price for " << v_product[v_product_found_subscript].v_name << " is $" <<
		v_product[v_product_found_subscript].v_price << endl;
	cout << "DOLLAR BILLS:" << endl;
	int v_dol;
	int v_quar;
	int v_dime;
	int v_nik;
	int v_pen;
	if (cin >> v_dol) {
		//cout << "OK" << endl;
	}
	else {
		cout << "Not a Number!" << endl;
		cin.clear();
		cin.ignore(9999999999, '\n');
		return true;
		//v_return_code = true;
		//continue;
	}
	cout << "QUARTERS:" << endl;
	if (cin >> v_quar) {
		//cout << "OK" << endl;
	}
	else {
		cout << "Not a Number!" << endl;
		cin.clear();
		cin.ignore(9999999999, '\n');
		return true;
		//v_return_code = true;
		//continue;
	}
	cout << "DIMES:" << endl;
	if (cin >> v_dime) {
		//cout << "OK" << endl;
	}
	else {
		cout << "Not a Number!" << endl;
		cin.clear();
		cin.ignore(9999999999, '\n');
		return true;
		//v_return_code = true;
		//continue;
	}
	cout << "NICKELS:" << endl;
	if (cin >> v_nik) {
		//cout << "OK" << endl;
	}
	else {
		cout << "Not a Number!" << endl;
		cin.clear();
		cin.ignore(9999999999, '\n');
		return true;
		//v_return_code = true;
		//continue;
	}
	cout << "PENNIES:" << endl;
	if (cin >> v_pen) {
		//cout << "OK" << endl;
	}
	else {
		cout << "Not a Number!" << endl;
		cin.clear();
		cin.ignore(9999999999, '\n');
		return true;
		//v_return_code = true;
		//continue;
	}

	v_inserted_cash = v_dol + (v_pen*.01) + (v_dime *0.1) + (v_quar*0.25) + (v_nik*0.05);


	//this->convertChange(v_inserted_cash);
	if (v_product[v_product_found_subscript].v_price > v_inserted_cash) {
		cout << "You did not insert enough money." << endl;
		cout << "Refunding $" << v_inserted_cash << endl;
		Sleep(5000);
		system("CLS");
		return false;
	}

	if (this->getCashOnHand() < 5) {
		cout << "We are in Exact Change mode. Checking your input." << endl;
		if (v_product[v_product_found_subscript].v_price != v_inserted_cash) {
			cout << "Dollar amounts don't match. I can't give change right now. Try again later." << endl;
			cout << "Refunding $" << v_inserted_cash << endl << endl;
			Sleep(5000);
			system("CLS");
			return false;
		}
		else {
			cout << "Yeah we're good." << endl;
		}
	}
	if (v_fuck_you) {
		cout << "Well fuck, the machine jammed. Refunding $" << v_inserted_cash << " and setting the Error State. "<<
			"You will need to go into the service menu to fix this." << endl<<endl;
		this->setJam();
		Sleep(10000);
		system("CLS");
		return false;
	}
	else {
		v_change = v_inserted_cash - v_product[v_product_found_subscript].v_price;
		cout << "CONGRATUWELLDONE: You got one " << v_product[v_product_found_subscript].v_name << " and recieved $" <<
			v_change << " back as change." << endl<<endl;
		v_product[v_product_found_subscript].v_qty--;
		double inserted= changeInserter(v_dol, v_quar, v_dime, v_nik, v_pen );
		bool change_rc = changeBreaker(v_change);
		if (change_rc) {
			this->addSalesQty(1);
			this->addCashOnHand(v_product[v_product_found_subscript].v_price);
			this->addSalesDollars(v_product[v_product_found_subscript].v_price);
			this->writePersist();
			this->writeTransactLog(v_product[v_product_found_subscript].v_loc_id, v_product[v_product_found_subscript].v_name,
				v_product[v_product_found_subscript].v_price);
		}
		else {
			cout << "Was not able to break change. Refunding $" << setprecision(2) << v_inserted_cash << ". Sorry dude." << endl;
			Sleep(7000);
			system("CLS");
			return false;
		}
		Sleep(7000);
		system("CLS");
		return true;
	}

}

bool Vendo::dispenseItem(double cash)
{
	//convertChange(cash);
	int v_prod = 0;
	double v_change;
	int v_product_found_subscript = 0;
	int v_number_of_affordable = 0;
	if (cash == 0) {
		return false;
	}
	for (int i = 0; i < v_plist_size; i++) {
		if (v_product[i].v_price <= cash && v_product[i].v_qty>0) {
			v_number_of_affordable++;
		}
	}
	if (v_number_of_affordable == 0) {
		cout <<"You did not insert enough cash to afford anything. Refunding $" << cash << endl<<endl;
		Sleep(5000);
		system("CLS");
		return false;
	}
	this->displayProductList(cash);
	bool v_fuck_you = this->theVendoGambit(); //true = jammed
	cout << "Please select a product ID. Only those you can afford & are not empty have been displayed." << endl<<endl;
	if (cin >> v_prod) {
		//cout << "OK" << endl;
	}
	else {
		cout << "Not a Number!" << endl;
		cin.clear();
		cin.ignore(9999999999, '\n');
		return true;
		//v_return_code = true;
		//continue;
	}
	bool v_found = false;
	for (int i = 0; i < v_plist_size; i++) {
		if (v_product[i].v_loc_id == v_prod) {
			v_found = true;
			v_product_found_subscript = i;
			break;
		}
	}

	if (!v_found) {
		cout << "No item found at this ID. Try again." << endl<<endl;
		Sleep(5000);
		system("CLS");
		return false;
	}
	if (v_product[v_product_found_subscript].v_qty <=0) {
		cout << "There aren't any left..." << endl;
		cout << "Refunding $" << cash << endl;
		//convertChange(cash);
		Sleep(5000);
		system("CLS");
		return false;
	}
	if (v_product[v_product_found_subscript].v_price > cash) {
		cout << "You did not insert enough money." << endl;
		cout << "Refunding $" << cash << endl;
		//convertChange(cash);
		Sleep(5000);
		system("CLS");
		return false;
	}

	if (this->getCashOnHand() < 2.50) {
		cout << "We are in Exact Change mode. Checking your input." << endl;
		if (v_product[v_product_found_subscript].v_price != cash) {
			cout << "Dollar amounts don't match. I can't give change right now. Try again later." << endl;
			cout << "Refunding $" << cash << endl << endl;
			//convertChange(cash);
			Sleep(5000);
			system("CLS");
			return false;
		}
		else {
			cout << "Yeah we're good." << endl << endl;
		}
	}
	if (v_fuck_you) {
		cout << "Well fuck, the machine jammed. Refunding $" << cash << " and setting the Error State. " <<
			"You will need to go into the service menu to fix this." << endl << endl;
		this->setJam();
		Sleep(5000);
		system("CLS");
		return false;
	}

	v_change = cash - v_product[v_product_found_subscript].v_price;
	cout << endl<< "CONGRATUWELLDONE: You got one " << v_product[v_product_found_subscript].v_name << " and recieved $" <<
		v_change << " back as change." << endl<<endl;
	//convertChange(cash);
	changeInserter(cash);
	bool change_rc = changeBreaker(v_change);
	if (change_rc) {
		v_product[v_product_found_subscript].v_qty--;
		this->addSalesQty(1);
		this->addCashOnHand(v_product[v_product_found_subscript].v_price);
		this->addSalesDollars(v_product[v_product_found_subscript].v_price);
		this->writePersist();
		this->writeTransactLog(v_product[v_product_found_subscript].v_loc_id, v_product[v_product_found_subscript].v_name, 
			v_product[v_product_found_subscript].v_price);
	}
	else {
		cout << "Was not able to break change. Refunding $" << setprecision(2) << cash << ". Sorry dude." << endl;
		Sleep(7000);
		system("CLS");
		return false;
	}
	Sleep(7000);
	system("CLS");
	return true;

}

void Vendo::setJam() {
	b_jammed = true;
}
bool Vendo::checkJam() {
	return b_jammed;
}
bool Vendo::theVendoGambit() { //true = jammed, false = not jammed
	//Let's say there's a 1 in 5 chance that your purchase gets stuck.
	int v_rand = rand();
	int v_rand_b = v_rand % 4; // 0-4 range
	bool v_result = false;
	if (v_rand_b == 3) {
		v_result = true;
	}
	return v_result;
}
double Vendo::getCashOnHand() {
	return v_cash_on_hand;
}

int Vendo::serviceMenu(int v_password_check) {
	for(;;){
		int v_choice=0;
		bool v_svc_result;
		if (this->servicePasswordCheck(v_password_check)) {
			cout << endl;
			cout << "[SERVICE MODE] Please make a selection:" << endl;
			cout << endl;
			cout << " (1) Restock a product" << endl;
			cout << " (2) Collect cash" << endl;
			cout << " (3) Clear errored state" << endl;
			cout << " (4) Exit Service Menu" << endl;
			cout << endl;
			if (cin >> v_choice) {
				//cout << "OK" << endl;
			}
			else {
				cout << "Not a Number!" << endl;
				cin.clear();
				cin.ignore(9999999999, '\n');
				return -1;
				//v_return_code = true;
				//continue;
			}
			v_svc_result = this->serviceMenuChoice(v_choice);
			if (!v_svc_result) {
				return 1;
			}
		}
		else {
			cout << "Sorry, service password was incorrect." << endl;
			Sleep(5000);
			system("CLS");
			return -1;
		}
		if (!v_svc_result) {
			system("CLS");
			break;
		}
	}
	Sleep(5000);
	//system("CLS");
	return true;

}


bool Vendo::serviceMenuChoice(int v_choice)
{
	int v_restock_id;
	int v_restock_qty;
	bool v_found_product=false;
	int v_found_product_subscript=-1;
	int v_total_empty_slots = 0;
	switch (v_choice) {
	case 4:
		cout << "Exiting service menu"<<endl;
		return false;
		break;
	case 69:
		cout << "LMAO nice" << endl;
		break;
	case 1:
		cout << "Enter a product ID to restock:" << endl;
		this->displayProductList();
		cout << endl;
		if (cin >> v_restock_id) {
			//cout << "OK" << endl;
		}
		else {
			cout << "Not a Number!" << endl;
			cin.clear();
			cin.ignore(9999999999, '\n');
			return true;
			//v_return_code = true;
			//continue;
		}
		//cin >> v_restock_id;
		cout << "How many are you adding?" << endl;
		if (cin >> v_restock_qty) {
			//cout << "OK" << endl;
		}
		else {
			cout << "Not a Number!" << endl;
			cin.clear();
			cin.ignore(9999999999, '\n');
			return true;
			//v_return_code = true;
			//continue;
		}
		//cin >> v_restock_qty;
		if (v_restock_qty < 0) {
			cout << "Thats a negative number. You know about math right?" << endl;
			break; //invalid qty to add
		}
		//we need to search for that id.
		for (int i = 0; i<v_plist_size; i++){
			if (v_restock_id == v_product[i].v_loc_id) {
				v_found_product = true;
				v_found_product_subscript = i;
				break;
			}
		}
		//did we find it
		if (v_found_product) {
			v_product[v_found_product_subscript].v_qty = v_product[v_found_product_subscript].v_qty + v_restock_qty;
			cout << "Product restocked" << endl;
		}
		else {
			cout << "Product not found." << endl;
		}
		break; //end restock
	case 2:
		cout << "Cash Rules Everything Around Me" << endl;
		cout << "$ " << this->getCashOnHand() << " is being removed from the machine." << endl;
		this->collectCash();
		cout << "New cash on hand: $" << this->getCashOnHand() << endl;
		break; //end wu tang clan
	case 3:
		cout << "Clearing jammed flag" << endl; //clearing the flag doesnt mean we restocked the machine
		this->clearJam();
		cout << "** Checking if the system is still in errored state..." << endl;
		if (this->checkEmpty()) {
			cout << "**** System still in error: All products empty" << endl;
			this->setJam();
		}
		else {
			cout << "**** we good homie." << endl;
		}
		break;
	default:
		cout << "Invalid choice... try again" << endl;
		break;
	}
	Sleep(5000);
	system("CLS");
	return true;
}

bool Vendo::checkEmpty() {
	int v_total_empty_slots = 0;
	for (int i = 0; i < v_plist_size; i++) {
		if (v_product[i].v_qty == 0) {
			v_total_empty_slots++;
		}
	}
	if (v_total_empty_slots >= v_plist_size) {
		return true;
	}
	else {
		return false;
	}
}

void Vendo::vendMenu() {
	int v_menu_input = 0;
	bool v_return_code = true;

	HANDLE  hConsole;
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, 10);

	while (v_return_code) {
		//Sleep(5000);
		system("CLS");
		cout << endl << "Welcome to the VendoTron 5000" << endl;
		cout << "The Ultimate in Vending Technology (TM)" << endl << endl;
		cout << "[SYSTEM HEALTH CHECK]" << endl; //We should check if the system is in an error state (empty or jammed)
		string txt_jammed = "FALSE";
		if (this->checkEmpty()) {
			this->setJam();
		}
		if (checkJam()){ 
			txt_jammed = "TRUE"; }

		cout << " * FLAGS *" << endl;



		cout << "     JAMMED: [";
		if (checkJam()) {
			SetConsoleTextAttribute(hConsole, 12);
		}
		cout << txt_jammed;
		SetConsoleTextAttribute(hConsole, 10);
		cout << "]" << endl;
		bool v_exact = false;
		string txt_exact = "FALSE";
		if (this->getCashOnHand() <= 5) {
			v_exact = true;
			txt_exact = "TRUE";
		}
		cout << "      EXACT: [";
		
		if (v_exact == true) {
			SetConsoleTextAttribute(hConsole, 12);
		}

		cout << txt_exact;
		SetConsoleTextAttribute(hConsole, 10);
		cout<< "]" << endl << endl;
		cout << "[END SYSTEM HEALTH CHECK]" << endl << endl;
		cout << endl << endl;
		printGrid();
		cout << endl;
		cout << "Please make a selection:" << endl;
		cout << " (1) Enter a product number" << endl;
		cout << " (2) Insert Cashola" << endl;
		cout << " (3) Display System Stats" << endl;
		cout << " (4) Service Menu" << endl;
		cout << " (0) Self-Destruct Sequence" << endl << endl;
		if (cin >> v_menu_input) {
			//cout << "OK" << endl;
		}
		else {
			cout << "Not a Number!" << endl;
			cin.clear();
			cin.ignore(9999999999,'\n');
			v_return_code = true;
			Sleep(5000);
			system("CLS");
			continue;
		}
		v_return_code = this->vendMenuChoice(v_menu_input);
	}
	writeTransactSessClose();
	//cout << "Debug: We got to the end of vendMenu() - choice was "<<v_menu_input << endl;

}



void Vendo::displayStats() { //just system statistics
	cout << "[SYSTEM STATS]" << endl;
	cout << "  [Lifetime Sales (Dollars)] " << this->getLifetimeSales() << endl;
	cout << "  [Lifetime Sales (Quantity)] " << this->getLifetimeQtySales() << endl;
	cout << "  [Cash on Hand] " << this->getCashOnHand() << endl;
	cout << "  [Change Breakdown]" << endl;
	cout << "     Dollars: " << v_coin.getDollars() << endl;
	cout << "    Quarters: " << v_coin.getQuarters() << endl;
	cout << "       Dimes: " << v_coin.getDimes() << endl;
	cout << "     Nickels: " << v_coin.getNickels() << endl;
	cout << "     Pennies: " << v_coin.getPennies() << endl;
	cout << "[END SYSTEM STATS]" << endl << endl;
	this->writePersist();
	Sleep(5000);
	system("CLS");
}

void Vendo::displayProductList() { //everything
	cout << "ID:" << setw(40) << "Name:" << setw(8) << "Price:" << setw(8) << "Qty:" << endl;
		//<< setw(40) << "Price:" << setw(8) << "Qty:" << endl;
	for (int i = 0; i < v_plist_size; i++) {
		cout << v_product[i].v_loc_id << setw(40)
			<< v_product[i].v_name << setw(8) << setprecision(2) << fixed << v_product[i].v_price << setw(8) << v_product[i].v_qty << endl;
	}
	cout << endl;
}

void Vendo::displayProductList(double v_cashola) { //only what you can afford
	cout <<"ID:" << setw(40) << "Name:" << setw(8) << "Price:" << setw(8) << "Qty:" << endl;
	for (int i = 0; i < v_plist_size; i++) {
		if (v_product[i].v_price <= v_cashola) { //iterate through the vector, compare price vs cash inserted
			cout << v_product[i].v_loc_id  << setw(40)
				<< v_product[i].v_name  << setw(8) << setprecision(2)<<fixed<< v_product[i].v_price  << setw(8) << v_product[i].v_qty << endl;
		}
	}
	cout << endl;
}


bool Vendo::vendMenuChoice(int v_choice) { //false to return & break out of the loop
	int v_pass = 0;
	int v_selected_product = 0;
	double v_inserted_cash = 0;
	switch (v_choice) {
	case 0: //Self Destruct
		return false;
		break;
	case 69:
		cout << "LMAO nice" << endl;
		break;
	case 1: //Select product by ID
		this->displayProductList();
		cout << "Please enter a product ID: (enter 0 to return to previous)" << endl;
		if (cin >> v_selected_product) {
			//cout << "OK" << endl;
		}
		else {
			cout << "Not a Number!" << endl;
			cin.clear();
			cin.ignore(9999999999, '\n');
			return true;
			//v_return_code = true;
			//continue;
		}
		
		this->dispenseItem(v_selected_product);
		break;
	case 2: //Insert Cash before selecting



		cout << "Please insert $" << endl;
		cout << "DOLLAR BILLS:" << endl;
		int v_dol;
		int v_quar;
		int v_dime;
		int v_nik;
		int v_pen;
		if (cin >> v_dol) {
			//cout << "OK" << endl;
		}
		else {
			cout << "Not a Number!" << endl;
			cin.clear();
			cin.ignore(9999999999, '\n');
			return true;
			//v_return_code = true;
			//continue;
		}
		cout << "QUARTERS:" << endl;
		if (cin >> v_quar) {
			//cout << "OK" << endl;
		}
		else {
			cout << "Not a Number!" << endl;
			cin.clear();
			cin.ignore(9999999999, '\n');
			return true;
			//v_return_code = true;
			//continue;
		}
		cout << "DIMES:" << endl;
		if (cin >> v_dime) {
			//cout << "OK" << endl;
		}
		else {
			cout << "Not a Number!" << endl;
			cin.clear();
			cin.ignore(9999999999, '\n');
			return true;
			//v_return_code = true;
			//continue;
		}
		cout << "NICKELS:" << endl;
		if (cin >> v_nik) {
			//cout << "OK" << endl;
		}
		else {
			cout << "Not a Number!" << endl;
			cin.clear();
			cin.ignore(9999999999, '\n');
			return true;
			//v_return_code = true;
			//continue;
		}
		cout << "PENNIES:" << endl;
		if (cin >> v_pen) {
			//cout << "OK" << endl;
		}
		else {
			cout << "Not a Number!" << endl;
			cin.clear();
			cin.ignore(9999999999, '\n');
			return true;
			//v_return_code = true;
			//continue;
		}

		v_inserted_cash = v_dol + (v_pen*.01) + (v_dime *0.1) + (v_quar*0.25) + (v_nik*0.05);
		//v_inserted_cash = ;
		this->dispenseItem(v_inserted_cash);
		break;
	case 3: //Stats for nerds as youtube would call it
		this->displayStats();
		break;
	case 4: //Call service menu
		cout << "Enter Service Password:" << endl;
		if (cin >> v_pass) {
			//cout << "OK" << endl;
		}
		else {
			cout << "Not a Number!" << endl;
			cin.clear();
			cin.ignore(9999999999, '\n');
			return true;
			//v_return_code = true;
			//continue;
		}
		//cin >> v_pass;
		this->serviceMenu(v_pass);
		break;
	default: //invalid choice
		cout << "Not a valid menu choice! Try again." << endl;
		break;
	};
	return true;
}

void Vendo::printGrid() {
	
	HANDLE  hConsole;
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	//every line is a loop because thats how you do it.

	int v_max_x=0;
	int v_max_y=0;
	int v_vec_size = v_product.size();
	for (int i = 0; i < v_vec_size; i++) {
		if (v_product[i].v_loc_horiz > v_max_x) {
			v_max_x = v_product[i].v_loc_horiz;
		}
		if (v_product[i].v_loc_vert > v_max_y) {
			v_max_y = v_product[i].v_loc_vert;
		}
	}
	int v_subscript = -1;
	//cout << "DEBUG: Max X: " << v_max_x << " Max Y: " << v_max_y << endl;
	for (int a = 0; a<v_max_x;a++){
		wcout << wchar_t(178) << wchar_t(178) << wchar_t(178) << wchar_t(178) << wchar_t(178) 
			<< wchar_t(178) << wchar_t(178) << wchar_t(178) << wchar_t(178) << wchar_t(178) << wchar_t(178);
	}
	wcout << wchar_t(178) << endl;

	bool v_empty_item = false;

	for (int b = 1; b <= v_max_y; b++) {
		for (int a = 1; a <= v_max_x; a++) { //blankline
			wcout << wchar_t(178) << "          ";
		}
		wcout << wchar_t(178) << endl;

		for (int a = 1; a <= v_max_x; a++) {
			for (int i = 0; i < v_product.size(); i++) {
				if (v_product[i].v_loc_horiz == a && v_product[i].v_loc_vert == b) {
					v_subscript = i;
					if (v_product[i].v_qty <= 0) {
						//system("Color F3"); lol nope
						v_empty_item = true;
					}
				}
			}
			 
			wcout << wchar_t(178);
			if (v_empty_item) {
				SetConsoleTextAttribute(hConsole, 12);
				wcout << "  ID: " << v_product[v_subscript].v_loc_id<<" ";
				SetConsoleTextAttribute(hConsole, 10);
			}
			else {
				wcout << "  ID: " << v_product[v_subscript].v_loc_id<<" ";
			}
			v_empty_item = false;

		}
		wcout << wchar_t(178) << endl;
		for (int a = 1; a <= v_max_x; a++) {
			for (int i = 0; i < v_product.size(); i++) {
				if (v_product[i].v_loc_horiz == a && v_product[i].v_loc_vert == b) {
					v_subscript = i;
					if (v_product[i].v_qty == 0) {
						v_empty_item = true;
					}
				}
			}
			wcout << wchar_t(178);
			if (v_empty_item) {
				SetConsoleTextAttribute(hConsole, 12);
				wcout << " QTY: " << setfill(L'0') << setw(3) << v_product[v_subscript].v_qty<<" ";
				SetConsoleTextAttribute(hConsole, 10);
			}
			else {
				wcout << " QTY: " << setfill(L'0') << setw(3) << v_product[v_subscript].v_qty<<" ";
			}
			v_empty_item = false;
		}
		wcout << wchar_t(178) << endl;
		v_empty_item = false;
		for (int a = 1; a <= v_max_x; a++) {
			for (int i = 0; i < v_product.size(); i++) {
				if (v_product[i].v_loc_horiz == a && v_product[i].v_loc_vert == b) {
					v_subscript = i;
					if (v_product[i].v_qty == 0) {
						v_empty_item = true;
					}
				}
			}
			wcout << wchar_t(178);
			if (v_empty_item) {
				SetConsoleTextAttribute(hConsole, 12);
				wcout << "   $ " << setprecision(2) << fixed << v_product[v_subscript].v_price<< " ";
				SetConsoleTextAttribute(hConsole, 10);
			}
			else {
				wcout << "   $ " << setprecision(2) << fixed << v_product[v_subscript].v_price<<" ";
			}
			v_empty_item = false;
		}
		wcout << wchar_t(178) << endl;
		for (int a = 1; a <= v_max_x; a++) { //blankline
			wcout << wchar_t(178) << "          ";
		}
		wcout << wchar_t(178) <<endl;
		for (int a = 1; a <= v_max_x; a++) {
			wcout << wchar_t(178) << wchar_t(178) << wchar_t(178) << wchar_t(178) << wchar_t(178)
				<< wchar_t(178) << wchar_t(178) << wchar_t(178) << wchar_t(178) << wchar_t(178) << wchar_t(178);
		}
		v_empty_item = false;
		wcout << setfill(L' ');
		wcout << wchar_t(178) << endl;
	}
	
}


void Vendo::writePersist() {
	//This method will write the files out to save the state of the machine.
	cout << "saving VEND_PRODUCTS.CSV..." << endl;
	fstream f_pricing("VEND_PRODUCTS.CSV", fstream::out);
	f_pricing << std::noskipws;
	int v_vec_maxsize = v_product.size();
	string v_outputline;
	for (int i = 0; i < v_vec_maxsize; i++) {
		v_outputline = to_string(v_product[i].v_loc_id) + "," + v_product[i].v_name + "," + 
			to_string(v_product[i].v_price) + "," + to_string(v_product[i].v_qty);
		f_pricing << v_outputline<<endl;
	} //remember - there will always be an extraneous line at the end of this file & you have to check for it on file read
	f_pricing.close();
	cout << "VEND_PRODUCTS.CSV was written to disk..." << endl;
	cout << "saving VEND_PERSIST.csv..." << endl;


	fstream f_statistics("VEND_PERSIST.CSV", fstream::out);
	f_statistics << std::noskipws;
	string v_stats = to_string(getLifetimeQtySales()) + "," + to_string(getLifetimeSales()) + "," + to_string(getCashOnHand());
	f_statistics << v_stats;
	f_statistics.close();
	cout << "VEND_PERSIST.CSV was written to disk..." << endl;


	fstream f_change("VEND_CHANGE.CSV", fstream::out);
	f_change << std::noskipws;
	cout << "saving VEND_CHANGE.CSV..." << endl;
	string v_change_outputline = to_string(v_coin.getDollars()) + "," + to_string(v_coin.getQuarters()) + "," +
		to_string(v_coin.getDimes()) + "," + to_string(v_coin.getNickels()) + ","+ to_string(v_coin.getPennies());
	f_change << v_change_outputline;
	f_change.close();
	cout << "VEND_CHANGE.CSV was written to disk..." << endl;
}

bool Vendo::changeBreaker(double c)
{
	bool v_fatal = false;
	int new_cash = c * 100;
	int v_change = new_cash % 100;
	int v_dollars = (new_cash - v_change) / 100;
	//cout << "v_dollars= " << v_dollars << endl;;
	//cout << "v_change = " << v_change << endl;
	new_cash = new_cash - v_dollars * 100;
	int v_quarters = v_change / 25;
	//cout << "v_quarters = " << v_quarters << endl;
	v_change = new_cash % 25;
	//cout << "v_change = " << v_change << endl;
	new_cash = new_cash - (v_quarters * 25);
	//cout << "new_cash = " << new_cash << endl;
	int v_dimes = v_change / 10;
	v_change = new_cash % 10;
	//cout << "v_dimes = " << v_dimes << endl;
	//cout << "v_change = " << v_change << endl;
	new_cash = new_cash - (v_dimes * 10);
	//cout << "new_cash = " << new_cash << endl;
	int v_nickels = v_change / 5;
	//cout << "v_nickels = " << v_nickels << endl;
	v_change = new_cash % 5;
	//cout << "v_change = " << v_change << endl;
	new_cash = new_cash - (v_nickels * 5);
	//cout << "new_cash = " << new_cash << endl;
	int v_pennies = v_change;
	//cout << "v_pennies = " << v_pennies << endl;
	new_cash = new_cash - v_pennies;
	//SANITY CHECK: NEW_CASH SHOULD NOW BE ZERO. 
	//Or at least less than 0.01 if you're a doofus who doesnt know how money works. 
	//but new_cash is an int so we dont need to worry about that.
	//if (new_cash > 0) {
	//	cout << "Not everything was dispersed!" << endl;
	//}
	//else {
	//	cout << "Everything was dispersed." << endl;
	//}
	if (v_pennies > change_pennies) {
		cout << "ERROR: Cannot break change [PENNIES], not enough coin in machine. Refunding." << endl;
		v_fatal = true;
	}

	if (v_nickels > change_nickels) {
		cout << "ERROR: Cannot break change [NICKELS], not enough coin in machine. Refunding." << endl;
		v_fatal = true;
	}

	if (v_dimes > change_dimes) {
		cout << "ERROR: Cannot break change [DIMES], not enough coin in machine. Refunding." << endl;
		v_fatal = true;
	}

	if (v_quarters > change_quarters) {
		cout << "ERROR: Cannot break change [QUARTERS], not enough coin in machine. Refunding." << endl;
		v_fatal = true;
	}

	if (v_dollars > change_dollars) {
		cout << "ERROR: Cannot break change [DOLLARS], not enough coin in machine. Refunding." << endl;
		v_fatal = true;
	}

	if (v_fatal == true) {
		return false;
	}

	cout << "Change Dispensed:" << endl;
	cout << "Dollars: " << v_dollars << ", Quarters:" << v_quarters << ", Dimes: " << v_dimes << ", Nickels: " << v_nickels
		<< ", Pennies: " << v_pennies << endl;
	change_pennies = change_pennies - v_pennies;
	v_coin.setPennies(change_pennies);
	change_dimes = change_dimes - v_dimes;
	v_coin.setDimes(change_dimes);
	change_nickels = change_nickels - v_nickels;
	v_coin.setNickels(change_nickels);
	change_quarters = change_quarters - v_quarters;
	v_coin.setQuarters(change_quarters);
	change_dollars = change_dollars - v_dollars;
	v_coin.setDollars(change_dollars);
	return true;

}


void Vendo::changeInserter(double c)
{
	int new_cash = c * 100;
	int v_change = new_cash % 100;
	int v_dollars = (new_cash - v_change) / 100;
	//cout << "v_dollars= " << v_dollars << endl;;
	//cout << "v_change = " << v_change << endl;
	new_cash = new_cash - v_dollars * 100;
	int v_quarters = v_change / 25;
	//cout << "v_quarters = " << v_quarters << endl;
	v_change = new_cash % 25;
	//cout << "v_change = " << v_change << endl;
	new_cash = new_cash - (v_quarters * 25);
	//cout << "new_cash = " << new_cash << endl;
	int v_dimes = v_change / 10;
	v_change = new_cash % 10;
	//cout << "v_dimes = " << v_dimes << endl;
	//cout << "v_change = " << v_change << endl;
	new_cash = new_cash - (v_dimes * 10);
	//cout << "new_cash = " << new_cash << endl;
	int v_nickels = v_change / 5;
	//cout << "v_nickels = " << v_nickels << endl;
	v_change = new_cash % 5;
	//cout << "v_change = " << v_change << endl;
	new_cash = new_cash - (v_nickels * 5);
	//cout << "new_cash = " << new_cash << endl;
	int v_pennies = v_change;
	//cout << "v_pennies = " << v_pennies << endl;
	new_cash = new_cash - v_pennies;
	//SANITY CHECK: NEW_CASH SHOULD NOW BE ZERO. 
	//Or at least less than 0.01 if you're a doofus who doesnt know how money works. 
	//but new_cash is an int so we dont need to worry about that.
	//if (new_cash > 0) {
	//	cout << "Not everything was dispersed!" << endl;
	//}
	//else {
	//	cout << "Everything was dispersed." << endl;
	//}

	cout << "Change Inserted:" << endl;
	cout << "Dollars: " << v_dollars << ", Quarters:" << v_quarters << ", Dimes: " << v_dimes << ", Nickels: " << v_nickels
		<< ", Pennies: " << v_pennies << endl;

	change_pennies = change_pennies + v_pennies;
	v_coin.setPennies(change_pennies);
	change_dimes = change_dimes + v_dimes;
	v_coin.setDimes(change_dimes);
	change_nickels = change_nickels + v_nickels;
	v_coin.setNickels(change_nickels);
	change_quarters = change_quarters + v_quarters;
	v_coin.setQuarters(change_quarters);
	change_dollars = change_dollars + v_dollars;
	v_coin.setDollars(change_dollars);


}

double Vendo::changeInserter(int v_dollars, int v_quarters, int v_dimes, int v_nickels, int v_pennies)
{


	cout << "Change Inserted:" << endl;
	cout << "Dollars: " << v_dollars << ", Quarters:" << v_quarters << ", Dimes: " << v_dimes << ", Nickels: " << v_nickels
		<< ", Pennies: " << v_pennies << endl;

	change_pennies = change_pennies + v_pennies;
	v_coin.setPennies(change_pennies);
	change_dimes = change_dimes + v_dimes;
	v_coin.setDimes(change_dimes);
	change_nickels = change_nickels + v_nickels;
	v_coin.setNickels(change_nickels);
	change_quarters = change_quarters + v_quarters;
	v_coin.setQuarters(change_quarters);
	change_dollars = change_dollars + v_dollars;
	v_coin.setDollars(change_dollars);

	return (v_pennies*0.01) + (v_nickels * 0.05) + (v_dimes * 0.1) + (v_quarters * 0.25) + v_dollars; //total inserted $$


}

void Vendo::displayChange() {
	cout << "Change Breakdown:" << endl;
	cout << "Dollars: " << change_dollars << ", Quarters:" << change_quarters << ", Dimes: " << change_dimes << ", Nickels: " << change_nickels
		<< ", Pennies: " << change_pennies << endl;
}

void Vendo::convertChange(double c) { //this just says what the efficient change distribution is, it wont actually do anything to the variables.
	//the only reason I wrote it was to test the change distribution without messing with my files
	int new_cash = c * 100;
	int v_change = new_cash % 100;
	int v_dollars = (new_cash - v_change) / 100;
	//cout << "v_dollars= " << v_dollars << endl;;
	//cout << "v_change = " << v_change << endl;
	new_cash = new_cash - v_dollars * 100;
	int v_quarters = v_change / 25;
	//cout << "v_quarters = " << v_quarters << endl;
	v_change = new_cash % 25;
	//cout << "v_change = " << v_change << endl;
	new_cash = new_cash - (v_quarters * 25);
	//cout << "new_cash = " << new_cash << endl;
	int v_dimes = v_change / 10;
	v_change = new_cash % 10;
	//cout << "v_dimes = " << v_dimes << endl;
	//cout << "v_change = " << v_change << endl;
	new_cash = new_cash - (v_dimes * 10);
	//cout << "new_cash = " << new_cash << endl;
	int v_nickels = v_change / 5;
	//cout << "v_nickels = " << v_nickels << endl;
	v_change = new_cash % 5;
	//cout << "v_change = " << v_change << endl;
	new_cash = new_cash - (v_nickels * 5);
	//cout << "new_cash = " << new_cash << endl;
	int v_pennies = v_change;
	//cout << "v_pennies = " << v_pennies << endl;
	new_cash = new_cash - v_pennies;
	//SANITY CHECK: NEW_CASH SHOULD NOW BE ZERO. 
	//Or at least less than 0.01 if you're a doofus who doesnt know how money works. 
	//but new_cash is an int so we dont need to worry about that.
	//if (new_cash > 0) {
	//	cout << "Not everything was dispersed!" << endl;
	//}
	//else {
	//	cout << "Everything was dispersed." << endl;
	//}

	cout << "Change Breakdown:" << endl;
	cout << "Dollars: " << v_dollars << ", Quarters:" << v_quarters << ", Dimes: " << v_dimes << ", Nickels: " << v_nickels
		<< ", Pennies: " << v_pennies << endl;

}

bool Vendo::dispenseItem(int dol, int quar, int dime, int nik, int pen)
{
	//convertChange(cash);
	int v_prod = 0;
	double v_change;
	int v_product_found_subscript = 0;
	int v_number_of_affordable = 0;
	double cash = dol + (quar*0.25) + (dime*0.1) + (nik*0.05) + (pen*0.01);
	if (cash == 0) {
		return false;
	}
	for (int i = 0; i < v_plist_size; i++) {
		if (v_product[i].v_price <= cash && v_product[i].v_qty>0) {
			v_number_of_affordable++;
		}
	}
	if (v_number_of_affordable == 0) {
		cout << "You did not insert enough cash to afford anything. Refunding $" << cash << endl << endl;
		Sleep(5000);
		system("CLS");
		return false;
	}
	this->displayProductList(cash);
	bool v_fuck_you = this->theVendoGambit(); //true = jammed
	cout << "Please select a product ID. Only those you can afford & are not empty have been displayed." << endl << endl;
	if (cin >> v_prod) {
		//cout << "OK" << endl;
	}
	else {
		cout << "Not a Number!" << endl;
		cin.clear();
		cin.ignore(9999999999, '\n');
		return true;
		//v_return_code = true;
		//continue;
	}
	bool v_found = false;
	for (int i = 0; i < v_plist_size; i++) {
		if (v_product[i].v_loc_id == v_prod) {
			v_found = true;
			v_product_found_subscript = i;
			break;
		}
	}

	if (!v_found) {
		cout << "No item found at this ID. Try again." << endl << endl;
		Sleep(5000);
		system("CLS");
		return false;
	}
	if (v_product[v_product_found_subscript].v_qty <= 0) {
		cout << "There aren't any left..." << endl;
		cout << "Refunding $" << cash << endl;
		//convertChange(cash);
		Sleep(5000);
		system("CLS");
		return false;
	}
	if (v_product[v_product_found_subscript].v_price > cash) {
		cout << "You did not insert enough money." << endl;
		cout << "Refunding $" << cash << endl;
		//convertChange(cash);
		Sleep(5000);
		system("CLS");
		return false;
	}

	if (this->getCashOnHand() < 2.50) {
		cout << "We are in Exact Change mode. Checking your input." << endl;
		if (v_product[v_product_found_subscript].v_price != cash) {
			cout << "Dollar amounts don't match. I can't give change right now. Try again later." << endl;
			cout << "Refunding $" << cash << endl << endl;
			//convertChange(cash);
			Sleep(5000);
			system("CLS");
			return false;
		}
		else {
			cout << "Yeah we're good." << endl << endl;
		}
	}
	if (v_fuck_you) {
		cout << "Well fuck, the machine jammed. Refunding $" << cash << " and setting the Error State. " <<
			"You will need to go into the service menu to fix this." << endl << endl;
		this->setJam();
		Sleep(5000);
		system("CLS");
		return false;
	}

	v_change = cash - v_product[v_product_found_subscript].v_price;
	cout << endl << "CONGRATUWELLDONE: You got one " << v_product[v_product_found_subscript].v_name << " and recieved $" <<
		v_change << " back as change." << endl << endl;
	//convertChange(cash);
	double v_insert = changeInserter(dol, quar, dime, nik, pen);
	bool change_rc = changeBreaker(v_change);
	if (change_rc) {
		v_product[v_product_found_subscript].v_qty--;
		this->addSalesQty(1);
		this->addCashOnHand(v_product[v_product_found_subscript].v_price);
		this->addSalesDollars(v_product[v_product_found_subscript].v_price);
		this->writePersist();
		this->writeTransactLog(v_product[v_product_found_subscript].v_loc_id, v_product[v_product_found_subscript].v_name,
			v_product[v_product_found_subscript].v_price);
	}
	else {
		cout << "Was not able to break change. Refunding $" << setprecision(2) << cash << ". Sorry dude." << endl;
		Sleep(7000);
		system("CLS");
		return false;
	}
	Sleep(7000);
	system("CLS");
	return true;

}

bool Vendo::writeTransactLog(int v_id, string v_name, double v_price)
{
	fstream f_history("VEND_HISTORY.xml", fstream::app | fstream::out );
	if (f_history.good())
	{
		//TIME IS A BITCH AINT IT
		string v_transact_ID = genSessionID(16);
		time_t v_now = time(NULL);
		struct tm timeinfo;
		localtime_s(&timeinfo, &v_now);
		char v_timechar[25];
		strftime(v_timechar, sizeof(v_timechar), "%m-%d-%Y %I:%M:%S %p", &timeinfo);
		string v_timestr = string(v_timechar);
		//string v_outputline = v_timestr + " --> " + to_string(v_id) + " / " + v_name + " / " + to_string(v_price);
		string v_outputline = "\t<TRANSACTION id=\""+v_transact_ID+  "\">\n\t\t<TIMESTAMP>" + v_timestr + "</TIMESTAMP>\n\t\t" +
			"<PRODUCT_ID>" + to_string(v_id) + "</PRODUCT_ID>\n\t\t<PRODUCT_NAME>" + v_name + "</PRODUCT_NAME>\n\t\t" +
			"<PRODUCT_PRICE>" + to_string(v_price) + "</PRODUCT_PRICE>\n\t</TRANSACTION>";

		f_history << v_outputline << endl;
		f_history.close();
		return true;
	}
	else {
		f_history.close();
		cout << "There was an error writing the Transaction Log file." << endl;
		return false;
	}
	
}
bool Vendo::writeTransactSess()
{
	fstream f_history("VEND_HISTORY.xml", fstream::app | fstream::out);
	f_history << noskipws;
	string v_final_line;
	string v_second_to_last;
	if (f_history.good())
	{
		string v_sess = genSessionID(32);
		string v_outputline = "<SESSION id=\"" + v_sess + "\">";
		f_history << v_outputline << endl;
		f_history.close();
		return true;
	}
	else {
		f_history.close();
		cout << "There was an error writing the Transaction Log file." << endl;
		return false;
	}

}

bool Vendo::writeTransactSessClose()
{
	fstream f_history("VEND_HISTORY.xml", fstream::app | fstream::out);
	f_history << noskipws;
	if (f_history.good())
	{
		//string v_sess = genSessionID(32);
		string v_outputline = "</SESSION>";
		f_history << v_outputline << endl;
		f_history.close();
		return true;
	}
	else {
		f_history.close();
		cout << "There was an error writing the Transaction Log file." << endl;
		return false;
	}

}

string Vendo::genSessionID(int len)
{
	srand(time(NULL));
	static const char ascii_array[62] = { '1','2','3','4','5','6','7','8','9','0',
		'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o',
		'p','q','r','s','t','u','v','w','x','y','z',
		'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O',
		'P','Q','R','S','T','U','V','W','X','Y','Z' };
	int ascii_val;// = rand() % 62;
	string build = "";
	for (int i = 0; i < len; i++) {
		ascii_val = rand() % 62;
		build = build + string(&ascii_array[ascii_val], 1);
	}

	return build;
}
