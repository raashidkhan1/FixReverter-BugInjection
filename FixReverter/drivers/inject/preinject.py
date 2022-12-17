import os
import json
import shutil
import sys
import itertools
from runapm import getIdOrders, pairBaseField


def copySrcCode(srcDir: str, destDir: str):
  print('[INFO] - copying source code to %s' % destDir)
  if os.path.exists(destDir):
    while reply:= (input("[WARNING] - %s already exists, overwrite? [Y/N]" % destDir).lower().strip()) not in {'y', 'n', 'Y', 'N'}:
      pass
    if reply in ['y', 'Y']:
      shutil.rmtree(destDir)
    else:
      return

  shutil.copytree(srcDir, destDir)


def prepareAPM(apmPath: str, tmpDir: str, entryFile: str, ignores: list) -> (int, set):
  # check if apm files exist
  if not os.path.exists(apmPath):
    print(f'[ERROR] - {apmPath} does not exist, run with --pattern first')
  apmSkipPath = os.path.join(tmpDir, 'apm_skip.json')
  if not os.path.exists(apmSkipPath):
    print(f'[ERROR] - {apmSkipPath} does not exist, run with --pattern first')

  with open(apmPath, 'r') as f:
    jData = json.load(f)

  # remove injections in ignored files
  tmpJData = []
  for j in jData:
    commandDir = j['commandDir']
    fileName = j['fileName']
    absFilePath = fileName if os.path.isabs(fileName) else os.path.join(commandDir, fileName)
    absFilePath = os.path.realpath(absFilePath)
    if absFilePath in ignores:
      print(f'[INFO] - a mathced pattern in this file is ignored {absFilePath}')
      continue
    tmpJData.append(j)
  jData = tmpJData

  jData = chooseCondExe(jData)

  jData = resolveNested(jData)

  # mark crash field and dump
  jData = processTraceVars(jData, tmpDir)

  filesDict = getFilesDict(jData)
  
  return jData[-1]['index'], filesDict


def chooseCondExe(jData: list) -> list:
  for j in jData:
    if j['pattern'] == 'COND_EXEC':
      j['ids'].sort(key= lambda x : x['order'])
      crashed_vars = []
      for i in range(len(j['ids'])):
        var = j['ids'][i]
        if 'traceVar' in var['tag'] and 'crash' in var and var['crash']:
          crashed_vars.append(i)

      if len(crashed_vars) > 1:
        crashed_vars.pop()
        for i in crashed_vars:
          j['ids'][i].pop('crash', None)
  return jData

def resolveNested(jData: list) -> list:
  jDataById = {}
  for j in jData:
    jDataById[j['index']] = j

  adjList = {}
  for fst, snd in itertools.permutations(jDataById.keys(), 2):
    fstInst = jDataById[fst]
    sndInst = jDataById[snd]

    if sndInst['pattern'] == 'ARRAY_ENLARGE' or sndInst['pattern'] == 'MEMCPY_MOVE':
      continue

    if fstInst['fileName'] != sndInst['fileName']:
      continue

    if isInside((fstInst['stmt']['start']['line'], fstInst['stmt']['start']['col']),
                (fstInst['stmt']['end']['line'], fstInst['stmt']['end']['col']),
                (sndInst['stmt']['body']['start']['line'], sndInst['stmt']['body']['start']['col']),
                (sndInst['stmt']['body']['end']['line'], sndInst['stmt']['body']['end']['col'])):
    # first is in range of second
      print(f'[INFO] - {fst} is inside {snd}')
      if fst not in adjList:
        adjList[fst] = []
      adjList[fst].append(snd)

  COND_PATTERNS = ['COND_EXEC', 'COND_ASSIGN', 'COND_ABORT']

  for departNode, destNodes in adjList.items():
    toRemove = []
    for fst, snd in itertools.permutations(destNodes, 2):
      if fst in adjList and snd in adjList[fst]:
        toRemove.append(snd)
    tmp = [i for i in destNodes if i not in toRemove]
    adjList[departNode] = tmp

  revAdjList = {}  
  for departNode, destNodes in adjList.items():
    for dest in destNodes:
      if dest not in revAdjList:
        revAdjList[dest] = []
      revAdjList[dest].append(departNode)

  rootNodes = [n for n in revAdjList if n not in adjList]

  for root in rootNodes:
    levels = []
    q = [root]
    
    while len(q) > 0:
      qSize = len(q)
      level = []
      for i in range(qSize):
        curr = q.pop(0)
        level.append(curr)
        if curr not in revAdjList:
          continue
        for next in revAdjList[curr]:
          q.append(next)
      levels.append(level)

    print(f'[INFO] - {root} is a root in tree structure')

    treeNodes = []
    for l in levels:
      print(f'[INFO] - processing level {l}')
      for node in l:
        treeNodes.append(node)
        if jDataById[node]['pattern'] in COND_PATTERNS:
          keep = l

    toremove = [i for i in treeNodes if i not in keep]
    print(f'[INFO] - keep {keep}, remove {toremove}')
    for node in toremove:
      jDataById.pop(node, None)

  return list(jDataById.values())

