//
//  Signature.h
//  snowcrash
//
//  Created by Zdenek Nemec on 5/14/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_SIGNATURE_H
#define SNOWCRASH_SIGNATURE_H

#include <sstream>
#include "SourceAnnotation.h"
#include "MarkdownNode.h"
#include "Section.h"
#include "RegexMatch.h"
#include "StringUtility.h"
#include "SectionParserData.h"

namespace snowcrash {

    /** 
     *  \brief Query whether a node has keyword-defined signature.
     *  \param node     A Markdown AST node to check.
     *  \return Type of the node if it has a recognized keyword signature, UndefinedType otherwise
     */
    extern SectionType SectionKeywordSignature(const mdp::MarkdownNodeIterator& node);

    /**
     *  \brief Recognize the type of section given the first line from a code block
     *  \param subject  The first line that needs to be recognized
     *  \return SectionType Type of the section if the line contains a keyword
     */
    extern SectionType RecognizeCodeBlockFirstLine(const mdp::ByteBuffer& subject);
}

namespace scpl {
    
    /**
     *  \brie   (List-defined) Signature Data
     *
     *  Data as parsed from a keyword-less list-defined signature.
     */
    struct Signature {
        typedef std::vector<mdp::ByteBuffer> Specifiers;
        typedef std::vector<mdp::ByteBuffer> Values;
        
        mdp::ByteBuffer identifier;             /// Signature identifier
        Values values;                          /// Signature values
        Specifiers specifiers;                  /// Signature specifiers
        mdp::ByteBuffer content;                /// Signature content
        mdp::ByteBuffer additionalContent;      /// Additional content following the signature
    };
    
    /**
     *  \brief  Traits charactestics of a list-defined signature
     *
     *  Traits describe identifier - defined or value/type defined signatures.
     *  If %expectIdentifier is true the signature must start with an identifier.
     *  If %expectIdentifier is false and %allowValue or %maxTraitsCount is set
     *  the signature must start with value, if any, or signature traits.
     */
    struct SignatureTraits {

        enum Trait {
            Identifier = (1 << 0), /// Expect an identifier in the signature
            Value = (1 << 1),      /// Optional value specification in the signature
            Array = (1 << 2),      /// Value can be an array (comma separated)
            Specifier = (1 << 3),  /// Optional specifiers in the signature
            Content = (1 << 4)     /// Optional content in the signature
        };
        
        typedef unsigned int Traits;

        const bool indentifierTrait;
        const bool valueTrait;
        const bool arrayTrait;
        const bool specifierTrait;
        const bool contentTrait;
        
        const std::string contentDelimiter; /// Signature content delimiter e.g. "-", "->", "..."
        const size_t maxSpecifierCount;     /// Max number of specifiers, 0 for no specifier

        SignatureTraits(Traits traits_ = Identifier | Value | Specifier | Content,
                        const char* contentDelimiter_ = "-",
                        size_t maxSpecifierCount_ = 2)
        :
        indentifierTrait(traits_ & Identifier),
        valueTrait(traits_ & Value),
        arrayTrait(traits_ & Array),
        specifierTrait(traits_ & Specifier),
        contentTrait(traits_ & Content),
        contentDelimiter(contentDelimiter_),
        maxSpecifierCount(maxSpecifierCount_)
        {}
        
        /** \return Default signature traits */
        static SignatureTraits& get() {
            static SignatureTraits s;
            return s;
        }
    };
    
    /**
     *  List-defined, line signature parser
     */
    struct SignatureParser {
        
        static const char ValueDelim = ':';
        static const char CSVDelim = ',';
        static const char SpecifiersDelim = '(';
        
