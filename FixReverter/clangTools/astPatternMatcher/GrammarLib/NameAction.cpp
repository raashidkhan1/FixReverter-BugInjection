#include "NameAction.h"

NameAction::NameAction(string s) : Action() {
  id = s;
}

bool NameAction::isName() {
  return true;
}

string NameAction::toString() {
  return "NamedAction " + id; 
}

string NameAction::getName() {
  return id.substr(5);
}
