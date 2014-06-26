//
//  test-Signature.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 7/5/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include "snowcrashtest.h"
#include "Signature.h"

static const mdp::ByteBuffer PropertySignatureFixture = "id: 42 (yes, no) - a good message";
static const mdp::ByteBuffer EscapedPropertySignatureFixture = "`*id*(data):3`: `42` (yes, no) - a good message";

static const mdp::ByteBuffer ElementSignatureFixture = "42 (number) - a good number";
static const mdp::ByteBuffer EscapedElementSignatureFixture = "`*42*(data):3` (number) - a good number";

using namespace snowcrash;
using namespace scpl;

struct SignatureParserHelper {
    
    static void parse(const mdp::ByteBuffer& signature,
                      const SignatureTraits& traits,
                      Report& report,
                      Signature& s) {
        
        mdp::MarkdownNode n;
        n.text = signature;
        n.sourceMap.push_back(mdp::Range(0, n.text.size()));
        mdp::MarkdownNodes v;
        v.push_back(n);
        
        Blueprint bp;
        SectionParserData pd(0, n.text, bp);
        
        SignatureParser::parseSignature(v.begin(), traits, pd, report, s);
    }
};

struct ElementTraits : public SignatureTraits {

    ElementTraits() : SignatureTraits(Value | Array | Specifier | Content) {}
    
    static ElementTraits& get() {
        static ElementTraits s;
        return s;
    }
};

TEST_CASE("signature", "[signature]")
{
    Report r;
    Signature s;
    
    SignatureParserHelper::parse(PropertySignatureFixture,
                                 SignatureTraits::get(), r, s);
    
    REQUIRE(r.error.code == Error::OK);
    REQUIRE(r.warnings.empty());
    REQUIRE(s.additionalContent.empty());
    
    REQUIRE(s.identifier == "id");
    REQUIRE(s.values.front() == "42");
    REQUIRE(s.specifiers.size() == 2);
    REQUIRE(s.specifiers[0] == "yes");
    REQUIRE(s.specifiers[1] == "no");
    REQUIRE(s.content == "a good message");
}

TEST_CASE("escaped signature", "[signature]")
{
    Report r;
    Signature s;
    
    SignatureParserHelper::parse(EscapedPropertySignatureFixture,
                                 SignatureTraits::get(), r, s);
    
    REQUIRE(r.error.code == Error::OK);
    REQUIRE(r.warnings.empty());
    REQUIRE(s.additionalContent.empty());
    
    REQUIRE(s.identifier == "*id*(data):3");
    REQUIRE(s.values.front() == "42");
    REQUIRE(s.specifiers.size() == 2);
    REQUIRE(s.specifiers[0] == "yes");
    REQUIRE(s.specifiers[1] == "no");
    REQUIRE(s.content == "a good message");
}

TEST_CASE("mutliline signature", "[signature]")
{
    Report r;
    Signature s;
    
    SignatureParserHelper::parse("id\nLine 2\nLine 3\n",
                                 SignatureTraits::get(), r, s);
    
    REQUIRE(r.error.code == Error::OK);
    REQUIRE(r.warnings.empty());
    REQUIRE(s.additionalContent == "Line 2\nLine 3\n");
    
    REQUIRE(s.identifier == "id");
    REQUIRE(s.values.empty());
    REQUIRE(s.specifiers.empty());
    REQUIRE(s.content.empty());
}

TEST_CASE("identifier only signature", "[signature]")
{
    Report r;
    Signature s;
    
    SignatureParserHelper::parse("id",
                                 SignatureTraits::get(), r, s);
    
    REQUIRE(r.error.code == Error::OK);
    REQUIRE(r.warnings.empty());
    REQUIRE(s.additionalContent.empty());
    
    REQUIRE(s.identifier == "id");
    REQUIRE(s.values.empty());
    REQUIRE(s.specifiers.empty());
    REQUIRE(s.content.empty());
}

