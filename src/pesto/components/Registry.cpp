//
// Created by marc on 17.07.18.
//

#include "Registry.h"

std::unordered_map<std::string, std::function<Pesto::Registry::factory>> Pesto::Registry::controllers;