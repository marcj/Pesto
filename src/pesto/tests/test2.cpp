#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include <memory>

TEST_CASE("Test basics 1") {
    char bla[] = "Abc";
    char *bla2 = bla;

    bla[0] = (char) 'N';

    REQUIRE(strcmp(bla, "Nbc") == 0);
    REQUIRE(strcmp(bla2, "Nbc") == 0);
}

struct Bla {
    int i = 0;
};

void incrementBla(Bla &bla) {
    bla.i++;
}


Bla &newBla() {
    auto &bla = *new Bla;
    bla.i++;

    return bla;
}

TEST_CASE("Test basics 2") {

    auto &bla = *new Bla;
    incrementBla(bla);
    REQUIRE(bla.i == 1);

    auto &bla2 = newBla();
    REQUIRE(bla.i == 1);
}

TEST_CASE("Test basics 3") {
    std::vector<std::string> list;

    list.emplace_back("A");
    list.emplace_back("B");
    list.emplace_back("C");

    for (auto item = std::next(list.begin()); item != list.end(); ++item) {
        printf("item: %s\n", item->c_str());
    }
}

#include "../core/Timer.cpp"
#include <time.h>

TEST_CASE("Test basics 4") {
    Pesto::Timer t;
//    nanosleep((const struct timespec[]){{0, 500000000L}}, NULL);
    auto start = std::chrono::high_resolution_clock::now();
    sleep(1);
//    int i = 2;
    printf("took %.12fms\n", t.elapsed() * 1000);
    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;

    printf("took %.12fms\n", elapsed.count() * 1000);
}


std::function<int(int)> make_lambda(int j) {
    return [=](int i) -> int {
        return i + j;
    };
}

TEST_CASE("Test basics 5") {
    Pesto::Timer t;

//    for (int i = 0; i < 1000; i++) {
//        make_lambda()(i);
//    }

    make_lambda(1)(1);
    printf("took %.12fms\n", t.elapsed() * 1000);

    t.reset();
    make_lambda(1)(2);
    printf("took %.12fms\n", t.elapsed() * 1000);

    t.reset();
    make_lambda(1)(3);
    printf("took %.12fms\n", t.elapsed() * 1000);

    t.reset();
    make_lambda(2)(3);
    printf("took %.12fms\n", t.elapsed() * 1000);

    t.reset();
    make_lambda(2)(3);
    printf("took %.12fms\n", t.elapsed() * 1000);

    t.reset();
    make_lambda(2)(3);
    printf("took %.12fms\n", t.elapsed() * 1000);

//    make_lambda()(2);
//    make_lambda()(3);
//    make_lambda()(4);

}


void increment_by_ref(int &i) {
    i++;
}

void increment_by_ptr(int *i) {
    (*i)++;
}

TEST_CASE("Test references 1") {
    int i = 5;
    increment_by_ref(i);
    REQUIRE(i == 6);
    increment_by_ptr(&i);
    REQUIRE(i == 7);

    int *j = new int(6);
    increment_by_ref(*j);
    REQUIRE(*j == 7);

    increment_by_ptr(j);
    REQUIRE(*j == 8);
}

TEST_CASE("Test std") {
    int i;

    i = std::stoi("5");
    REQUIRE(i == 5);

    i = std::stoi("-05");
    REQUIRE(i == -5);

    i = std::stoi("15px");
    REQUIRE(i == 15);

    i = std::stoi("-15px");
    REQUIRE(i == -15);

    i = std::stoi("55 px");
    REQUIRE(i == 55);
}

TEST_CASE("Test pointer delete") {
    int *ip = new int(5);

    REQUIRE(*ip == 5);

    (*ip)++;

    REQUIRE(*ip == 6);

    delete ip;

    REQUIRE(ip != nullptr);
}


TEST_CASE("Test vector erase") {
    std::vector<std::string> list;

    list.push_back("a");
    list.push_back("b");
    list.push_back("c");
    list.push_back("d");

    REQUIRE(list.size() == 4);

    list.erase(std::remove(list.begin(), list.end(), "b"), list.end());

    REQUIRE(list.size() == 3);
    REQUIRE(list[0] == "a");
    REQUIRE(list[1] == "c");
    REQUIRE(list[2] == "d");
}

class TestFactoryBase {
public:
    virtual int foo() {
        return 0;
    }
};

class TestFactoryDerivated : public TestFactoryBase {
public:
    int foo() override {
        return 1;
    }
};

std::shared_ptr<TestFactoryBase> TestFactoryCreate(int variant = 0) {
    if (variant == 1) {
        return std::shared_ptr<TestFactoryBase>(new TestFactoryDerivated());
    }

    return std::shared_ptr<TestFactoryBase>(new TestFactoryBase());
}


TEST_CASE("Test factory") {
    auto f0 = TestFactoryCreate(0);
    REQUIRE(f0->foo() == 0);
    auto f1 = TestFactoryCreate(1);
    REQUIRE(f1->foo() == 1);
}