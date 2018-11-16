#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "../core/utils.h"
#include "../TextCursor.h"
#include <tuple>


TEST_CASE("TextCursor basic") {
    auto typeface = SkTypeface::MakeFromName("Arial Unicode MS",
                                             SkFontStyle(500, 5, SkFontStyle::Slant::kUpright_Slant));
    SkPaint paint;
    paint.setTextSize(16);
    paint.setTypeface(typeface);

    Pesto::TextCursor c(paint, "MMM");

    printf("widthOfM = %f\n", c.widthOf("M"));
    printf("widthOf👀 = %f\n", c.widthOf("👀"));
    printf("totalWidth = %f\n", c.totalWidth());

    REQUIRE(c.widthOf("M") * 3 == c.totalWidth());
}

TEST_CASE("TextCursor skia utf8") {
    const char *hello = "Hello World";
    REQUIRE(SkUTF8_CountUnichars(hello) == 11);

    const char *utf8 = "👀Hello ϔ";
    REQUIRE(SkUTF8_CountUnichars(utf8) == 8);

    int i = 0;
    while (SkUnichar c = SkUTF8_NextUnichar(&utf8)) {
        char utf8Res[4];
        size_t len = SkUTF8_FromUnichar(c, utf8Res);
        for (size_t i = 0; i < len; i++) {
            printf("%c", utf8Res[i]);
        }
        for (size_t i = 0; i < len; i++) {
            printf(" %#08x", utf8Res[i]);
        }
        printf("\n");
        i++;
    }
    REQUIRE(i == 8);

    SkPaint paint;
    Pesto::TextCursor c(paint, "👀Hello ϔ");

    REQUIRE(c.characterCount() == 8);

}

TEST_CASE("TextCursor sharp-wrap") {
    Pesto::utils::setupErrorListener();

    auto typeface = SkTypeface::MakeFromName("Arial Unicode MS",
                                             SkFontStyle(500, 5, SkFontStyle::Slant::kUpright_Slant));
    SkPaint paint;
    paint.setTextSize(16);
    paint.setTypeface(typeface);

    Pesto::TextCursor c(paint, "Hello World");
    float width;
    std::string text;

    std::tie(width, text) = c.nextSharpWrap(c.measureWidth("He"));
    REQUIRE(text == "He");
    REQUIRE(width == c.measureWidth("He"));

    c.reset();
    std::tie(width, text) = c.nextSharpWrap(1); //we get at least one character
    REQUIRE(text == "H");
    REQUIRE(width == c.measureWidth("H"));

    c.reset();
    std::tie(width, text) = c.nextSharpWrap(1231); //we still get only one word
    REQUIRE(text == "Hello");
    REQUIRE(width == c.measureWidth("Hello"));

    c.reset();
    std::tie(width, text) = c.nextSharpWrap(c.measureWidth("Hello "));
    REQUIRE(text == "Hello");
    REQUIRE(width == c.measureWidth("Hello"));

    std::tie(width, text) = c.nextSharpWrap(c.measureWidth("World"));
    REQUIRE(text == "World");
    REQUIRE(width == c.measureWidth("World"));

    //end
    std::tie(width, text) = c.nextSharpWrap(5);
    REQUIRE(text == "");
    REQUIRE(width == 0);


    Pesto::TextCursor c2(paint, "  Hello    World   ");
    std::tie(width, text) = c2.nextSharpWrap(c.measureWidth("He"));
    REQUIRE(text == "He");
    REQUIRE(width == c2.measureWidth("He"));

    std::tie(width, text) = c2.nextSharpWrap(c2.measureWidth("llo "));
    REQUIRE(width == c2.measureWidth("llo"));
    REQUIRE(text == "llo");

    std::tie(width, text) = c2.nextSharpWrap(c2.measureWidth(" World "));
    REQUIRE(width == c2.measureWidth("World"));
    REQUIRE(text == "World");

    std::tie(width, text) = c2.nextSharpWrap(123123); //end
    REQUIRE(text == "");
    REQUIRE(width == 0.0f);
}

