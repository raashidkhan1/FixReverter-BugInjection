#include "Validator.h"

Validator::Validator(bool parseInfo)
{
  valid = false;
  grammar = nullptr;
  mainParse = nullptr;
  this->parseInfo = parseInfo;
  if (parseInfo)
    std::cerr << "Begin Validator\n";
}

Validator::Validator(Grammar* g, bool parseInfo)
{
  grammar = g;
  valid = true;
  this->parseInfo = parseInfo;
  ParseData* p = new ParseData();
  mainParse = p;
  parseQueue.push_back(p);
  if (parseInfo)
    std::cerr << "Begin Validator\n";
}

// destructor
Validator::~Validator()
{
  for(auto pd: parseQueue)
      delete pd;
  parseQueue.clear();
}

// copy constructor
Validator::Validator(const Validator& other)
{
  grammar = other.grammar;
  valid = other.valid;
  parseInfo = other.parseInfo;
  mainParse = other.mainParse;
  
  for (auto pd: other.parseQueue)
    {
      ParseData* newpd = new ParseData(*pd);
      parseQueue.push_back(newpd);
    }
}

// move constructor
Validator::Validator(Validator&& other)
{
  grammar = other.grammar;
  valid = other.valid;
  parseInfo = other.parseInfo;
  mainParse = other.mainParse;
  
  for (auto pd: other.parseQueue)
    {
      ParseData* newpd = pd;
      parseQueue.push_back(newpd);
      newpd = nullptr;
    }
}

// copy assignment operator
Validator& Validator::operator=(const Validator& other)
{
  if (this != &other)
    {
      for(auto pd: parseQueue)
        delete pd;
      parseQueue.clear();
      
      for (auto pd: other.parseQueue)
        {
          ParseData* newpd = new ParseData(*pd);
          parseQueue.push_back(newpd);
        }
      grammar = other.grammar;
      valid = other.valid;
      parseInfo = other.parseInfo;
      mainParse = other.mainParse;
    }
    
  return *this;
}

// move assignment operator
Validator& Validator::operator=(Validator&& other)
{
  if (this != &other)
    {
      for(auto pd: parseQueue)
        delete pd;
      parseQueue.clear();
      
      for (auto pd: other.parseQueue)
        {
          ParseData* newpd = pd;
          parseQueue.push_back(newpd);
          newpd = nullptr;
        }
      grammar = other.grammar;
      valid = other.valid;
      parseInfo = other.parseInfo;
      mainParse = other.mainParse;
    }
    
  return *this;
    
}

