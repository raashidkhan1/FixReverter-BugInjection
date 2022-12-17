#include "Action.h"

bool Action::isTerminal()
{
  return false;
}

void Action::constructUpdateList()
{
  return;
}

bool Action::isAction()
{
  return true;
}

bool Action::isArray()
{
  return false;
}

bool Action::isID()
{
  return false;
}

bool Action::isName()
{
  return false;
}
