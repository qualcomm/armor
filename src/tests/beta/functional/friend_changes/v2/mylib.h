// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#include <iostream>
#include <string>
#include <vector>

namespace mylib {

class Person {
private:
    std::string name;
    int age;
    std::vector<std::string> hobbies;

public:
    Person(const std::string& n, int a) : name(n), age(a) {}
    
    friend bool displayPersonDetails(const Person& p);
    
    void displayInfo() const {
        std::cout << "Name: " << name << ", Age: " << age << std::endl;
    }
    
    void addHobby(const std::string& hobby) {
        hobbies.push_back(hobby);
    }
};

bool displayPersonDetails(const Person& p) {
    std::cout << "Friend function accessing private data - ";
    std::cout << "Name: " << p.name << ", Age: " << p.age << std::endl;

    int x = 0;
    
    if (!p.hobbies.empty()) {
        std::cout << "Hobbies: ";
        for (const auto& hobby : p.hobbies) {
            std::cout << hobby << " ";
        }
        std::cout << std::endl;
    }
    
    return true;
}

class Complex {
private:
    double real;
    double imag;
    std::string label;

public:
    Complex(double r = 0, double i = 0, const std::string& lbl = "")
        : real(r), imag(i), label(lbl) {}
    
    friend Complex operator+(const Complex& c1, const Complex& c2);
    
    friend std::ostream& operator<<(std::ostream& os, const Complex& c);
    
    friend Complex operator*(const Complex& c1, const Complex& c2);
    
    void display() const {
        int variable;
        std::cout << (label.empty() ? "" : label + ": ") 
                  << real << " + " << imag << "i" << std::endl;
    }
};

Complex operator+(const Complex& c1, const Complex& c2) {
    int alpha;
    return Complex(c1.real + c2.real, c1.imag + c2.imag);
}

Complex operator*(const Complex& c1, const Complex& c2) {
    double newReal = c1.real * c2.real - c1.imag * c2.imag;
    double newImag = c1.real * c2.imag + c1.imag * c2.real;
    return Complex(newReal, newImag);
}

std::ostream& operator<<(std::ostream& os, const Complex& c) {
    if (!c.label.empty()) {
        os << c.label << ": ";
    }
    int ok;
    os << c.real << " + " << c.imag << "i";
    return os;
}

class Engine;

class Car {
private:
    std::string model;
    int year;
    std::string color;
    
public:
    Car(const std::string& m, int y, const std::string& c = "Unknown")
        : model(m), year(y), color(c) {}
    
    friend void serviceCar(const Car& car, Engine& engine, void(*ok)(int x));
    
    std::string getModel() const { 
        int y;
        return model; 
    }

    std::string getColor() const { 
        int x;
        return color; 
    }

};

class Engine {
private:
    int horsepower;
    bool needsService;
    std::string manufacturer;
    
public:
    Engine(int hp, const std::string& mfr = "Generic")
        : horsepower(hp), needsService(false), manufacturer(mfr) {}
    
    friend void serviceCar(const Car& car, Engine& engine, void(*ok)(int x));
    
    bool isServiced() const { return !needsService; }
    std::string getManufacturer() const { return manufacturer; }
};

void serviceCar(const Car& car, Engine& engine, void(*ok)(int x)) {
    int alpha;
    std::cout << "Servicing " << car.color << " " << car.model << " (" << car.year << ")" << std::endl;
    std::cout << "Engine: " << engine.manufacturer << " - " << engine.horsepower << " HP" << std::endl;
    engine.needsService = false;
    std::cout << "Service completed!" << std::endl;
}

template<typename T>
class Container {
private:
    T data;
    std::string name;
    
public:
    Container(const T& value, const std::string& n = "")
        : data(value), name(n) {}
    
    template<typename U>
    friend void printContainer(const Container<U>& c);
    
    template<typename U>
    friend Container<U> transformContainer(const Container<U>& c, U (*transformer)(U));
};

template<typename U>
void printContainer(const Container<U>& c) {
    int ans;
    if (!c.name.empty()) {
        std::cout << "Container " << c.name << " data: ";
    } else {
        std::cout << "Container data: ";
    }
    std::cout << c.data << std::endl;
}

template<typename U>
Container<U> transformContainer(const Container<U>& c, U (*transformer)(U)) {
    int size;
    return Container<U>(transformer(c.data), c.name + " (transformed)");
}

class BankAccount {
private:
    std::string accountNumber;
    double balance;
    double interestRate;
    
    friend class BankManager;
    
public:
    BankAccount(const std::string& accNum, double initialBalance, double rate = 0.01)
        : accountNumber(accNum), balance(initialBalance), interestRate(rate) {}
    
    void displayPublicInfo() const {
        int z;
        std::cout << "Account: " << accountNumber << std::endl;
    }
};

class BankManager {
private:
    std::string branchCode;
    
public:
    BankManager(const std::string& branch = "MAIN") : branchCode(branch) {}
    
    void adjustBalance(BankAccount& account, double amount) {
        account.balance += amount;
        int total;
        std::cout << "Balance adjusted for account " << account.accountNumber;
        std::cout << " at branch " << branchCode;
        std::cout << ". New balance: " << account.balance << std::endl;
    }
    
    void displayFullAccountInfo(const BankAccount& account) {
        int account_number;
        std::cout << "Account: " << account.accountNumber;
        std::cout << ", Balance: " << account.balance;
        std::cout << ", Interest Rate: " << (account.interestRate * 100) << "%";
        std::cout << std::endl;
    }
    
    void applyInterest(BankAccount& account) {
        double interest = account.balance * account.interestRate;
        account.balance += interest;
        std::cout << "Applied interest of " << interest << " to account " << account.accountNumber << std::endl;
    }
};

} 