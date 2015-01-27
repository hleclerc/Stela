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


#include "../Ssa/GlobalVariables.h"
#include "../Ssa/Type.h"
#include "Codegen.h"
#include "OutReg.h"

OutReg::OutReg( Type *type, String name ) : type( type ), name( name )/*, ptr( false )*/ {
}

//Stream &OutReg::write( Codegen *cc ) {
//    //    if ( ptr ) {
//    //        if ( type == ip->type_SI8  ) { cc->on << "v_SI8[ "  << name << " >> 4 ];"; return *cc->on; }
//    //        if ( type == ip->type_PI8  ) { cc->on << "v_PI8[ "  << name << " >> 4 ];"; return *cc->on; }
//    //        if ( type == ip->type_SI16 ) { cc->on << "v_SI16[ " << name << " >> 5 ];"; return *cc->on; }
//    //        if ( type == ip->type_PI16 ) { cc->on << "v_PI16[ " << name << " >> 5 ];"; return *cc->on; }
//    //        if ( type == ip->type_SI32 ) { cc->on << "v_SI32[ " << name << " >> 6 ];"; return *cc->on; }
//    //        if ( type == ip->type_PI32 ) { cc->on << "v_PI32[ " << name << " >> 6 ];"; return *cc->on; }
//    //        if ( type == ip->type_SI64 ) { cc->on << "v_SI64[ " << name << " >> 7 ];"; return *cc->on; }
//    //        if ( type == ip->type_PI64 ) { cc->on << "v_PI64[ " << name << " >> 7 ];"; return *cc->on; }

//    //        if ( type == ip->type_FP32 ) { cc->on << "v_FP32[ " << name << " >> 6 ];"; return *cc->on; }
//    //        if ( type == ip->type_FP64 ) { cc->on << "v_FP64[ " << name << " >> 7 ];"; return *cc->on; }

//    //        PRINT( *type );
//    //        TODO;
//    //    }
//    // else
//    *cc->on << name;
//    return *cc->on;
//}

void OutReg::write_to_stream( Stream &os ) const {
    os << name;
}
