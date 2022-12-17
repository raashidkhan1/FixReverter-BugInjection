import os
import json
import subprocess
import shlex
import sys
import collections
import multiprocessing
from itertools import repeat, chain
from functools import reduce

def runapm(tmpDir: str, srcDir: str, compileDBPath: str, matcherPath: str, grammarPath: str, initApmName: str='initialAPM', apmName: str='apm.json'):
  if not os.path.exists(compileDBPath):
    print('[ERROR] - compile commands file does not exist: %s')
    sys.exit(0)
  if not os.path.exists(grammarPath):
    print('[ERROR] - grammar file does not exist: %s' % grammarPath)
    sys.exit(0)

  # create empty files
  initApmName = 'initialAPM.json'
  jsonPath = os.path.join(tmpDir, initApmName)
  if os.path.exists(jsonPath):
    input('[WARNING] - %s already exists, press enter to remove or ctrl+c to exit' % jsonPath)

  with open(compileDBPath, 'r') as f:
    files = json.load(f)

  filePaths = []
  for f in files:
    if f['file'].endswith('.c'):    # AST pattern matcher can only process C files now
      filePath = os.path.realpath(os.path.join(f['directory'], f['file']))
      filePaths.append(filePath)

  multiprocessing.freeze_support()
  with multiprocessing.Pool(processes=20) as pool:
    lines = pool.starmap(runapm_worker, zip(filePaths, repeat(matcherPath), 
                        repeat(tmpDir), repeat(grammarPath), repeat(compileDBPath)))
  lines = list(chain(*lines))

  with open(jsonPath, 'w+') as f:
    f.writelines(lines)
  print('[INFO] - generated initialAPM.json')

  postprocessapm(initApmName, tmpDir, srcDir, apmName)

def runapm_worker(filePath, matcherPath, tmpDir, grammarPath, compileDBPath):
  print('[INFO] - running AST pattern matcher on %s' % filePath)
  jsonPath = os.path.join(tmpDir, f'apm.tmp.{multiprocessing.current_process().name}')
  try:
    open(jsonPath, 'w+').close()
    args = shlex.split(' '.join([matcherPath, filePath, '-o', jsonPath, '-g', grammarPath, '-p', compileDBPath]))
    p = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, err = p.communicate()
    if err:
      print(' '.join(args))
      print(err)
    with open(jsonPath, 'r') as f:
      lines = f.readlines()
  finally:
    os.remove(jsonPath)
  return lines