TEST_CASE("identifier description signature", "[signature]")
{
    Report r;
    Signature s;
    
    SignatureParserHelper::parse("id - a good - info",
                                 SignatureTraits::get(), r, s);

    REQUIRE(r.error.code == Error::OK);
    REQUIRE(r.warnings.empty());
    REQUIRE(s.additionalContent.empty());
    
    REQUIRE(s.identifier == "id");
    REQUIRE(s.values.empty());
    REQUIRE(s.specifiers.empty());
    REQUIRE(s.content == "a good - info");
}

TEST_CASE("identifier value signature", "[signature]")
{
    Report r;
    Signature s;
    
    SignatureParserHelper::parse("id : a good data",
                                 SignatureTraits::get(), r, s);
    
    REQUIRE(r.error.code == Error::OK);
    REQUIRE(r.warnings.empty());
    REQUIRE(s.additionalContent.empty());
    
    REQUIRE(s.identifier == "id");
    REQUIRE(s.values.front() == "a good data");
    REQUIRE(s.specifiers.empty());
    REQUIRE(s.content.empty());
}

TEST_CASE("identifier traits signature", "[signature]")
{
    Report r;
    Signature s;
    
    SignatureParserHelper::parse("id (number)",
                                 SignatureTraits::get(), r, s);
    
    REQUIRE(r.error.code == Error::OK);
    REQUIRE(r.warnings.empty());
    REQUIRE(s.additionalContent.empty());
    
    REQUIRE(s.identifier == "id");
    REQUIRE(s.values.empty());
    REQUIRE(s.specifiers.size() == 1);
    REQUIRE(s.specifiers[0] == "number");
    REQUIRE(s.content.empty());
}

TEST_CASE("element signature", "[signature]")
{
    Report r;
    Signature s;
    
    SignatureParserHelper::parse(ElementSignatureFixture,
                                 ElementTraits::get(), r, s);
    
    REQUIRE(r.error.code == Error::OK);
    REQUIRE(r.warnings.empty());
    REQUIRE(s.additionalContent.empty());
    
    REQUIRE(s.identifier.empty());
    REQUIRE(s.values.front() == "42");
    REQUIRE(s.specifiers.size() == 1);
    REQUIRE(s.specifiers[0] == "number");
    REQUIRE(s.content == "a good number");
}

TEST_CASE("escaped element signature", "[signature]")
{
    Report r;
    Signature s;
    
    SignatureParserHelper::parse(EscapedElementSignatureFixture,
                                 ElementTraits::get(), r, s);
    
    REQUIRE(r.error.code == Error::OK);
    REQUIRE(r.warnings.empty());
    REQUIRE(s.additionalContent.empty());
    
    REQUIRE(s.identifier.empty());
    REQUIRE(s.values.front() == "*42*(data):3");
    REQUIRE(s.specifiers.size() == 1);
    REQUIRE(s.specifiers[0] == "number");
    REQUIRE(s.content == "a good number");
}

TEST_CASE("element value only signature", "[signature]")
{
    Report r;
    Signature s;
    
    SignatureParserHelper::parse("42",
                                 ElementTraits::get(), r, s);
    
    REQUIRE(r.error.code == Error::OK);
    REQUIRE(r.warnings.empty());
    REQUIRE(s.additionalContent.empty());
    
    REQUIRE(s.identifier.empty());
    REQUIRE(s.values.front() == "42");
    REQUIRE(s.specifiers.empty());
    REQUIRE(s.content.empty());
}

TEST_CASE("element traits only signature", "[signature]")
{
    Report r;
    Signature s;
    
    SignatureParserHelper::parse("(number)",
                                 ElementTraits::get(), r, s);
    
    REQUIRE(r.error.code == Error::OK);
    REQUIRE(r.warnings.empty());
    REQUIRE(s.additionalContent.empty());
    
    REQUIRE(s.identifier.empty());
    REQUIRE(s.values.empty());
    REQUIRE(s.specifiers.size() == 1);
    REQUIRE(s.specifiers[0] == "number");
    REQUIRE(s.content.empty());
}

TEST_CASE("element traits description signature", "[signature]")
{
    Report r;
    Signature s;
    
    SignatureParserHelper::parse("(number) - something () cool",
                                 ElementTraits::get(), r, s);
    
    REQUIRE(r.error.code == Error::OK);
    REQUIRE(r.warnings.empty());
    REQUIRE(s.additionalContent.empty());
    
    REQUIRE(s.identifier.empty());
    REQUIRE(s.values.empty());
    REQUIRE(s.specifiers.size() == 1);
    REQUIRE(s.specifiers[0] == "number");
    REQUIRE(s.content == "something () cool");
}

