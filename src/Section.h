//
//  Section.h
//  snowcrash
//
//  Created by Zdenek Nemec on 11/1/13.
//  Copyright (c) 2013 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_SECTION_H
#define SNOWCRASH_SECTION_H

#include <string>

namespace snowcrash {
    
    /**
     *  API Blueprint Sections Types.
     */
    enum SectionType {
        UndefinedSectionType,           /// Undefined section
        BlueprintSectionType,           /// Blueprint overview
        ResourceGroupSectionType,       /// Resource group
        ResourceSectionType,            /// Resource
        ResourceMethodSectionType,      /// Resource & Action combined (abbrev)
        ActionSectionType,              /// Action
        RequestSectionType,             /// Request
        RequestBodySectionType,         /// Request & Payload body combined (abbrev)
        ResponseSectionType,            /// Response
        ResponseBodySectionType,        /// Response & Body combined (abbrev)
        ModelSectionType,               /// Model
        ModelBodySectionType,           /// Model & Body combined (abbrev)
        BodySectionType,                /// Payload Body
        DanglingBodySectionType,        /// Dangling Body (unrecognised section considered to be Body)
        SchemaSectionType,              /// Payload Schema
        DanglingSchemaSectionType,      /// Dangling Schema (unrecognised section considered to be Schema)
        HeadersSectionType,             /// Headers
        ForeignSectionType,             /// Foreign, unexpected section
        ParametersSectionType,          /// Parameters
        ParameterSectionType,           /// One Parameter definition
        ValuesSectionType,              /// Value enumeration
        ValueSectionType,               /// One Value
        MSONObjectSectionType,          /// MSON Object
        MSONPropertySectionType,        /// MSON Object property
        MSONPropertiesSectionType,      /// Collection of MSON Object properties
        MSONElementSectionType,         /// MSON Array element
        MSONElementsSectionType         /// Collection of MSON Array elements
    };
    
    /** \return Human readable name for given %SectionType */
    extern std::string SectionName(const SectionType& section);
    
    struct SectionTraits {
        const bool singleton;   /// True if the section is singleton
        
        SectionTraits(bool singleton_ = false)
        : singleton(singleton_) {}
        
        /** \return Default section traits */
        static SectionTraits& get() {
            static SectionTraits s;
            return s;
        }
    };
    
    /** 
     *  \brief Get %SectionTraits for a %SectionType
     *
     *  TODO: This should be a function of respective section parser
     */
    extern SectionTraits& GetSectionTraits(const SectionType& section);
}

#endif
