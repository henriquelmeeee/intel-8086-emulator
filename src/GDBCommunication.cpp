#include <iostream>
#include <stdlib.h>

#include "GDBSocket.h"

namespace GDBCommunication {
  bool handle_p_command(word* registerValue) {
    char response[1024];
    sprintf(response, "$%x#CC", *registerValue);
    return true;
  }
}