        /**
         *  \brief Parse list-defined line signature
         *
         *  \param  node    Node its text is to be parsed as the signature
         *  \param  t       Signature traits
         *  \param  pd      Parser data
         *  \param  report  Output report buffer
         *  \param  remainingContent    Remaining additional lines of node text buffer
         *  \param  s       Parsed signature
         */
        static void parseSignature(const mdp::MarkdownNodeIterator& node,
                                   const SignatureTraits& t,
                                   const snowcrash::SectionParserData& pd,
                                   snowcrash::Report& report,
                                   Signature& s) {
            
            static const char* const CSVTraitsRegex = "^\\(([^)]+)\\)";            
            
            mdp::ByteBuffer work = snowcrash::GetFirstLine(node->text, s.additionalContent);
            
            if (t.indentifierTrait) {
                Signature::Values values;
                size_t matchSize = RetrieveValues(node, work, t, pd, report, values);
                if (!values.empty())
                    s.identifier = values.front();

                work = work.substr(matchSize);
                snowcrash::TrimStringStart(work);
                
                if (s.identifier.empty()) {
                    missingIdentifierWarning(node, work, t, pd, report);
                }
            }
            
            if (t.valueTrait) {
                if (!t.indentifierTrait || work.find_first_of(ValueDelim) == 0) {
                    
                    if (t.indentifierTrait) {
                        work = work.substr(1);
                        snowcrash::TrimStringStart(work);
                    }
                    
                    size_t matchSize = RetrieveValues(node, work, t, pd, report, s.values);
                    
                    work = work.substr(matchSize);
                    snowcrash::TrimStringStart(work);

                    if (t.indentifierTrait && s.values.empty()) {
                        missingValueWarning(node, work, t, pd, report);
                    }
                }
            }
            
            if (t.specifierTrait) {
                if (work.find_first_of(SpecifiersDelim) == 0) {
                    snowcrash::CaptureGroups groups;
                    if (snowcrash::RegexCapture(work, CSVTraitsRegex, groups, 2) && groups.size() == 2) {
                        snowcrash::Split(groups[1], CSVDelim, s.specifiers);
                        for (Signature::Specifiers::iterator it = s.specifiers.begin();
                             it != s.specifiers.end();
                             ++it) {
                            snowcrash::TrimString(*it);
                        }
                        
                        work = work.substr(groups[0].size());
                        snowcrash::TrimStringStart(work);
                    }
                }
            }
            
            if (!t.indentifierTrait && s.values.empty() && s.specifiers.empty()) {
                missingElementDefinitionWarning(node, work, t, pd, report);
            }

            if (t.contentTrait) {
                if (work.find(t.contentDelimiter) == 0) {
                    s.content = work.substr(t.contentDelimiter.size());
                    snowcrash::TrimStringStart(s.content);
                    work.clear();
                }
            }
            
            if (!work.empty()) {
                unexpectedContentWarning(node, work, t, pd, report);
            }
        }
        
        /** \brief Retrieve escaped or unescaped values from the subject */
        static size_t RetrieveValues(const mdp::MarkdownNodeIterator& node,
                                     const mdp::ByteBuffer& subject,
                                     const SignatureTraits& t,
                                     const snowcrash::SectionParserData& pd,
                                     snowcrash::Report& report,
                                     Signature::Values& values) {
            
            mdp::ByteBuffer work = subject;
            
            static const char EscapeCharacter = '`';
            static const char* const EscapeCSVDelims = ", \t";
            
            if (work.find_first_of(EscapeCharacter) == 0) {
                size_t length = 0;
                
                // Escaped sequence
                do {
                    work.erase(0, 1);
                    length += 1;
                    
                    size_t pos = work.find_first_of(EscapeCharacter);
                    if (pos != mdp::ByteBuffer::npos) {
                        // Pick the value
                        values.push_back(work.substr(0, pos));

                        // Clean up the work buffer
                        work.erase(0, pos + 1);
                        length += pos + 1;
                        
                        pos = work.find_first_not_of(EscapeCSVDelims);
                        if (pos != mdp::ByteBuffer::npos) {
                            work.erase(0, pos);
                            length += pos;
                        }
                    }
                    else {
                        mismatchedEscapeSequenceWarnign(node, work, t, pd, report);
                    }
                }
                while (t.arrayTrait &&
                       work.find_first_of(EscapeCharacter) == 0);
                
                return length;
                
            }
            else {
                // Break by comma
                static const char* ValueDelims = ":(";
                std::string delims(ValueDelims);
                delims += t.contentDelimiter;
                
                size_t pos = work.find_first_of(delims);
                work = work.substr(0, pos);
                snowcrash::Split(work, CSVDelim, values);
                
                // Trim values
                for (Signature::Values::iterator it = values.begin();
                     it != values.end();
                     ++it) {
                    snowcrash::TrimString(*it);
                }
            
                return work.length();
            }
            return 0;
        }
        
