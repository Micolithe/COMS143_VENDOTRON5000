// The_VendoTron_5000.cpp : Defines the ULTIMATE IN VENDING TECHNOLOGY
// ---------------------------------------------------
// COMS143-01: Programming in C++ II
// Spring 2017
// Sussex County Community College
// Programmed by Alex Kronish
// ---------------------------------------------------



#include  "stdafx.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdio>
#include <vector>
#include <Windows.h>
#include <thread>
#include "vendo.h"


using namespace std;

void audioThread(bool*);
void vendoThread(bool*);
void nowLoading();

//extern const HWND hwndCallback = GetConsoleWindow(); //i dont know why this doesnt work?

int main() {

	nowLoading();

	HANDLE  hConsole;
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, 10);
	
	srand(time(0));
	
	bool v_running = true;
	bool *p_running = &v_running;
	
	thread thr_vendo(vendoThread, p_running); //get that paper
	thread thr_audio(audioThread, p_running); //play that track

	while (v_running) { //we definitely shouldn't leave main while the threads are running
		Sleep(10000); //wait 10 seconds and check again.
	}    
	
	thr_vendo.join();
	thr_audio.join();
	cout << "Both threads terminated." << endl;
	//char c;
	//cin >> c;
    return 0;
}

void audioThread(bool *p_running) { //MIDIS are bullshit!!!!!!
	srand(time(0));
	int v_mins[5] = {5,1,1,2,2};
	int v_secs[5] = {53,11,50,22,16};
	int v_millis;
	int v_rando=0;

	LPCWSTR v_midi_file[5] = { L"open AUDIO1.MID type sequencer alias a1",
		L"open AUDIO2.MID type sequencer alias a2",
		L"open AUDIO3.MID type sequencer alias a3",
		L"open AUDIO4.MID type sequencer alias a4",
		L"open AUDIO5.MID type sequencer alias a5" };
	LPCWSTR v_midi_rewind[5] = { L"seek a1 to start",
		L"seek a2 to start",
		L"seek a3 to start",
		L"seek a4 to start",
		L"seek a5 to start" };
	LPCWSTR v_midi_play[5] = { L"play a1",
		L"play a2",
		L"play a3",
		L"play a4",
		L"play a5" };
	
	cout << "*** Midi Loop Thread Starting....\n";

	while (*p_running){ //if the Vendo thread signifies it's over, dont go for another round
		v_rando = rand() % 5;
		v_millis = ((v_mins[v_rando] * 60) + v_secs[v_rando]) * 1000; 
		//calculate milliseconds to sleep for until a new king is chosen
		mciSendString(v_midi_file[v_rando], NULL, 0, NULL);
		mciSendString(v_midi_rewind[v_rando], NULL, 0, NULL);
		mciSendString(v_midi_play[v_rando],NULL,0,NULL);
		Sleep(v_millis);
		//this shit is MAD CLUMSY but i couldn't get hwnd callbacks & mcisendstring notify to work
	}
	cout << "*** Midi thread terminating" << endl;

}

void vendoThread(bool *p_running)
{	
	cout << "*** VendoTron 5000 Thread Starting....\n";
	Vendo *o_machine = new Vendo(42069);
	o_machine->vendMenu(); //vendMenu kicks off the program
	*p_running = false; //thread over, we can return to main by setting the running flag to false to kill the loop
	cout << "VendoTron has terminated. v_running should be false now." << endl;
	cout << "Running destructor" << endl;
	o_machine->writePersist(); //Save our work before we free the memory.
	delete o_machine;
	cout << "Destructor complete. Program will shut down soon." << endl;
	cout << "*** Vendo Thread Terminating..." << endl;
}

void nowLoading() {
	wchar_t vert = wchar_t(186);
	wchar_t full = wchar_t(178);
	wchar_t empt = wchar_t(176);
	wchar_t tl = wchar_t(201);
	wchar_t bl = wchar_t(200);
	wchar_t horiz = wchar_t(205);
	wchar_t tr = wchar_t(187);
	wchar_t br = wchar_t(188);
	int v_count_full = 0;
	int v_count_empty = 16;
	while(v_count_full <=16) {
		//16 frames of loading
		system("CLS");
		wcout << tl << horiz << horiz << horiz << horiz << horiz << horiz << horiz << horiz << horiz << horiz 
			<< horiz << horiz << horiz << horiz << horiz << horiz << horiz << horiz << tr << endl;
		wcout << vert << "  NOW LOADING...  " << vert << endl;
		wcout << vert << "                  " << vert << endl;
		wcout << vert << " ";

		for (int i = 0; i < v_count_empty; i++) {
			wcout << empt;
		}

		for (int j = 0; j < v_count_full; j++) {
			wcout << full;
		}
		wcout << " " << vert << endl;
		wcout << wchar_t(186) << "                  " << wchar_t(186) << endl;
		wcout << bl << horiz << horiz << horiz << horiz << horiz << horiz << horiz << horiz << horiz
			<< horiz << horiz << horiz << horiz << horiz << horiz << horiz << horiz << horiz << br << endl;
		v_count_empty--;
		v_count_full++;
		Sleep(rand() % (500 + 1 - 150) + 150);
	}
	Sleep(500);
	system("CLS");
}