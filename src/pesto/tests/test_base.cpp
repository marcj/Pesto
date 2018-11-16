#define CATCH_CONFIG_MAIN

#include "catch.hpp"

#include "../Base.h"


TEST_CASE("Test EventDispatcher: Base") {
    Pesto::EventDispatcher<int> changes;

    int i = 0;
    REQUIRE(i == 0);

    changes.listen([&i](int j) {
        i += j;
    });

    REQUIRE(changes.listeners.size() == 1);
    changes.fire(1);
    REQUIRE(i == 1);

    REQUIRE(changes.listeners.size() == 1);
    changes.fire(2);
    REQUIRE(i == 3);
}


TEST_CASE("Test EventDispatcher: Once") {
    Pesto::EventDispatcher<bool> changes;

    int i = 0;
    REQUIRE(i == 0);

    changes.once([&i](bool) {
        i++;
    });

    REQUIRE(changes.listeners.size() == 1);

    changes.fire(true);

    REQUIRE(changes.listeners.empty());
    REQUIRE(i == 1);

    changes.fire(true);
    REQUIRE(i == 1);
}

TEST_CASE("Test EventDispatcher: Once 2") {
    Pesto::EventDispatcher<bool> changes;

    int i = 0;
    int j = 0;
    REQUIRE(i == 0);
    REQUIRE(j == 0);

    changes.once([&i](bool) {
        i++;
    });

    changes.once([&j](bool) {
        j++;
    });

    REQUIRE(changes.listeners.size() == 2);

    changes.fire(true);

    REQUIRE(changes.listeners.empty());
    REQUIRE(i == 1);
    REQUIRE(j == 1);

    changes.fire(true);
    REQUIRE(i == 1);
    REQUIRE(j == 1);
}

TEST_CASE("Test EventDispatcher: mix") {
    Pesto::EventDispatcher<bool> changes;

    int i = 0;
    int j = 0;
    REQUIRE(i == 0);
    REQUIRE(j == 0);

    changes.once([&i](bool) {
        i++;
    });

    changes.listen([&j](bool) {
        j++;
    });

    REQUIRE(changes.listeners.size() == 2);

    changes.fire(true);

    REQUIRE(changes.listeners.size() == 1);
    REQUIRE(i == 1);
    REQUIRE(j == 1);

    changes.fire(true);
    REQUIRE(i == 1);
    REQUIRE(j == 2);
}

class LifetimeObject: public Pesto::LifeTimeEventDispatcher {

};

TEST_CASE("Test EventDispatcher: lifetime") {
    Pesto::EventDispatcher<bool> changes;

    int i = 0;
    REQUIRE(i == 0);

    {
        changes.listen([&i](bool) {
            i++;
        });

        REQUIRE(changes.listeners.size() == 1);
        changes.fire(true);
        REQUIRE(changes.listeners.size() == 1);
        REQUIRE(i == 1);
    }

    REQUIRE(changes.listeners.size() == 1);
    changes.fire(true);
    REQUIRE(i == 2);


    Pesto::EventDispatcher<bool> changes2;
    int i2 = 0;
    {
        LifetimeObject dieSoonPlease;

        changes2.listenWhile([&i2](bool) {
            i2++;
        }, dieSoonPlease);

        REQUIRE(changes2.listeners.size() == 1);
        changes2.fire(true);
        REQUIRE(changes2.listeners.size() == 1);
        REQUIRE(i2 == 1);
    }

    REQUIRE(changes2.listeners.empty());
    changes2.fire(true);
    REQUIRE(i2 == 1);
}
