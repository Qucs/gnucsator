
#include <globals.h>

#include "component.h"
#include "substrate.h"
#include "msline.h"
#include "mscoupled.h"

mscoupled c0;
DISPATCHER<CARD>::INSTALL d0(&device_dispatcher, "MCOUPLED", &c0);
