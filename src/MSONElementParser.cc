//
//  MSONElementParser.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 7/17/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include "MSONElementsParser.h"
#include "MSONPropertiesParser.h"

using namespace mson;
using namespace snowcrash;

MarkdownNodeIterator SectionProcessor<mson::Element>::processNestedSection(const MarkdownNodeIterator& node,
                                                                           const MarkdownNodes& siblings,
                                                                           SectionParserData& pd,
                                                                           Report& report,
                                                                           mson::Element& out)
{
    MarkdownNodeIterator cur = node;
    switch (pd.sectionContext()) {
        case MSONElementSectionType: {
            Element e;
            cur = MSONElementParser::parse(node, siblings, pd, report, e);
            if (ElementValue<ArrayValue>::isDefined(out)) {
                Elements& elements = ElementValue<ArrayValue>::get(out);
                elements.push_back(e);
            }
            else {
                Elements elements;
                elements.push_back(e);
                ElementValue<ArrayValue>::set(elements, out);
            }
        }
            break;
            
        case MSONElementsSectionType: {
            Elements elements;
            cur = MSONElementsParser::parse(node, siblings, pd, report, elements);
            ElementValue<ArrayValue>::set(elements, out);
        }
            break;
            
        case MSONPropertySectionType: {
            Property p;
            cur = MSONPropertyParser::parse(node, siblings, pd, report, p);
            if (ElementValue<ObjectValue>::isDefined(out)) {
                Properties& properties = ElementValue<ObjectValue>::get(out);
                properties.push_back(p);
            }
            else {
                Properties properties;
                properties.push_back(p);
                ElementValue<ObjectValue>::set(properties, out);
            }
        }
            break;
            
        case MSONPropertiesSectionType: {
            Properties properties;
            cur = MSONPropertiesParser::parse(node, siblings, pd, report, properties);
            ElementValue<ObjectValue>::set(properties, out);
        }
            
            break;
            
        default:
            break;
    };
    
    if ((pd.sectionContext() == MSONPropertySectionType || pd.sectionContext() == MSONPropertiesSectionType) &&
        out.type != ObjectDataType) {
        
        if (out.type != UndefinedDataType) {
            // WARN: Mismatched data types
            std::stringstream ss;
            ss << "unexpected property for parent of '" << out.type << "', assuming 'object' instead";
            mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
            report.warnings.push_back(snowcrash::Warning(ss.str(),
                                                         snowcrash::SignatureSyntaxWarning,
                                                         sourceMap));
        }
        
        // Implicit ObjectDataType
        out.type = ObjectDataType;
    }

    return cur;

}

SectionType SectionProcessor<mson::Element>::nestedSectionType(const MarkdownNodeIterator& node,
                                                               const MarkdownNodes& siblings,
                                                               const mson::Element& context) {
    
    // 1. expect property by default
    // 2. if type is array expect element
    // 3. if node is not first of the siblings after signature it is a description unless 4.
    // 4. 'Elements' / 'Properites' -> is always a nested section
    
    SectionType section = UndefinedSectionType;
    bool expectProperty = (context.type != mson::ArrayDataType);    // True if properties are expected, false of elements

    // Check second sibling
    //          If it is an MSON property or element => no description
    //
    // TODO: Remove following line, introduce MarkdownNodeConstIterator
    //          to be done after the snowcrash architecture refactor
    MarkdownNodes& collection = const_cast<MarkdownNodes&>(siblings);
    MarkdownNodeIterator cur = collection.begin();
    std::advance(cur, 1);
    if (cur == collection.end())
        return UndefinedSectionType;
    if (expectProperty) {
        section = SectionProcessor<mson::Property>::sectionType(cur);
    }
    else {
        section = SectionProcessor<mson::Element>::sectionType(cur);
    }

    // Look for a direct property or element
    bool hasDescriptionNodes = (section == UndefinedSectionType);  // True the section has any description node, false otherwise
    if (!hasDescriptionNodes) {
        if (expectProperty) {
            section = SectionProcessor<mson::Property>::sectionType(node);
        }
        else {
            section = SectionProcessor<mson::Element>::sectionType(node);
        }
        if (section != UndefinedSectionType)
            return section;
    }

    // Look for nested properties or elements section
    if ((section = SectionProcessor<mson::Elements>::sectionType(node))
        != UndefinedSectionType) {
        return section;
    }

    if ((section = SectionProcessor<mson::Properties>::sectionType(node))
        != UndefinedSectionType) {
        return section;
    }

    return UndefinedSectionType;
}

