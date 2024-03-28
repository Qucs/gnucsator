
#include <globals.h>

#include "qucsator/component.h"
#include "microstrip/substrate.h"
#include "microstrip/bondwire.h"
#include "microstrip/msline.h"
#include "microstrip/mscoupled.h"
#include "microstrip/mslange.h"
#include "microstrip/msopen.h"
#include "microstrip/msstep.h"
// #include "microstrip/mstee.h"

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
msline c12;
DISPATCHER<CARD>::INSTALL d12(&device_dispatcher, msline::cirdef.type, &c12);

// CIR_MSMBEND = 0;
msopen c14;
DISPATCHER<CARD>::INSTALL d14(&device_dispatcher, msopen::cirdef.type, &c14);
// CIR_MSRSTUB = 0;	
msstep c16;
DISPATCHER<CARD>::INSTALL d16(&device_dispatcher, msstep::cirdef.type, &c16);
//mstee c17;
//DISPATCHER<CARD>::INSTALL d17(&device_dispatcher, mstee::cirdef.type, &c17);
//
// CIR_MSVIA = 0;	
// CIR_SPIRALIND = 0;	

}
