//
//  MSONElementsParser.h
//  snowcrash
//
//  Created by Zdenek Nemec on 7/16/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_MSONELEMENTSPARSER_H
#define SNOWCRASH_MSONELEMENTSPARSER_H

#include "SectionParser.h"
#include "Signature.h"
#include "MSON.h"
#include "MSONElementParser.h"
#include "MSONPropertyParser.h"

namespace snowcrash {
    
    /**
     *  MSON Elements Section Processor
     */
    template<>
    struct SectionProcessor<mson::Elements> : public SectionProcessorBase<mson::Elements> {
        
        static MarkdownNodeIterator processDescription(const MarkdownNodeIterator& node,
                                                       SectionParserData& pd,
                                                       Report& report,
                                                       mson::Elements& elements) {
            
            return ++MarkdownNodeIterator(node);
        }
        
        static MarkdownNodeIterator processNestedSection(const MarkdownNodeIterator& node,
                                                         const MarkdownNodes& siblings,
                                                         SectionParserData& pd,
                                                         Report& report,
                                                         mson::Elements& elements) {
            
            if (pd.sectionContext() != MSONElementSectionType)
                return node;
            
            mson::Element e;
            MarkdownNodeIterator cur = MSONElementParser::parse(node, siblings, pd, report, e);
            elements.push_back(e);
            return cur;
        }
        
        static bool isDescriptionNode(const MarkdownNodeIterator& node,
                                      const MarkdownNodes& siblings,
                                      const mson::Elements& context,
                                      SectionType sectionType) {
            return false;
        }
        
        static SectionType sectionType(const MarkdownNodeIterator& node) {
            if ((node->type != mdp::ListItemMarkdownNodeType ||
                 node->children().empty()))
                return UndefinedSectionType;
            
            mdp::ByteBuffer subject = node->children().front().text;
            TrimString(subject);
            
            static const char* const ElementsRegex = "^[Ee]lements?$";
            
            if (RegexMatch(subject, ElementsRegex))
                return MSONElementsSectionType;
            
            return UndefinedSectionType;
        }
        
        static SectionType nestedSectionType(const MarkdownNodeIterator& node,
                                             const MarkdownNodes& siblings,
                                             const mson::Elements& context) {
            
            return SectionProcessor<mson::Element>::sectionType(node);
        }
    };
    
    /** MSON Elements Section Parser */
    typedef SectionParser<mson::Elements, ListSectionAdapter> MSONElementsParser;
}

#endif