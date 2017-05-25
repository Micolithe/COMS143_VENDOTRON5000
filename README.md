# COMS143_VENDOTRON5000
The Ultimate in Vending Machine Technology

# Description:
Spring 2017 final project for COMS 143 at Sussex County College - Advanced Programming in C++,
Project was to create a vending machine using C++

Unfortunately I don't know where I put the hardcopy of the project requirements so this is from memory:
- Minimum of two classes
- Keep track of cash in the machine broken out by change
- Keep track of the item stock in the machine and prevent you from buying items with a QTY of zero
- Functional service menu to restock and clear errors
- Random chance for the machine to jam and refund your money instead, at which point you have to enter the service menu to clear the jam.
- Display the items in a grid as if you were standing in front of the machine
- Keep track of lifetime sales stats (total qty, total dollar amount)
- Exact Change Mode if there is less than a certain amount in the machine (I chose $5)
- Prevent dispensing of item if not enough money is entered, or if the machinie is unable to disperse your change.

I did some extra stuff because I wanted to learn a little more, like conditional highlighting, multithreading because i couldn't get my hwnd callbacks to work and wanted to play audio, transaction level logging, state persistence through the use of three CSV files.


# Design Flowchart: (is actually more complex than this but this is what I came up with first before i started coding)

![alt tag](https://raw.githubusercontent.com/Micolithe/COMS143_VENDOTRON5000/master/The%20VendoTron%205000.png)
