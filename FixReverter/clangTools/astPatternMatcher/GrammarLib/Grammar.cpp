#include "Grammar.h"

Grammar::Grammar(string patternFile)
{
  setTerminals();
  parseGrammar(patternFile);
  constructSets();
  initialState = new State(tokens);
}

void Grammar::setTerminals()
{
  vector<string> ts = TERMINALS;
  for (unsigned int i = 0; i < ts.size(); ++i)
    {
      tokens.push_back(new Terminal(ts[i]));
    }
}

void Grammar::parseGrammar(string patternFile)
{
  std::ifstream grammarFile;
  grammarFile.open(patternFile);
  string line;
  while (getline(grammarFile, line))
    {
      string lhs = "";
      vector<string> rule;
      string id = "";
      for (unsigned int i = 0; i < line.length(); ++i)
        {
          if (line[i] != ' ' && line[i] != '\n')
            {
              // read in token ids
              id = id + line[i];
            }
          else if (id != "" && lhs == "")
            {
              // read the lhs token of a production and store in action
              lhs = id;
              id = "";
            }
          else if (id != "" && id != ":=")
            { 
              // push rhs tokens onto rule vector
              rule.push_back(id);
              id = "";
            }
          else if (id == ":=")
            id = "";
        }
        
      // token ids can't contain whitespace
      if (id.find(' ') == std::string::npos &&
          id.find('\n') == std::string::npos)
        {
          rule.push_back(id);
        }
        
      // skip rules with no lhs
      if (lhs == "")
        continue;
    
      // store rhs of production
      vector<PatternToken*> ruleT;
      for (unsigned int r = 0; r < rule.size(); ++r)
        {
          string primary = rule[r];
          string action = "";
          // if : was found
          // split into token id (primary) and tag (action)
          if (primary.find(':') != std::string::npos)
            {
              action = primary.substr(primary.find_first_of(':')+1);
              primary = primary.substr(0,primary.find_first_of(':'));
            }
          
          // find current token in tokens vector
          unsigned int i;
          for (i = 0; i < tokens.size(); ++i)
            {
              if (tokens[i]->isThisPatternToken(primary))
                {
                  ruleT.push_back(tokens[i]);
                  break;
                }
            }
            
          // current token not found in tokens vector
          if (i == tokens.size())
            {
              // must be non terminal because all terminals are included in token vector parsing
              NonTerminal* nt = new NonTerminal(primary);
              tokens.push_back(nt);
	      
	      ruleT.push_back(nt);
	    }
          
          if (action != "")
            {
              // skip if action is already in tokens vector
              for (i = 0; i < tokens.size(); ++i)
                {
                  if (tokens[i]->isThisPatternToken(action))
                    {
                      ruleT.push_back(tokens[i]);
                      break;
                    }
                }
              if (i == tokens.size())
                {
                  Action* act = determineAction(action);
                  tokens.push_back(act);
                  ruleT.push_back(act);
                }
            }
        }
      bool added = false;
      for (unsigned int i = 0; i < tokens.size(); ++i)
        {
          // if non terminal is already in tokens vector, add new production
          // to the same non terminal
          if (tokens[i]->isThisPatternToken(lhs))
            {
              added = true;
              if (tokens[i]->isTerminal())
                {
                  std::cout << "ERROR: tried to give rule to terminal\n";
                }
              else
                {
                  ((NonTerminal*)tokens[i])->addRule(ruleT);
                }
            }
        }
      if (!added)
        {
          // otherwise add it to tokens vector
          NonTerminal* x = new NonTerminal(lhs, ruleT);
          tokens.push_back(x);
        }
    }
}

void Grammar::constructSets()
{
  for (unsigned int t = 0; t < tokens.size(); ++t)
    {
      getBeginSet(tokens[t]);
    }
  
  for (unsigned int t = 0; t < tokens.size(); ++t)
    {
      tokens[t]->constructUpdateList();
    }

  for (unsigned int t = 0; t < tokens.size(); ++t)
    {
      getFollowSet(tokens[t]);
    }
}

vector<PatternToken*> Grammar::getBeginSet(PatternToken* t)
{
  // terminals return themselves for begin set
  if (t->isBeginSetComplete())
    return t->getBeginSet();
  // since all terminals/actions start as complete, this is for sure a NonTerminal
  NonTerminal* n = (NonTerminal*)t;
  for (unsigned int i = 0; i < n->numRules(); ++i)
    {
      vector<PatternToken*> rule = n->getRule(i);
      // if a production is lambda, it isn't in the list
      unsigned long spot;
      for (spot = 0; spot < rule.size(); ++spot)
        {
          // if a non terminal appears on the left end of the current production,
          // its beginset is a subset of this non terminal's begin set
          n->addToBeginSet(getBeginSet(rule[spot]));
          
          // begin sets are terminals that begin a production, so stop as soon as a
          // terminal is reached.
          if (rule[spot]->isTerminal() ||
              !((NonTerminal*)rule[spot])->canBeLambda())
            break;
        }
      if (spot == rule.size())
        n->makeLambda();
    }
  t->finishedBeginSet();
  return t->getBeginSet();
}

void Grammar::getFollowSet(PatternToken* t)
{
  // only non terminals have follow sets (to reduce to)
  if (t->isTerminal() || t->isAction())
    return;

  NonTerminal* n = (NonTerminal*)t;
  for (unsigned int i = 0; i < n->numRules(); ++i)
    {
      vector<PatternToken*> rule = n->getRule(i);
      PatternToken *first, *second;
      for (unsigned int r = 0; r < rule.size(); ++r)
        {
          second = rule[r];
          // if a token is a non terminal and the token immediately after it is not action, then
          // the begin set of the second is a subset of the follow set of the first 
          if (r > 0 && !first->isTerminal() && !first->isAction() && !second->isAction()) {
            ((NonTerminal*)first)->addToFollowSet(second->getBeginSet());
          }
          first = second;
        }
      
    }
}

string Grammar::toString()
{
  string ret = "Tokens:\n";
  for (unsigned int t = 0; t < tokens.size(); ++t)
    {
      ret += tokens[t]->toString() + "\n";
    }
  ret += "States:\n";
  ret += initialState->toString();
  return ret;
}

State* Grammar::getRoot()
{
  return initialState;
}

Action* Grammar::determineAction(std::string s) {

  if (s[s.length()-2] == '[' && s[s.length()-1] == ']')
    return new ArrayAction(s);
  else if (!s.find("name-"))
    return new NameAction(s);
  else
    return new IDAction(s);
  
}
