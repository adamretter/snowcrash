//
//  test-MSONElementParser.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 7/5/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include "snowcrashtest.h"
#include "MSONElementParser.h"

using namespace snowcrash;
using namespace snowcrashtest;
using namespace mson;

const mdp::ByteBuffer CanonicalElementFixture = \
"- `Hello World` (string) - A message for the world out there\n";

const mdp::ByteBuffer CanonicalArrayElementFixture = \
"- `home`, `green` (array)";

TEST_CASE("recognize element", "[mson][element]")
{
    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    markdownParser.parse(CanonicalElementFixture, markdownAST);
    
    REQUIRE(!markdownAST.children().empty());
    REQUIRE(SectionProcessor<mson::Element>::sectionType(markdownAST.children().begin()) == MSONElementSectionType);
}

TEST_CASE("parse canonical element", "[mson][element]")
{
    mson::Element element;
    Report report;
    SectionParserHelper<Element, MSONElementParser>::parse(CanonicalElementFixture, MSONObjectSectionType, report, element);
    
    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.empty());

    REQUIRE(ElementValue<StringValue>::isDefined(element));
    REQUIRE(ElementValue<StringValue>::get(element) == "Hello World");
    REQUIRE(element.description == "A message for the world out there");
    REQUIRE(element.type == mson::StringDataType);
}

TEST_CASE("parse canonical array element", "[mson][element]")
{
    mson::Element element;
    Report report;
    SectionParserHelper<Element, MSONElementParser>::parse(CanonicalArrayElementFixture, MSONObjectSectionType, report, element);
    
    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.empty());
    
    REQUIRE(ElementValue<ArrayValue>::isDefined(element));
    Elements& values = ElementValue<ArrayValue>::get(element);
    REQUIRE(values.size() == 2);
    REQUIRE(ElementValue<StringValue>::isDefined(values[0]));
    REQUIRE(ElementValue<StringValue>::get(values[0]) == "home");
    REQUIRE(ElementValue<StringValue>::isDefined(values[1]));
    REQUIRE(ElementValue<StringValue>::get(values[1]) == "green");
    
    REQUIRE(element.description.empty());
    REQUIRE(element.type == mson::ArrayDataType);
}

TEST_CASE("parse unescaped element with the required specifier", "[mson][element]")
{
    mson::Element element;
    Report report;
    SectionParserHelper<Element, MSONElementParser>::parse("- 42 (number)", MSONObjectSectionType, report, element);
    
    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.empty());
    
    REQUIRE(ElementValue<StringValue>::isDefined(element));
    REQUIRE(ElementValue<StringValue>::get(element) == "42");
    REQUIRE(element.type == mson::NumberDataType);
}

TEST_CASE("parse canonical element with additional description", "[mson][element]")
{
    mdp::ByteBuffer source = CanonicalElementFixture;
    source += \
    "  line 1\n"\
    "  line 2\n\n"\
    "  paragraph\n";

    mson::Element element;
    Report report;
    SectionParserHelper<Element, MSONElementParser>::parse(source, MSONObjectSectionType, report, element);
    
    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.empty());
    
    REQUIRE(ElementValue<StringValue>::isDefined(element));
    REQUIRE(ElementValue<StringValue>::get(element) == "Hello World");
    REQUIRE(element.description == "A message for the world out there\nline 1\nline 2\n\nparagraph\n");
    REQUIRE(element.type == mson::StringDataType);
}

