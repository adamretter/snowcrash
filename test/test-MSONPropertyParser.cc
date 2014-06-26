//
//  test-MSONPropertyParser.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 6/23/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include "snowcrashtest.h"
#include "MSONPropertyParser.h"

using namespace snowcrash;
using namespace snowcrashtest;
using namespace mson;

const mdp::ByteBuffer CanonicalPropertyFixture = \
"- `id-1`: `42` (number) - Identifier of the resource\n";

TEST_CASE("recognize unescaped property", "[mson][property]")
{
    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    markdownParser.parse(CanonicalPropertyFixture, markdownAST);
    
    REQUIRE(!markdownAST.children().empty());
    REQUIRE(SectionProcessor<mson::Property>::sectionType(markdownAST.children().begin()) == MSONPropertySectionType);
}

TEST_CASE("recognize escaped property", "[mson][property]")
{
    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    markdownParser.parse(CanonicalPropertyFixture, markdownAST);
    
    REQUIRE(!markdownAST.children().empty());
    REQUIRE(SectionProcessor<mson::Property>::sectionType(markdownAST.children().begin()) == MSONPropertySectionType);
}

TEST_CASE("parse canonical property", "[mson][property]")
{
    mson::Property property;
    Report report;
    SectionParserHelper<mson::Property, MSONPropertyParser>::parse(CanonicalPropertyFixture, MSONObjectSectionType, report, property);
    
    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.empty());
    
    REQUIRE(property.name == "id-1");
    REQUIRE(property.templated == false);
    REQUIRE(property.description == "Identifier of the resource");
    REQUIRE(ElementValue<StringValue>::isDefined(property));
    REQUIRE(ElementValue<StringValue>::get(property) == "42");
    REQUIRE(property.type == mson::NumberDataType);
    REQUIRE(property.required == false);
}

TEST_CASE("parse property nested element", "[mson][property]")
{
    mdp::ByteBuffer source = \
    "- tags: home, green (required)\n"\
    "    - (string)\n";
    
    mson::Property property;
    Report report;
    SectionParserHelper<Property, MSONPropertyParser>::parse(source, MSONObjectSectionType, report, property);
    
    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.empty());
    
    REQUIRE(property.name == "tags");
    REQUIRE(property.description.empty());
    REQUIRE(property.type == mson::ArrayDataType);
    REQUIRE(property.required == true);
    
    REQUIRE(ElementValue<ArrayValue>::isDefined(property));
    Elements& values = ElementValue<ArrayValue>::get(property);
    REQUIRE(values.size() == 3);
    
    REQUIRE(values[0].description.empty());
    REQUIRE(values[0].type == mson::UndefinedDataType);
    REQUIRE(ElementValue<StringValue>::isDefined(values[0]));
    REQUIRE(ElementValue<StringValue>::get(values[0]) == "home");
    
    REQUIRE(values[1].description.empty());
    REQUIRE(values[1].type == mson::UndefinedDataType);
    REQUIRE(ElementValue<StringValue>::isDefined(values[1]));
    REQUIRE(ElementValue<StringValue>::get(values[1]) == "green");
    
    REQUIRE(values[2].description.empty());
    REQUIRE(values[2].type == mson::StringDataType);
}

TEST_CASE("parse property of an object type", "[mson][property]")
{
    mdp::ByteBuffer source = \
    "- address\n"\
    "    - street\n"\
    "    - city\n"\
    "    - state\n";
    
    mson::Property property;
    Report report;
    SectionParserHelper<Property, MSONPropertyParser>::parse(source, MSONObjectSectionType, report, property);
    
    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.empty());
    
    REQUIRE(property.name == "address");
    REQUIRE(property.description.empty());
    REQUIRE(property.type == mson::ObjectDataType);
    REQUIRE(property.required == false);
    
    REQUIRE(ElementValue<ObjectValue>::isDefined(property));
    Properties& properties = ElementValue<ObjectValue>::get(property);
    REQUIRE(properties.size() == 3);
    
    REQUIRE(properties[0].name == "street");
    REQUIRE(properties[0].description.empty());
    REQUIRE(properties[0].type == mson::UndefinedDataType);
    REQUIRE(!ElementValue<StringValue>::isDefined(properties[0]));
    
    REQUIRE(properties[1].name == "city");
    REQUIRE(properties[1].description.empty());
    REQUIRE(properties[1].type == mson::UndefinedDataType);
    REQUIRE(!ElementValue<StringValue>::isDefined(properties[1]));
    
    REQUIRE(properties[2].name == "state");
    REQUIRE(properties[2].description.empty());
    REQUIRE(properties[2].type == mson::UndefinedDataType);
    REQUIRE(!ElementValue<StringValue>::isDefined(properties[2]));
}

