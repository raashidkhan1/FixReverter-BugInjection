#include "State.h"

int State::count = 0;
State::State(vector<PatternToken*> allTokens)
{
  for (unsigned int i = 0; i < allTokens.size(); ++i)
    {
      if (!allTokens[i]->isTerminal() && !allTokens[i]->isAction())
        {
	  NonTerminal* n = (NonTerminal*)allTokens[i];
          for (unsigned int i = 0; i < n->numRules(); ++i)
            prods.push_back(Production(n, n->getRule(i), 0));
        }
    }
  root = this;
  id = count;
  count++;
  createPaths();
}


State::State(State* r, vector<Production> ps)
{
  prods = ps;
  root = r;
  id = count;
  count++;
}

void State::createPaths()
{
  vector<Production> tempProds = prods;
  for (unsigned int i = 0; i < tempProds.size(); ++i)
    {
      if (tempProds[i].pos == tempProds[i].body.size())
        {
          int x = 0;
          for (unsigned int j = 0; j < tempProds[i].body.size(); ++j)
            if (tempProds[i].body[j]->isAction())
              ++x;
          reds.push_back(Reduction(tempProds[i].head->getId(),tempProds[i].body.size() - x,
                                   tempProds[i].head->getFollowSet(), tempProds[i].body));
        }
      else
        {
          PatternToken* next = tempProds[i].body[tempProds[i].pos];
          vector<Production> nextSet;
          int growth = 1;
          if (tempProds[i].body.size() > tempProds[i].pos + 1 &&
              tempProds[i].body[tempProds[i].pos + 1]->isAction())
            {
              growth = 2;
            }
          nextSet.push_back(Production(tempProds[i].head,tempProds[i].body, tempProds[i].pos + growth));
          //get all productions that shift on the same token
          for (unsigned int j = i + 1; j < tempProds.size(); ++j)
            {
              if (tempProds[j].body[tempProds[j].pos] == next)
                {
                  nextSet.push_back(Production(tempProds[j].head,tempProds[j].body, tempProds[j].pos +
                                               ((tempProds[j].body.size() > tempProds[j].pos + 1 &&
                                                 tempProds[j].body[tempProds[j].pos + 1]->isAction())
                                                ?2:1)));
                  tempProds.erase(tempProds.begin()+j);
                  j--;
                }
            }
          //expand all productions
          for (unsigned int j = 0; j < nextSet.size(); ++j)
            {
              Production pr = nextSet[j];
              if (pr.pos < pr.body.size() && !pr.body[pr.pos]->isTerminal() &&
                  !pr.body[pr.pos]->isAction())
                {
                  NonTerminal* nt = (NonTerminal*)pr.body[pr.pos];
                  for (unsigned int z = 0; z < nt->numRules(); ++z)
                    {
                      Production newP(nt, nt->getRule(z), 0);
                      bool found = false;
                      for (unsigned int n = 0; n < nextSet.size() && !found; ++n)
                        found = found || nextSet[n].eq(newP);
                      if (!found)
                        nextSet.push_back(newP);
                    }
                }
            }
          //nextSet is complete
          vector<int> visited;
          State* prodPath = root->searchStates(visited,nextSet);
          if (prodPath != nullptr)
          //if production set already exists
            {
              paths.insert(pair<string,State*>(next->getId(), prodPath));
            }
          else
          //if it does not
            {
              State* newS = new State(root, nextSet);
              paths.insert(pair<string,State*>(next->getId(), newS));
              newS->createPaths();
            }
        }
    }
    
    conflict = !getConflicts();
}

State* State::searchStates(vector<int>& v, vector<Production> p)
{
  if(std::find(v.begin(), v.end(), id) != v.end())
    return (State*)nullptr;
  v.push_back(id);
  unsigned int i;
  //if the set of productions exists inside this state's list
  for (i = 0; i < p.size(); ++i)
    {
      bool found = false;
      for (unsigned int j = 0; j < prods.size(); ++j)
        {
          if (p[i].eq(prods[j]))
            {
              found = true;
              break;
            }
        }
      if (!found)
        break;
    }
  //if the set was complete, and the sizes are the same
  //it is this State
  if (i == p.size() && p.size() == prods.size())
    {
      return this;
    }
  //not found
  for (map<string,State*>::iterator it = paths.begin(); it != paths.end(); ++it)
    {
      State* foundS= it->second->searchStates(v,p);
      if (foundS != nullptr)
        return foundS;
    }
  return nullptr;
}

int State::getId()
{
  return id;
}

string State::toString()
{
  vector<int> visited;
  return toString(visited);
}