TEST_CASE("TextCursor sharp-wrap preserve whitespace") {
    auto typeface = SkTypeface::MakeFromName("Arial Unicode MS",
                                             SkFontStyle(500, 5, SkFontStyle::Slant::kUpright_Slant));
    SkPaint paint;
    paint.setTextSize(16);
    paint.setTypeface(typeface);
    float width;
    std::string text;

    Pesto::TextCursor c(paint, "Hello World");
    c.setPreserveWhiteSpace(true);

    std::tie(width, text) = c.nextSharpWrap(c.measureWidth("He"));
    REQUIRE(text == "He");
    REQUIRE(width == c.measureWidth("He"));

    c.reset();
    std::tie(width, text) = c.nextSharpWrap(1); //we get at least one character
    REQUIRE(text == "H");
    REQUIRE(width == c.measureWidth("H"));

    c.reset();
    std::tie(width, text) = c.nextSharpWrap(c.measureWidth("Hello "));
    REQUIRE(text == "Hello ");
    REQUIRE(width == c.measureWidth("Hello "));

    std::tie(width, text) = c.nextSharpWrap(c.measureWidth("World"));
    REQUIRE(text == "World");
    REQUIRE(width == c.measureWidth("World"));

    //end
    std::tie(width, text) = c.nextSharpWrap(5);
    REQUIRE(text == "");
    REQUIRE(width == 0);


    Pesto::TextCursor c2(paint, "  Hello    World   ");
    c2.setPreserveWhiteSpace(true);
    std::tie(width, text) = c2.nextSharpWrap(c.measureWidth("  "));
    REQUIRE(text == "  ");
    REQUIRE(width == c2.measureWidth("  "));

    std::tie(width, text) = c2.nextSharpWrap(c.measureWidth("He"));
    REQUIRE(text == "He");
    REQUIRE(width == c2.measureWidth("He"));

    std::tie(width, text) = c2.nextSharpWrap(c2.measureWidth("llo "));
    REQUIRE(text == "llo ");
    REQUIRE(width == c2.measureWidth("llo "));

    std::tie(width, text) = c2.nextSharpWrap(c2.measureWidth("   Wor"));
    REQUIRE(text == "   Wor");
    REQUIRE(width == c2.measureWidth("   Wor"));

    std::tie(width, text) = c2.nextSharpWrap(123123); //end
    REQUIRE(text == "ld   ");
    REQUIRE(width == c2.measureWidth("ld   "));
}


TEST_CASE("TextCursor word-wrap") {
    auto typeface = SkTypeface::MakeFromName("Arial Unicode MS",
                                             SkFontStyle(500, 5, SkFontStyle::Slant::kUpright_Slant));
    SkPaint paint;
    paint.setTextSize(16);
    paint.setTypeface(typeface);

    Pesto::TextCursor c(paint, "Hello World");
    float width;
    std::string text;

    std::tie(width, text) = c.nextWordWrap(c.measureWidth("Hello "));
    REQUIRE(text == "Hello");
    REQUIRE(width == c.measureWidth("Hello"));

    c.reset();

    std::tie(width, text) = c.nextWordWrap(c.measureWidth("Hello"));
    REQUIRE(text == "Hello");
    REQUIRE(width == c.measureWidth("Hello"));

    c.reset();

    std::tie(width, text) = c.nextWordWrap(c.measureWidth("He"));
    REQUIRE(text == "Hello");
    REQUIRE(width == c.measureWidth("Hello"));

    c.reset();

    std::tie(width, text) = c.nextWordWrap(0);
    REQUIRE(text == "Hello");
    REQUIRE(width == c.measureWidth("Hello"));

    c.reset();

    std::tie(width, text) = c.nextWordWrap(c.measureWidth("Hello Wor"));
    REQUIRE(text == "Hello");
    REQUIRE(width == c.measureWidth("Hello"));

    c.reset();

    std::tie(width, text) = c.nextWordWrap(c.measureWidth("Hello World"));
    REQUIRE(text == "Hello"); //we get always only the first word
    REQUIRE(width == c.measureWidth("Hello"));
    std::tie(width, text) = c.nextWordWrap(c.measureWidth("Hello World"));
    REQUIRE(text == "World"); //now the next
    REQUIRE(width == c.measureWidth("World"));

    c.reset();
    c.nextWordWrap(0); //eat Hello
    std::tie(width, text) = c.nextWordWrap(c.measureWidth("World"));
    REQUIRE(text == "World");
    REQUIRE(width == c.measureWidth("World"));

    c.reset();
    std::tie(width, text) = c.nextWordWrap(0); //eat Hello
    REQUIRE(text == "Hello");
    std::tie(width, text) = c.nextWordWrap(c.measureWidth("Wor"));
    REQUIRE(text == "World");
    REQUIRE(width == c.measureWidth("World"));

    c.reset();
    c.nextWordWrap(0); //eat Hello
    std::tie(width, text) = c.nextWordWrap(0);
    REQUIRE(text == "World");
    REQUIRE(width == c.measureWidth("World"));

    std::tie(width, text) = c.nextWordWrap(0);
    REQUIRE(text == "");
    REQUIRE(width == 0.0f);

    std::tie(width, text) = c.nextWordWrap(5); //end
    REQUIRE(text == "");
    REQUIRE(width == 0.0f);

    std::tie(width, text) = c.nextWordWrap(5); //end
    REQUIRE(text == "");
    REQUIRE(width == 0.0f);


    //test whitespace is removed
    Pesto::TextCursor c2(paint, "  Hello    World   ");
    std::tie(width, text) = c2.nextWordWrap(5);
    REQUIRE(text == "Hello");
    REQUIRE(width == c2.measureWidth("Hello"));

    std::tie(width, text) = c2.nextWordWrap(123123);
    REQUIRE(text == "World");
    REQUIRE(width == c2.measureWidth("World"));

    std::tie(width, text) = c2.nextWordWrap(123123); //end
    REQUIRE(text == "");
    REQUIRE(width == 0.0f);

    std::tie(width, text) = c2.nextWordWrap(123123); //end
    REQUIRE(text == "");
    REQUIRE(width == 0.0f);
}


