
def updateDict(i, flags, index, nameList, patternDict):
  patName = nameList[index][0]
  idName = nameList[index][1]
  if not flags[index] and i['tag'] == idName:
    flags[index] = True
    patternDict[patName] += 1

def splitPattern(jData, printSimple=True):
  nameList = [('nullCheck', 'traceVarNullCheck[]'),
              ('numCmp', 'traceVarNumCmp[]'),
              ('ptrRange', 'traceVarPtrRange[]'),
              ('nullCheckField', 'traceVarNullCheckBase[]'),
              ('numCmpField', 'traceVarNumCmpBase[]'),
              ('ptrRangeField', 'traceVarPtrRangeBase[]')]

  patternDict = {pair[0] : 0 for pair in nameList}

  for j in jData:
    flags = [False for pair in nameList]
    for i in j['ids']:
      for k in range(len(nameList)):
        updateDict(i, flags, k, nameList, patternDict)
  if printSimple:
    return [patternDict.values()]
  return patternDict