void Validator::processToken(string t, json j, int pos)
{
  if (valid)
    {
      if (parseInfo) {
	std::cerr << "processing " << t << "\n";
      }
    // process the token in all current branches
    // processing may add more branches onto the queue, so only process branches that
    // existed before any processing occurs
    int currQueueSize = parseQueue.size();
    for (int i = 0; i < currQueueSize; i++)
      processTokenRecursive(t, j, pos, parseQueue[i]);
    
    // if a branch fails, deallocate it and remove it from the queue
    for (int i = parseQueue.size()-1; i >= 0; i--)
      if (!parseQueue[i]->valid)
        {
          ParseData* del = parseQueue[i];
          parseQueue.erase(parseQueue.begin() + i);
          delete del;
        }
    
    // if all branches failed, then validator failed
    valid = parseQueue.size() > 0;
    if (!valid && parseInfo)
      std::cerr << "Validator Failed"<< std::endl;
    }
}
void Validator::processTokenRecursive(string t, json j, int pos, ParseData* pd)
{
  if (!pd->valid)
    return;
  if (pos >= 0)
    {
      j["order"] = pos;
      pd->tokenOrder.push_back(j);
    }
  if (parseInfo) 
    {
    // print out the parse stacks of all current branches
	std::string stackPrintout = "";
    for (auto printPd: parseQueue)
      {
        // don't print failed branches
        if (!printPd->valid)
          continue;
        stackPrintout = stackPrintout + "\n";
        // mark current branch stackprintout with a dot
        if (printPd == pd)
          stackPrintout += ". ";
        for (auto s: printPd->stack) 
          {
            stackPrintout = stackPrintout + (std::string)(s.j["val"]) + ", ";
        }
      }
    std::cerr << "\nProcessing: " << t << ", value: " << (std::string)j["val"] << "\n";
    std::cerr <<  "Stacks:"<< stackPrintout << "\n";
    }
  
  State* curState;
  if (pd->stack.size() == 0)
    curState = grammar->getRoot();
  else
    curState = pd->stack.back().state;

  // stores whether a reduction/shift has occurred
  bool processed = false;
  ParseData* oldPd;
  
  // if a conflict exists, new branches will be created
  // oldPd stores a copy of the current branch for this purpose
  if (curState->hasConflicts())  
    oldPd = new ParseData(*pd);
  else
    oldPd = pd;

  if (curState->isReduction(t))
    {
      processed = true;
      vector<Reduction> reds = curState->getReductions(t);
      for(unsigned int k = 0; k < reds.size(); k++)
        {
          if (parseInfo)
            std::cerr << "Reducing to token: " << reds[k].retTok << "\n";
	      std::string tokenPopped = ""; // for storing value of reduced items
          
          ParseData* currPd;
          // if there is a shift-reduce conflict, then the shift occurs on the original branch, so make 
          // a new branch for all reductions.
          // if there is a reduce-reduce conflict, then only the first reduction is on the original branch,
          // so make a new branch for all subsequent reductions
          if (curState->isShift(t) || k > 0)
            {
              if (parseInfo)
                std::cerr << "Creating new branch" << "\n";
              currPd = new ParseData(*oldPd);
              parseQueue.push_back(currPd);
            }
          else 
            {
              currPd = pd;  
            }
          
          // if the current stack has at least as many items as the reduction rule, reduce.
          if (currPd->stack.size() >= reds[k].numReduce)
            {
	      int popped = 0; // for storing value of reduced items
              
              for(int x = reds[k].ruleBody.size() - 1; x >= 0; --x)
                {
                  // non array action matching exact value of tokens with that action
                  if (reds[k].ruleBody[x]->isAction() && ((Action*)reds[k].ruleBody[x])->isID())
                    {
                      string act = reds[k].ruleBody[x]->getId();
                      bool found = false;
                      for (unsigned int y = 0; y < currPd->namedObjs.size(); ++y)
                        if (act == currPd->namedObjs[y].first)
                          {
                            if (currPd->namedObjs[y].second["val"] != currPd->stack.back().j["val"])
                              {
                                // in this case, the values of the tagged token do not match
                                if (parseInfo)
                                  std::cerr << "Validator failed inconsistent id. " << std::endl;
                                currPd->valid = false;
                                return;
                              }
                            found = true;
                          }
                        else if (currPd->namedObjs[y].second["val"] == currPd->stack.back().j["val"])
                          {
                            // in this case, the name has been matched to some other identifier
                            if (parseInfo)
                              std::cerr << "Validator failed id already matched, token: " << t << std::endl;
                            currPd->valid = false;
                            return;
                          }
                      if (!found){
                        currPd->namedObjs.push_back(std::make_pair(act,currPd->stack.back().j));
		      }
                    }
                  // array action matching storing all ids tagged with the same action
                  else if (reds[k].ruleBody[x]->isAction() && ((Action*)reds[k].ruleBody[x])->isArray())
                    {
                      string act = reds[k].ruleBody[x]->getId();
                      unsigned int i;
                      for (i = 0; i < currPd->arrayObjs.size(); ++i)
                        if (currPd->arrayObjs[i].first == act)
                          {
                            currPd->arrayObjs[i].second.push_back(currPd->stack.back().j);
                            break;
                          }
                      if (i == currPd->arrayObjs.size())
                        {
                          vector<json> v;
                          v.push_back(currPd->stack.back().j);
                          currPd->arrayObjs.push_back(std::make_pair(act,v));
                        }
                    }
		  else if (reds[k].ruleBody[x]->isAction() && ((Action*)reds[k].ruleBody[x])->isName())
		    {
		      string n = ((NameAction*)reds[k].ruleBody[x])->getName();
		      if (n != currPd->stack.back().j) {
			currPd->valid = false;
			return;
		      }
		    }
                  else
		    {
		      // tokens in a reduction are popped right to left.
		      // this concatenates the current popped token to the left of the current string
		      tokenPopped = (std::string)currPd->stack.back().j["val"] + ((popped > 0) ? " " : "") + tokenPopped;
		      currPd->stack.pop_back();
		      popped++;
		    }
                }
              tokenPopped = reds[k].retTok + "( " + tokenPopped + " )";
	      json jpopped;
	      jpopped["val"] = tokenPopped;
              
              // process the newly reduced token
              processTokenRecursive(reds[k].retTok,jpopped,-1, currPd);
              
              // process the token again. recursion ends with shifts or failures
              processTokenRecursive(t,j,-1, currPd);
            }
          else
            {
              std::cout << "Error, tried to pop too much off stack\n";
            }
        }
    }
  if (oldPd != pd)
    delete oldPd;

  if (curState->isShift(t))
    {
      processed = true;
      if (parseInfo)
        std::cerr << "Shifting\n";
      pd->stack.push_back(StackFrame(curState->shift(t), j));
    }
  // if neither shift nor reduction has occurred, this branch has failed
  if (!processed)
    {
       if (parseInfo)
         std::cerr << "Branch failed at state " << curState->getId() << std::endl;
       pd->valid = false;
    }
    
}

string Validator::getVal(ParseData* p, vector<PatternToken*> v, int spot)
{
  int sub = 1;
  for (int i = 0; i < spot; ++i) {
    if (v[i]->isAction()) {
      sub++;
    }
  }
  return p->stack[spot-sub].j["val"];
}

json Validator::getJson(ParseData* p, vector<PatternToken*> v, int spot)
{
  int sub = 1;
  for (int i = 0; i < spot; ++i) {
    if (v[i]->isAction()) {
      sub++;
    }
  }
  return p->stack[spot-sub].j;
}