TEST_CASE("parse minimal array element", "[mson][element]")
{
    mson::Element element;
    Report report;
    SectionParserHelper<Element, MSONElementParser>::parse("- 1, 2, 3", MSONObjectSectionType, report, element);
    
    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.empty());
    
    REQUIRE(ElementValue<ArrayValue>::isDefined(element));
    Elements& values = ElementValue<ArrayValue>::get(element);
    REQUIRE(values.size() == 3);
    REQUIRE(ElementValue<StringValue>::isDefined(values[0]));
    REQUIRE(ElementValue<StringValue>::get(values[0]) == "1");
    REQUIRE(ElementValue<StringValue>::isDefined(values[1]));
    REQUIRE(ElementValue<StringValue>::get(values[1]) == "2");
    REQUIRE(ElementValue<StringValue>::isDefined(values[2]));
    REQUIRE(ElementValue<StringValue>::get(values[2]) == "3");
    
    REQUIRE(element.description.empty());
    REQUIRE(element.type == mson::ArrayDataType);
}

TEST_CASE("parse nested element", "[mson][element]")
{
    mdp::ByteBuffer source = \
    "- home, green (array)\n"\
    "    - (string)\n";
    
    mson::Element element;
    Report report;
    SectionParserHelper<Element, MSONElementParser>::parse(source, MSONObjectSectionType, report, element);
    
    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.empty());

    REQUIRE(element.description.empty());
    REQUIRE(element.type == mson::ArrayDataType);
    
    REQUIRE(ElementValue<ArrayValue>::isDefined(element));
    Elements& values = ElementValue<ArrayValue>::get(element);
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
    REQUIRE(!ElementValue<StringValue>::isDefined(values[2]));
}

TEST_CASE("parser object element", "[mson][element]")
{
    mdp::ByteBuffer source = \
    "- (object)\n"\
    "    - street\n"\
    "    - city\n"\
    "    - state\n";
    
    mson::Element element;
    Report report;
    SectionParserHelper<Element, MSONElementParser>::parse(source, MSONObjectSectionType, report, element);
    
    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.empty());
    
    REQUIRE(element.description.empty());
    REQUIRE(element.type == mson::ObjectDataType);
    
    REQUIRE(ElementValue<ObjectValue>::isDefined(element));
    Properties& properties = ElementValue<ObjectValue>::get(element);
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

TEST_CASE("parse element of array type with values and interrfered types", "[mson][element]")
{
    mdp::ByteBuffer source = \
    "- home, green (array: string)\n";
    
    mson::Element element;
    Report report;
    SectionParserHelper<Element, MSONElementParser>::parse(source, MSONObjectSectionType, report, element);
    
    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.empty());
    
    REQUIRE(element.description.empty());
    REQUIRE(element.type == mson::ArrayDataType);
    
    REQUIRE(ElementValue<ArrayValue>::isDefined(element));
    Elements& values = ElementValue<ArrayValue>::get(element);
    REQUIRE(values.size() == 2);
    
    REQUIRE(values[0].description.empty());
    REQUIRE(values[0].type == mson::StringDataType);
    REQUIRE(ElementValue<StringValue>::isDefined(values[0]));
    REQUIRE(ElementValue<StringValue>::get(values[0]) == "home");
    
    REQUIRE(values[1].description.empty());
    REQUIRE(values[1].type == mson::StringDataType);
    REQUIRE(ElementValue<StringValue>::isDefined(values[1]));
    REQUIRE(ElementValue<StringValue>::get(values[1]) == "green");
}

TEST_CASE("parse element of array type with interrfered types", "[mson][element]")
{
    mdp::ByteBuffer source = \
    "- (array: number)\n";
    
    mson::Element element;
    Report report;
    SectionParserHelper<Element, MSONElementParser>::parse(source, MSONObjectSectionType, report, element);
    
    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.empty());
    
    REQUIRE(element.description.empty());
    REQUIRE(element.type == mson::ArrayDataType);
    
    REQUIRE(ElementValue<ArrayValue>::isDefined(element));
    Elements& values = ElementValue<ArrayValue>::get(element);
    REQUIRE(values.size() == 1);
    
    REQUIRE(values[0].description.empty());
    REQUIRE(values[0].type == mson::NumberDataType);
    REQUIRE(!ElementValue<StringValue>::isDefined(values[0]));
}
