//
// Created by marc on 04.06.18.
//

#include "../xml/tinyxml2.h"

int main(int args, char ** argv) {
    tinyxml2::XMLDocument doc;

    doc.Parse("<root><div id=\"1\"></div><div id=\"2\">Hi</div></root>");

    auto root = doc.FirstChildElement("root");

    for( tinyxml2::XMLElement* ele = root->FirstChildElement();
         ele;
         ele = ele->NextSiblingElement() )
    {

        printf("%s#%s\n", ele->Name(), ele->Attribute("id"));
//        if (ele->ToText()) {
//            printf("Text: %s\n", ele->ToText()->Value());
//        }
    }

    return 0;
}