import os
import json
import shlex
import subprocess
import shutil
import multiprocessing
import itertools
from itertools import repeat


def runMutantLocator(filesDict: dict, tmpDir: str, locatorPath: str, compileDBPath: str, cores: int) -> list:
  injectTmpPath = os.path.join(tmpDir, 'injectTmp')
  if os.path.exists(injectTmpPath):
    shutil.rmtree(injectTmpPath)
  os.mkdir(injectTmpPath)

  multiprocessing.freeze_support()
  with multiprocessing.Pool(processes=cores) as pool:
    mlPathPairs = pool.starmap(runml_worker, zip(filesDict.items(), range(len(filesDict)), repeat(injectTmpPath), 
                        repeat(locatorPath), repeat(compileDBPath)))

  return mlPathPairs


def runml_worker(filesDictEntry: tuple, injID: int, injectTmpPath: str, locatorPath: str, compileDBPath: str) -> list:
  filePath, jData = filesDictEntry

  


  fileName = os.path.basename(filePath)
  tmpApmPath = os.path.join(injectTmpPath, f'{injID}-{fileName}.apm')
  tmpMlPath = os.path.join(injectTmpPath, f'{injID}-{fileName}.cmd')

  with open(tmpApmPath, 'w+') as f:
    json.dump(jData, f, indent=2)

  print(f'[INFO] - running mutant locator on {filePath}')
  args = shlex.split(' '.join([locatorPath, filePath, '-i', tmpApmPath, '-o', tmpMlPath, '-p', compileDBPath]))
  p = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
  out, err = p.communicate()
  if err:
    print(' '.join(args))
    print(err)
  return filePath, tmpMlPath


def rewriteInjections(filesDict: dict, tmpDir: str, srcDir: str, 
                        locatorPath: str, rewriterPath: str, compileDBPath: str, 
                        cores: int):
  mlPathPairs = runMutantLocator(filesDict, tmpDir, locatorPath, compileDBPath, cores)

  #rewrite source code
  multiprocessing.freeze_support()
  with multiprocessing.Pool(processes=cores) as pool:
    inj_nums = pool.starmap(runRewriter, zip(mlPathPairs, repeat(rewriterPath), repeat(compileDBPath)))
  total_injections = sum(inj_nums)

  print(f'[INFO] - {total_injections} AST patterns are injected')


def copySrcCode(srcDir: str, destDir: str):
  print('[INFO] - copying source code to %s' % destDir)
  if os.path.exists(destDir):
    while reply:= (input("[WARNING] - %s already exists, overwrite? [Y/N]" % destDir).lower().strip()) not in {"y", "n"}:
      pass
    if reply == 'y':
      shutil.rmtree(destDir)
    else:
      return

  shutil.copytree(srcDir, destDir)


class mlJson:
  def __init__(self, jData):
    self.jData = jData

  def __eq__(self, other):
    return other and self.jData['index'] == other.jData['index'] \
                 and self.jData['file'] == other.jData['file'] \
                 and self.jData['operator'] == other.jData['operator'] \
                 and self.jData['start']['line'] == other.jData['start']['line'] \
                 and self.jData['start']['col'] == other.jData['start']['col'] \
                 and self.jData['content'] == other.jData['content']

  def __hash__(self):
      return hash((self.jData['index'], self.jData['file'], self.jData['operator'], self.jData['start']['line'], self.jData['start']['col'], self.jData['content']))

def runRewriter(file_ml_pair: tuple, rewriterPath: str, compileDBPath: str):
  targetPath, mlPath = file_ml_pair
  print(f'[INFO] - rewriting {targetPath} with cmd {mlPath}')

  with open(mlPath, 'r') as f:
    jData = json.load(f)

  if len(jData) == 0:
    print(f'[INFO] - skipping empty cmd file {mlPath}')
    return 0

  # deduplicate
  mlJSet = set([mlJson(j) for j in jData])
  jData = [m.jData for m in mlJSet]

  jData = sorted(jData, key = lambda x: (x['start']['line'], x['start']['col']), reverse=True)
  with open(mlPath, 'w+') as f:
    json.dump(jData, f, indent=2)

  args = [rewriterPath, '-i', mlPath, '-p', compileDBPath, targetPath]
  p = subprocess.Popen(shlex.split(' '.join(args)), stdout=subprocess.PIPE, stderr=subprocess.PIPE)
  out, err = p.communicate()
  if err:
    print(' '.join(args))
    print(err)
  indices = set([j['index'] for j in jData])
  return len(indices)

def injectFlagsDec(files: set, entryFile: str, buildSavers: list):
  print('[INFO] - injecting FixReverter flag delcrations')
  if entryFile in files:
    files.remove(entryFile)
  for filePath in files:
    if filePath in buildSavers:
      continue
    with open(filePath, 'rb') as f:
      codes = f.readlines()
    codes.insert(0, b'#endif\n')
    codes.insert(0, b'extern short FIXREVERTER[];\n')
    codes.insert(0, b'#include <stdio.h>\n')
    codes.insert(0, b'#ifdef FRCOV\n')
    print('[INFO] - injected FixReverter flag delcrations into %s' % filePath)
    with open(filePath, 'wb+') as f:
      f.writelines(codes)