def isInside(fstBeginLoc: tuple, sndBeginLoc: tuple,
                fstEndLoc: tuple, sndEndLoc: tuple) -> bool:
  beginCond = int(fstBeginLoc[0]) > int(sndBeginLoc[0]) or \
                (int(fstBeginLoc[0]) == int(sndBeginLoc[0]) and int(fstBeginLoc[1]) > int(sndBeginLoc[1]))
  endCond = int(fstEndLoc[0]) < int(sndEndLoc[0]) or \
                (int(fstEndLoc[0]) == int(sndEndLoc[0]) and int(fstEndLoc[1]) < int(sndEndLoc[1]))
  return beginCond and endCond


def processTraceVars(jData: list, tmpDir: str) -> list:
  print('[INFO] - marking crash field on traced fields')

  jData.sort(key=lambda k: k['index'])
  for j in jData:
    j['ids'].sort(key=lambda k: k['order'])
    idOrders = getIdOrders(j)
    baseIds, _ = pairBaseField(idOrders)
    jIndex = j['index']

    removeOrder = []
    for ident in j['ids']:
      if j['pattern'] == 'COND_ASSIGN':
        if 'traceVarAssign' in ident['tag']:
          removeOrder.append(ident['order'])
          ident['crash'] = False
        elif 'traceVarNumCmp' in ident['tag'] \
            or 'traceVarPtrRange' in ident['tag'] \
            or 'traceVarNullCheck' in ident['tag']:
          ident['crash'] = True
      elif j['pattern'] == 'COND_EXEC' and 'traceVarAssign' in ident['tag']:
        removeOrder.append(ident['order'])

    if len(removeOrder) > 0:
      keepId = []
      keepToken = []

      for ident in j['ids']:
        if ident['order'] not in removeOrder:
          keepId.append(ident)

      for token in j['tokenOrder']:
        if token['order'] not in removeOrder:
          keepToken.append(token)
        
      j['ids'] = keepId
      j['tokenOrder'] = keepToken

    for order in baseIds:
      if len(baseIds[order]) == 0:
        print(f'[ERROR] - error on {jIndex}: base traced var does not have a field')
        sys.exit(0)
      elif len(baseIds[order]) > 1:
        print(f'[ERROR] - error on {jIndex}: access path > 1')
        sys.exit(0)
      else:
        baseIds[order][0]['crash'] = idOrders[order]['crash'] if 'crash' in idOrders[order] else False

    # mark non-crash traced vars
    for i in j['ids']:
      if 'traceVar' in i['tag'] and 'crash' not in i:
        i['crash'] = False

  injectPath = os.path.join(tmpDir, 'inject.json')
  with open(injectPath, 'w+') as f:
    json.dump(jData, f, indent=2)
  print('[INFO] - dumped into %s' % injectPath)
  return jData


def getFilesDict(jData: list) -> dict:
  print('[INFO] - extracting file paths info from apm file')

  filesDict = {}
  for j in jData:
    commandDir = j['commandDir']
    fileName = j['fileName']
    absFilePath = fileName if os.path.isabs(fileName) else os.path.join(commandDir, fileName)
    absFilePath = os.path.realpath(absFilePath)
    if absFilePath in filesDict:
      filesDict[absFilePath].append(j)
    else:
      filesDict[absFilePath] = [j]

  print('[INFO] - extracted file info')
  return filesDict


def preinject(srcDir: str, workingDir: str, entryFile: str,
            ignores: list):
  tmpDir = os.path.join(workingDir, 'tmp')
  apmPath = os.path.join(tmpDir, 'dda.json')
  mlPath = os.path.join(tmpDir, 'cmd.ml')

  # copy source directory for future use
  bkDir = os.path.join(tmpDir, 'backup')
  copyDir = os.path.join(bkDir, 'prog_before_inject')
  copySrcCode(srcDir, copyDir)
  # copy entry file
  copyEntryFile = os.path.join(bkDir, 'entry_before_inject')
  shutil.copyfile(entryFile, copyEntryFile)

  indices, filesDict = prepareAPM(apmPath, tmpDir, entryFile, ignores)

  return indices, filesDict
