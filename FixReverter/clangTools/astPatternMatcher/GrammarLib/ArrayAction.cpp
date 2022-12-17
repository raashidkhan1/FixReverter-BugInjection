#include "ArrayAction.h"

ArrayAction::ArrayAction(string s) : Action() {
  id = s;
}

bool ArrayAction::isArray() {
  return true;
}


string ArrayAction::toString() {
  return "ArrayAction " + id; 
}

