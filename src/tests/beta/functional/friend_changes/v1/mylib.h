// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#include <iostream>
#include <string>

namespace mylib {

class Person {
private:
    std::string name;
    int age;

public:
    Person(const std::string& n, int a) : name(n), age(a) {}
    
    friend void displayPersonDetails(const Person& p);
    
    void displayInfo() const {
        std::cout << "Name: " << name << ", Age: " << age << std::endl;
    }
};

void displayPersonDetails(const Person& p) {
    std::cout << "Friend function accessing private data - ";
    std::cout << "Name: " << p.name << ", Age: " << p.age << std::endl;
}

class Complex {
private:
    double real;
    double imag;

public:
    Complex(double r = 0, double i = 0) : real(r), imag(i) {}
    
    friend Complex operator+(const Complex& c1, const Complex& c2);
    
    friend std::ostream& operator<<(std::ostream& os, const Complex& c);
    
    void display() const {
        std::cout << real << " + " << imag << "i" << std::endl;
    }
};

Complex operator+(const Complex& c1, const Complex& c2) {
    return Complex(c1.real + c2.real, c1.imag + c2.imag);
}

std::ostream& operator<<(std::ostream& os, const Complex& c) {
    os << c.real << " + " << c.imag << "i";
    return os;
}

class Engine;

class Car {
private:
    std::string model;
    int year;
    
public:
    Car(const std::string& m, int y) : model(m), year(y) {}
    
    friend void serviceCar(const Car& car, Engine& engine, void(*ok)(int x, int y));
    
    std::string getModel() const { return model; }
};

class Engine {
private:
    int horsepower;
    bool needsService;
    
public:
    Engine(int hp) : horsepower(hp), needsService(false) {}
    
    friend void serviceCar(const Car& car, Engine& engine, void(*ok)(int x, int y));
    
    bool isServiced() const { return !needsService; }
};

void serviceCar(const Car& car, Engine& engine, void(*ok)(int x, int y)) {
    std::cout << "Servicing " << car.model << " (" << car.year << ")" << std::endl;
    std::cout << "Engine horsepower: " << engine.horsepower << std::endl;
    engine.needsService = false;
    std::cout << "Service completed!" << std::endl;
}

template<typename T>
class Container {
private:
    T data;
    
public:
    Container(const T& value) : data(value) {}
    
    template<typename U>
    friend void printContainer(const Container<U>& c);
};

template<typename U>
void printContainer(const Container<U>& c) {
    std::cout << "Container data: " << c.data << std::endl;
}

class BankAccount {
private:
    std::string accountNumber;
    double balance;
    
    friend class BankManager;
    
public:
    BankAccount(const std::string& accNum, double initialBalance)
        : accountNumber(accNum), balance(initialBalance) {}
    
    void displayPublicInfo() const {
        std::cout << "Account: " << accountNumber << std::endl;
    }
};

class BankManager {
public:
    void adjustBalance(BankAccount& account, double amount) {
        account.balance += amount;
        std::cout << "Balance adjusted for account " << account.accountNumber;
        std::cout << ". New balance: " << account.balance << std::endl;
    }
    
    void displayFullAccountInfo(const BankAccount& account) {
        std::cout << "Account: " << account.accountNumber;
        std::cout << ", Balance: " << account.balance << std::endl;
    }
};

}