//
//  test-MSONPropertiesParser.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 7/17/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include "snowcrashtest.h"
#include "MSONPropertiesParser.h"

using namespace snowcrash;
using namespace snowcrashtest;
using namespace mson;

const mdp::ByteBuffer CanonicalPropertiesFixture = \
"- Properties\n"\
"   - a: 1\n"\
"   - b: 2 - this is it!\n"\
"   - c: 3 (number)\n";

TEST_CASE("recognize properties", "[mson][properties]")
{
    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    markdownParser.parse(CanonicalPropertiesFixture, markdownAST);
    
    REQUIRE(!markdownAST.children().empty());
    REQUIRE(SectionProcessor<mson::Properties>::sectionType(markdownAST.children().begin()) == MSONPropertiesSectionType);
}

TEST_CASE("parse canonical properties", "[mson][properties]")
{
    Properties properties;
    Report report;
    SectionParserHelper<Properties, MSONPropertiesParser>::parse(CanonicalPropertiesFixture, MSONObjectSectionType, report, properties);
    
    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.empty());
    
    REQUIRE(properties.size() == 3);
    
    REQUIRE(properties[0].name == "a");
    REQUIRE(ElementValue<StringValue>::isDefined(properties[0]));
    REQUIRE(ElementValue<StringValue>::get(properties[0]) == "1");
    REQUIRE(properties[0].description.empty());
    REQUIRE(properties[0].type == UndefinedDataType);
    REQUIRE(properties[0].required == false);

    REQUIRE(properties[1].name == "b");
    REQUIRE(ElementValue<StringValue>::isDefined(properties[1]));
    REQUIRE(ElementValue<StringValue>::get(properties[1]) == "2");
    REQUIRE(properties[1].description == "this is it!");
    REQUIRE(properties[1].type == UndefinedDataType);
    REQUIRE(properties[1].required == false);
    
    REQUIRE(properties[2].name == "c");
    REQUIRE(ElementValue<StringValue>::isDefined(properties[2]));
    REQUIRE(ElementValue<StringValue>::get(properties[2]) == "3");
    REQUIRE(properties[2].description.empty());
    REQUIRE(properties[2].type == NumberDataType);
    REQUIRE(properties[2].required == false);
}