def postprocessapm(initApmName: str, tmpDir:str, srcDir: str, apmName: str):
  jsonPath = os.path.join(tmpDir, initApmName)
  jData = []

  print('[INFO] - post-processing ast pattern matcher results')
  ApmStmt = collections.namedtuple('ApmStmt', ['file', 'line', 'col', 'pattern'])
  StmtLoc = collections.namedtuple('StmtLoc', ['file', 'line', 'col'])
  with open(jsonPath, 'r') as f:
    # TODO:remove duplicates in astPatternMatcher
    stmtSet = set()
    for line in f:
      if line.startswith('{'):
        j = json.loads(line)

        if 'COND' in j['pattern']:
          ifBeginLoc = StmtLoc(j['stmt']['start']['file'], j['stmt']['start']['line'], j['stmt']['start']['col'])
          ifEndLoc = StmtLoc(j['stmt']['end']['file'], j['stmt']['end']['line'], j['stmt']['end']['col'])
          bodyBeginLoc = StmtLoc(j['stmt']['body']['start']['file'], j['stmt']['body']['start']['line'], j['stmt']['body']['start']['col'])
          bodyEndLoc = StmtLoc(j['stmt']['body']['end']['file'], j['stmt']['body']['end']['line'], j['stmt']['body']['end']['col'])   
          if ifBeginLoc == bodyBeginLoc and ifEndLoc == bodyEndLoc:
            print(f'[WARNING] - this if statement is expanded from a macro: {line}')
            continue

        #otherwise this line is invalid
        if 'traceVar' not in line:
          print(f'[WARNING] - this line does not have any traced variable: {line}')
          continue

        if '<Spelling=' in line and j['pattern'] != 'ARRAY_ENLARGE':
          newIds = []
          removeField = False
          for ident in sorted(j['ids'], key = lambda x:x['order']):
            if 'traceVar' in ident['tag'] and '<Spelling=' in ident['col'] and 'Field' not in ident['tag']:
              # definition comes from spelling
              if 'Base' in ident['tag']:
                removeField = True
              continue
    
            if removeField and 'Field' in ident['tag']:
              removeField = False
              continue
            
            newIds.append(ident)
          
          j['ids'] = newIds

          hasTrace = False
          for ident in j['ids']:
            if 'traceVar' in ident['tag']:
              hasTrace = True

          if not hasTrace:
            print(f'[WARNING] - all traced variables are removed due to spelling: {line}')
            continue

        stmt = ApmStmt(j['stmt']['start']['file'], j['stmt']['start']['line'], j['stmt']['start']['col'], j['pattern'])

        if stmt in stmtSet:
          print(f'[WARNING] - this line is a duplicate match: {line}')
          continue

        stmtSet.add(stmt)
        cmdDir = j['commandDir']
        fileName = j['fileName']
        realFilePath = getRealPath(cmdDir, fileName)

        idOrders = getIdOrders(j)
        baseIds, hasTraced = pairBaseField(idOrders)

        for order in baseIds:
          if len(baseIds[order]) == 0:
            print(f'[ERROR] - error on {line}: base traced var does not have a field')
            sys.exit(0)
          elif len(baseIds[order]) > 1:
            print(f'[WARNING] - order {order} of {line} has access path > 1, mark as untraced')
            idOrders[order]['tag'] = 'un' + idOrders[order]['tag']
            for i in baseIds[order]:
              i['tag'] = 'un' + i['tag']
          else:
            hasTraced = True
            idOrders[order]['offset'] = baseIds[order][0]['offset']          

        if hasTraced:
          jData.append(j)
        else:
          print(f'[WARNING] - all ids marked as untraced, remove from apm file: {line}')

  # index each json trace
  count = 1

  sem_jData = []
  non_sem_jData = []
  non_sem_patterns = ['ARRAY_ENLARGE', 'MEMCPY_MOVE']

  for j in jData:
    j['index'] = count
    count += 1
    if j['pattern'] in non_sem_patterns:
      non_sem_jData.append(j)
    else:
      sem_jData.append(j)

  apmPath = os.path.join(tmpDir, apmName)
  if os.path.exists(apmPath):
    input('[WARNING] - %s already exists, press enter to remove or ctrl+c to exit' % apmPath)
  with open(apmPath, 'w+') as f:
    json.dump(sem_jData, f, indent=2)
  print(f'[INFO] - dumped preprocessed AST pattern matcher results into {apmName}')

  apmPath = os.path.join(tmpDir, 'apm_skip.json')
  if os.path.exists(apmPath):
    input('[WARNING] - %s already exists, press enter to remove or ctrl+c to exit' % apmPath)
  with open(apmPath, 'w+') as f:
    json.dump(non_sem_jData, f, indent=2)
  print(f'[INFO] - dumped preprocessed AST pattern matcher results into apm_skip.json')

def getIdOrders(j):
  idOrders = {}
  for i in j['ids']:
    idOrders[i['order']] = i
  return idOrders

def pairBaseField(idOrders):
  hasTraced = False
  baseIds = {}
  for order in sorted(idOrders.keys()):
    i = idOrders[order]
    if 'Base' in i['tag']:
      fieldIds = []
      baseIds[order] = fieldIds
    elif 'Field' in i['tag']:
      fieldIds.append(i)
    elif 'traceVar' in i['tag']:
      hasTraced = True
  return baseIds, hasTraced

def getRealPath(cmdDir: str, filePath: str) -> str:
  if not os.path.isabs(filePath):
    filePath = os.path.realpath(os.path.join(cmdDir, filePath))
  else:
    filePath = os.path.realpath(filePath)
  return filePath
  
