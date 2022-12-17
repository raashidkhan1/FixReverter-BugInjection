import os
import json

def countReachTrigger(workingDir, printNum=True, printList=False, dumpDict=False):
  jsonPath = os.path.join(workingDir, 'reachTrigger.json')
  with open(jsonPath, 'r') as f:
    jData = json.load(f)

  reach = set()
  trigger = set()

  for j in jData:
    reach.update(j['reach'])
    trigger.update(j['trigger'])

  if printNum:
    print('[INFO] - number of reaches is %d' % len(reach))
    print('[INFO] - number of triggers is %d' % len(trigger))

  if printList:
    print('[INFO] - list of reaches is:')
    print(sorted(reach))
    print('[INFO] - list of triggers is:')
    print(sorted(trigger))

  sumDict = {'reach': sorted(reach), 'trigger': sorted(trigger)}
  if dumpDict:
    sumPath =  os.path.join(workingDir, 'reachTriggerSum.json')
    with open(sumPath, 'w+') as f:
      json.dump(sumDict, f, indent=2)
  return sumDict
    

  