TEST_CASE("TextCursor word-wrap nextLine") {
    auto typeface = SkTypeface::MakeFromName("Arial Unicode MS",
                                             SkFontStyle(500, 5, SkFontStyle::Slant::kUpright_Slant));
    SkPaint paint;
    paint.setTextSize(16);
    paint.setTypeface(typeface);

    Pesto::TextCursor c(paint, "Hello World, Pesto !");
    c.setAutoWrap(true);
    float width;
    std::string text;

    std::tie(width, text) = c.nextLine(c.measureWidth("   Hello    World, Pesto !  "));
    REQUIRE(text == "Hello World, Pesto !");
    REQUIRE(width == c.measureWidth("Hello World, Pesto !"));

    c.reset();
    std::tie(width, text) = c.nextLine(c.measureWidth("Hello "));
    REQUIRE(text == "Hello");
    REQUIRE(width == c.measureWidth("Hello"));

    std::tie(width, text) = c.nextLine(c.measureWidth("123"));
    REQUIRE(text == "");
    REQUIRE(width == 0.f);

    std::tie(width, text) = c.nextLine(c.measureWidth("World, Pes"));
    REQUIRE(text == "World,");
    REQUIRE(width == c.measureWidth("World,"));

    std::tie(width, text) = c.nextLine(1, true);
    REQUIRE(text == "Pesto");
    REQUIRE(width == c.measureWidth("Pesto"));

    std::tie(width, text) = c.nextLine(1, true);
    REQUIRE(text == "!");
    REQUIRE(width == c.measureWidth("!"));

    std::tie(width, text) = c.nextLine(10);
    REQUIRE(text == "");
    REQUIRE(width == 0.0f);

    std::tie(width, text) = c.nextLine(1, true);
    REQUIRE(text == "");
    REQUIRE(width == 0.0f);
}


