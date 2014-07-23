//
//  Signature.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 6/20/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include "Signature.h"
#include "SectionParser.h"
#include "ActionParser.h"
#include "AssetParser.h"
#include "HeadersParser.h"
#include "PayloadParser.h"
#include "ParametersParser.h"
#include "ResourceParser.h"
#include "ResourceGroupParser.h"

using namespace snowcrash;

bool snowcrash::HasSectionKeywordSignature(const mdp::MarkdownNodeIterator& node)
{
    // Note: Every-keyword defined section should be listed here...
    return (SectionProcessor<Headers>::sectionType(node) != UndefinedSectionType ||
            SectionProcessor<Asset>::sectionType(node) != UndefinedSectionType ||
            SectionProcessor<Payload>::sectionType(node) != UndefinedSectionType ||
            SectionProcessor<Values>::sectionType(node) != UndefinedSectionType ||
            SectionProcessor<Parameters>::sectionType(node) != UndefinedSectionType ||
            SectionProcessor<Action>::sectionType(node) != UndefinedSectionType ||
            SectionProcessor<Resource>::sectionType(node) != UndefinedSectionType ||
            SectionProcessor<ResourceGroup>::sectionType(node) != UndefinedSectionType);
}

SectionType snowcrash::RecognizeCodeBlockFirstLine(const mdp::ByteBuffer& subject)
{
    SectionType type = UndefinedSectionType;

    if (RegexMatch(subject, HeadersRegex)) {
        return HeadersSectionType;
    }
    else if (RegexMatch(subject, BodyRegex)) {
        return BodySectionType;
    }
    else if (RegexMatch(subject, SchemaRegex)) {
        return SchemaSectionType;
    }

    return type;
}