TEST_CASE("parse property of an array type", "[mson][property]")
{
    mdp::ByteBuffer source = \
    "- address (array)\n"\
    "    - street\n"\
    "    - city\n"\
    "    - state\n";
    
    mson::Property property;
    Report report;
    SectionParserHelper<Property, MSONPropertyParser>::parse(source, MSONObjectSectionType, report, property);
    
    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.empty());
    
    REQUIRE(property.name == "address");
    REQUIRE(property.description.empty());
    REQUIRE(property.type == mson::ArrayDataType);
    REQUIRE(property.required == false);
    
    REQUIRE(ElementValue<ArrayValue>::isDefined(property));
    Elements& elements = ElementValue<ArrayValue>::get(property);
    REQUIRE(elements.size() == 3);
    
    REQUIRE(elements[0].description.empty());
    REQUIRE(elements[0].type == mson::UndefinedDataType);
    REQUIRE(ElementValue<StringValue>::isDefined(elements[0]));
    REQUIRE(ElementValue<StringValue>::get(elements[0]) == "street");
    
    REQUIRE(elements[1].description.empty());
    REQUIRE(elements[1].type == mson::UndefinedDataType);
    REQUIRE(ElementValue<StringValue>::isDefined(elements[1]));
    REQUIRE(ElementValue<StringValue>::get(elements[1]) == "city");
    
    REQUIRE(elements[2].description.empty());
    REQUIRE(elements[2].type == mson::UndefinedDataType);
    REQUIRE(ElementValue<StringValue>::isDefined(elements[2]));
    REQUIRE(ElementValue<StringValue>::get(elements[2]) == "state");
}