TEST_CASE("TextCursor word-wrap nextLine preserve whitespace") {

    auto typeface = SkTypeface::MakeFromName("Arial Unicode MS",
                                             SkFontStyle(500, 5, SkFontStyle::Slant::kUpright_Slant));
    SkPaint paint;
    paint.setTextSize(16);
    paint.setTypeface(typeface);

    Pesto::TextCursor c(paint, "   Hello    World, Pesto !  ");
    c.setAutoWrap(true);
    c.setPreserveWhiteSpace(true);
    float width;
    std::string text;

    std::tie(width, text) = c.nextLine(c.measureWidth("   Hello    World, Pesto !  "));
    REQUIRE(text == "   Hello    World, Pesto !  ");
    REQUIRE(width == c.measureWidth("   Hello    World, Pesto !  "));

    c.reset();
    std::tie(width, text) = c.nextLine(c.measureWidth("Hello "));
    REQUIRE(text == ""); //doesnt fit
    std::tie(width, text) = c.nextLine(c.measureWidth("   Hello"));
    REQUIRE(text == "   Hello");
    REQUIRE(width == c.measureWidth("   Hello"));

    std::tie(width, text) = c.nextLine(3);
    REQUIRE(text == ""); //doesnt fit
    std::tie(width, text) = c.nextLine(c.measureWidth("    World, "));
    REQUIRE(text == "    World, ");
    REQUIRE(width == c.measureWidth("    World, "));

    std::tie(width, text) = c.nextLine(1);
    REQUIRE(text == "");

    std::tie(width, text) = c.nextLine(1, true);
    REQUIRE(text == "Pesto");

    std::tie(width, text) = c.nextLine(1, true);
    REQUIRE(text == " ");
    REQUIRE(width == c.measureWidth(" "));

    std::tie(width, text) = c.nextLine(1, true);
    REQUIRE(text == "!");
    REQUIRE(width == c.measureWidth("!"));

    std::tie(width, text) = c.nextLine(2323);
    REQUIRE(text == "  ");
    REQUIRE(width == c.measureWidth("  "));

    std::tie(width, text) = c.nextLine(2323);
    REQUIRE(text == "");
    REQUIRE(width == 0.0f);
}


TEST_CASE("TextCursor word-wrap nextLine multiline") {
    auto typeface = SkTypeface::MakeFromName("Arial Unicode MS",
                                             SkFontStyle(500, 5, SkFontStyle::Slant::kUpright_Slant));
    SkPaint paint;
    paint.setTextSize(16);
    paint.setTypeface(typeface);

    Pesto::TextCursor c(paint, "   Hello\t    World, Pesto! \n Foo bar. ");
    c.setAutoWrap(true);
    c.setBreakOnNewLine(true);
    float width;
    std::string text;

    std::tie(width, text) = c.nextWordWrap(1);
    REQUIRE(text == "Hello");
    std::tie(width, text) = c.nextWordWrap(1);
    REQUIRE(text == "World,");
    std::tie(width, text) = c.nextWordWrap(1);
    REQUIRE(text == "Pesto!");
    std::tie(width, text) = c.nextWordWrap(1);
    REQUIRE(text == "Foo");
    std::tie(width, text) = c.nextWordWrap(1);
    REQUIRE(text == "bar.");

    c.reset();
    std::tie(width, text) = c.nextLine(1, true);
    REQUIRE(text == "Hello");
    std::tie(width, text) = c.nextLine(1, true);
    REQUIRE(text == "World,");
    std::tie(width, text) = c.nextLine(1, true);
    REQUIRE(text == "Pesto!");
    std::tie(width, text) = c.nextLine(1, true);
    REQUIRE(text == ""); //new line now
    std::tie(width, text) = c.nextLine(1, true);
    REQUIRE(text == "Foo");
    std::tie(width, text) = c.nextLine(1, true);
    REQUIRE(text == "bar.");

    c.reset();
    std::tie(width, text) = c.nextLine(1231233);
    REQUIRE(text == "Hello World, Pesto!");
    std::tie(width, text) = c.nextLine(1231233);
    REQUIRE(text == "Foo bar.");

    c.reset();
    std::tie(width, text) = c.nextLine(c.measureWidth("Hello Wor"));
    REQUIRE(text == "Hello");

    c.reset();
    std::tie(width, text) = c.nextLine(c.measureWidth("Hello World, "));
    REQUIRE(text == "Hello World,");

    std::tie(width, text) = c.nextLine(c.measureWidth("Pesto"));
    REQUIRE(text == "");
    std::tie(width, text) = c.nextLine(c.measureWidth("Pesto!"));
    REQUIRE(text == "Pesto!");
    std::tie(width, text) = c.nextLine(123123132);
    REQUIRE(text == ""); //new line
    std::tie(width, text) = c.nextLine(123123132);
    REQUIRE(text == "Foo bar.");
}