        /** \brief Compose missing element definition warning */
        static void missingElementDefinitionWarning(const mdp::MarkdownNodeIterator& node,
                                                    const mdp::ByteBuffer& work,
                                                    const SignatureTraits& t,
                                                    const snowcrash::SectionParserData& pd,
                                                    snowcrash::Report& report) {
            
            // WARN: Missing element definition */
            mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
            report.warnings.push_back(snowcrash::Warning("missing element value or trait(s), 'expected '<value> (<traits>)'",
                                              snowcrash::SignatureSyntaxWarning,
                                              sourceMap));
        }
        
        /** \brief Compose missing identifier warning */
        static void missingIdentifierWarning(const mdp::MarkdownNodeIterator& node,
                                             const mdp::ByteBuffer& work,
                                             const SignatureTraits& t,
                                             const snowcrash::SectionParserData& pd,
                                             snowcrash::Report& report) {
            
            // WARN: Missing identifier */
            mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
            report.warnings.push_back(snowcrash::Warning("missing expected identifier",
                                              snowcrash::SignatureSyntaxWarning,
                                              sourceMap));
        }

        /** \brief Compose missing value warning */
        static void missingValueWarning(const mdp::MarkdownNodeIterator& node,
                                        const mdp::ByteBuffer& work,
                                        const SignatureTraits& t,
                                        const snowcrash::SectionParserData& pd,
                                        snowcrash::Report& report) {
            
            // WARN: Missing value after ':` */
            mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
            report.warnings.push_back(snowcrash::Warning("missing value after ':', expected '<identifier>: <value>'",
                                              snowcrash::SignatureSyntaxWarning,
                                              sourceMap));
        }
        
        /** \brief Compose unexpected content warning */
        static void unexpectedContentWarning(const mdp::MarkdownNodeIterator& node,
                                             const mdp::ByteBuffer& work,
                                             const SignatureTraits& t,
                                             const snowcrash::SectionParserData& pd,
                                             snowcrash::Report& report) {
            
            // WARN: Unexpected content
            std::stringstream ss;
            ss << "ignoring '" << work << "', expected '";
            if (t.indentifierTrait) {
                ss << "<identifier>";
                
                if (t.valueTrait)
                    ss << ":";
            }
            
            if (t.valueTrait) {
                if (t.indentifierTrait)
                    ss << " ";
                
                ss << "<value>";
            }
            
            if (t.specifierTrait) {
                ss << " (";
                for (size_t i = 0; i < t.maxSpecifierCount; ++i) {
                    ss << "<t" << i + 1 << ">";
                    if (i < t.maxSpecifierCount - 1)
                        ss << ", ";
                }
                
                ss << ")";
            }
            
            if (t.contentTrait) {
                ss << " " << t.contentDelimiter << " <content>";
            }
            
            ss << "'";
            
            mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
            report.warnings.push_back(snowcrash::Warning(ss.str(),
                                              snowcrash::SignatureSyntaxWarning,
                                              sourceMap));
        }

        /** \brief Compose mismatched escape sequence warning */
        static void mismatchedEscapeSequenceWarnign(const mdp::MarkdownNodeIterator& node,
                                                    const mdp::ByteBuffer& work,
                                                    const SignatureTraits& t,
                                                    const snowcrash::SectionParserData& pd,
                                                    snowcrash::Report& report) {
            
            // WARN: Mismatched escape sequence
            std::stringstream ss;
            ss << "Mismatched escape sequence – missing closing '`' in '" << work << "'";
            mdp::CharactersRangeSet sourceMap = mdp::BytesRangeSetToCharactersRangeSet(node->sourceMap, pd.sourceData);
            report.warnings.push_back(snowcrash::Warning(ss.str(),
                                                         snowcrash::SignatureSyntaxWarning,
                                                         sourceMap));
        }
    };
}

#endif
