//
//  test-MSON.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 6/26/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include "snowcrashtest.h"
#include "MSON.h"

using namespace mson;

TEST_CASE("element copy", "[mson]")
{
    StringValue sv("Hello world!");
    REQUIRE(sv.v == "Hello world!");
    
    Element e1;
    e1.value.reset(::new StringValue("Element 1"));
    REQUIRE(ElementValue<StringValue>::get(e1) == "Element 1");
    
    Element e2;
    e2.value.reset(::new StringValue("Element 2"));
    REQUIRE(ElementValue<StringValue>::get(e2) == "Element 2");
    
    Element e3(e2);
    REQUIRE(ElementValue<StringValue>::get(e3) == "Element 2");
    ElementValue<StringValue>::get(e3) = "Element 3";
    REQUIRE(ElementValue<StringValue>::get(e3) == "Element 3");
    REQUIRE(ElementValue<StringValue>::get(e2) == "Element 2");
    
    std::vector<Element> elements;
    elements.push_back(e1);
    elements.push_back(e2);
    
    ArrayValue av(elements);
    REQUIRE(av.v.size() == 2);
}