TEST_CASE("TextCursor word-wrap nextLine multiline preserve whitespace") {
    auto typeface = SkTypeface::MakeFromName("Arial Unicode MS",
                                             SkFontStyle(500, 5, SkFontStyle::Slant::kUpright_Slant));
    SkPaint paint;
    paint.setTextSize(16);
    paint.setTypeface(typeface);

    Pesto::TextCursor c(paint, "   Hello\t    World, Pesto! \n Foo bar. ");
    c.setAutoWrap(true);
    c.setBreakOnNewLine(true);
    c.setPreserveWhiteSpace(true);
    float width;
    std::string text;

    std::tie(width, text) = c.nextWordWrap(1);
    REQUIRE(text == "   ");
    std::tie(width, text) = c.nextWordWrap(1);
    REQUIRE(text == "Hello");
    std::tie(width, text) = c.nextWordWrap(c.measureWidth("\t    "));
    REQUIRE(text == "\t    ");
    std::tie(width, text) = c.nextWordWrap(1);
    REQUIRE(text == "World,");
    std::tie(width, text) = c.nextWordWrap(1);
    REQUIRE(text == " ");
    std::tie(width, text) = c.nextWordWrap(1);
    REQUIRE(text == "Pesto!");
    std::tie(width, text) = c.nextWordWrap(1);
    REQUIRE(text == " \n ");
    std::tie(width, text) = c.nextWordWrap(1);
    REQUIRE(text == "Foo");
    std::tie(width, text) = c.nextWordWrap(1);
    REQUIRE(text == " ");
    std::tie(width, text) = c.nextWordWrap(1);
    REQUIRE(text == "bar.");
    std::tie(width, text) = c.nextWordWrap(1);
    REQUIRE(text == " ");
    std::tie(width, text) = c.nextWordWrap(1);
    REQUIRE(text == "");

    c.reset();
    std::tie(width, text) = c.nextLine(1);
    REQUIRE(text == "");//space doesnt fit into 1 px
    std::tie(width, text) = c.nextLine(1, true);
    REQUIRE(text == "   ");
    std::tie(width, text) = c.nextLine(1, true);
    REQUIRE(text == "Hello");
    std::tie(width, text) = c.nextLine(1, true);
    REQUIRE(text == "\t    ");
    std::tie(width, text) = c.nextLine(1, true);
    REQUIRE(text == "World,");
    std::tie(width, text) = c.nextLine(1, true);
    REQUIRE(text == " ");
    std::tie(width, text) = c.nextLine(1, true);
    REQUIRE(text == "Pesto!");
    std::tie(width, text) = c.nextLine(1, true);
    REQUIRE(text == " ");
    //here is \n
    std::tie(width, text) = c.nextLine(1, true);
    REQUIRE(text == " ");
    std::tie(width, text) = c.nextLine(1, true);
    REQUIRE(text == "Foo");
    std::tie(width, text) = c.nextLine(1, true);
    REQUIRE(text == " ");
    std::tie(width, text) = c.nextLine(1, true);
    REQUIRE(text == "bar.");
    std::tie(width, text) = c.nextLine(1, true);
    REQUIRE(text == " ");
    std::tie(width, text) = c.nextLine(1, true);
    REQUIRE(text == "");

    c.reset();
    std::tie(width, text) = c.nextLine(1231233);
    REQUIRE(text == "   Hello\t    World, Pesto! ");
    std::tie(width, text) = c.nextLine(1231233);
    REQUIRE(text == " Foo bar. ");

    c.reset();
    std::tie(width, text) = c.nextLine(c.measureWidth("   Hello"));
    REQUIRE(text == "   Hello");
    std::tie(width, text) = c.nextLine(c.measureWidth("\t    W"));
    REQUIRE(text == "");
    std::tie(width, text) = c.nextLine(c.measureWidth("\t    World, "));
    REQUIRE(text == "\t    World, ");
    std::tie(width, text) = c.nextLine(c.measureWidth(" Pesto! "));
    REQUIRE(text == "Pesto! ");
    std::tie(width, text) = c.nextLine(123);
    REQUIRE(text == " Foo bar. ");
}