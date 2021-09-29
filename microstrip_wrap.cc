
#include <globals.h>

#include "qucsator/component.h"
#include "microstrip/substrate.h"
#include "microstrip/bondwire.h"
#include "microstrip/msline.h"
#include "microstrip/mscoupled.h"
#include "microstrip/mslange.h"

namespace {

bondwire c0;
DISPATCHER<CARD>::INSTALL d0(&device_dispatcher, bondwire::cirdef.type, &c0);

// CIR_CIRCULARLOOP = 0;
// CIR_CPWGAP = 0;
// CIR_CPWLINE = 0;
// CIR_CPWOPEN = 0;
// CIR_CPWSHORT = 0;
// CIR_CPWSTEP = 0;
// CIR_MSCORNER = 0;	

mscoupled c8;
DISPATCHER<CARD>::INSTALL d8(&device_dispatcher, mscoupled::cirdef.type, &c8);

// CIR_MSCROSS = 0;	
// CIR_MSGAP = 0;	

mslange c11;
DISPATCHER<CARD>::INSTALL d11(&device_dispatcher, mslange::cirdef.type, &c11);

// CIR_MSLINE = 0;
// CIR_MSMBEND = 0;
// CIR_MSOPEN = 0;
// CIR_MSRSTUB = 0;	
// CIR_MSSTEP = 0;	
// CIR_MSTEE = 0;
// CIR_MSVIA = 0;	
// CIR_SPIRALIND = 0;	

}
