//
//  MSONPropertiesParser.h
//  snowcrash
//
//  Created by Zdenek Nemec on 7/17/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_MSONPROPERTIESPARSHER_H
#define SNOWCRASH_MSONPROPERTIESPARSHER_H

#include "SectionParser.h"
#include "Signature.h"
#include "MSON.h"
#include "MSONElementParser.h"
#include "MSONPropertyParser.h"

namespace snowcrash {
    
    /**
     *  MSON Properties Section Processor
     */
    template<>
    struct SectionProcessor<mson::Properties> : public SectionProcessorBase<mson::Properties> {
        
        static MarkdownNodeIterator processDescription(const MarkdownNodeIterator& node,
                                                       SectionParserData& pd,
                                                       Report& report,
                                                       mson::Properties& properties) {
            
            return ++MarkdownNodeIterator(node);
        }
        
        static MarkdownNodeIterator processNestedSection(const MarkdownNodeIterator& node,
                                                         const MarkdownNodes& siblings,
                                                         SectionParserData& pd,
                                                         Report& report,
                                                         mson::Properties& properties) {
            
            if (pd.sectionContext() != MSONPropertySectionType)
                return node;
            
            mson::Property p;
            MarkdownNodeIterator cur = MSONPropertyParser::parse(node, siblings, pd, report, p);
            properties.push_back(p);
            return cur;
        }
        
        static bool isDescriptionNode(const MarkdownNodeIterator& node,
                                      const MarkdownNodes& siblings,
                                      const mson::Properties& context,
                                      SectionType sectionType) {
            return false;
        }
        
        static SectionType sectionType(const MarkdownNodeIterator& node) {
            if ((node->type != mdp::ListItemMarkdownNodeType ||
                 node->children().empty()))
                return UndefinedSectionType;
            
            mdp::ByteBuffer subject = node->children().front().text;
            TrimString(subject);
            
            static const char* const PropertiesRegex = "^[Pp]roperties?$";
            
            if (RegexMatch(subject, PropertiesRegex))
                return MSONPropertiesSectionType;
            
            return UndefinedSectionType;
        }
        
        static SectionType nestedSectionType(const MarkdownNodeIterator& node,
                                             const MarkdownNodes& siblings,
                                             const mson::Properties& context) {
            
            return SectionProcessor<mson::Property>::sectionType(node);
        }
    };
    
    /** MSON Properties Section Parser */
    typedef SectionParser<mson::Properties, ListSectionAdapter> MSONPropertiesParser;
}

#endif