string State::toString(vector<int>& v)
{
  if(std::find(v.begin(), v.end(), id) != v.end())
    return "";
  v.push_back(id);
  
  string ret = "";
  ret += "State" + std::to_string(id) + "\n";
  ret += "\tProductions\n";
  for (unsigned int i = 0; i < prods.size(); ++i)
    ret += "\t\t" + prods[i].toString() + "\n";
  ret += "\tReductions\n";
  for (unsigned int i = 0; i < reds.size(); ++i)
    ret += "\t\t" + reds[i].toString() + "\n";
  ret += "\tNexts:";
  for (map<string,State*>::iterator it = paths.begin(); it != paths.end(); ++it)
    ret +=  "(" + it->first + "," + std::to_string(it->second->getId()) + ") ";
  ret += "\n";
  if (!getConflicts())
    ret += "has conflict\n";
  for (map<string,State*>::iterator it = paths.begin(); it != paths.end(); ++it)
    ret += it->second->toString(v);
  if (isEndState())
    ret += "is end state\n";
  return ret;
}

bool State::isShift(string s)
{
  for (map<string,State*>::iterator it = paths.begin(); it != paths.end(); ++it)
    if (s == it->first)
      return true;
  return false;
}

bool State::isReduction(string s)
{
  for (unsigned int i = 0; i < reds.size(); ++i)
    {
      for (unsigned int j = 0; j < reds[i].reduceSet.size(); ++j)
        {
          if (reds[i].reduceSet[j]->isThisPatternToken(s))
            return true;
        }
    }
  return false;
}

// returns all reductions 
vector<Reduction> State::getReductions(string s)
{
  vector<Reduction> ret;
  for (unsigned int i = 0; i < reds.size(); ++i)
    {
      for (unsigned int j = 0; j < reds[i].reduceSet.size(); ++j)
        {
          if (reds[i].reduceSet[j]->isThisPatternToken(s))
            ret.push_back(reds[i]);
        }
    }
  return ret;
    
}

vector<PatternToken*> State::getFinalRules()
{
  for (int i = 0; i < prods.size(); ++i)
    {
      if (prods[i].pos == prods[i].body.size())
	return prods[i].body;
    }
  vector<PatternToken*> r;
  return r;
}

State* State::shift(string s)
{
  for (map<string,State*>::iterator it = paths.begin(); it != paths.end(); ++it)
    if (s == it->first)
      return it->second;
  return nullptr;
}

// return the first reduction that matches
Reduction State::reduce(string s)
{
  for (unsigned int i = 0; i < reds.size(); ++i)
    {
      for (unsigned int j = 0; j < reds[i].reduceSet.size(); ++j)
        {
          if (reds[i].reduceSet[j]->isThisPatternToken(s))
            return reds[i];
        }
    }
  return Reduction("",0,vector<PatternToken*>(),vector<PatternToken*>());
  
}

bool State::isEndState()
{
  bool isEnd = false;
  for (unsigned int i = 0; i < reds.size(); ++i)
    isEnd = isEnd || (reds[i].reduceSet.size() == 0);
  return (paths.size() == 0) && isEnd && (reds.size() > 0);
}

string State::getPattern()
{
  for (unsigned int i = 0; i < reds.size(); ++i)
    if (reds[i].reduceSet.size() == 0)
      return reds[i].retTok;
  return "";
}

// return true if no conflict and false if either r/r or r/s conflict exists
bool State::getConflicts()
{
  // store tokens that trigger reductions on this state
  vector<string> all;
  for (unsigned int i = 0; i < reds.size(); ++i)
    for (unsigned int j = 0; j < reds[i].reduceSet.size(); ++j)
      {
        string thisTok = reds[i].reduceSet[j]->getId();
        unsigned int k;
        for (k = 0; k < all.size(); ++k)
          if (thisTok == all[k])
            break;
        if (k < all.size())
          {
            // if k < all.size(), the previous for loop exited before reaching the end,
            // so the current reduction rule shares the same next token as a previous rule
            // indicating a reduce-reduce conflict
            return false;
          }
          
        // if the current reduction doesnt cause a conflict, save its next token to all
        all.push_back(thisTok);
      }
  //search shifts
  for (map<string,State*>::iterator it = paths.begin(); it != paths.end(); ++it)
    {
      string thisTok = it->first;
      unsigned int k;
      for (k = 0; k < all.size(); ++k)
        if (thisTok == all[k])
          break;
      if (k < all.size())
        {
          // if k < all.size(), the previous for loop exited before reaching the end,
          // so the current shift rule shares the same next token as a reduction rule
          // indicating a shift-reduce conflict
          return false;
        }
    }
  // no conflict
  return true;
}

bool State::hasConflicts()
{
    return conflict;
}