TEST_CASE("parse a complex property", "[mson][property]")
{
    //- id - An identifier object
    //    
    //    Lorem ipsum
    //
    //    - 1
    //    - 2
    //    - 3
    //
    //    Dolor sit amet
    //    
    //    - Properties
    //        - url: `http://google.com` (required)
    //        - address
    //            - street
    //            - city
    //            - state
    //            - tags: home, green
    
    mdp::ByteBuffer source = \
    "- id - An identifier object\n"\
    "    \n"\
    "    Lorem ipsum\n"\
    "\n"\
    "    - 1\n"\
    "    - 2\n"\
    "    - 3\n"\
    "\n"\
    "    Dolor sit amet\n"\
    "    \n"\
    "    - Properties\n"\
    "        - url: `http://google.com` (required)\n"\
    "        - address\n"\
    "            - street\n"\
    "            - city\n"\
    "            - state\n"\
    "            - tags: home, green\n"\
    "\n";
    
    mson::Property property;
    Report report;
    SectionParserHelper<Property, MSONPropertyParser>::parse(source, MSONObjectSectionType, report, property);
    
    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.empty());
    
    REQUIRE(property.name == "id");
    REQUIRE(property.description == "An identifier object\n\nLorem ipsum\n\n- 1\n\n- 2\n\n- 3\n\nDolor sit amet\n");
    REQUIRE(property.type == mson::ObjectDataType);
    REQUIRE(property.required == false);
    
    REQUIRE(ElementValue<ObjectValue>::isDefined(property));
    Properties& properties = ElementValue<ObjectValue>::get(property);
    REQUIRE(properties.size() == 2);

    REQUIRE(properties[0].name == "url");
    REQUIRE(properties[0].description.empty());
    REQUIRE(properties[0].type == mson::UndefinedDataType);
    REQUIRE(properties[0].required == true);
    REQUIRE(ElementValue<StringValue>::isDefined(properties[0]));
    REQUIRE(ElementValue<StringValue>::get(properties[0]) == "http://google.com");
    
    REQUIRE(properties[1].name == "address");
    REQUIRE(properties[1].description.empty());
    REQUIRE(properties[1].type == mson::ObjectDataType);
    REQUIRE(properties[1].required == false);
    REQUIRE(ElementValue<ObjectValue>::isDefined(properties[1]));
    
    Properties& addressProperties = ElementValue<ObjectValue>::get(properties[1]);
    REQUIRE(addressProperties.size() == 4);
    
    REQUIRE(addressProperties[0].name == "street");
    REQUIRE(addressProperties[0].description.empty());
    REQUIRE(addressProperties[0].type == mson::UndefinedDataType);
    REQUIRE(!ElementValue<StringValue>::isDefined(addressProperties[0]));
    
    REQUIRE(addressProperties[1].name == "city");
    REQUIRE(addressProperties[1].description.empty());
    REQUIRE(addressProperties[1].type == mson::UndefinedDataType);
    REQUIRE(!ElementValue<StringValue>::isDefined(addressProperties[1]));
    
    REQUIRE(addressProperties[2].name == "state");
    REQUIRE(addressProperties[2].description.empty());
    REQUIRE(addressProperties[2].type == mson::UndefinedDataType);
    REQUIRE(!ElementValue<StringValue>::isDefined(addressProperties[2]));
    
    REQUIRE(addressProperties[3].name == "tags");
    REQUIRE(addressProperties[3].description.empty());
    REQUIRE(addressProperties[3].type == mson::ArrayDataType);
    REQUIRE(ElementValue<ArrayValue>::isDefined(addressProperties[3]));
    
    Elements& elements = ElementValue<ArrayValue>::get(addressProperties[3]);
    REQUIRE(elements.size() == 2);
    
    REQUIRE(elements[0].description.empty());
    REQUIRE(elements[0].type == mson::UndefinedDataType);
    REQUIRE(ElementValue<StringValue>::isDefined(elements[0]));
    REQUIRE(ElementValue<StringValue>::get(elements[0]) == "home");
    
    REQUIRE(elements[1].description.empty());
    REQUIRE(elements[1].type == mson::UndefinedDataType);
    REQUIRE(ElementValue<StringValue>::isDefined(elements[1]));
    REQUIRE(ElementValue<StringValue>::get(elements[1]) == "green");
}

TEST_CASE("parse templated property", "[mson][property]")
{
    mdp::ByteBuffer source = \
    "- {address}\n"\
    "    - street\n"\
    "    - city\n"\
    "    - state\n";
    
    mson::Property property;
    Report report;
    SectionParserHelper<Property, MSONPropertyParser>::parse(source, MSONObjectSectionType, report, property);
    
    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.empty());
    
    REQUIRE(property.name == "address");
    REQUIRE(property.templated == true);
    REQUIRE(property.description.empty());
    REQUIRE(property.type == mson::ObjectDataType);
    REQUIRE(property.required == false);
    
    REQUIRE(ElementValue<ObjectValue>::isDefined(property));
    Properties& properties = ElementValue<ObjectValue>::get(property);
    REQUIRE(properties.size() == 3);
    
    REQUIRE(properties[0].name == "street");
    REQUIRE(properties[0].templated == false);
    REQUIRE(properties[0].description.empty());
    REQUIRE(properties[0].type == mson::UndefinedDataType);
    REQUIRE(!ElementValue<StringValue>::isDefined(properties[0]));
    
    REQUIRE(properties[1].name == "city");
    REQUIRE(properties[1].templated == false);
    REQUIRE(properties[1].description.empty());
    REQUIRE(properties[1].type == mson::UndefinedDataType);
    REQUIRE(!ElementValue<StringValue>::isDefined(properties[1]));
    
    REQUIRE(properties[2].name == "state");
    REQUIRE(properties[2].templated == false);
    REQUIRE(properties[2].description.empty());
    REQUIRE(properties[2].type == mson::UndefinedDataType);
    REQUIRE(!ElementValue<StringValue>::isDefined(properties[2]));
}

