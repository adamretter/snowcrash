//
//  Section.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 5/14/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include "Section.h"

using namespace snowcrash;

std::string snowcrash::SectionName(const SectionType& section) {

    switch (section) {
        case ModelSectionType:
        case ModelBodySectionType:
            return "model";
            
        case RequestSectionType:
        case RequestBodySectionType:
            return "request";
            
        case ResponseSectionType:
        case ResponseBodySectionType:
            return "response";
            
        case BodySectionType:
        case DanglingBodySectionType:
            return "message-body";
            
        case SchemaSectionType:
        case DanglingSchemaSectionType:
            return "message-schema";
            
        case HeadersSectionType:
            return "headers";
            
        default:
            return "section";
    }
}

/** 
 *  \brief Traits representing a generic singleton section
 *
 *  NOTE: Singleton section should be the default in the future.
 */
struct SingletonSectionTraits : SectionTraits {
    SingletonSectionTraits()
    : SectionTraits(true) {}
    
    /** \return Singleton section traits */
    static SingletonSectionTraits& get() {
        static SingletonSectionTraits s;
        return s;
    }
};

SectionTraits& snowcrash::GetSectionTraits(const SectionType& section)
{
    // TODO: This should be moved to respective section parser
    switch (section) {
        case MSONElementsSectionType:
        case BodySectionType:
            return SingletonSectionTraits::get();
        default:
            return SectionTraits::get();
    }
}
