#include "NonTerminal.h"
#include <iostream>
NonTerminal::NonTerminal(string s) : PatternToken(s)
{
  lambda = false;
}

NonTerminal::NonTerminal(string s, vector<PatternToken*> r) : PatternToken(s)
{
  lambda = false;
  addRule(r);
}

bool NonTerminal::isTerminal()
{
  return false;
}

void NonTerminal::addRule(vector<PatternToken*> r)
{
  rules.push_back(r);
  if (r.size() == 0)
    lambda = true;
}

bool NonTerminal::canBeLambda()
{
  return lambda;
}

unsigned int NonTerminal::numRules()
{
  return rules.size();
}

vector<PatternToken*> NonTerminal::getRule(int r)
{
  return rules[r];
}

void NonTerminal::makeLambda()
{
  lambda = true;
}

// Follow set is the set of terminals that appear immediately to the right of a non-terminal
// in some valid sentence. Parsing follow set symbols trigger reductions to this non terminal.
void NonTerminal::addToFollowSet(vector<PatternToken*> s)
{
  vector<PatternToken*> remaining;
  for (unsigned int i = 0; i < s.size(); ++i)
    {
      if(std::find(followSet.begin(), followSet.end(), s[i]) == followSet.end())
        {
          followSet.push_back(s[i]);
          remaining.push_back(s[i]);
        }
    }
    
  
  if (remaining.size() > 0)
    for (unsigned int i = 0; i < updateList.size(); ++i)
      // only non terminals can be in the update list
      ((NonTerminal*)updateList[i])->addToFollowSet(remaining);
}

vector<PatternToken*> NonTerminal::getFollowSet()
{
  return followSet;
}

void NonTerminal::addToUpdateList(PatternToken* t)
{
  updateList.push_back(t);
}

string NonTerminal::toString()
{
  string ret = "NonTerminal:" + id + "\nRules:\n";
  for (unsigned int i = 0; i < rules.size(); ++i)
    {
      string ruleString = "\t";
      vector<PatternToken*> rule = rules[i];
      for (unsigned int j = 0; j < rule.size(); ++j)
        ruleString += rule[j]->getId() + " ";
      ruleString += "\n";
      ret += ruleString;
    }
  if (lambda)
    {
      ret += "\t \u03BB\n";
    }
  ret += "Firsts: ";
  for (unsigned int i = 0; i < beginSet.size(); ++i)
    ret += beginSet[i]->getId() + " ";
  ret += "\nFollow Set: ";
  for (unsigned int i = 0; i < followSet.size(); ++i)
    ret += followSet[i]->getId() + " ";
  ret += "\nUpdate List: ";
  for (unsigned int i = 0; i < updateList.size(); ++i)
    ret += updateList[i]->getId() + " ";
  return ret;
}

// Update list contains all non-terminals that appear at the right end of this non-terminal's 
// productions. The follow set of terminal is a subset of the follow sets of the symbols
// in its update list
void NonTerminal::constructUpdateList()
{
  for (unsigned int i = 0; i < rules.size(); ++i)
    {
      vector<PatternToken*> rule = rules[i];
      
      // begin from the right end of each production
      for (unsigned int j = rule.size() - 1; j >= 0; --j)
        {
          PatternToken* t = rule[j];
          
          // look only for the right most non-terminals, so break as soon as we encounter a terminal
          if (t->isTerminal())
            break;
        
          // if a non-terminal at the right end of the rule has an action, we still include it
          if (t->isAction() && j > 0) {
            if (!rule[j-1]->isTerminal() && !rule[j-1]->isAction())
              continue;
            else
              break;
          }
          
          // add non terminal to update list
          if(std::find(updateList.begin(), updateList.end(), t) == updateList.end())
            {
              updateList.push_back(t);
            }
            
          // if we don't allow lambda production, then only the right most non-terminal can be in update list
          if (!((NonTerminal*)t)->canBeLambda())
            break;
        }
    }
}
