//
//  test-MSONElementsParser.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 7/17/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include "snowcrashtest.h"
#include "MSONElementsParser.h"

using namespace snowcrash;
using namespace snowcrashtest;
using namespace mson;

const mdp::ByteBuffer CanonicalElementsFixture = \
"- Elements\n"\
"   - 1\n"\
"   - 2 - this is it!\n"\
"   - 3 (number)\n";

TEST_CASE("recognize elements", "[mson][elements]")
{
    mdp::MarkdownParser markdownParser;
    mdp::MarkdownNode markdownAST;
    markdownParser.parse(CanonicalElementsFixture, markdownAST);
    
    REQUIRE(!markdownAST.children().empty());
    REQUIRE(SectionProcessor<mson::Elements>::sectionType(markdownAST.children().begin()) == MSONElementsSectionType);
}

TEST_CASE("parse canonical elements", "[mson][elements]")
{
    mson::Elements elements;
    Report report;
    SectionParserHelper<Elements, MSONElementsParser>::parse(CanonicalElementsFixture, MSONObjectSectionType, report, elements);
    
    REQUIRE(report.error.code == Error::OK);
    REQUIRE(report.warnings.empty());
    
    REQUIRE(elements.size() == 3);

    REQUIRE(ElementValue<StringValue>::isDefined(elements[0]));
    REQUIRE(ElementValue<StringValue>::get(elements[0]) == "1");
    REQUIRE(elements[0].description.empty());
    REQUIRE(elements[0].type == UndefinedDataType);

    REQUIRE(ElementValue<StringValue>::isDefined(elements[1]));
    REQUIRE(ElementValue<StringValue>::get(elements[1]) == "2");
    REQUIRE(elements[1].description == "this is it!");
    REQUIRE(elements[1].type == UndefinedDataType);
    
    REQUIRE(ElementValue<StringValue>::isDefined(elements[2]));
    REQUIRE(ElementValue<StringValue>::get(elements[2]) == "3");
    REQUIRE(elements[2].description.empty());
    REQUIRE(elements[2].type == NumberDataType);
}