def injectFlagsDef(files: list, indices: int):
  print('[INFO] - injecting FixReverter flag definitions')
  for filePath in files:
    with open(filePath, 'rb') as f:
      codes = f.readlines()
    codes.insert(0, b'#endif\n')
    codes.insert(0, b'short FIXREVERTER[%s];\n' % bytes(str(indices+1), encoding='utf8'))
    codes.insert(0, b'#include <stdio.h>\n')
    codes.insert(0, b'#ifdef FRCOV\n')
    print('[INFO] - injected FixReverter flag definitions into %s' % filePath)
    with open(filePath, 'wb+') as f:
      f.writelines(codes)


def injectEntryFile(filePath: str, indices: int, 
                    locatorPath: str, rewriterPath: str,
                    tmpDir: str, entryFunc: str, 
                    setFlagCodePath: str, compileDBPath: str,
                    fuzzerInclusions: str):
  print('[INFO] - modifying entry file')
  mainFuncTmp = os.path.join(tmpDir, 'mainTmp')
  if os.path.exists(mainFuncTmp):
    shutil.rmtree(mainFuncTmp)
  os.mkdir(mainFuncTmp)

  print('[INFO] - running main function locator on %s' % filePath)
  cmdPath = os.path.join(mainFuncTmp, 'main.cmd')

  args = [locatorPath, filePath, '-o', cmdPath, '-n', entryFunc, '-p', compileDBPath]
  if fuzzerInclusions:
    args += ["--", fuzzerInclusions]
  p = subprocess.Popen(shlex.split(" ".join(args)), stdout=subprocess.PIPE, stderr=subprocess.PIPE)
  out, err = p.communicate()
  if err:
    print(' '.join(args))
    print(err)

  with open(cmdPath, 'r') as f:
    jData = json.load(f)

  jData = sorted(jData, key = lambda x: (x['start']['line'], x['start']['col']), reverse=True)

  with open(cmdPath, 'w+') as f:
    json.dump(jData, f, indent=2)

  runRewriter((filePath, cmdPath), rewriterPath, compileDBPath)

  with open(filePath, 'rb') as f:
    codes = f.readlines()

  codes.insert(0, b'#endif\n')
  codes.insert(0, b'short FIXREVERTER[FIXREVERTER_SIZE];\n')
  codes.insert(0, b'#define FIXREVERTER_SIZE %s\n' % bytes(str(indices+1), encoding='utf8'))
  codes.insert(0, b'#ifdef FRCOV\n')

  macros = ['#ifdef FRCOV\n',
            '#include <stdio.h>\n',
            '#include <stdlib.h>\n',
            '#include <string.h>\n'
            '#endif\n']

  with open(setFlagCodePath, 'r') as f:
    setFlagCodes = f.readlines()

  for i in range(len(codes)):
    line = codes[i]
    try:
      lineStr = line.decode('utf-8')

      macroReplace = replaceToken(lineStr, 'FixReverterMacroReplacement', macros)
      mainReplace = replaceToken(lineStr, 'FixReverterMainReplacement', setFlagCodes)

      if macroReplace:
        print('[INFO] - injecting necessary macros into %s' % filePath)
        codes[i] = macroReplace.encode('utf-8')
      elif mainReplace:
        codes[i] = mainReplace.encode('utf-8')
        print('[INFO] - injecting necessary codes into %s' % filePath)

    except UnicodeDecodeError:
      continue

  with open(filePath, 'wb+') as f:
    f.writelines(codes)

def replaceToken(line: str, token: str, toReplace: list) -> str:
  pos = line.find(token)
  if pos >= 0:
    indent = line[:pos]
    line = indent.join(toReplace)
    line = indent + line
    return line
  return None

def inject(srcDir: str, workingDir: str, 
            mainLocatorPath: str, mutantLocatorPath: str,
            rewriterPath: str, entryFile: str, entryFunc: str,
            compileDBPath: str, setFlagCodePath: str, 
            fuzzerInclusions: str, buildSavers: list,
            cores: int, indices: int, filesDict:dict):
  tmpDir = os.path.join(workingDir, 'tmp')

  rewriteInjections(filesDict, tmpDir, srcDir, 
                      mutantLocatorPath,
                      rewriterPath, compileDBPath,
                      cores)
  injectFlagsDec(set(filesDict.keys()), entryFile, buildSavers)
  injectFlagsDef(buildSavers, indices)
  injectEntryFile(entryFile, indices,
                    mainLocatorPath, rewriterPath, 
                    tmpDir, entryFunc, 
                    setFlagCodePath, compileDBPath,
                    fuzzerInclusions)