TEST_CASE("identifier with element traits", "[signature]")
{
    Report r;
    Signature s;
    
    SignatureParserHelper::parse(PropertySignatureFixture,
                                 ElementTraits::get(), r, s);
    
    REQUIRE(r.error.code == Error::OK);
    REQUIRE(r.warnings.size() == 1);
    REQUIRE(s.additionalContent.empty());
    
    REQUIRE(s.identifier.empty());
    REQUIRE(s.values.front() == "id");
    REQUIRE(s.specifiers.empty());
    REQUIRE(s.content.empty());
}

TEST_CASE("property signature without identifier", "[signature]")
{
    Report r;
    Signature s;
    
    SignatureParserHelper::parse("(x)",
                                 SignatureTraits::get(), r, s);
    
    REQUIRE(r.error.code == Error::OK);
    REQUIRE(r.warnings.size() == 1);
    REQUIRE(s.additionalContent.empty());
    
    REQUIRE(s.identifier.empty());
    REQUIRE(s.values.empty());
    REQUIRE(s.specifiers.size() == 1);
    REQUIRE(s.specifiers[0] == "x");
    REQUIRE(s.content.empty());
}

TEST_CASE("property signature colon without a value", "[signature]")
{
    Report r;
    Signature s;
    
    SignatureParserHelper::parse("id: (number)",
                                 SignatureTraits::get(), r, s);
    
    REQUIRE(r.error.code == Error::OK);
    REQUIRE(r.warnings.size() == 1);
    REQUIRE(s.additionalContent.empty());
    
    REQUIRE(s.identifier == "id");
    REQUIRE(s.values.empty());
    REQUIRE(s.specifiers.size() == 1);
    REQUIRE(s.specifiers[0] == "number");
    REQUIRE(s.content.empty());
}

TEST_CASE("element signature value and traits", "[signature]")
{
    Report r;
    Signature s;
    
    SignatureParserHelper::parse("- content is the king",
                                 ElementTraits::get(), r, s);
    
    REQUIRE(r.error.code == Error::OK);
    REQUIRE(r.warnings.size() == 1);
    REQUIRE(s.additionalContent.empty());
    
    REQUIRE(s.identifier.empty());
    REQUIRE(s.values.empty());
    REQUIRE(s.specifiers.empty());
    REQUIRE(s.content == "content is the king");
}

TEST_CASE("escaped array element", "[signature]")
{
    Report r;
    Signature s;

    SignatureParserHelper::parse( "`1 `, `2, 3`",
                                 ElementTraits::get(), r, s);
    
    REQUIRE(r.error.code == Error::OK);
    REQUIRE(r.warnings.empty());
    REQUIRE(s.additionalContent.empty());
    
    REQUIRE(s.identifier.empty());
    REQUIRE(s.values.size() == 2);
    REQUIRE(s.values[0] == "1 ");
    REQUIRE(s.values[1] == "2, 3");
    REQUIRE(s.specifiers.empty());
    REQUIRE(s.content.empty());
}

TEST_CASE("unescaped array element", "[signature]")
{
    Report r;
    Signature s;
    
    SignatureParserHelper::parse( "1 , 2, 3 (optional, array) - numbers",
                                 ElementTraits::get(), r, s);
    
    REQUIRE(r.error.code == Error::OK);
    REQUIRE(r.warnings.empty());
    REQUIRE(s.additionalContent.empty());
    
    REQUIRE(s.identifier.empty());
    REQUIRE(s.values.size() == 3);
    REQUIRE(s.values[0] == "1");
    REQUIRE(s.values[1] == "2");
    REQUIRE(s.values[2] == "3");
    REQUIRE(s.specifiers.size() == 2);
    REQUIRE(s.specifiers[0] == "optional");
    REQUIRE(s.specifiers[1] == "array");
    REQUIRE(s.content == "numbers");
}
