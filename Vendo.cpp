#include "stdafx.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdio>
#include <vector>
#include "Vendo.h"

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
	//INIT PRODUCT LIST
	fstream f_pricing("VEND_PRODUCTS.CSV", fstream::in);  //get product definition file
	f_pricing >> std::noskipws; //spaces are allowed its pretty important the program will crash & burn without this line.
	cout << "Opening product definitions file for reading: VEND_PRODUCTS.CSV" << endl;
	string v_loc_id_temp; //getline will only do strings so i need temp string vars for this.
	string v_name_temp;
	string v_price_temp;
	string v_qty_temp;
	int v_horiz_temp;
	int v_vert_temp;
	bool v_fatal = false;
	int loc = 0;
	double pr = 0;
	int q = 0;
	//Read the file, place each line into the struct
	if (!f_pricing.good() || f_pricing.bad()) {
		v_fatal = true;
		cout << "[FATAL] Error reading the pricing file! Please check for VEND_PRODUCTS.CSV, that it exists & is readable..." << endl;
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
	//cout << "constructor now complete" << endl;

	
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
		return false;
	}

	if (v_product[v_product_found_subscript].v_qty <= 0) { //none left
		cout << "There aren't any left. Try again." << endl;
		//cout << "Refunding $" << v_inserted_cash << endl;
		return false;
	}

	cout << "Insert cash! Price for " << v_product[v_product_found_subscript].v_name << " is $" <<
		v_product[v_product_found_subscript].v_price << endl;
	cin >>v_inserted_cash;
	if (v_product[v_product_found_subscript].v_price > v_inserted_cash) {
		cout << "You did not insert enough money." << endl;
		cout << "Refunding $" << v_inserted_cash << endl;
		return false;
	}

	if (this->getCashOnHand() < 2.50) {
		cout << "We are in Exact Change mode. Checking your input." << endl;
		if (v_product[v_product_found_subscript].v_price != v_inserted_cash) {
			cout << "Dollar amounts don't match. I can't give change right now. Try again later." << endl;
			cout << "Refunding $" << v_inserted_cash << endl << endl;
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
		return false;
	}
	else {
		v_change = v_inserted_cash - v_product[v_product_found_subscript].v_price;
		cout << "CONGRATUWELLDONE: You got one " << v_product[v_product_found_subscript].v_name << " and recieved $" <<
			v_change << " back as change." << endl<<endl;
		v_product[v_product_found_subscript].v_qty--;
		this->addSalesQty(1);
		this->addCashOnHand(v_product[v_product_found_subscript].v_price);
		this->addSalesDollars(v_product[v_product_found_subscript].v_price);
		return true;
	}

}

bool Vendo::dispenseItem(double cash)
{
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
		return false;
	}
	this->displayProductList(cash);
	bool v_fuck_you = this->theVendoGambit(); //true = jammed
	cout << "Please select a product ID. Only those you can afford & are not empty have been displayed." << endl<<endl;
	cin >> v_prod;
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
		return false;
	}
	if (v_product[v_product_found_subscript].v_qty <=0) {
		cout << "There aren't any left..." << endl;
		cout << "Refunding $" << cash << endl;
		return false;
	}
	if (v_product[v_product_found_subscript].v_price > cash) {
		cout << "You did not insert enough money." << endl;
		cout << "Refunding $" << cash << endl;
		return false;
	}

	if (this->getCashOnHand() < 2.50) {
		cout << "We are in Exact Change mode. Checking your input." << endl;
		if (v_product[v_product_found_subscript].v_price != cash) {
			cout << "Dollar amounts don't match. I can't give change right now. Try again later." << endl;
			cout << "Refunding $" << cash << endl << endl;
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
		return false;
	}
	else {
		v_change = cash - v_product[v_product_found_subscript].v_price;
		cout << endl<< "CONGRATUWELLDONE: You got one " << v_product[v_product_found_subscript].v_name << " and recieved $" <<
			v_change << " back as change." << endl<<endl;
		v_product[v_product_found_subscript].v_qty--;
		this->addSalesQty(1);
		this->addCashOnHand(v_product[v_product_found_subscript].v_price);
		this->addSalesDollars(v_product[v_product_found_subscript].v_price);
		return true;
	}
	return true;

}

void Vendo::setJam() {
	b_jammed = true;
}
bool Vendo::checkJam() {
	return b_jammed;
}
bool Vendo::theVendoGambit() { //true = jammed, false = not jammed
	//Let's say there's a 1 in 20 chance that your purchase gets stuck.
	int v_rand = rand();
	int v_rand_b = v_rand % 20; // 0-19 range
	bool v_result = false;
	if (v_rand_b == 10) {
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
			cin >> v_choice;
			v_svc_result = this->serviceMenuChoice(v_choice);
			if (!v_svc_result) {
				return 1;
			}
		}
		else {
			cout << "Sorry, service password was incorrect." << endl;
			return -1;
		}
		if (!v_svc_result) {
			break;
		}
	}
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
		cin >> v_restock_id;
		cout << "How many are you adding?" << endl;
		cin >> v_restock_qty;
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
			v_product[v_found_product_subscript].v_qty = v_restock_qty;
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
	cout << endl<<"Welcome to the VendoTron 5000" << endl;
	cout << "The Ultimate in Vending Technology (TM)" << endl<<endl;
	int v_menu_input = 0;
	bool v_return_code = true;
	while (v_return_code) {

		cout << "[SYSTEM HEALTH CHECK]" << endl; //We should check if the system is in an error state (empty or jammed)
		if (this->checkEmpty()) {
			this->setJam();
		}
		if (this->checkJam()) {
			cout << "*** The VendoTron has detected an error state. Please enter the service menu to fix. ***" << endl;
		}
		else {
			cout << "System Health is good" << endl << endl;
		}
		if (this->getCashOnHand() <= 2.50) {
			cout << "EXACT CHANGE ONLY." << endl<<endl;
		}
		cout << "[END SYSTEM HEALTH CHECK]" << endl << endl;
		cout << "Please make a selection:" << endl;
		cout << " (1) Enter a product number" << endl;
		cout << " (2) Insert Cashola" << endl;
		cout << " (3) Display System Stats" << endl;
		cout << " (4) Service Menu" << endl;
		cout << " (0) Self-Destruct Sequence" << endl << endl;
		cin >> v_menu_input;
		v_return_code = this->vendMenuChoice(v_menu_input);
	}
	cout << "Debug: We got to the end of vendMenu() - choice was "<<v_menu_input << endl;

}



void Vendo::displayStats() { //just system statistics
	cout << "[SYSTEM STATS]" << endl;
	cout << "  [Lifetime Sales (Dollars)] " << this->getLifetimeSales() << endl;
	cout << "  [Lifetime Sales (Quantity)] " << this->getLifetimeQtySales() << endl;
	cout << "  [Cash on Hand] " << this->getCashOnHand() << endl;
	cout << "[END SYSTEM STATS]" << endl << endl;
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
		cin >> v_selected_product;
		this->dispenseItem(v_selected_product);
		break;
	case 2: //Insert Cash before selecting
		cout << "Please insert a dollars: (enter 0 to return to previous)" << endl;
		cin >> v_inserted_cash;
		this->dispenseItem(v_inserted_cash);
		break;
	case 3: //Stats for nerds as youtube would call it
		this->displayStats();
		break;
	case 4: //Call service menu
		cout << "Enter Service Password:" << endl;
		cin >> v_pass;
		this->serviceMenu(v_pass);
		break;
	default: //invalid choice
		cout << "Not a valid menu choice! Try again." << endl;
		break;
	};
	return true;
}