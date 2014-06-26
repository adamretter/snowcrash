//
//  MSONElementParser.h
//  snowcrash
//
//  Created by Zdenek Nemec on 7/5/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_MSONELEMENTPARSER_H
#define SNOWCRASH_MSONELEMENTPARSER_H

#include "SectionParser.h"
#include "Signature.h"
#include "MSON.h"

namespace mson {
    
    using namespace scpl;
    
    /** \brief Element Signature helper */
    struct ElementSignature {
        
        /** \brief Applies signature to an element */
        static void setSignature(const mdp::MarkdownNodeIterator& node,
                                 const scpl::Signature& signature,
                                 const snowcrash::SectionParserData& pd,
                                 snowcrash::Report& report,
                                 Element& e)
        {
            DataType inferredElementType = UndefinedDataType;
            
            // Process specifiers
            if (!signature.specifiers.empty()) {
                setSpecifiers(node, signature.specifiers, pd, inferredElementType, report, e);
            }
            
            // Proces any in-place values
            if (!signature.values.empty()) {
                if (signature.values.size() > 1 && e.type != ArrayDataType) {
                    if (e.type == UndefinedDataType) {
                        e.type = ArrayDataType; // Implicit array data type
                    }
                    else {
                        // WARN: Mismatched data type specifier
                        std::stringstream ss;
                        ss << "mismatched type specifier, assuming 'array'";
                        mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                        report.warnings.push_back(snowcrash::Warning(ss.str(),
                                                                     snowcrash::SignatureSyntaxWarning,
                                                                     sourceMap));
                    }
                }
                
                switch (e.type) {
                    case ArrayDataType: {
                        Elements elements;
                        for (Signature::Values::const_iterator it = signature.values.begin();
                             it != signature.values.end();
                             ++it) {
                            Element child;
                            child.type = inferredElementType;
                            ElementValue<StringValue>::set(*it, child);
                            elements.push_back(child);
                        }
                        
                        ElementValue<ArrayValue>::set(elements, e);
                    }
                        break;
                        
                    case ObjectDataType:
                        // TODO:
                        break;
                        
                    case StringDataType:
                    case NumberDataType:
                    case BooleanDataType:
                    default:
                        ElementValue<StringValue>::set(signature.values.front(), e);
                }
            }
            else if (inferredElementType != UndefinedDataType) {
                // Abbreviated array type syntax - create an arbitrary element to hold the type
                Elements elements;
                Element child;
                child.type = inferredElementType;
                elements.push_back(child);
                ElementValue<ArrayValue>::set(elements, e);
            }
            
            // Process in-line description
            if (!signature.content.empty())
                e.description = signature.content;
            
            // Process any additional description
            if (!signature.additionalContent.empty()) {
                if (!e.description.empty())
                    e.description += "\n";
                
                e.description += signature.additionalContent + "\n";
            }
        }
        
        /**
         *  \brief  Convert a string into %DataType.
         *  
         *  \param  s   String to be converted
         *  \param  inferredElementType Element %DataType in the case of an array %DataType
         *  \return %DataType as converted from the string
         */
        static DataType dataTypeFromString(const std::string& s, DataType& inferredElementType) {
            if (s == "string") {
                return StringDataType;
            }
            else if (s == "number") {
                return NumberDataType;
            }
            else if (s == "object") {
                return ObjectDataType;
            }
            else if (s == "array") {
                return ArrayDataType;
            }
            else if (s == "bool" || s== "boolean") {
                return BooleanDataType;
            }
            else {
                std::vector<std::string> spec = snowcrash::SplitOnFirst(s, ':');
                if (spec.empty())
                    return UndefinedDataType;
                
                snowcrash::TrimString(spec.front());
                if (spec.front() != "array")
                    return UndefinedDataType;
                
                DataType t;
                snowcrash::TrimString(spec.back());
                inferredElementType = dataTypeFromString(spec.back(), t);
                return ArrayDataType;
            }
        }
        
        /**
         *  \brief Process & applies signature specifiers
         *
         *  \param  node    Node from which were the specifiers retrieved
         *  \param  specifiers  Set of specifiers
         *  \param  pd      Parser data context
         *  \param  inferredElementType Element %DataType in the case of an array %DataType specifier
         *  \param  report  Parser report object
         *  \param  e       Output %Element to apply specifiers onto
         */
        static void setSpecifiers(const mdp::MarkdownNodeIterator& node,
                                  const scpl::Signature::Specifiers& specifiers,
                                  const snowcrash::SectionParserData& pd,
                                  DataType& inferredElementType,
                                  snowcrash::Report& report,
                                  Element& e) {
            
            size_t typeSpecifiers = 0;
            
            for (scpl::Signature::Specifiers::const_iterator cit = specifiers.begin();
                 cit != specifiers.end();
                 ++cit) {
                
                e.type = dataTypeFromString(*cit, inferredElementType);
                if (e.type != UndefinedDataType) {
                    ++typeSpecifiers;
                }
                else {
                    // WARN: Unrecognized signature specifier
                    std::stringstream ss;
                    ss << "unexpected specifier '" << *cit << "'";
                    mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                    report.warnings.push_back(snowcrash::Warning(ss.str(),
                                                                 snowcrash::SignatureSyntaxWarning,
                                                                 sourceMap));
                }
                
                if (typeSpecifiers > 1) {
                    typeSpecifiers = 1;
                    
                    // WARN: Too many type specifiers
                    std::stringstream ss;
                    ss << "too many type specifiers, expected 'string', 'number', 'object', 'array' or 'bool'";
                    mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
                    report.warnings.push_back(snowcrash::Warning(ss.str(),
                                                                 snowcrash::SignatureSyntaxWarning,
                                                                 sourceMap));
                }
            }
        }
    };
}

// NOTE: Pending mson namespace
namespace snowcrash {
    
    /**
     *  MSON Element Section Processor
     */
    template<>
    struct SectionProcessor<mson::Element> : public SectionProcessorBase<mson::Element> {
        
        /** MSON Element Signature Traits */
        struct ElementTraits : public scpl::SignatureTraits {
            ElementTraits()
            : SignatureTraits(Value | Array | Specifier | Content, "-", 1) {}
            
            static ElementTraits& get() {
                static ElementTraits s;
                return s;
            }
        };
        
        static MarkdownNodeIterator processSignature(const MarkdownNodeIterator& node,
                                                     SectionParserData& pd,
                                                     bool& parsingRedirect,
                                                     Report& report,
                                                     mson::Element& element) {
            
            scpl::Signature s;
            scpl::SignatureParser::parseSignature(node, ElementTraits::get(), pd, report, s);            
            mson::ElementSignature::setSignature(node, s, pd, report, element);

            return ++MarkdownNodeIterator(node);
        }
        
        static MarkdownNodeIterator processNestedSection(const MarkdownNodeIterator& node,
                                                         const MarkdownNodes& siblings,
                                                         SectionParserData& pd,
                                                         Report& report,
                                                         mson::Element& out);
        
        static SectionType sectionType(const MarkdownNodeIterator& node) {
            if (/*!HasSectionKeywordSignature(node) && */ // TODO: Is this check needed?
                (node->type != mdp::ListItemMarkdownNodeType ||
                node->children().empty()))
                return UndefinedSectionType;
            
            return MSONElementSectionType;
        }
        
        static SectionType nestedSectionType(const MarkdownNodeIterator& node,
                                             const MarkdownNodes& siblings,
                                             const mson::Element& context);
    };
    
    /** MSON Element Section Parser */
    typedef SectionParser<mson::Element, ListSectionAdapter> MSONElementParser;
}

#endif
