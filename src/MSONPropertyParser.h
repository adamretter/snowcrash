//
//  MSONPropertyParser.
//  snowcrash
//
//  Created by Zdenek Nemec on 6/23/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_MSONPROPERTYPARSER_H
#define SNOWCRASH_MSONPROPERTYPARSER_H

#include "SectionParser.h"
#include "MSON.h"
#include "MSONElementParser.h"

namespace mson {
    
    using namespace scpl;
    
    /** \brief Property Signature helper */
    struct PropertySignature : ElementSignature {
        
        static void setSignature(const mdp::MarkdownNodeIterator& node,
                                 const scpl::Signature& signature,
                                 const snowcrash::SectionParserData& pd,
                                 snowcrash::Report& report,
                                 Property& p) {

            // Work copy of the signature being processed
            scpl::Signature workingSignature = signature;

            // Property name
            p.name = workingSignature.identifier;
            
            // Templated?
            if (!p.name.empty() &&
                p.name[0] == '{' &&
                *p.name.rbegin() == '}') {
                
                p.templated = true;
                p.name.erase(p.name.begin());
                p.name.erase(p.name.end() - 1);
            }
            
            // Requirement signature
            if (!workingSignature.specifiers.empty()) {

                // Process & extract requirement specifiers
                size_t requirementSpecifiers = 0;
                
                scpl::Signature::Specifiers::iterator it = workingSignature.specifiers.begin();
                while (it != workingSignature.specifiers.end()) {
                    
                    if (*it == "optional") {
                        p.required = false;
                        it = workingSignature.specifiers.erase(it);
                        ++requirementSpecifiers;
                    }
                    else if (*it == "required") {
                        p.required = true;
                        it = workingSignature.specifiers.erase(it);
                        ++requirementSpecifiers;
                    }
                    else {
                        ++it;
                    }
                    
                    if (requirementSpecifiers > 1) {
                        requirementSpecifiers = 1;
                        
                        // WARN: Too many requirements traits
                        std::stringstream ss;
                        ss << "too many requirement specifiers, expected 'optional' or 'required'";
                        mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                        report.warnings.push_back(snowcrash::Warning(ss.str(),
                                                                     snowcrash::SignatureSyntaxWarning,
                                                                     sourceMap));
                    }
                }
            }

            // The rest is processed by element signature helper
            ElementSignature::setSignature(node, workingSignature, pd, report, p);
        }
    };
}


// NOTE: Pending mson namespace
namespace snowcrash {
    
    /**
     *  MSON Property Section Processor
     */
    template<>
    struct SectionProcessor<mson::Property> : public SectionProcessorBase<mson::Property> {
        
        /** MSON Property Signature Traits */
        struct PropertyTraits : public scpl::SignatureTraits {
            PropertyTraits() : SignatureTraits(Identifier | Value | Array | Specifier | Content) {}
            
            static PropertyTraits& get() {
                static PropertyTraits s;
                return s;
            }
        };
        
        static MarkdownNodeIterator processSignature(const MarkdownNodeIterator& node,
                                                     SectionParserData& pd,
                                                     bool& parsingRedirect,
                                                     Report& report,
                                                     mson::Property& property) {
            
            scpl::Signature s;
            scpl::SignatureParser::parseSignature(node, PropertyTraits::get(), pd, report, s);
            mson::PropertySignature::setSignature(node, s, pd, report, property);
            
            return ++MarkdownNodeIterator(node);
        }
        
        static MarkdownNodeIterator processNestedSection(const MarkdownNodeIterator& node,
                                                         const MarkdownNodes& siblings,
                                                         SectionParserData& pd,
                                                         Report& report,
                                                         mson::Property& out)
        {
            return SectionProcessor<mson::Element>::processNestedSection(node, siblings, pd, report, out);
        }

        
        static SectionType sectionType(const MarkdownNodeIterator& node) {
            if ((node->type != mdp::ListItemMarkdownNodeType ||
                 node->children().empty()))
                return UndefinedSectionType;
            
            return MSONPropertySectionType;
        }
        
        static SectionType nestedSectionType(const MarkdownNodeIterator& node,
                                             const MarkdownNodes& siblings,
                                             const mson::Property& context)  {
            
            return SectionProcessor<mson::Element>::nestedSectionType(node, siblings, context);
        }
    };
    
    /** MSON Property Section Parser */
    typedef SectionParser<mson::Property, ListSectionAdapter> MSONPropertyParser;
}

#endif
