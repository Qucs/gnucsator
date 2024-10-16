/*--------------------------------------------------------------------------*/
#include "config.h"
/*--------------------------------------------------------------------------*/
static void prepare_env()
{
  static const char* plugpath="PLUGPATH=" GNUCAP_PLUGPATH
                              "\0         (reserved space)                 ";

  std::string ldlpath = OS::getenv("LD_LIBRARY_PATH");
  if (ldlpath != "") {
    ldlpath += ":";
  }else{ untested();
  }
  assert(strlen("PLUGPATH=") == 9);
  trace1("plugpath", ldlpath + (plugpath+9));
  OS::setenv("GNUCAP_PLUGPATH", ldlpath + (plugpath+9), false);
}
/*--------------------------------------------------------------------------*/