void Validator::checkFinalActions()
{
  for (unsigned long z = 0; z < parseQueue.size(); ++z)
    {
      ParseData* currPd = parseQueue[z];
      vector<PatternToken*> finalProd = currPd->stack.back().state->getFinalRules();
      if (finalProd.size() == 0)
	{
	  currPd->valid = false;
	  continue;
	}
      for(int x = finalProd.size() - 1; x >= 0; --x)
	{
	  // non array action matching exact value of tokens with that action
	  if (finalProd[x]->isAction() && ((Action*)finalProd[x])->isID())
	    {
	      string act = finalProd[x]->getId();
	      bool found = false;
	      json tagj = getJson(currPd, finalProd,x);
	      string tag = tagj["val"];
	      for (unsigned int y = 0; y < currPd->namedObjs.size(); ++y) {
		
		if (act == currPd->namedObjs[y].first)
		  {
		    if (currPd->namedObjs[y].second["val"] != tag)
		      {
			// in this case, the values of the tagged token do not match
			if (parseInfo)
			  std::cerr << "Validator failed inconsistent id. " << std::endl;
			currPd->valid = false;
			continue;
		      }
		    found = true;
		  }
		else if (currPd->namedObjs[y].second["val"] == tag)
		  {
		    // in this case, the name has been matched to some other identifier
		    currPd->valid = false;
		    continue;
		  }}
	      if (!found){
		currPd->namedObjs.push_back(std::make_pair(act,tagj));
	      }
	    }
	  // array action matching storing all ids tagged with the same action
	  else if (finalProd[x]->isAction() && ((Action*)finalProd[x])->isArray())
	    {
	      string act = finalProd[x]->getId();
	      unsigned int i;
	      for (i = 0; i < currPd->arrayObjs.size(); ++i)
		if (currPd->arrayObjs[i].first == act)
		  {
		    currPd->arrayObjs[i].second.push_back(getVal(currPd, finalProd,x));
		    break;
		  }
	      if (i == currPd->arrayObjs.size())
		{
		  vector<json> v;
		  v.push_back(getVal(currPd, finalProd,x));
		  currPd->arrayObjs.push_back(std::make_pair(act,v));
		}
	    }
	  else if (finalProd[x]->isAction() && ((Action*)finalProd[x])->isName())
	    {
	      string n = ((NameAction*)finalProd[x])->getName();
	      string tv = getVal(currPd, finalProd,x);
	      if (n != tv) {
		currPd->valid = false;
		continue;
	      }
	    }
	}
    }
  bool val = false;
  for (unsigned long z = 0; z < parseQueue.size(); ++z)
    {
      val = val || parseQueue[z]->valid;
    }
  if (!val)
    valid = false;

}

string Validator::getPattern()
{
  checkFinalActions();
  if (!valid)
    {
      ParseData emptyParse;
      mainParse = &emptyParse;
      return "";
    }
  // if there are multiple valid parses, then return the first one 
  for (auto pd: parseQueue)
    if (pd->stack.back().state->isEndState() && pd->stack.back().state->getPattern() != "")
      {
        mainParse = pd;
        return pd->stack.back().state->getPattern();
      }
  ParseData emptyParse;
  mainParse = &emptyParse;
  return "";
}

vector<json> Validator::getJsons()
{
  vector<json> js;
  for (unsigned int i = 0; i < mainParse->namedObjs.size(); ++i)
    {
      // if a namedObj does not have order (tagged non-terminal), don't include it
      if (!mainParse->namedObjs[i].second.contains("order"))
        continue;
      mainParse->namedObjs[i].second["tag"] = mainParse->namedObjs[i].first;
      js.push_back(mainParse->namedObjs[i].second);
    }
  for (unsigned int i = 0; i < mainParse->arrayObjs.size();++i)
    {
      for (unsigned int j = 0; j < mainParse->arrayObjs[i].second.size();++j)
        {
          // if an arrayObj does not have order (tagged non-terminal), don't include it
          if (!mainParse->arrayObjs[i].second[j].contains("order"))
            continue;
          mainParse->arrayObjs[i].second[j]["tag"] = mainParse->arrayObjs[i].first;
          js.push_back(mainParse->arrayObjs[i].second[j]);
        }
    }
  return js;
}

vector<json> Validator::getTokenOrder()
{
  return mainParse->tokenOrder;
}

bool Validator::isValid() { 
  return this->valid;
}

bool Validator::allParsesValid() { 
  for (auto pd: parseQueue)
    if (!pd->valid)
      return false;
  return valid;
}

int Validator::size()
{
    return parseQueue.size();
}

// helper function for processing t_less3
// extracts valid parses
vector<ParseData*> Validator::popSuccessfulParses()
{
  vector<ParseData*> ret;
  for (int i = parseQueue.size()-1; i>=0; i--)
    {
      ParseData* pd = parseQueue[i];
      if (pd->valid)
        {
          ret.push_back(pd);
          parseQueue.erase(parseQueue.begin() + i);
        }
    }
  return ret;
}

// helper function for processing t_less3
void Validator::pushSuccessfulParses(vector<ParseData*> toAppend)
{
  parseQueue.insert(parseQueue.end(), toAppend.begin(), toAppend.end());
}

