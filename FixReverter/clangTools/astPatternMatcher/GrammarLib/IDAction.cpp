#include "IDAction.h"

IDAction::IDAction(string s) : Action() {
  id = s;
}

bool IDAction::isID() {
  return true;
}

string IDAction::toString() {
  return "IDAction " + id; 
}
