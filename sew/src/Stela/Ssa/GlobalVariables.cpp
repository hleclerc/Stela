/****************************************************************************
**
** Copyright (C) 2014 SocaDB
**
** This file is part of the SocaDB toolkit/database.
**
** SocaDB is free software. You can redistribute this file and/or modify
** it under the terms of the Apache License, Version 2.0 (the "License").
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
**
** You should have received a copy of the Apache License, Version 2.0
** along with this program. If not, see
** <http://www.apache.org/licenses/LICENSE-2.0.html>.
**
**
** Commercial License Usage
**
** Alternatively, licensees holding valid commercial SocaDB licenses may use
** this file in accordance with the commercial license agreement provided
** with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and SocaDB.
**
**
****************************************************************************/

#include "GlobalVariables.h"
#include "Class.h"
#include "Type.h"

GlobalVariables *ip = 0;

template<class T>
struct Type_ : Type {
    Type_( Class *c ) : Type( c ) {}
    virtual void write_val( Stream &os, const PI8 *data, const PI8 *knwn ) {
        os << *reinterpret_cast<const T *>( data );
    }
};

GlobalVariables::GlobalVariables() {
    #define DECL_BT( T ) class_##T = new Class;
    #include "DeclParmClass.h"
    #include "DeclBaseClass.h"
    #undef DECL_BT

    #define DECL_BT( T ) type_##T = new Type_<T>( class_##T ); class_##T->types << type_##T; type_##T->aryth = true;
    #include "DeclArytTypes.h"
    #undef DECL_BT

    #define DECL_BT( T ) type_##T = new Type( class_##T ); class_##T->types << type_##T;
    #define DONT_WANT_DeclArytTypes_h
    #include "DeclBaseClass.h"
    #undef DONT_WANT_DeclArytTypes_h
    #undef DECL_BT

    type_Type      ->_len = 64; type_Type      ->_ali = 32; type_Type      ->_pod = 1;
    type_Void      ->_len =  0; type_Void      ->_ali =  1; type_Void      ->_pod = 1;
    type_Error     ->_len =  0; type_Error     ->_ali =  1; type_Error     ->_pod = 1;
    type_Def       ->_len = 64; type_Def       ->_ali = 32; type_Def       ->_pod = 1;
    type_Class     ->_len = 64; type_Class     ->_ali = 32; type_Class     ->_pod = 1;
    type_SurdefList->_len = 64; type_SurdefList->_ali = 32; type_SurdefList->_pod = 1;

    type_ST  = sizeof( void * ) == 8 ? type_SI64 : type_SI32;
    ptr_size = 8 * sizeof( void * );

    pc = 0;
}
